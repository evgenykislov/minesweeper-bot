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

#ifndef DNNCLASSIFIER_H
#define DNNCLASSIFIER_H

#include <cstdint>
#include <vector>

class Classifier
{
 public:
  typedef std::vector<std::vector<char>> Field;
  enum StepAction {
    kOpenWithSure = 0,
    kOpenWithProbability = 1,
    kMarkAsMine = 2,
  };

  Classifier() {}
  virtual ~Classifier() {}
  virtual bool LoadModel(std::vector<uint8_t>&& data) = 0;
  virtual void GetStep(const Field& field, unsigned int mines_amount, unsigned int& step_row, unsigned int& step_col, StepAction& step) = 0;

 protected:
  const char kHideCell = '.';
  void GetTestField(Field& field);

  bool IsClosedCell(char cell) { return cell == '.'; }
  bool IsMineCell(char cell) { return cell == '*'; }
  bool IsValueCell(char cell, unsigned int& value) {
    switch (cell) {
      case '0': value = 0; break;
      case '1': value = 1; break;
      case '2': value = 2; break;
      case '3': value = 3; break;
      case '4': value = 4; break;
      case '5': value = 5; break;
      case '6': value = 6; break;
      case '7': value = 7; break;
      case '8': value = 7; break;
      default:
        return false;
    }
    return true;
  }

 private:
  Classifier(const Classifier&) = delete;
  Classifier(Classifier&&) = delete;
  Classifier& operator=(const Classifier&) = delete;
  Classifier& operator=(Classifier&&) = delete;
};

#endif // DNNCLASSIFIER_H
