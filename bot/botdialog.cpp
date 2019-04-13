#include <cassert>
#include <mutex>

#include <QDesktopWidget>
#include <QFile>

#include <uiohook.h>

#include "botdialog.h"
#include "celltypedialog.h"
#include "ui_botdialog.h"

using namespace std;

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
  , state_(kIdle)
  , ui_(new Ui::BotDialog)
  , corners_interval_(0)
  , click_index_(0)
{
  setWindowFlag(Qt::WindowStaysOnTopHint);
  ui_->setupUi(this);
  ui_->CornersLbl->hide();
  connect(&timer_200ms_, &QTimer::timeout, this, &BotDialog::TimerTick, Qt::QueuedConnection);
  connect(this, &BotDialog::DoClickPosition, this, &BotDialog::OnClickPosition, Qt::QueuedConnection);
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
    timer_200ms_.start(kTimerInterval);
    state_ = kCornersSelection;
  }
  catch (exception&) {
    state_ = kIdle;
    timer_200ms_.stop();
  }
}

void BotDialog::CornersCompleted(QRect frame) {
  CornersCancel();
  if (state_ == kCornersSelection) {
    scr_.SetScreenID(QApplication::desktop()->screenNumber(this));
    scr_.SetApproximatelyRect(frame);
    state_ = kSizeModification;
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

void BotDialog::CornersCancel() {
  timer_200ms_.stop();
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
  try {
    solver.GetStep(field, row, col, sure_step);
    scr_.MakeStep(row, col);
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
