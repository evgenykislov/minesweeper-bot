#ifndef EVENTFILTER_H
#define EVENTFILTER_H

#include <QObject>
#include <QEvent>

class EventFilter : public QObject
{
  Q_OBJECT

public:
  explicit EventFilter(QObject *parent = nullptr);
  void Clear();
  bool GetRect(QRect& rect);

 signals:
  void TwoClicks();

 protected:
  virtual bool eventFilter(QObject *obj, QEvent *event);

 private:
  enum {
    kMinimalClickAmount = 2,
  };
  int top_;
  int bottom_;
  int left_;
  int right_;
  size_t click_index_;
};

#endif // EVENTFILTER_H
