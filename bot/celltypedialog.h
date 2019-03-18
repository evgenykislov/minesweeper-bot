#ifndef CELLTYPEDIALOG_H
#define CELLTYPEDIALOG_H

#include <QDialog>

namespace Ui {
class CellTypeDialog;
}

class CellTypeDialog : public QDialog
{
  Q_OBJECT

public:
  explicit CellTypeDialog(QWidget *parent = 0);
  ~CellTypeDialog();

  void SetImage(const QImage& image);
  char GetCellType();

 public slots:
  void OnSelectType();

private:
  enum {
    kImageSize = 48,
    kImageSizeWithBorder = 64,
  };

  const QColor border_backcolor = QColor(192, 192, 192);
  const QColor border_linecolor = QColor(0, 0, 0);
  const int border_line_step = 4;

  Ui::CellTypeDialog* ui;
  char cell_type_;
};

#endif // CELLTYPEDIALOG_H
