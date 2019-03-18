#include "botdialog.h"

#include "ui_botdialog.h"
#include "eventfilter.h"

BotDialog::BotDialog(QWidget *parent)
  : QDialog(parent)
  , state_(kIdle)
  , ui_(new Ui::BotDialog)
  , corners_interval_(0)
{
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
  removeEventFilter(&filter_);
  releaseMouse();
  if (state_ == kCornersSelection) {
    QRect rect;
    if (!filter_.GetRect(rect)) {
      // Errors in corners selection
      state_ = kIdle;
    }
    else {
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
}

void BotDialog::CornersCancelled() {
  removeEventFilter(&filter_);
  releaseMouse();
}

void BotDialog::TimerTick() {
  if (state_ == kCornersSelection) {
    corners_interval_ += kTimerInterval;
    if (corners_interval_ >= kCornersTimeout) {
      CornersCancelled();
    }
    else {
      int progress = int(double(corners_interval_) / kCornersTimeout * kProgressScale);
      ui_->CornersBar->setValue(progress);
    }
  }
}
