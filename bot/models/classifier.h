#ifndef DNNCLASSIFIER_H
#define DNNCLASSIFIER_H

#include <cstdint>
#include <vector>

class Classifier
{
 public:
  typedef std::vector<std::vector<char>> Field;
  struct ProbeValues {
    float clear_probe;
    float mine_probe;
    float unknown_probe;
  };
  typedef std::vector<ProbeValues> TestResponse;

  Classifier() {}
  virtual ~Classifier() {}
  virtual bool LoadModel(std::vector<uint8_t>&& data) = 0;
  virtual void GetStep(const Field& field, unsigned int& step_row, unsigned int& step_col, bool& sure_step) = 0;
  virtual void GetTestResponse(TestResponse& response) = 0;

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
