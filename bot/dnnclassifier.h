#ifndef DNNCLASSIFIER_H
#define DNNCLASSIFIER_H

#include <vector>

#include <QString>

class DnnClassifier
{
public:
  typedef std::vector<std::vector<char>> Field;
  DnnClassifier() {}
  virtual ~DnnClassifier() {}
  virtual bool LoadModel(const QString& file_name) = 0;
  virtual void GetStep(const Field& field, unsigned int& step_row, unsigned int& step_col, bool& sure_step) = 0;

 private:
  DnnClassifier(const DnnClassifier&) = delete;
  DnnClassifier(DnnClassifier&&) = delete;
  DnnClassifier& operator=(const DnnClassifier&) = delete;
  DnnClassifier& operator=(DnnClassifier&&) = delete;
};

#endif // DNNCLASSIFIER_H
