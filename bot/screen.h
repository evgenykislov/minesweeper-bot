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
