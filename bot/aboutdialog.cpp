#include "aboutdialog.h"
#include "licensesdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AboutDialog)
{
  ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
  delete ui;
}

void AboutDialog::OnLicenses() {
  LicensesDialog dlg(this);
  dlg.exec();
}
