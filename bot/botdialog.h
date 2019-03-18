#ifndef BOTDIALOG_H
#define BOTDIALOG_H

#include <QDialog>
#include <QTimer>

#include "eventfilter.h"

namespace Ui {
class BotDialog;
}

class BotDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit BotDialog(QWidget *parent = 0);
  ~BotDialog();

 public slots:
  void OnCornersBtn();
  void CornersCompleted();

 private:
  enum {
    kTimerInterval = 200,
    kCornersTimeout = 7000,
    kProgressScale = 100,
  };

  enum SettingState {
    kIdle,
    kCornersSelection,
  } state_;

  Ui::BotDialog* ui_;
  EventFilter filter_;
  QTimer timer_200ms_;
  size_t corners_interval_;

  void CornersCancelled();

 private slots:
  void TimerTick();
};

#endif // BOTDIALOG_H
