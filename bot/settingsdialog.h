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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit SettingsDialog(QWidget *parent = nullptr);
  ~SettingsDialog();

  struct Parameters {
    // Gaming
    bool auto_restart_game_;
    // Training steps
    bool save_all_steps_;
    QString steps_save_folder_;
    // Error Tracking
    bool save_unexpected_error_steps_;
    bool save_steps_before_wrong_mine_;
    bool save_probability_steps_;
    bool save_fully_closed_steps_;
  };

  void Set(const Parameters& params);
  void Get(Parameters& params);

 public slots:
  void OnAccept();
  void OnProbabilityChanged(int);

 private:
  Ui::SettingsDialog *ui_;
  Parameters params_;

  void UpdateChecks();
};

#endif // SETTINGSDIALOG_H
