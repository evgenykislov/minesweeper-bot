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
  ui_->start_index_edt_->setText(QString("%1").arg(params_.steps_start_index_));
  ui_->finish_index_edt_->setText(QString("%1").arg(params_.steps_finish_index_));
  ui_->unexpected_error_chk_->setChecked(params_.save_unexpected_error_steps_);
  ui_->wrong_mine_chk_->setChecked(params_.save_steps_before_wrong_mine_);
  ui_->probability_steps_chk_->setChecked(params_.save_probability_steps_);
}

void SettingsDialog::Get(Parameters& params) {
  params = params_;
}

void SettingsDialog::OnAccept() {
  params_.auto_restart_game_ = ui_->autorestart_chk_->checkState() == Qt::Checked;
  params_.save_all_steps_ = ui_->save_steps_chk_->checkState() == Qt::Checked;
  params_.steps_save_folder_ = ui_->save_folder_edt_->text();
  bool ok;
  params_.steps_start_index_ = ui_->start_index_edt_->text().toUInt(&ok);
  if (!ok) {
    QMessageBox::warning(this, tr("Input error"), tr("Type number in start index field"));
    return;
  }
  params_.steps_finish_index_ = ui_->finish_index_edt_->text().toUInt(&ok);
  if (!ok) {
    QMessageBox::warning(this, tr("Input error"), tr("Type number in finish index field"));
    return;
  }
  params_.save_unexpected_error_steps_ = ui_->unexpected_error_chk_->checkState() == Qt::Checked;
  params_.save_steps_before_wrong_mine_ = ui_->wrong_mine_chk_->checkState() == Qt::Checked;
  params_.save_probability_steps_ = ui_->probability_steps_chk_->checkState() == Qt::Checked;
  emit accept();
}
