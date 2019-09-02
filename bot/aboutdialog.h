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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AboutDialog(QWidget *parent = 0);
  ~AboutDialog();

 public slots:
  void OnLicenses();

private:
  Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
