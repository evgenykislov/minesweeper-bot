#ifndef BOTDIALOG_H
#define BOTDIALOG_H

#include <thread>

#include <QDialog>
#include <QTimer>

#include "screen.h"
#include "models/tetragonal_neural.h"

namespace Ui {
class BotDialog;
}

class BotDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit BotDialog(QWidget *parent = 0);
  ~BotDialog();

 signals:
  void DoClickPosition(int xpos, int ypos);

 public slots:
  void OnCornersBtn();
  void OnRun();
  void LoseFocus();

 private:
  enum {
    kTimerInterval = 200,
    kCornersTimeout = 20000,
    kProgressScale = 100,
    kClickAmount = 2,
  };

  enum SettingState {
    kIdle,
    kCornersSelection,
    kSizeModification,
    kRun,
    kWaitForImageRecognition,
  } state_;

  const unsigned int kImageSize = 48;
  const unsigned int kImageSizeWithBorder = 64;
  const QColor border_backcolor = QColor(192, 192, 192);
  const QColor border_linecolor = QColor(0, 0, 0);
  const int border_line_step = 4;


  Ui::BotDialog* ui_;
  QTimer timer_200ms_;
  size_t corners_interval_;
  BotScreen scr_;
  std::list<QImage> unknown_images_;
  std::unique_ptr<std::thread> hook_thread_;
  QPoint clicks_[kClickAmount];
  unsigned int click_index_;
  ModelTetragonalNeural solver;

  void CornersCancel();
  void MakeStep(const FieldType& field);
  void ShowUnknownImages();
  void UpdateUnknownImages();
  void CornersCompleted(QRect frame);
  void FormImage(const QImage& image, QPixmap& pixels);
  void ShowCornerImages();

 private slots:
  void TimerTick();
  void OnClickPosition(int xpos, int ypos);
};

#endif // BOTDIALOG_H
