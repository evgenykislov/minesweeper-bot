#ifndef DNNCLASSIFIER_H
#define DNNCLASSIFIER_H

#include <cstdint>
#include <vector>

class Classifier
{
 public:
  typedef std::vector<std::vector<char>> Field;
  enum StepAction {
    kOpenWithSure = 0,
    kOpenWithProbability = 1,
    kMarkAsMine = 2,
  };

  Classifier() {}
  virtual ~Classifier() {}
  virtual bool LoadModel(std::vector<uint8_t>&& data) = 0;
  virtual void GetStep(const Field& field, unsigned int mines_amount, unsigned int& step_row, unsigned int& step_col, StepAction& step) = 0;

 protected:
  const char kHideCell = '.';
  void GetTestField(Field& field);

 private:
  Classifier(const Classifier&) = delete;
  Classifier(Classifier&&) = delete;
  Classifier& operator=(const Classifier&) = delete;
  Classifier& operator=(Classifier&&) = delete;
};

#endif // DNNCLASSIFIER_H
