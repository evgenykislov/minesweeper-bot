#include <QScreen>
#include <QGuiApplication>

#include "screen.h"

Screen::Screen()
  : approx_row_amount_(0)
  , approx_col_amount_(0)
  , row_amount_(0)
  , col_amount_(0)
  , screen_absent_(true)
  , field_undetected_(true)
  , game_over_(false)
  , cell_width_(0)
  , cell_height_(0)
{
}

void Screen::SetApproximatelyRect(const QRect& rect) {
  approx_field_rect_ = rect;
  RefineRect();
}

void Screen::SetFieldSize(unsigned int row_amount, unsigned int col_amount) {
  approx_row_amount_ = row_amount;
  approx_col_amount_ = col_amount;
  RefineRect();
}

bool Screen::GetField(FieldType& field
  , bool& screen_absent
  , bool& field_undetected
  , bool& game_over
  , bool& unknown_images) {
  ProcessScreen();
  screen_absent = screen_absent_;
  field_undetected = field_undetected_;
  game_over = game_over_;
  unknown_images = !unknown_images_.empty();
  if (screen_absent || field_undetected || game_over || unknown_images) {
    field.clear();
    return false;
  }
  field = field_;
  return true;
}

void Screen::GetUnknownImages(std::list<QImage>& images) {
  images.swap(unknown_images_);
  unknown_images_.clear();
}

void Screen::ProcessScreen() {
  QScreen *screen = QGuiApplication::primaryScreen();
  if (!screen) {
    screen_absent_ = true;
    return;
  }
  screen_absent_ = false;
  auto pixmap = screen->grabWindow(0);
  auto field = pixmap.copy(field_rect_);
  if (field.size() != field_rect_.size()) {
    field_undetected_ = true;
    return;
  }
  if (cell_height_ == 0 || cell_width_ == 0) {
    field_undetected_ = true;
    return;
  }
  field_undetected_ = false;
  ClearField();
  unknown_images_.clear();
  for (unsigned int row_index = 0; row_index != row_amount_; ++row_index) {
    for (unsigned int col_index = 0; col_index != col_amount_; ++col_index) {
      auto cell = field.copy(col_index * cell_width_, row_index * cell_height_, cell_width_, cell_height_).toImage();
      // Find cell type
      bool cell_found = false;
      for (auto& info: cells_storage_) {
        if (info.cell_image_ == cell) {
          cell_found  =true;
          field_[row_index][col_index] = info.cell_type_;
          break;
        }
      }
      if (cell_found) continue;
      for (auto& image: unknown_images_) {
        if (image == cell) {
          cell_found = true;
          break;
        }
      }
      if (cell_found) continue;
      unknown_images_.push_back(cell);
    }
  }
}

void Screen::RefineRect() {
  row_amount_ = approx_row_amount_;
  col_amount_ = approx_col_amount_;
  if (row_amount_ == 0 || col_amount_ == 0) {
    cell_height_ = 0;
    cell_width_ = 0;
    return;
  }
  cell_height_ = (unsigned int)(approx_field_rect_.height() / row_amount_ + 0.5);
  cell_width_ = (unsigned int)(approx_field_rect_.width() / col_amount_ + 0.5);
  field_rect_.setTopLeft(approx_field_rect_.topLeft());
  field_rect_.setHeight(cell_height_ * row_amount_);
  field_rect_.setWidth(cell_width_ * col_amount_);
}

void Screen::ClearField() {
  field_.resize(row_amount_);
  for (size_t row_index = 0; row_index != row_amount_; ++row_index) {
    field_[row_index].resize(col_amount_);
    for (size_t col_index = 0; col_index != col_amount_; ++col_index) {
      field_[row_index][col_index] = ' ';
    }
  }
}
