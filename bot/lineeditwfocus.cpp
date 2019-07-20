#include "lineeditwfocus.h"

LineEditWFocus::LineEditWFocus(QWidget* parent): QLineEdit(parent) {
}

void LineEditWFocus::focusOutEvent(QFocusEvent* event) {
  QLineEdit::focusOutEvent(event);
  emit LoseFocus();
}



