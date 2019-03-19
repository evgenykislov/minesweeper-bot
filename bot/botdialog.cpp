#include <cassert>

#include <QDesktopWidget>

#include "botdialog.h"
#include "celltypedialog.h"
#include "ui_botdialog.h"
#include "eventfilter.h"

BotDialog::BotDialog(QWidget *parent)
  : QDialog(parent)
  , state_(kIdle)
  , ui_(new Ui::BotDialog)
  , corners_interval_(0)
{
  setWindowFlag(Qt::WindowStaysOnTopHint);
  ui_->setupUi(this);
  ui_->CornersLbl->hide();
  connect(&filter_, &EventFilter::TwoClicks, this, &BotDialog::CornersCompleted, Qt::QueuedConnection);
  connect(&timer_200ms_, &QTimer::timeout, this, &BotDialog::TimerTick, Qt::QueuedConnection);
}

BotDialog::~BotDialog()
{
  delete ui_;
}

void BotDialog::OnCornersBtn() {
  filter_.Clear();
  installEventFilter(&filter_);
  grabMouse();
  corners_interval_ = 0;
  timer_200ms_.start(kTimerInterval);
  state_ = kCornersSelection;
}

void BotDialog::CornersCompleted() {
  CornersCancel();
  removeEventFilter(&filter_);
  releaseMouse();
  if (state_ == kCornersSelection) {
    QRect rect;
    if (!filter_.GetRect(rect)) {
      // Errors in corners selection
      state_ = kIdle;
    }
    else {
      scr_.SetScreenID(QApplication::desktop()->screenNumber(this));
      scr_.SetApproximatelyRect(rect);
      state_ = kSizeModification;
    }
  }
}

void BotDialog::OnRun() {
  if (state_ != kSizeModification) {
    return;
  }
  auto row_amount = ui_->row_edt_->text().toUInt();
  auto col_amount = ui_->col_edt_->text().toUInt();
  scr_.SetFieldSize(row_amount, col_amount);
  state_ = kRun;
  timer_200ms_.start(kTimerInterval);
}

void BotDialog::OnImageClick() {


}

void BotDialog::CornersCancel() {
  timer_200ms_.stop();
  removeEventFilter(&filter_);
  releaseMouse();
}

void BotDialog::MakeStep(const FieldType& field) {
  static size_t step_index = 0;
  if (step_index >= field[0].size()) {
    return;
  }
  scr_.MakeStep(0, step_index);
}

void BotDialog::ShowUnknownImages() {
  scr_.GetUnknownImages(unknown_images_);
  assert(!unknown_images_.empty());
  UpdateUnknownImages();
}

void BotDialog::UpdateUnknownImages() {
  if (unknown_images_.empty()) {
    // Run the gaming
    state_ = kRun;
    timer_200ms_.start(kTimerInterval);
    return;
  }
  CellTypeDialog dlg;
  dlg.SetImage(unknown_images_.front());
  if (dlg.exec() == QDialog::Rejected) {
    // Nothing to do. Wait for ...
    return;
  }
  scr_.SetImageType(unknown_images_.front(), dlg.GetCellType());
  unknown_images_.pop_front();
  UpdateUnknownImages();
}

void BotDialog::TimerTick() {
  if (state_ == kCornersSelection) {
    corners_interval_ += kTimerInterval;
    if (corners_interval_ >= kCornersTimeout) {
      CornersCancel();
      state_ = kIdle;
      return;
    }
    int progress = int(double(corners_interval_) / kCornersTimeout * kProgressScale);
    ui_->CornersBar->setValue(progress);
    return;
  }
  if (state_ == kRun) {
    FieldType field;
    bool no_screen;
    bool no_field;
    bool game_over;
    bool unknown_images;
    auto valid_field = scr_.GetField(field, no_screen, no_field, game_over, unknown_images);
    if (valid_field) {
      MakeStep(field);
      return;
    }
    timer_200ms_.stop();
    // There are some stoppers
    if (no_screen || no_field) {
      state_ = kIdle;
      return;
    }
    if (game_over) {
      state_ = kIdle;
      return;
    }
    if (unknown_images) {
      state_ = kWaitForImageRecognition;
      ShowUnknownImages();
    }
  }
}
