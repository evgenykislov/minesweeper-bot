#include "eventfilter.h"

#include <algorithm>

#include <QMouseEvent>

using namespace std;

EventFilter::EventFilter(QObject *parent) : QObject(parent) {
  Clear();
}

bool EventFilter::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    auto mouse_event = static_cast<QMouseEvent*>(event);
    switch (click_index_) {
      case 0:
        right_ = left_ = mouse_event->globalX();
        top_ = bottom_ = mouse_event->globalY();
        break;
      case 1:
        right_ = mouse_event->globalX();
        bottom_ = mouse_event->globalY();
        if (right_ < left_) swap(right_, left_);
        if (bottom_ < top_) swap(bottom_, top_);
        break;
    }
    ++click_index_;
    if (click_index_ == kMinimalClickAmount) {
      emit TwoClicks();
    }
  }
  return QObject::eventFilter(obj, event);
}

void EventFilter::Clear() {
  top_ = 0;
  bottom_ = 0;
  left_= 0;
  right_ = 0;
  click_index_ = 0;
}

bool EventFilter::GetRect(QRect& rect) {
  if (click_index_ < kMinimalClickAmount) {
    return false;
  }
  rect.setLeft(left_);
  rect.setRight(right_);
  rect.setTop(top_);
  rect.setBottom(bottom_);
  return true;
}
