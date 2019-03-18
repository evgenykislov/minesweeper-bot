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
  void State(bool& field_detected, bool& game_over, bool& unknown_images);
  bool GetUnknownImages(std::list<QPixmap>& images);
  void SetImageSymbol(const QPixmap& image, char symbol);

 private:
  QRect field_rect_;
  unsigned int row_amount_;
  unsigned int col_amount_;
};

#endif // SCREEN_H
