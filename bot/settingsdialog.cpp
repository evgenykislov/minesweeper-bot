/* Minesweeper Bot: Cross-platform bot for playing in minesweeper game.
 * Copyright (C) 2019 Evgeny Kislov.
 * https://www.evgenykislov.com/minesweeper-bot
 * https://github.com/evgenykislov/minesweeper-bot
 *
 * This file is part of Minesweeper Bot.
 *
 * Minesweeper Bot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Minesweeper Bot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Minesweeper Bot.  If not, see <https://www.gnu.org/licenses/>.
 */

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
