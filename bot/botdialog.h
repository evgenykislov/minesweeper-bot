#ifndef BOTDIALOG_H
#define BOTDIALOG_H

#include <chrono>
#include <condition_variable>
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
  void DoStartUpdate();
  void DoClickPosition(int xpos, int ypos);
  void DoGameStopped(bool no_screen, bool no_field, bool unknown_images);
  void DoGameOver();
  void DoGameComplete();

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
  void OnSettings();

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

  const float kReceiveFieldTimeout = 0.5;
  const int64_t kMouseIdleInterval = 3000; // 3 seconds of mouse idle before automatic gaming
  const std::chrono::milliseconds kMouseIdleRecheckInterval = std::chrono::milliseconds(200);
  const int64_t kWaitMouseProcessing = 100;
  const char kClosedCellSymbol = '.';
  const unsigned int kDefaultStartIndex = 0;
  const unsigned int kDefaultFinishIndex = 99999;

  bool top_left_corner_defined_;
  bool bottom_right_corner_defined_;
  unsigned int row_amount_;
  unsigned int col_amount_;
  unsigned int mines_amount_;
  std::mutex mines_amount_lock_;

  const unsigned int kImageSize = 48;
  const unsigned int kImageSizeWithBorder = 64;
  const QColor border_backcolor = QColor(192, 192, 192);
  const QColor border_linecolor = QColor(0, 0, 0);
  const int border_line_step = 4;


  Ui::BotDialog* ui_;
  QTimer pointing_timer_; // Timer for waiting user selects corners, restart point
  size_t pointing_interval_;
  BotScreen scr_;
  std::list<QImage> unknown_images_;
  std::unique_ptr<std::thread> hook_thread_;
  std::unique_ptr<std::thread> gaming_thread_;

  PointingTarget pointing_target_;
  QPoint top_left_corner_;
  QPoint bottom_right_corner_;
  QPoint restart_point_;

  ModelTetragonalNeural solver;
  unsigned int save_counter_;
  // Gaming thread synchronize
  bool finish_gaming_;
  bool resume_gaming_;
  std::condition_variable gaming_stopper_;
  std::mutex gaming_lock_;
  // Settings
  bool auto_restart_game_;
  bool save_steps_;
  QString save_folder_;
  unsigned int finish_index_;
  std::mutex save_lock_;

  void PointingCancel();
  void ShowUnknownImages();
  void UpdateUnknownImages();
  void CornersCompleted();
  void FormImage(const QImage& image, QPixmap& pixels);
  void ShowCornerImages();
  void StopGame();
  void RestartGame();
  void SaveStep(const Field& field, unsigned int step_row, unsigned int step_col, bool success);
  void StartPointing(PointingTarget target);
  void Gaming(); // Thread for gaming procedure
  void InformGameStopper(bool no_screen, bool no_field, bool unknown_images);
  void LoadSettings();
  void SaveSettings();
  void UnhookMouseByTimeout();
  bool IsMouseIdle();

 private slots:
  void PointingTick();
  void OnClickPosition(int xpos, int ypos);
  void OnGameOver();
  void OnGameComplete();
  void OnStartUpdate();
};

#endif // BOTDIALOG_H
