#ifndef LICENSESDIALOG_H
#define LICENSESDIALOG_H

#include <QDialog>

namespace Ui {
class LicensesDialog;
}

class LicensesDialog : public QDialog
{
  Q_OBJECT

public:
  explicit LicensesDialog(QWidget *parent = 0);
  ~LicensesDialog();

private:
  Ui::LicensesDialog *ui;
};

#endif // LICENSESDIALOG_H
