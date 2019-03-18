#ifndef BOTDIALOG_H
#define BOTDIALOG_H

#include <QDialog>
#include <QTimer>

#include "eventfilter.h"
#include "screen.h"

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
  void OnRun();
  void OnImageClick();

 private:
  enum {
    kTimerInterval = 200,
    kCornersTimeout = 20000,
    kProgressScale = 100,
  };

  enum SettingState {
    kIdle,
    kCornersSelection,
    kSizeModification,
    kRun,
    kWaitForImageRecognition,
  } state_;

  Ui::BotDialog* ui_;
  EventFilter filter_;
  QTimer timer_200ms_;
  size_t corners_interval_;
  Screen scr_;
  std::list<QImage> unknown_images_;

  void CornersCancel();
  void MakeStep(const FieldType& field);
  void ShowUnknownImages();
  void UpdateUnknownImages();

 private slots:
  void TimerTick();
};

#endif // BOTDIALOG_H
