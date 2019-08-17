#include <QMessageBox>

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

void SettingsDialog::Set(const Parameters& params) {
  params_ = params;
  ui_->autorestart_chk_->setChecked(params_.auto_restart_game_);
  ui_->save_steps_chk_->setChecked(params_.save_all_steps_);
  ui_->save_folder_edt_->setText(params_.steps_save_folder_);
  ui_->unexpected_error_chk_->setChecked(params_.save_unexpected_error_steps_);
  ui_->wrong_mine_chk_->setChecked(params_.save_steps_before_wrong_mine_);
  ui_->probability_steps_chk_->setChecked(params_.save_probability_steps_);
  ui_->full_close_chk_->setChecked(params_.save_fully_closed_steps_);
  UpdateChecks();
}

void SettingsDialog::Get(Parameters& params) {
  params = params_;
}

void SettingsDialog::OnAccept() {
  params_.auto_restart_game_ = ui_->autorestart_chk_->checkState() == Qt::Checked;
  params_.save_all_steps_ = ui_->save_steps_chk_->checkState() == Qt::Checked;
  params_.steps_save_folder_ = ui_->save_folder_edt_->text();
  params_.save_unexpected_error_steps_ = ui_->unexpected_error_chk_->checkState() == Qt::Checked;
  params_.save_steps_before_wrong_mine_ = ui_->wrong_mine_chk_->checkState() == Qt::Checked;
  params_.save_probability_steps_ = ui_->probability_steps_chk_->checkState() == Qt::Checked;
  params_.save_fully_closed_steps_ = ui_->full_close_chk_->checkState() == Qt::Checked;
  emit accept();
}

void SettingsDialog::OnProbabilityChanged(int) {
  UpdateChecks();
}

void SettingsDialog::UpdateChecks() {
  ui_->full_close_chk_->setEnabled(ui_->probability_steps_chk_->checkState() == Qt::Checked);
}
