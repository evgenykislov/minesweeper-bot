#include "licensesdialog.h"
#include "ui_licensesdialog.h"

LicensesDialog::LicensesDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::LicensesDialog)
{
  ui->setupUi(this);
}

LicensesDialog::~LicensesDialog()
{
  delete ui;
}
