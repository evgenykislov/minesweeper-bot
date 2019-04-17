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
  void OnLeftField();
  void OnRightField();
  void OnTopField();
  void OnBottomField();

 private:
  enum {
    kTimerInterval = 200,
    kCornersTimeout = 20000,
    kProgressScale = 100,
    kClickAmount = 2,
  };

  bool corners_defined_;
  bool measures_defined_;

  const unsigned int kImageSize = 48;
  const unsigned int kImageSizeWithBorder = 64;
  const QColor border_backcolor = QColor(192, 192, 192);
  const QColor border_linecolor = QColor(0, 0, 0);
  const int border_line_step = 4;


  Ui::BotDialog* ui_;
  QTimer corners_timer_; // Timer for waiting user selects corners
  QTimer game_timer_; // Timer for making game steps
  size_t corners_interval_;
  BotScreen scr_;
  std::list<QImage> unknown_images_;
  std::unique_ptr<std::thread> hook_thread_;
  QPoint clicks_[kClickAmount];
  unsigned int click_index_;
  ModelTetragonalNeural solver;
  // Step variables
  size_t step_counter_;
  FieldType step_field_;
  unsigned int step_row_;
  unsigned int step_column_;
  bool step_success_;

  void CornersCancel();
  void MakeStep(const FieldType& field);
  void ShowUnknownImages();
  void UpdateUnknownImages();
  void CornersCompleted(QRect frame);
  void FormImage(const QImage& image, QPixmap& pixels);
  void ShowCornerImages();
  void StopGame();
  void SaveStep();

 private slots:
  void CornersTick();
  void GameTick();
  void OnClickPosition(int xpos, int ypos);
};

#endif // BOTDIALOG_H
