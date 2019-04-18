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
  void OnBottomRightCorner();
  void OnRestartPoint();

 private:
  enum {
    kTimerInterval = 200,
    kCornersTimeout = 20000,
    kProgressScale = 100,
    kClickAmount = 2,
  };

  enum PointingTarget {
    kEmptyTarget,
    kTopLeftCorner,
    kBottomRightCorner,
    kRestartButton,
  };

  bool top_left_corner_defined_;
  bool bottom_right_corner_defined_;
  bool measures_defined_;

  const unsigned int kImageSize = 48;
  const unsigned int kImageSizeWithBorder = 64;
  const QColor border_backcolor = QColor(192, 192, 192);
  const QColor border_linecolor = QColor(0, 0, 0);
  const int border_line_step = 4;


  Ui::BotDialog* ui_;
  QTimer pointing_timer_; // Timer for waiting user selects corners, restart point
  QTimer game_timer_; // Timer for making game steps
  size_t pointing_interval_;
  BotScreen scr_;
  std::list<QImage> unknown_images_;
  std::unique_ptr<std::thread> hook_thread_;
  PointingTarget pointing_target_;
  QPoint top_left_corner_;
  QPoint bottom_right_corner_;
  QPoint restart_point_;

  ModelTetragonalNeural solver;
  // Step variables
  size_t step_counter_;
  FieldType step_field_;
  unsigned int step_row_;
  unsigned int step_column_;
  bool step_success_;

  void PointingCancel();
  void MakeStep(const FieldType& field);
  void ShowUnknownImages();
  void UpdateUnknownImages();
  void CornersCompleted();
  void FormImage(const QImage& image, QPixmap& pixels);
  void ShowCornerImages();
  void StopGame();
  void RestartGame();
  void SaveStep();
  void StartPointing();

 private slots:
  void PointingTick();
  void GameTick();
  void OnClickPosition(int xpos, int ypos);
};

#endif // BOTDIALOG_H
