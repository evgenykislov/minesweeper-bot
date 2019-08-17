#include "field.h"

Field::Field()
{

}

bool Field::IsFullClosed() {
  for (unsigned int row = 0; row < size(); ++row) {
    for (unsigned int col = 0; col < (*this)[row].size(); ++col) {
      switch ((*this)[row][col]) {
        case ' ':
        case '.':
          break;
        default:
          return false;
      }
    }
  }
  return true;
}
