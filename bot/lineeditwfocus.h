#ifndef LINEEDITWFOCUS_H
#define LINEEDITWFOCUS_H

#include <QLineEdit>

class LineEditWFocus : public QLineEdit
{
  Q_OBJECT
 public:
  LineEditWFocus(QWidget* parent = nullptr);

 signals:
  void LoseFocus();

 protected:
  virtual void focusOutEvent(QFocusEvent* event) override;
};

#endif // LINEEDITWFOCUS_H
