#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <list>

#include <QPixmap>
using FieldType = std::vector<std::vector<char>>;

class Screen
{
public:
  Screen();

  void SetApproximatelyRect(const QRect& rect);
  void SetFieldSize(unsigned int row_amount, unsigned int col_amount);
  void SetScreenID(int id);

  bool GetField(FieldType& field
    , bool& screen_absent
    , bool& field_undetected
    , bool& game_over
    , bool& unknown_images);
  void GetUnknownImages(std::list<QImage>& images);
  void SetImageType(const QImage& image, char cell_type);
  void MakeStep(unsigned int row, unsigned int col);

 private:
  const unsigned int kCutMargin = 2;
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

  int cell_width_;
  int cell_height_;

  void ProcessScreen();
  void RefineRect();
  void ClearField();

  struct CellInfo {
    QImage cell_image_;
    char cell_type_;
  };

  std::vector<CellInfo> cells_storage_;
  std::vector<std::vector<char>> field_;
  int screen_id_;
};

#endif // SCREEN_H
