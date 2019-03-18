#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <list>

#include <QPixmap>

class Screen
{
public:
  Screen();

  void SetApproximatelyRect(const QRect& rect);
  void SetFieldSize(unsigned int row_amount, unsigned int col_amount);

  bool Field(std::vector<std::vector<char>>& field); //< Get the game field from screen(desktop)
  void State(bool& screen_absent, bool& field_undetected, bool& game_over, bool& unknown_images);
  bool GetUnknownImages(std::list<QImage>& images);
  void SetImageSymbol(const QPixmap& image, char symbol);

 private:
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
};

#endif // SCREEN_H
