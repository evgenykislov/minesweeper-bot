#ifndef FIELD_H
#define FIELD_H

#include <vector>

class Field: public std::vector<std::vector<char>>
{
 public:
  Field();
  bool IsFullClosed();

 private:
};

#endif // FIELD_H
