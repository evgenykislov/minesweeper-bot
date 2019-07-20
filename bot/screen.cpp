#include <chrono>
#include <thread>

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QTest>

#include <fakeinput.hpp>

// Workaround of 'feature' of X11
#undef None

#include "screen.h"

#include "easylogging++.h"


using namespace std;
using namespace std::chrono;

BotScreen::BotScreen()
  : user_field_rect_(0, 0, 0, 0)
  , row_amount_(0)
  , col_amount_(0)
  , cell_width_(0)
  , cell_height_(0)
  , screen_id_(-1)
{
}

BotScreen::~BotScreen() {

}

void BotScreen::SetFrameRect(const QRect& rect) {
  lock_guard<mutex> locker(parameters_lock_);
  user_field_rect_ = rect;
  RefineRect();
}

void BotScreen::SetRestartPoint(const QPoint& point) {
  lock_guard<mutex> locker(parameters_lock_);
  restart_point_ = point;
}

void BotScreen::SetFieldSize(unsigned int row_amount, unsigned int col_amount) {
  lock_guard<mutex> locker(parameters_lock_);
  row_amount_ = row_amount;
  col_amount_ = col_amount;
  RefineRect();
}

void BotScreen::SetScreenID(int id) {
  lock_guard<mutex> locker(parameters_lock_);
  screen_id_ = id;
}

void BotScreen::GetStableField(Field& field
    , float timeout_sec
    , bool& game_over
    , bool& error_screen_absent
    , bool& error_field_undetected
    , bool& error_unknown_images
    , bool& error_timeout) {
  auto start = steady_clock::now();
  auto finish = start + duration<float>(timeout_sec);
  error_timeout = false;
  Field last_field;
  while (finish >= steady_clock::now()) {
    GetField(field, game_over, error_screen_absent, error_field_undetected, error_unknown_images);
    if (error_screen_absent || error_field_undetected || error_unknown_images) {
      return;
    }
    if (field == last_field) {
      return;
    }
    swap(last_field, field);
    this_thread::sleep_for(duration<float>(kSnapshotInterval));
  }
  error_timeout = true;
}

void BotScreen::GetChangedField(Field& field
    , const Field& base_field
    , float timeout_sec
    , bool& game_over
    , bool& error_screen_absent
    , bool& error_field_undetected
    , bool& error_unknown_images
    , bool& error_timeout) {
  auto start = steady_clock::now();
  auto finish = start + duration<float>(timeout_sec);
  error_timeout = false;
  while (finish >= steady_clock::now()) {
    GetField(field, game_over, error_screen_absent, error_field_undetected, error_unknown_images);
    if (error_screen_absent || error_field_undetected || error_unknown_images) {
      return;
    }
    if (field != base_field) {
      break;
    }
    this_thread::sleep_for(duration<float>(kSnapshotInterval));
  }
  auto next_start = steady_clock::now();
  if (finish < next_start) {
    error_timeout = true;
    return;
  }
  duration<float> next_timeout = finish - next_start;
  GetStableField(field, next_timeout.count(), game_over
    , error_screen_absent, error_field_undetected, error_unknown_images, error_timeout);
}

bool BotScreen::GetImageByPosition(unsigned int row, unsigned int col, QImage& image) {
  QRect field_rect;
  int cell_width;
  int cell_height;
  {
    lock_guard<mutex> locker(parameters_lock_);
    if (row >= row_amount_ || col >= col_amount_) { return false; }
    field_rect = field_rect_;
    cell_width = cell_width_;
    cell_height = cell_height_;
  }
  QScreen *screen = QGuiApplication::primaryScreen();
  if (!screen) { return false; }
  auto pixmap = screen->grabWindow(0);
  auto field = pixmap.copy(field_rect);
  if (field.size() != field_rect.size()) { return false; }
  if (cell_height == 0 || cell_width == 0) { return false; }
  image = field.copy(col * cell_width, row * cell_height, cell_width, cell_height).toImage();
  return true;
}

bool BotScreen::GetRestartImage(QImage& image) {
  QPoint p1, p2;
  {
    lock_guard<mutex> locker(parameters_lock_);
    p1 = restart_point_;
  }
  p2 = p1;
  p1.rx() -= kRestartImageSize / 2;
  p1.ry() -= kRestartImageSize / 2;
  p2.rx() += kRestartImageSize / 2;
  p2.ry() += kRestartImageSize / 2;
  QScreen *screen = QGuiApplication::primaryScreen();
  if (!screen) { return false; }
  auto pixmap = screen->grabWindow(0);
  image = pixmap.copy(QRect(p1, p2)).toImage();
  return true;
}

void BotScreen::GetUnknownImages(std::list<QImage>& images) {
  lock_guard<mutex> locker(parameters_lock_);
  images.swap(unknown_images_);
  unknown_images_.clear();
}

void BotScreen::SetImageType(const QImage& image, char cell_type) {
  lock_guard<mutex> locker(parameters_lock_);
  storage_.SetImageState(cell_type, image);
}

void BotScreen::MakeStep(unsigned int row, unsigned int col) {
  MakeCellClick(row, col, true);
}

void BotScreen::MakeMark(unsigned int row, unsigned int col) {
  MakeCellClick(row, col, false);
}

void BotScreen::MakeRestart() {
  QPoint point;
  {
    lock_guard<mutex> locker(parameters_lock_);
    point = restart_point_;
  }
  MakeClick(point, true);
}

void BotScreen::RefineRect() {
  if (row_amount_ < kMinimalCellsAtDim || col_amount_ < kMinimalCellsAtDim) {
    cell_height_ = 0;
    cell_width_ = 0;
    return;
  }
  cell_height_ = (unsigned int)(user_field_rect_.height() / (row_amount_ - 1) + 0.5);
  cell_width_ = (unsigned int)(user_field_rect_.width() / (col_amount_ - 1) + 0.5);
  field_rect_.setTopLeft(user_field_rect_.topLeft() - QPoint(cell_width_ / 2, cell_height_ / 2));
  field_rect_.setHeight(cell_height_ * row_amount_);
  field_rect_.setWidth(cell_width_ * col_amount_);
  LOG(INFO) << "Set screen field rect:"
    << field_rect_.left() << ", "
    << field_rect_.top() << ", "
    << field_rect_.right() << ", "
    << field_rect_.bottom() << ". Cells: "
    << row_amount_ << ", " << col_amount_;

}

void BotScreen::FormatField(Field& field) {
  lock_guard<mutex> locker(parameters_lock_);
  field.resize(row_amount_);
  for (size_t row_index = 0; row_index != row_amount_; ++row_index) {
    field[row_index].resize(col_amount_);
    for (size_t col_index = 0; col_index != col_amount_; ++col_index) {
      field[row_index][col_index] = ' ';
    }
  }
}

void BotScreen::MakeClick(const QPoint& point, bool left_button) {
  FakeInput::MouseButton button = FakeInput::Mouse_Left;
  if (!left_button) {
    button = FakeInput::Mouse_Right;
  }
  FakeInput::Mouse::moveTo(point.x(), point.y());
  FakeInput::Mouse::pressButton(button);
  FakeInput::Mouse::releaseButton(button);
}

void BotScreen::GetField(Field& field_1
  , bool& game_over
  , bool& error_screen_absent
  , bool& error_field_undetected
  , bool& error_unknown_images) {

  QRect field_rect;
  int cell_width;
  int cell_height;
  list<QImage> unknown_images;
  {
    lock_guard<mutex> locker(parameters_lock_);
    field_rect = field_rect_;
    cell_width = cell_width_;
    cell_height = cell_height_;
    unknown_images_.clear();
  }
  game_over = false;
  error_screen_absent = false;
  error_field_undetected = false;

  QScreen *screen = QGuiApplication::primaryScreen();
  if (!screen) {
    error_screen_absent = true;
    return;
  }
  auto pixmap = screen->grabWindow(0);
  auto pix_field = pixmap.copy(field_rect);
  if (pix_field.size() != field_rect.size()) {
    error_field_undetected = true;
    return;
  }
  if (cell_height == 0 || cell_width == 0) {
    error_field_undetected = true;
    return;
  }
  FormatField(field_1);
  for (unsigned int row_index = 0; row_index != row_amount_; ++row_index) {
    for (unsigned int col_index = 0; col_index != col_amount_; ++col_index) {
      auto cell = pix_field.copy(col_index * cell_width, row_index * cell_height, cell_width, cell_height).toImage();
      // Find cell type
      char cell_type;
      if (storage_.GetStateByImage(cell, cell_type)) {
        if (cell_type == kGameOverCell || cell_type == kWrongMineCell) {
          game_over = true;
        }
        field_1[row_index][col_index] = cell_type;
        continue;
      }
      bool cell_found = false;
      for (auto& image: unknown_images) {
        if (image == cell) {
          cell_found = true;
          break;
        }
      }
      if (cell_found) continue;
      unknown_images.push_back(cell);
    }
  }
  error_unknown_images = !unknown_images.empty();
  if (error_screen_absent || error_field_undetected || error_unknown_images) {
    field_1.clear();
  }
  {
    lock_guard<mutex> locker(parameters_lock_);
    swap(unknown_images_, unknown_images);
  }
}

void BotScreen::MakeCellClick(unsigned int row, unsigned int col, bool left_button) {
  int left;
  int top;
  int screen_id;
  {
    lock_guard<mutex> locker(parameters_lock_);
    left = field_rect_.left() + col * cell_width_ + cell_width_ / 2;
    top = field_rect_.top() + row * cell_height_ + cell_height_ / 2;
    screen_id = screen_id_;
  }
  QWidget* screen = QApplication::desktop()->screen(screen_id);
  QPoint point(left, top);
  MakeClick(point, left_button);
}
