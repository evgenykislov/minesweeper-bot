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
  void MoveField(int move_horizontal, int move_vertical);
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
  void GetUnknownImages(std::list<QImage>& images);
  void SetImageType(const QImage& image, char cell_type);
  void MakeStep(unsigned int row, unsigned int col);
  void MakeRestart();

 private:
  BotScreen(const BotScreen&) = delete;
  BotScreen(BotScreen&&) = delete;
  BotScreen& operator=(const BotScreen&) = delete;
  BotScreen& operator=(BotScreen&&) = delete;

  const float kSnapshotInterval = 0.02; // Interval of screen snapshoting, in seconds

  const char kGameOverCell = 'x';
  QRect user_field_rect_; // Rect of field, entered by user
  QRect field_rect_; // Rect of field, recalculated for rows/columns
  unsigned int row_amount_;
  unsigned int col_amount_;
  std::list<QImage> unknown_images_;
  QPoint restart_point_;
  std::mutex parameters_lock_;

  int cell_width_;
  int cell_height_;

  int screen_id_;
  ImagesStorage storage_;

  void RefineRect();
  void FormatField(Field& field); //!< Clear field and set right format (rows, columns)
  void MakeClick(const QPoint& point);
  void GetField(Field& field
    , bool& game_over
    , bool& error_screen_absent
    , bool& error_field_undetected
    , bool& error_unknown_images);

};

#endif // SCREEN_H
