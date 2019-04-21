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

void SettingsDialog::Set(bool auto_restart_game, bool save_steps, const QString& save_folder, unsigned int start_index) {
  auto_restart_game_ = auto_restart_game;
  save_steps_ = save_steps;
  save_folder_ = save_folder;
  start_index_ = start_index;
  ui_->autorestart_chk_->setChecked(auto_restart_game_);
  ui_->save_steps_chk_->setChecked(save_steps_);
  ui_->save_folder_edt_->setText(save_folder_);
  ui_->start_index_edt_->setText(QString("%1").arg(start_index_));
}

void SettingsDialog::Get(bool& auto_restart_game, bool& save_steps, QString& save_folder, unsigned int& start_index) {
  auto_restart_game = auto_restart_game_;
  save_steps = save_steps_;
  save_folder = save_folder_;
  start_index = start_index_;
}

void SettingsDialog::OnAccept() {
  auto_restart_game_ = ui_->autorestart_chk_->checkState() == Qt::Checked;
  save_steps_ = ui_->save_steps_chk_->checkState() == Qt::Checked;
  save_folder_ = ui_->save_folder_edt_->text();
  emit accept();
}
