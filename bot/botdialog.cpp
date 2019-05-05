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
  , finish_gaming_(false)
  , resume_gaming_(false)
  , stop_gaming_(false)
  , auto_restart_game_(false)
  , save_steps_(false)
  , finish_index_(kDefaultFinishIndex)
  , settings_(QSettings::UserScope, ORGANIZATION, APPLICATION)
  , level_(kBeginnerLevel)
{
  mouse_unhook_timeout_ = mouse_move_time_ = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
  setWindowFlag(Qt::WindowStaysOnTopHint);
  ui_->setupUi(this);
  ui_->level_btngrp_->setId(ui_->beginner_level_rdb_, kBeginnerLevel);
  ui_->level_btngrp_->setId(ui_->intermediate_level_rdb_, kIntermediateLevel);
  ui_->level_btngrp_->setId(ui_->expert_level_rdb_, kExpertLevel);
  ui_->level_btngrp_->setId(ui_->custom_level_rdb_, kCustomLevel);
  ui_->CornersLbl->hide();
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

void BotDialog::OnCornersBtn() {
  StartPointing(kTopLeftCorner);
}

void BotDialog::CornersCompleted() {
  if (!top_left_corner_defined_ || !bottom_right_corner_defined_) {
    return;
  }
  // Apply field frame
  QRect rect;
  rect.setTopLeft(top_left_corner_);
  rect.setBottomRight(bottom_right_corner_);
  rect.normalized();
  scr_.SetScreenID(QApplication::desktop()->screenNumber(this));
  scr_.SetFrameRect(rect);
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
  auto row_amount = ui_->row_edt_->text().toUInt();
  auto col_amount = ui_->col_edt_->text().toUInt();
  QImage top_left;
  QPixmap top_left_pixel;
  scr_.GetImageByPosition(0, 0, top_left);
  FormImage(top_left, top_left_pixel);
  ui_->cell_top_left_->setPixmap(top_left_pixel);
  QImage top_right;
  QPixmap top_right_pixel;
  scr_.GetImageByPosition(0, col_amount - 1, top_right);
  FormImage(top_right, top_right_pixel);
  ui_->cell_top_right_->setPixmap(top_right_pixel);
  QImage bottom_left;
  QPixmap bottom_left_pixel;
  scr_.GetImageByPosition(row_amount - 1, 0, bottom_left);
  FormImage(bottom_left, bottom_left_pixel);
  ui_->cell_bottom_left_->setPixmap(bottom_left_pixel);
  QImage bottom_right;
  QPixmap bottom_right_pixel;
  scr_.GetImageByPosition(row_amount - 1, col_amount - 1, bottom_right); // TODO check return
  FormImage(bottom_right, bottom_right_pixel);
  ui_->cell_bottom_right_->setPixmap(bottom_right_pixel);
  QImage restart;
  QPixmap restart_pixel;
  scr_.GetRestartImage(restart); // TODO check return
  FormImage(restart, restart_pixel);
  ui_->cell_restart_->setPixmap(restart_pixel);
}

void BotDialog::StopGame() {
  unique_lock<mutex> locker(gaming_lock_);
  stop_gaming_ = true;
  gaming_stopper_.notify_one();
}

void BotDialog::RestartGame() {
}

void BotDialog::SaveStep(const Field& field, unsigned int step_row, unsigned int step_col, bool success) {
  // save field
  QString folder;
  size_t index;
  {
    lock_guard<mutex> locker(save_lock_);
    if (!save_steps_) {
      // Dont save anything
      return;
    }
    if (save_counter_ > finish_index_) {
      // All steps were saved
      return;
    }
    folder = save_folder_;
    index = save_counter_;
  }
  QDir file_folder(folder);
  file_folder.mkpath("."); // TODO check return
  QString folder_name = file_folder.absolutePath() + QDir::separator();
  stringstream file_name;
  file_name << "field_" << setfill('0') << setw(5) << index << ".txt";
  stringstream field_filename;
  field_filename << folder_name.toStdString() << file_name.str();
  ofstream field_file(field_filename.str(), ios_base::trunc);
  if (!field_file) {
    // TODO can't save
    return;
  }
  for (auto row_iter = field.begin(); row_iter != field.end(); ++row_iter) {
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
  step_filename << folder_name.toStdString() << "step_" << setfill('0') << setw(5) << index << ".txt";
  ofstream step_file(step_filename.str(), ios_base::trunc);
  if (!step_file) {
    // TODO can't save
    return;
  }
  step_file << step_row << " " << step_col << " " << success << " " << file_name.str() << endl;
  step_file.close();
  {
    lock_guard<mutex> locker(save_lock_);
    ++save_counter_;
  }
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
        LOG(INFO) << "Game over";
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
      // Save step with state
      SaveStep(field, row, col, !game_over);
      if (game_over) {
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
  }
  top_left_corner_ = settings_.value("screen/topleft").toPoint();
  bottom_right_corner_ = settings_.value("screen/bottomright").toPoint();
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
  }
  settings_.setValue("screen/topleft", top_left_corner_);
  settings_.setValue("screen/bottomright", bottom_right_corner_);
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
  scr_.MoveField(-1, 0);
  ShowCornerImages();
}

void BotDialog::OnRightField() {
  scr_.MoveField(1, 0);
  ShowCornerImages();
}

void BotDialog::OnTopField() {
  scr_.MoveField(0, -1);
  ShowCornerImages();
}

void BotDialog::OnBottomField() {
  scr_.MoveField(0, 1);
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
    dlg.Set(auto_restart_game_, save_steps_, save_folder_, save_counter_, finish_index_);
  }
  if (dlg.exec() == QDialog::Accepted) {
    {
      lock_guard<mutex> locker(save_lock_);
      dlg.Get(auto_restart_game_, save_steps_, save_folder_, save_counter_, finish_index_);
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
  if (pointing_interval_ >= kCornersTimeout) {
    PointingCancel();
    return;
  }
  int progress = int(double(pointing_interval_) / kCornersTimeout * kProgressScale);
  ui_->CornersBar->setValue(progress);
}

void BotDialog::UpdateTick() {
  ShowCornerImages();
}

void BotDialog::OnClickPosition(int xpos, int ypos) {
  PointingCancel();
  QPoint point(xpos, ypos);
  switch (pointing_target_) {
    case kTopLeftCorner:
      top_left_corner_ = point;
      top_left_corner_defined_ = true;
      CornersCompleted();
      break;
    case kBottomRightCorner:
      bottom_right_corner_ = point;
      bottom_right_corner_defined_ = true;
      CornersCompleted();
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
  QRect rect;
  rect.setTopLeft(top_left_corner_);
  rect.setBottomRight(bottom_right_corner_);
  rect.normalized();
  scr_.SetFrameRect(rect);
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
