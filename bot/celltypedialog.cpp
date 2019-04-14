#include <cassert>

#include <QPainter>

#include "celltypedialog.h"
#include "ui_celltypedialog.h"

CellTypeDialog::CellTypeDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::CellTypeDialog)
  , cell_type_('?')
{
  ui->setupUi(this);
}

CellTypeDialog::~CellTypeDialog()
{
  delete ui;
}

void CellTypeDialog::SetImage(const QImage& image) {
  // Scale the image to given size
  auto scaled_image = image.scaled(kImageSize, kImageSize, Qt::KeepAspectRatio);
  QImage border_image(kImageSizeWithBorder, kImageSizeWithBorder, QImage::Format_RGB888);
  QPainter painter;
  painter.begin(&border_image);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.setPen(border_linecolor);
  painter.fillRect(border_image.rect(), border_backcolor);
  for (int y = -kImageSizeWithBorder; y < kImageSizeWithBorder; y += border_line_step) {
    painter.drawLine(0, y, kImageSizeWithBorder, y + kImageSizeWithBorder);
  }
  assert(kImageSizeWithBorder >= kImageSize);
  unsigned int border_width = (kImageSizeWithBorder - kImageSize) / 2;
  painter.drawImage(QPointF(border_width, border_width), scaled_image);
  painter.end();
  ui->cell_img_->setPixmap(QPixmap::fromImage(border_image));
}

char CellTypeDialog::GetCellType() {
  return cell_type_;
}

void CellTypeDialog::OnSelectType() {
  auto sender_btn = sender();
  if (sender_btn == ui->closed_btn_) { cell_type_ = '.'; }
  else if (sender_btn == ui->bomb_btn_) { cell_type_ = '*'; }
  else if (sender_btn == ui->out_btn_) { cell_type_ = ' '; }
  else if (sender_btn == ui->a0_btn_) { cell_type_ = '0'; }
  else if (sender_btn == ui->a1_btn_) { cell_type_ = '1'; }
  else if (sender_btn == ui->a2_btn_) { cell_type_ = '2'; }
  else if (sender_btn == ui->a3_btn_) { cell_type_ = '3'; }
  else if (sender_btn == ui->a4_btn_) { cell_type_ = '4'; }
  else if (sender_btn == ui->a5_btn_) { cell_type_ = '5'; }
  else if (sender_btn == ui->a6_btn_) { cell_type_ = '6'; }
  else if (sender_btn == ui->a7_btn_) { cell_type_ = '7'; }
  else if (sender_btn == ui->a8_btn_) { cell_type_ = '8'; }
  else if (sender_btn == ui->death_btn_) { cell_type_ = 'x'; }
  else {
    assert(0);
  }  emit accept();
}
