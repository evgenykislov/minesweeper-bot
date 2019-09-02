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

#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <list>

#include <QPixmap>

#include "field.h"
#include "imagesstorage.h"

class BotScreen
{
public:
  BotScreen();
  virtual ~BotScreen();

  void SetFrameRect(const QRect& rect);
  void SetRestartPoint(const QPoint& point);
  void SetFieldSize(unsigned int row_amount, unsigned int col_amount);
  void SetScreenID(int id);

  /*! \brief Get stable field. With thread blocking */
  void GetStableField(Field& field
    , float timeout_sec
    , bool& game_over
    , bool& error_screen_absent
    , bool& error_field_undetected
    , bool& error_unknown_images
    , bool& error_timeout);
  /*! \brief Get next-stable field. With thread blocking */
  void GetChangedField(Field& field
    , const Field& base_field
    , float timeout_sec
    , bool& game_over
    , bool& error_screen_absent
    , bool& error_field_undetected
    , bool& error_unknown_images
    , bool& error_timeout);
  bool GetImageByPosition(unsigned int row, unsigned int col, QImage& image);
  bool GetRestartImage(QImage& image);
  void GetUnknownImages(std::list<QImage>& images);
  void SetImageType(const QImage& image, char cell_type);
  void MakeStep(unsigned int row, unsigned int col);
  void MakeMark(unsigned int row, unsigned int col);
  void MakeRestart();

 private:
  BotScreen(const BotScreen&) = delete;
  BotScreen(BotScreen&&) = delete;
  BotScreen& operator=(const BotScreen&) = delete;
  BotScreen& operator=(BotScreen&&) = delete;

  const float kSnapshotInterval = 0.02f; // Interval of screen snapshoting, in seconds
  const unsigned int kRestartImageSize = 24;

  const char kGameOverCell = 'x';
  const char kWrongMineCell = '-';
  const unsigned int kMinimalCellsAtDim = 2;
  const unsigned int kMinimalCellSize = 3;
  QRect user_field_rect_; // Rect of field, entered by user
  QRect field_rect_; // Rect of field, recalculated for rows/columns
  unsigned int row_amount_;
  unsigned int col_amount_;
  std::list<QImage> unknown_images_;
  QPoint restart_point_;
  std::mutex parameters_lock_;

  unsigned int cell_width_;
  unsigned int cell_height_;

  int screen_id_;
  ImagesStorage storage_;

  void RefineRect();
  void FormatField(Field& field); //!< Clear field and set right format (rows, columns)
  void MakeClick(const QPoint& point, bool left_button);
  void GetField(Field& field
    , bool& game_over
    , bool& error_screen_absent
    , bool& error_field_undetected
    , bool& error_unknown_images);
  void MakeCellClick(unsigned int row, unsigned int col, bool left_button);
};

#endif // SCREEN_H
