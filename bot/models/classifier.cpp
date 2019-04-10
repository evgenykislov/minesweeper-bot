#include "classifier.h"

using namespace std;

void Classifier::GetTestField(Field& field) {
  field.clear();
  vector<char> line = {' ', '.', '*', '0', '1', '2', '3', '4', '5', '6', '7', '8'};
  field.push_back(line);
}
