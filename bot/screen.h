#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <list>

#include <QPixmap>

#include "imagesstorage.h"

using FieldType = std::vector<std::vector<char>>;

class BotScreen
{
public:
  BotScreen();
  virtual ~BotScreen();

  void SetApproximatelyRect(const QRect& rect);
  void SetRestartPoint(const QPoint& point);
  void SetFieldSize(unsigned int row_amount, unsigned int col_amount);
  void MoveField(int move_horizontal, int move_vertical);
  void SetScreenID(int id);

  bool GetField(FieldType& field
    , bool& screen_absent
    , bool& field_undetected
    , bool& game_over
    , bool& unknown_images);
  bool GetImageByPosition(unsigned int row, unsigned int col, QImage& image);
  void GetUnknownImages(std::list<QImage>& images);
  void SetImageType(const QImage& image, char cell_type);
  void MakeStep(unsigned int row, unsigned int col);
  void RestartGame();

 private:
  BotScreen(const BotScreen&) = delete;
  BotScreen(BotScreen&&) = delete;
  BotScreen& operator=(const BotScreen&) = delete;
  BotScreen& operator=(BotScreen&&) = delete;

  const unsigned int kCutMargin = 0;
  const char kGameOverCell = 'x';
  QRect approx_field_rect_;
  QRect field_rect_;
  unsigned int approx_row_amount_;
  unsigned int approx_col_amount_;
  unsigned int row_amount_;
  unsigned int col_amount_;
  bool screen_absent_;
  bool field_undetected_;
  bool game_over_;
  std::list<QImage> unknown_images_;
  QPoint restart_point_;

  int cell_width_;
  int cell_height_;

  void ProcessScreen();
  void RefineRect();
  void ClearField();
  void MakeClick(const QPoint& point);

  std::vector<std::vector<char>> field_;
  int screen_id_;
  ImagesStorage storage_;
};

#endif // SCREEN_H
