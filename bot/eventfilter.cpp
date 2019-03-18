#include "eventfilter.h"

#include <QMouseEvent>

EventFilter::EventFilter(QObject *parent) : QObject(parent) {

}

bool EventFilter::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    auto mouse_event = static_cast<QMouseEvent*>(event);
    qDebug("Mouse click at %d;%d", mouse_event->globalX(), mouse_event->globalY());
  }
  else if (event->type() == QEvent::MouseMove) {
    auto mouse_event = static_cast<QMouseEvent*>(event);
    qDebug("Mouse move %d;%d", mouse_event->globalX(), mouse_event->globalY());
  }
  return QObject::eventFilter(obj, event);
}

void EventFilter::Clear() {

}
