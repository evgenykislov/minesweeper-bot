#include <atomic>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QPainter>

#include <uiohook.h>

#include "common.h"
#include "botdialog.h"
#include "celltypedialog.h"
#include "ui_botdialog.h"
#include "settingsdialog.h"
#include "easylogging++.h"

using namespace std;
using namespace std::chrono;

function<void(int xpos, int ypos)> hook_lambda_;
atomic<int64_t> mouse_move_time_; // Time last mouse move
atomic<int64_t> mouse_unhook_timeout_; // Time of mouse unhook (for automatic movement, etc)

bool HookLogger(unsigned int level, const char* format, ...) {
  return true;
}

void HookHandle(uiohook_event* const event) {
  assert(hook_lambda_);
  if (!event) {
    return;
  }
  int64_t cur_time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
  if (cur_time > mouse_unhook_timeout_) {
    if (event->type == EVENT_MOUSE_MOVED) {
      mouse_move_time_ = cur_time;
    }
    else if (event->type == EVENT_MOUSE_CLICKED) {
      if (event->data.mouse.button == MOUSE_BUTTON1) {
        hook_lambda_(event->data.mouse.x, event->data.mouse.y);
      }
    }
  }
}


BotDialog::BotDialog(QWidget *parent)
  : QDialog(parent)
  , top_left_corner_defined_(false)
  , bottom_right_corner_defined_(false)
  , custom_row_amount_(0)
  , custom_col_amount_(0)
  , custom_mines_amount_(0)
  , row_amount_(0)
  , col_amount_(0)
  , mines_amount_(0)
  , ui_(new Ui::BotDialog)
  , pointing_interval_(0)
  , pointing_target_(kEmptyTarget)
  , save_counter_(kDefaultStartIndex)
  , step_index_(0)
  , finish_gaming_(false)
  , resume_gaming_(false)
  , stop_gaming_(false)
  , auto_restart_game_(false)
  , save_steps_(false)
  , finish_index_(kDefaultFinishIndex)
  , settings_(QSettings::UserScope, ORGANIZATION, APPLICATION)
  , level_(kBeginnerLevel)
  , save_unexpected_error_steps_(false)
  , save_steps_before_wrong_mine_(false)
  , save_probability_steps_(false)
{
  mouse_unhook_timeout_ = mouse_move_time_ = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
  setWindowFlag(Qt::WindowStaysOnTopHint);
  ui_->setupUi(this);
  ui_->level_btngrp_->setId(ui_->beginner_level_rdb_, kBeginnerLevel);
  ui_->level_btngrp_->setId(ui_->intermediate_level_rdb_, kIntermediateLevel);
  ui_->level_btngrp_->setId(ui_->expert_level_rdb_, kExpertLevel);
  ui_->level_btngrp_->setId(ui_->custom_level_rdb_, kCustomLevel);
  ui_->CornersLbl->hide();
  ui_->progress_lbl_->hide();
  connect(&pointing_timer_, &QTimer::timeout, this, &BotDialog::PointingTick, Qt::QueuedConnection);
  connect(&update_timer_, &QTimer::timeout, this, &BotDialog::UpdateTick, Qt::QueuedConnection);
  connect(this, &BotDialog::DoClickPosition, this, &BotDialog::OnClickPosition, Qt::QueuedConnection);
  connect(this, &BotDialog::DoGameStopped, this, &BotDialog::OnGameStopped, Qt::QueuedConnection);
  connect(this, &BotDialog::DoGameOver, this, &BotDialog::OnGameOver, Qt::QueuedConnection);
  connect(this, &BotDialog::DoGameComplete, this, &BotDialog::OnGameComplete, Qt::QueuedConnection);
  connect(this, &BotDialog::DoGameStoppedByUser, this, &BotDialog::OnGameStoppedByUser, Qt::QueuedConnection);
  connect(ui_->row_edt_, &LineEditWFocus::LoseFocus, this, &BotDialog::LoseFocus, Qt::QueuedConnection);
  connect(ui_->col_edt_, &LineEditWFocus::LoseFocus, this, &BotDialog::LoseFocus, Qt::QueuedConnection);
  connect(ui_->mines_edt_, &LineEditWFocus::LoseFocus, this, &BotDialog::LoseFocus, Qt::QueuedConnection);
  connect(this, &BotDialog::DoStartUpdate, this, &BotDialog::OnStartUpdate, Qt::QueuedConnection);
  hook_lambda_ = [this](int xpos, int ypos) {
    emit DoClickPosition(xpos, ypos);
  };

  // TODO barrier
  hook_set_logger_proc(HookLogger); // Set empty logger for disable debug output from uiohook library
  hook_set_dispatch_proc(HookHandle);
  hook_thread_.reset(new std::thread([](){
    hook_run();
  }));

  LoadSettings();
  // Update level buttons
  SetLevelButtonsStates();
  UpdateGamingByLevel();

  QFile model_file("model.bin");
  if (model_file.open(QIODevice::ReadOnly)) {
    QByteArray byte_data = model_file.readAll();
    model_file.close();
    vector<uint8_t> vect_data((uint8_t*)(byte_data.data()), (uint8_t*)(byte_data.data() + byte_data.size()));
    solver.LoadModel(std::move(vect_data));
  }
  // Start gaming thread
  auto gaming_thread = new std::thread([this](){
    Gaming();
  });
  gaming_thread_.reset(gaming_thread);
  emit DoStartUpdate();
  update_timer_.start(kUpdateTimerInterval);
}

BotDialog::~BotDialog()
{
  update_timer_.stop();
  // Stop hook thread
  if (hook_thread_ && hook_thread_->joinable()) {
    hook_stop();
    hook_thread_->join();
  }
  hook_thread_.reset();
  // Stop gaming thread
  if (gaming_thread_ && gaming_thread_->joinable()) {
    {
      unique_lock<mutex> locker(gaming_lock_);
      finish_gaming_ = true;
      gaming_stopper_.notify_one();
    }
    gaming_thread_->join();
  }
  PointingCancel();
  hook_set_dispatch_proc(nullptr);
  hook_lambda_ = nullptr;
  delete ui_;
}

void BotDialog::OnTopLeftCorner() {
  StartPointing(kTopLeftCorner);
}

void BotDialog::UpdateCorners() {
  if (!top_left_corner_defined_ || !bottom_right_corner_defined_) {
    return;
  }
  // Apply field frame
  scr_.SetScreenID(QApplication::desktop()->screenNumber(this));
  field_frame_.normalized();
  scr_.SetFrameRect(field_frame_);
  ShowCornerImages();
}

void BotDialog::FormImage(const QImage& image, QPixmap& pixels) {
  // Scale the image to given size
  auto scaled_image = image.scaled(kImageSize, kImageSize, Qt::KeepAspectRatio);
  QImage border_image(kImageSizeWithBorder, kImageSizeWithBorder, QImage::Format_RGB888);
  QPainter painter;
  painter.begin(&border_image);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.setPen(border_linecolor);
  painter.fillRect(border_image.rect(), border_backcolor);
  for (int y = -kImageSizeWithBorder; y < (int)kImageSizeWithBorder; y += border_line_step) {
    painter.drawLine(0, y, kImageSizeWithBorder, y + kImageSizeWithBorder);
  }
  assert(kImageSizeWithBorder >= kImageSize);
  unsigned int border_width = (kImageSizeWithBorder - kImageSize) / 2;
  painter.drawImage(QPointF(border_width, border_width), scaled_image);
  painter.end();
  pixels = QPixmap::fromImage(border_image);
}

void BotDialog::ShowCornerImages() {
  if (row_amount_ == 0 || col_amount_ == 0) {
    return;
  }
  QImage top_left;
  QPixmap top_left_pixel;
  if (scr_.GetImageByPosition(0, 0, top_left)) {
    FormImage(top_left, top_left_pixel);
  }
  ui_->cell_top_left_->setPixmap(top_left_pixel);
  QImage top_right;
  QPixmap top_right_pixel;
  if (scr_.GetImageByPosition(0, col_amount_ - 1, top_right)) {
    FormImage(top_right, top_right_pixel);
  }
  ui_->cell_top_right_->setPixmap(top_right_pixel);
  QImage bottom_left;
  QPixmap bottom_left_pixel;
  if (scr_.GetImageByPosition(row_amount_ - 1, 0, bottom_left)) {
    FormImage(bottom_left, bottom_left_pixel);
  }
  ui_->cell_bottom_left_->setPixmap(bottom_left_pixel);
  QImage bottom_right;
  QPixmap bottom_right_pixel;
  if (scr_.GetImageByPosition(row_amount_ - 1, col_amount_ - 1, bottom_right)) {
    FormImage(bottom_right, bottom_right_pixel);
  }
  ui_->cell_bottom_right_->setPixmap(bottom_right_pixel);
  QImage restart;
  QPixmap restart_pixel;
  if (scr_.GetRestartImage(restart)) {
    FormImage(restart, restart_pixel);
  }
  ui_->cell_restart_->setPixmap(restart_pixel);
}

void BotDialog::StopGame() {
  unique_lock<mutex> locker(gaming_lock_);
  stop_gaming_ = true;
  gaming_stopper_.notify_one();
}

void BotDialog::RestartGame() {
}

void BotDialog::SaveStep(StepTypeForSave step_type, const StepInfo& info) {
  // save field
  QString folder;
  QString file_name;
  {
    stringstream name_str;
    lock_guard<mutex> locker(save_lock_);
    file_name = QString::fromUtf8("field_%09d.txt").arg(step_index_);
    switch (step_type) {
      case kTrainSteps:
        if (!save_steps_) {
          // Dont save anything
          return;
        }
        if (save_counter_ > finish_index_) {
          // All steps were saved
          return;
        }
        folder = save_folder_;
        file_name = QString::fromUtf8("field_%09d.txt").arg(save_counter_);
        ++save_counter_;
        break;
      case kUnexpectedErrorSteps:
        folder = QString::fromUtf8(kUnexpectedErrorFolder.c_str());
        break;
      case kWrongMineSteps:
        folder = QString::fromUtf8(kWrongMineFolder.c_str());
        break;
      case kProbabilitySteps:
        folder = QString::fromUtf8(kProbabilityFolder.c_str());
        break;
      default:
        assert(false);
    }
  }

  QDir file_folder(folder);
  file_folder.mkpath("."); // TODO check return
  QString folder_name = file_folder.absolutePath() + QDir::separator();
  QString full_path = folder_name + file_name;
  ofstream field_file(full_path.toStdString(), ios_base::trunc);
  if (!field_file) {
    // TODO can't save
    return;
  }
  for (auto row_iter = info.field_.begin(); row_iter != info.field_.end(); ++row_iter) {
    for (auto col_iter = row_iter->begin(); col_iter != row_iter->end(); ++col_iter) {
      field_file << *col_iter;
    }
    field_file << endl;
  }
  if (!field_file) {
    // TODO saving error
    return;
  }
  field_file.close();
  // Store predicted step
  stringstream step_filename;
  step_filename << folder_name.toStdString() << "step_" << setfill('0') << setw(5) << step_index_ << ".txt"; // TODO step_index_ ??
  ofstream step_file(step_filename.str(), ios_base::trunc);
  if (!step_file) {
    // TODO can't save
    return;
  }
  step_file << info.row_ << " " << info.col_ << " " << info.success_ << " " << file_name.toStdString() << endl;
  step_file.close();
  {
    lock_guard<mutex> locker(save_lock_);
    ++save_counter_;
  }
}

void BotDialog::SaveWrongMine(unsigned int row, unsigned int col) {
  // save field
  QString folder;
  QString file_name;
  {
    stringstream name_str;
    lock_guard<mutex> locker(save_lock_);
    file_name = QString::fromUtf8("step_%09d_wrong_mine.txt").arg(step_index_);
    folder = QString::fromUtf8(kWrongMineFolder.c_str());
  }

  QDir file_folder(folder);
  file_folder.mkpath("."); // TODO check return
  QString folder_name = file_folder.absolutePath() + QDir::separator();
  QString full_path = folder_name + file_name;
  ofstream file(full_path.toStdString(), ios_base::trunc);
  if (!file) {
    // TODO can't save
    return;
  }
  file << "Reason: wrong mine" << endl;
  file << "row:" << row << endl;
  file << "col:" << col << endl;
}

void BotDialog::StartPointing(PointingTarget target) {
  try {
    PointingCancel();
    // Start new corners request
    pointing_interval_ = 0;
    pointing_timer_.start(kPointingTimerInterval);
    UnhookMouseByTimeout();
    pointing_target_ = target;
  }
  catch (exception&) {
    PointingCancel();
  }
}

void BotDialog::Gaming() {
  // Thread for gaming procedure
  LOG(INFO) << "Game thread has started";
  unsigned int mines_amount;
  std::list<StepInfo> wrong_tail; // Tail of steps before wrong mine
  while (true) {
    // Wait for user action (run, restart, etc)
    LOG(INFO) << "Wait game resume";
    {
      unique_lock<mutex> locker(gaming_lock_);
      gaming_stopper_.wait(locker, [this](){
        return finish_gaming_ || resume_gaming_;
      });
      if (finish_gaming_) {
        LOG(INFO) << "Game thread finishing";
        break;
      }
      resume_gaming_ = false;
    }
    {
      lock_guard<mutex> locker(mines_amount_lock_);
      mines_amount = mines_amount_;
    }
    while (true) {
      {
        lock_guard<mutex> locker(gaming_lock_);
        if (finish_gaming_) {
          LOG(INFO) << "Finishing game" << endl;
          break;
        }
        if (stop_gaming_) {
          stop_gaming_ = false;
          LOG(INFO) << "Stopping game";
          DoGameStoppedByUser();
          break;
        }
      }
      // Check mouse aren't moved
      if (!IsMouseIdle()) {
        LOG(INFO) << "Wait for mouse idle";
        this_thread::sleep_for(kMouseIdleRecheckInterval);
        continue;
      }
      // Get save settings
      StepInfo step_info;
      bool save_train_steps;
      bool save_unexpected_steps;
      bool save_wrong_mine_steps;
      bool save_probability_steps;
      {
        lock_guard<mutex> locker(save_lock_);
        save_train_steps = save_steps_;
        save_unexpected_steps = save_unexpected_error_steps_;
        save_wrong_mine_steps = save_steps_before_wrong_mine_;
        save_probability_steps = save_probability_steps_;
        step_info.step_index_ = step_index_;
        ++step_index_;
      }

      // Get field
      LOG(INFO) << "Get field";
      Field field;
      bool game_over;
      bool error_no_screen;
      bool error_no_field;
      bool error_unknown_images;
      bool error_timeout;
      scr_.GetStableField(field, kReceiveFieldTimeout, game_over, error_no_screen, error_no_field, error_unknown_images, error_timeout);
      if (error_no_screen || error_no_field || error_unknown_images) {
        LOG(INFO) << "Game stopped by reason";
        emit DoGameStopped(error_no_screen, error_no_field, error_unknown_images);
        break;
      }
      if (game_over) {
        if (save_wrong_mine_steps) {
          // Save the tail if wrong mine detected
          unsigned int wrong_row;
          unsigned int wrong_col;
          if (FieldHasWrongMine(field, wrong_row, wrong_col)) {
            LOG(INFO) << "Wrong mine detected before step";
            while (!wrong_tail.empty()) {
              SaveStep(kWrongMineSteps, wrong_tail.front());
              wrong_tail.pop_front();
            }
            SaveWrongMine(wrong_row, wrong_col);
          }
        }
        emit DoGameOver();
        break;
      }
      // Check game completed
      unsigned int closed_cells_amount = 0;
      unsigned int mark_mines_amount = 0;
      for (auto row_iter = field.begin(); row_iter != field.end(); ++row_iter) {
        for (auto col_iter = row_iter->begin(); col_iter != row_iter->end(); ++col_iter) {
          if (*col_iter == kClosedCellSymbol) {
            ++closed_cells_amount;
          }
          else if (*col_iter == kMineMarkSymbol) {
            ++mark_mines_amount;
          }
        }
      }
      if ((closed_cells_amount + mark_mines_amount) <= mines_amount) {
        LOG(INFO) << "Game complete (all cells are opened)";
        emit DoGameComplete();
        break;
      }
      // Calculate new step
      LOG(INFO) << "Get new step";
      unsigned int row;
      unsigned int col;
      Classifier::StepAction step;
      solver.GetStep(field, mines_amount, row, col, step);
      LOG(INFO) << "Got new step";
      // Detect wait, mouse move, etc

      // Make step
      // Check mouse aren't moved
      if (!IsMouseIdle()) {
        this_thread::sleep_for(kMouseIdleRecheckInterval);
        continue;
      }
      UnhookMouseByTimeout();
      switch (step) {
        case Classifier::kOpenWithSure:
        case Classifier::kOpenWithProbability:
          scr_.MakeStep(row, col);
          break;
        case Classifier::kMarkAsMine:
          scr_.MakeMark(row, col);
          break;
        default:
          assert(false);
      }
      // Wait for update complete
      Field next_field;
      scr_.GetChangedField(next_field, field, kReceiveFieldTimeout, game_over, error_no_screen, error_no_field, error_unknown_images, error_timeout);
      // Detect success
      if (error_no_screen || error_no_field || error_unknown_images) {
        emit DoGameStopped(error_no_screen, error_no_field, error_unknown_images);
        break;
      }
      step_info.field_ = field;
      step_info.row_ = row;
      step_info.col_ = col;
      step_info.step_ = step;
      if (save_train_steps) {
        SaveStep(kTrainSteps, step_info);
      }
      if (save_unexpected_steps && step == Classifier::kOpenWithSure && game_over) {
        SaveStep(kUnexpectedErrorSteps, step_info);
      }
      if (save_wrong_mine_steps) {
        // Store steps before may-be wrong mine
        wrong_tail.push_back(step_info);
        while (wrong_tail.size() > kWrongMineTailSize) {
          wrong_tail.pop_front();
        }
        // Save the tail if wrong mine detected
        unsigned int wrong_row;
        unsigned int wrong_col;
        if (FieldHasWrongMine(next_field, wrong_row, wrong_col)) {
          LOG(INFO) << "Wrong mine detected";
          while (!wrong_tail.empty()) {
            SaveStep(kWrongMineSteps, wrong_tail.front());
            wrong_tail.pop_front();
          }
          SaveWrongMine(wrong_row, wrong_col);
        }
      }
      if (save_probability_steps && step == Classifier::kOpenWithProbability) {
        SaveStep(kProbabilitySteps, step_info);
      }
      if (game_over) {
        LOG(INFO) << "Game over";
        emit DoGameOver();
        break;
      }
    }
  }
  LOG(INFO) << "Game thread has finished";
}

void BotDialog::InformGameStopper(bool no_screen, bool no_field, bool unknown_images) {

}

void BotDialog::LoadSettings() {
  auto_restart_game_ = settings_.value("user/autorestart", false).toBool();
  {
    lock_guard<mutex> locker(save_lock_);
    save_steps_ = settings_.value("train/save_steps", false).toBool();
    save_folder_ = settings_.value("train/folder", ".").toString();
    save_unexpected_error_steps_ = settings_.value("track/unexpected_errors", false).toBool();
    save_steps_before_wrong_mine_ = settings_.value("track/wrong_mines", false).toBool();
    save_probability_steps_ = settings_.value("track/probability_steps", false).toBool();
  }
  QPoint point;
  point = settings_.value("screen/topleft", kUndefinedPoint).toPoint();
  if (point != kUndefinedPoint) {
    field_frame_.setTopLeft(point);
    top_left_corner_defined_ = true;
  }
  point = settings_.value("screen/bottomright", kUndefinedPoint).toPoint();
  if (point != kUndefinedPoint) {
    field_frame_.setBottomRight(point);
    bottom_right_corner_defined_ = true;
  }
  field_frame_.normalized();
  restart_point_ = settings_.value("screen/restart").toPoint();
  custom_row_amount_ = settings_.value("game/row", 0).toUInt();
  custom_col_amount_ = settings_.value("game/col", 0).toUInt();
  {
    lock_guard<mutex> locker(mines_amount_lock_);
    level_ = (GameLevelID)(settings_.value("game/level", 1).toUInt());
    custom_mines_amount_ = settings_.value("game/mines", 0).toUInt();
  }
}

void BotDialog::SaveSettings() {
  settings_.setValue("user/autorestart", auto_restart_game_);
  {
    lock_guard<mutex> locker(save_lock_);
    settings_.setValue("train/save_steps", save_steps_);
    settings_.setValue("train/folder", save_folder_);
    settings_.setValue("track/unexpected_errors", save_unexpected_error_steps_);
    settings_.setValue("track/wrong_mines", save_steps_before_wrong_mine_);
    settings_.setValue("track/probability_steps", save_probability_steps_);
  }
  if (top_left_corner_defined_) {
    settings_.setValue("screen/topleft", field_frame_.topLeft());
  }
  if (bottom_right_corner_defined_) {
    settings_.setValue("screen/bottomright", field_frame_.bottomRight());
  }
  settings_.setValue("screen/restart", restart_point_);
  settings_.setValue("game/row", custom_row_amount_);
  settings_.setValue("game/col", custom_col_amount_);
  {
    lock_guard<mutex> locker(mines_amount_lock_);
    settings_.setValue("game/level", level_);
    settings_.setValue("game/mines", custom_mines_amount_);
  }
  settings_.sync();
}

void BotDialog::UnhookMouseByTimeout() {
  int64_t tick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
  mouse_unhook_timeout_ = tick + kWaitMouseProcessing;
}

bool BotDialog::IsMouseIdle() {
  int64_t tick = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
  return (tick - mouse_move_time_.load()) > kMouseIdleInterval;
}

void BotDialog::UpdateGamingByLevel() {
  lock_guard<mutex> locker(mines_amount_lock_);
  switch (level_) {
    case kBeginnerLevel:
      row_amount_ = 9;
      col_amount_ = 9;
      mines_amount_ = 10;
      break;
    case kIntermediateLevel:
      row_amount_ = 16;
      col_amount_ = 16;
      mines_amount_ = 40;
      break;
    case kExpertLevel:
      row_amount_ = 16;
      col_amount_ = 30;
      mines_amount_ = 99;
      break;
    case kCustomLevel:
      row_amount_ = custom_row_amount_;
      col_amount_ = custom_col_amount_;
      mines_amount_ = custom_mines_amount_;
      break;
    default:
      // Set beginned level
      row_amount_ = 9;
      col_amount_ = 9;
      mines_amount_ = 10;
  }
  scr_.SetFieldSize(row_amount_, col_amount_);
  ShowCornerImages();
}

void BotDialog::SetLevelButtonsStates() {
  ui_->beginner_level_rdb_->setChecked(level_ == kBeginnerLevel);
  ui_->intermediate_level_rdb_->setChecked(level_ == kIntermediateLevel);
  ui_->expert_level_rdb_->setChecked(level_ == kExpertLevel);
  ui_->custom_level_rdb_->setChecked(level_ == kCustomLevel);
}

bool BotDialog::FieldHasWrongMine(const Field& field, unsigned int& row, unsigned int& col) {
  for (unsigned int r = 0; r < field.size(); ++r) {
    for (unsigned int c = 0; c < field[r].size(); ++c) {
      if (field[r][c] == kWrongMineSymbol) {
        row = r;
        col = c;
        return true;
      }
    }
  }
  return false;
}

void BotDialog::OnRun() {
  SaveSettings();
  {
    unique_lock<mutex> locker(gaming_lock_);
    resume_gaming_ = true;
    gaming_stopper_.notify_one();
  }
}

void BotDialog::LoseFocus() {
  custom_row_amount_ = ui_->row_edt_->text().toUInt();
  custom_col_amount_ = ui_->col_edt_->text().toUInt();
  {
    lock_guard<mutex> locker(mines_amount_lock_);
    custom_mines_amount_ = ui_->mines_edt_->text().toUInt();
  }
  UpdateGamingByLevel();
}

void BotDialog::OnLeftField() {
  field_frame_.adjust(1, 0, 0, 0);
  scr_.SetFrameRect(field_frame_);
  ShowCornerImages();
}

void BotDialog::OnRightField() {
  field_frame_.adjust(-1, 0, 0, 0);
  scr_.SetFrameRect(field_frame_);
  ShowCornerImages();
}

void BotDialog::OnTopField() {
  field_frame_.adjust(0, 1, 0, 0);
  scr_.SetFrameRect(field_frame_);
  ShowCornerImages();
}

void BotDialog::OnBottomField() {
  field_frame_.adjust(0, -1, 0, 0);
  scr_.SetFrameRect(field_frame_);
  ShowCornerImages();
}

void BotDialog::OnBottomRightCorner() {
  StartPointing(kBottomRightCorner);
}

void BotDialog::OnRestartPoint() {
  StartPointing(kRestartButton);
}

void BotDialog::OnSettings() {
  SettingsDialog dlg(this);
  {
    lock_guard<mutex> locker(save_lock_);
    SettingsDialog::Parameters params;
    params.auto_restart_game_ = auto_restart_game_;
    params.save_all_steps_ = save_steps_;
    params.steps_save_folder_ = save_folder_;
    params.steps_start_index_ = save_counter_;
    params.steps_finish_index_ = finish_index_;
    params.save_unexpected_error_steps_ = save_unexpected_error_steps_;
    params.save_steps_before_wrong_mine_ = save_steps_before_wrong_mine_;
    params.save_probability_steps_ = save_probability_steps_;
    dlg.Set(params);
  }
  if (dlg.exec() == QDialog::Accepted) {
    SettingsDialog::Parameters params;
    dlg.Get(params);
    {
      lock_guard<mutex> locker(save_lock_);
      auto_restart_game_ = params.auto_restart_game_;
      save_steps_ = params.save_all_steps_;
      save_folder_ = params.steps_save_folder_;
      save_counter_ = params.steps_start_index_;
      finish_index_ = params.steps_finish_index_;
      save_unexpected_error_steps_ = params.save_unexpected_error_steps_;
      save_steps_before_wrong_mine_ = params.save_steps_before_wrong_mine_;
      save_probability_steps_ = params.save_probability_steps_;
    }
    SaveSettings();
  }
}

void BotDialog::OnStop() {
  StopGame();
}

void BotDialog::OnLevelChanged(int button_id) {
  level_ = (GameLevelID)button_id;
  UpdateGamingByLevel();
}

void BotDialog::PointingCancel() {
  pointing_timer_.stop();
  ui_->progress_lbl_->hide();
}

void BotDialog::ShowUnknownImages() {
  scr_.GetUnknownImages(unknown_images_);
  assert(!unknown_images_.empty());
  UpdateUnknownImages();
}

void BotDialog::UpdateUnknownImages() {
  if (unknown_images_.empty()) {
    return;
  }
  CellTypeDialog dlg(this);
  dlg.SetImage(unknown_images_.front());
  if (dlg.exec() == QDialog::Rejected) {
    unknown_images_.clear();
    StopGame();
    return;
  }
  scr_.SetImageType(unknown_images_.front(), dlg.GetCellType());
  unknown_images_.pop_front();
  UpdateUnknownImages();
}

void BotDialog::PointingTick() {
  pointing_interval_ += kPointingTimerInterval;
  if (pointing_interval_ >= kPointingTimeout) {
    PointingCancel();
    return;
  }
  int progress_width = int(double(pointing_interval_) / kPointingTimeout * ui_->progress_back_->width());
  if (progress_width > 0) {
    ui_->progress_lbl_->show();
    ui_->progress_lbl_->resize(progress_width, ui_->progress_lbl_->height());
  }
  else {
    ui_->progress_lbl_->hide();
  }
}

void BotDialog::UpdateTick() {
  ShowCornerImages();
}

void BotDialog::OnClickPosition(int xpos, int ypos) {
  QPoint point(xpos, ypos);
  auto internal_click_position = mapFromGlobal(point);
  auto window_rect = rect();
  if (window_rect.contains(internal_click_position)) {
    // Click was made inside bot window
    // Wait for click outside
    return;
  }
  // So, click was made outside bot window
  PointingCancel();
  switch (pointing_target_) {
    case kTopLeftCorner:
      LOG(INFO) << "Get top-left corner: " << point.x() << ": " << point.y();
      field_frame_.setTopLeft(point);
      top_left_corner_defined_ = true;
      UpdateCorners();
      break;
    case kBottomRightCorner:
      LOG(INFO) << "Get bottom-right corner: " << point.x() << ": " << point.y();
      field_frame_.setBottomRight(point);
      bottom_right_corner_defined_ = true;
      UpdateCorners();
      break;
    case kRestartButton:
      restart_point_ = point;
      scr_.SetRestartPoint(point);
      break;
  }
  pointing_target_ = kEmptyTarget;
}

void BotDialog::OnGameStopped(bool no_screen, bool no_field, bool unknown_images) {
  if (unknown_images) {
    ShowUnknownImages();
    unique_lock<mutex> locker(gaming_lock_);
    resume_gaming_ = true;
    gaming_stopper_.notify_one();
  }
}

void BotDialog::OnGameOver() {
  if (auto_restart_game_) {
    UnhookMouseByTimeout();
    scr_.MakeRestart();
    unique_lock<mutex> locker(gaming_lock_);
    resume_gaming_ = true;
    gaming_stopper_.notify_one();
  }
}

void BotDialog::OnGameComplete() {
  // TODO
}

void BotDialog::OnStartUpdate() {
  // Set screen frame
  scr_.SetFrameRect(field_frame_);
  scr_.SetRestartPoint(restart_point_);
  ui_->row_edt_->setText(QString("%1").arg(custom_row_amount_));
  ui_->col_edt_->setText(QString("%1").arg(custom_col_amount_));
  {
    lock_guard<mutex> locker(mines_amount_lock_);
    ui_->mines_edt_->setText(QString("%1").arg(custom_mines_amount_));
  }
  UpdateGamingByLevel();
}

void BotDialog::OnGameStoppedByUser() {
  // TODO
}
