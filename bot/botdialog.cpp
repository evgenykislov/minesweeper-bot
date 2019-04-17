#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

#include <QDesktopWidget>
#include <QFile>
#include <QPainter>

#include <uiohook.h>

#include "botdialog.h"
#include "celltypedialog.h"
#include "ui_botdialog.h"

using namespace std;
using namespace std::chrono;

function<void(int xpos, int ypos)> hook_lambda_;

void HookHandle(uiohook_event* const event) {
  assert(hook_lambda_);
  if (!event) {
    return;
  }
  if (event->type != EVENT_MOUSE_CLICKED) {
    return;
  }
  if (event->data.mouse.button != MOUSE_BUTTON1) {
    return;
  }
  hook_lambda_(event->data.mouse.x, event->data.mouse.y);
}


BotDialog::BotDialog(QWidget *parent)
  : QDialog(parent)
  , corners_defined_(false)
  , measures_defined_(false)
  , ui_(new Ui::BotDialog)
  , corners_interval_(0)
  , click_index_(0)
  , step_counter_(0)
  , step_row_(0)
  , step_column_(0)
  , step_success_(false)
{
  setWindowFlag(Qt::WindowStaysOnTopHint);
  ui_->setupUi(this);
  ui_->CornersLbl->hide();
  connect(&corners_timer_, &QTimer::timeout, this, &BotDialog::CornersTick, Qt::QueuedConnection);
  connect(&game_timer_, &QTimer::timeout, this, &BotDialog::GameTick, Qt::QueuedConnection);
  connect(this, &BotDialog::DoClickPosition, this, &BotDialog::OnClickPosition, Qt::QueuedConnection);
  connect(ui_->row_edt_, &LineEditWFocus::LoseFocus, this, &BotDialog::LoseFocus, Qt::QueuedConnection);
  connect(ui_->col_edt_, &LineEditWFocus::LoseFocus, this, &BotDialog::LoseFocus, Qt::QueuedConnection);
  connect(ui_->mines_edt_, &LineEditWFocus::LoseFocus, this, &BotDialog::LoseFocus, Qt::QueuedConnection);
  hook_lambda_ = [this](int xpos, int ypos) {
    emit DoClickPosition(xpos, ypos);
  };
  hook_set_dispatch_proc(HookHandle);

  QFile model_file("model.bin");
  if (model_file.open(QIODevice::ReadOnly)) {
    QByteArray byte_data = model_file.readAll();
    model_file.close();
    vector<uint8_t> vect_data((uint8_t*)(byte_data.data()), (uint8_t*)(byte_data.data() + byte_data.size()));
    solver.LoadModel(std::move(vect_data));
  }
}

BotDialog::~BotDialog()
{
  CornersCancel();
  hook_set_dispatch_proc(nullptr);
  hook_lambda_ = nullptr;
  delete ui_;
}

void BotDialog::OnCornersBtn() {
  try {
    CornersCancel();
    // Start new corners request
    assert(!hook_thread_);
    click_index_ = 0;
    hook_thread_.reset(new std::thread([](){
      hook_run();
    }));
    corners_interval_ = 0;
    corners_timer_.start(kTimerInterval);
  }
  catch (exception&) {
    corners_defined_ = false;
    corners_timer_.stop();
  }
}

void BotDialog::CornersCompleted(QRect frame) {
  CornersCancel();
  corners_defined_ = true;
  scr_.SetScreenID(QApplication::desktop()->screenNumber(this));
  scr_.SetApproximatelyRect(frame);
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
  FormImage(top_left, top_left_pixel);
  ui_->cell_top_right_->setPixmap(top_left_pixel);
  QImage bottom_left;
  QPixmap bottom_left_pixel;
  scr_.GetImageByPosition(row_amount - 1, 0, bottom_left);
  FormImage(bottom_left, bottom_left_pixel);
  ui_->cell_bottom_left_->setPixmap(bottom_left_pixel);
  QImage bottom_right;
  QPixmap bottom_right_pixel;
  scr_.GetImageByPosition(row_amount - 1, col_amount - 1, bottom_right);
  FormImage(bottom_left, bottom_right_pixel);
  ui_->cell_bottom_right_->setPixmap(bottom_right_pixel);
}

void BotDialog::StopGame() {
  game_timer_.stop();
}

void BotDialog::SaveStep() {
  // save field
  stringstream field_filename;
  field_filename << "field_" << setfill('0') << setw(5) << step_counter_ << ".txt";
  ofstream field_file(field_filename.str(), ios_base::trunc);
  if (!field_file) {
    // TODO can't save
    return;
  }
  for (auto row_iter = step_field_.begin(); row_iter != step_field_.end(); ++row_iter) {
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
  step_filename << "step_" << setfill('0') << setw(5) << step_counter_ << ".txt";
  ofstream step_file(step_filename.str(), ios_base::trunc);
  if (!step_file) {
    // TODO can't save
    return;
  }
  step_file << step_row_ << " " << step_column_ << " " << step_success_ << " " << field_filename.str() << endl;
  step_file.close();
}

void BotDialog::OnRun() {
  if (!corners_defined_ || !measures_defined_) {
    // TODO inform user about
    return;
  }
  game_timer_.start(kTimerInterval);
}

void BotDialog::LoseFocus() {
  auto row_amount = ui_->row_edt_->text().toUInt();
  auto col_amount = ui_->col_edt_->text().toUInt();
  auto mines_amount = ui_->mines_edt_->text().toUInt();
  scr_.SetFieldSize(row_amount, col_amount);
  ShowCornerImages();
  measures_defined_ = row_amount > 0 && col_amount > 0 && mines_amount > 0;
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

void BotDialog::CornersCancel() {
  corners_defined_ = false;
  corners_timer_.stop();
  if (hook_thread_ && hook_thread_->joinable()) {
    hook_stop();
    hook_thread_->join();
  }
  hook_thread_.reset();
}

void BotDialog::MakeStep(const FieldType& field) {
  unsigned int row;
  unsigned int col;
  bool sure_step;
  ++step_counter_;
  try {
    step_field_ = field;
    auto step_start = steady_clock::now();
    solver.GetStep(field, row, col, sure_step);
    auto before_make_step = steady_clock::now();
    scr_.MakeStep(row, col);
    step_row_ = row;
    step_column_ = col;
    auto after_step = steady_clock::now();
    duration<double> calc_time = before_make_step - step_start;
    duration<double> mouse_time = after_step - before_make_step;
    auto debug_message = QString::fromUtf8(u8"Step: %1, Intervals: %2 + %3").arg(step_counter_).arg(calc_time.count()).arg(mouse_time.count());
    ui_->debug_lbl_->setText(debug_message);
  }
  catch (exception&) {
    // TODO Can't make a step
  }
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

void BotDialog::CornersTick() {
  corners_interval_ += kTimerInterval;
  if (corners_interval_ >= kCornersTimeout) {
    CornersCancel();
    return;
  }
  int progress = int(double(corners_interval_) / kCornersTimeout * kProgressScale);
  ui_->CornersBar->setValue(progress);
}

void BotDialog::GameTick() {
  if (!unknown_images_.empty()) {
    return;
  }
  FieldType field;
  bool no_screen;
  bool no_field;
  bool game_over;
  bool unknown_images;
  auto valid_field = scr_.GetField(field, no_screen, no_field, game_over, unknown_images);
  if (valid_field) {
    step_success_ = true;
    if (ui_->save_steps_chk_->checkState() == Qt::Checked) {
      SaveStep();
    }
    MakeStep(field);
    return;
  }
  // There are some stoppers
  if (no_screen || no_field) {
    StopGame();
    return;
  }
  if (game_over) {
    step_success_ = false;
    if (ui_->save_steps_chk_->checkState() == Qt::Checked) {
      SaveStep();
    }
    StopGame();
    return;
  }
  if (unknown_images) {
    ShowUnknownImages();
  }
}

void BotDialog::OnClickPosition(int xpos, int ypos) {
  if (click_index_ >= kClickAmount) {
    return;
  }
  clicks_[click_index_] = QPoint(xpos, ypos);
  ++click_index_;
  if (click_index_ == kClickAmount) {
    QRect rect;
    rect.setTopLeft(clicks_[0]);
    rect.setBottomRight(clicks_[1]);
    CornersCompleted(rect.normalized());
  }
}
