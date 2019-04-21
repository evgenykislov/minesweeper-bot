#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::SettingsDialog)
{
  ui_->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
  delete ui_;
}

void SettingsDialog::OnAccept() {
  emit accept();
}
