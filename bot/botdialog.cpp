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
