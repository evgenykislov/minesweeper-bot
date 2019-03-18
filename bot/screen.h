#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <list>

#include <QPixmap>

class Screen
{
public:
  Screen();

  void SetLeftTopCorner(int y, int x);

  bool Field(std::vector<std::vector<char>>& field); //< Get the game field from screen(desktop)
  void State(bool& field_detected, bool& game_over, bool& unknown_images);
  bool GetUnknownImages(std::list<QPixmap>& images);
  void SetImageSymbol(const QPixmap& image, char symbol);
};

#endif // SCREEN_H
