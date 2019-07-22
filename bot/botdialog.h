#ifndef BOTDIALOG_H
#define BOTDIALOG_H

#include <chrono>
#include <condition_variable>
#include <thread>

#include <QDialog>
#include <QSettings>
#include <QTimer>

#include "screen.h"
#include "models/tetragonal_neural.h"
#include "models/bruteforce.h"

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
  void DoGameStoppedByUser();

 public slots:
  void OnTopLeftCorner();
  void OnRun();
  void LoseFocus();
  void OnLeftField();
  void OnRightField();
  void OnTopField();
  void OnBottomField();
  void OnBottomRightCorner();
  void OnRestartPoint();
  void OnSettings();
  void OnStop();
  void OnLevelChanged(int button_id);

 private:
  enum {
    kPointingTimerInterval = 200,
    kPointingTimeout = 20000,
    kClickAmount = 2,
  };

  enum PointingTarget {
    kEmptyTarget,
    kTopLeftCorner,
    kBottomRightCorner,
    kRestartButton,
  };

  enum GameLevelID {
    kBeginnerLevel = 1,
    kIntermediateLevel = 2,
    kExpertLevel = 3,
    kCustomLevel = 4,
  };

  enum StepTypeForSave {
    kTrainSteps,
    kUnexpectedErrorSteps,
    kWrongMineSteps,
    kProbabilitySteps,
  };

  struct StepInfo {
    Field field_;
    unsigned int row_;
    unsigned int col_;
    Classifier::StepAction step_;
    bool success_;
    size_t step_index_;
  };

  const float kReceiveFieldTimeout = 0.5;
  const int64_t kMouseIdleInterval = 3000; // 3 seconds of mouse idle before automatic gaming
  const std::chrono::milliseconds kMouseIdleRecheckInterval = std::chrono::milliseconds(200);
  const int64_t kWaitMouseProcessing = 100;
  const char kClosedCellSymbol = '.';
  const char kMineMarkSymbol = '*';
  const char kWrongMineSymbol = '-';
  const unsigned int kDefaultStartIndex = 0;
  const unsigned int kDefaultFinishIndex = 99999;
  const int kUpdateTimerInterval = 800;
  const std::string kUnexpectedErrorFolder = u8"unexpected";
  const std::string kWrongMineFolder = u8"wrongmine";
  const std::string kProbabilityFolder = u8"probability";
  const size_t kWrongMineTailSize = 20;
  const QPoint kUndefinedPoint = { INT_MIN, INT_MIN };

  bool top_left_corner_defined_;
  bool bottom_right_corner_defined_;
  unsigned int custom_row_amount_;
  unsigned int custom_col_amount_;
  unsigned int custom_mines_amount_;
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
  QTimer update_timer_; // Timer for dialog refresh
  size_t pointing_interval_;
  BotScreen scr_;
  std::list<QImage> unknown_images_;
  std::unique_ptr<std::thread> hook_thread_;
  std::unique_ptr<std::thread> gaming_thread_;

  PointingTarget pointing_target_;
  QRect field_frame_;
  QPoint restart_point_;

  // ModelTetragonalNeural solver;
  BruteForce solver;
  unsigned int save_counter_;
  size_t step_index_;
  // Gaming thread synchronize
  bool finish_gaming_;
  bool resume_gaming_; // Flag to resume game (flag is resetted in game resume)
  bool stop_gaming_; // Flag to finish game (flag is resetted in game stop)
  std::condition_variable gaming_stopper_;
  std::mutex gaming_lock_;
  // Settings
  bool auto_restart_game_;
  bool save_steps_;
  QString save_folder_;
  unsigned int finish_index_;
  QSettings settings_;
  std::mutex save_lock_;
  GameLevelID level_;
  bool save_unexpected_error_steps_;
  bool save_steps_before_wrong_mine_;
  bool save_probability_steps_;

  void PointingCancel();
  void ShowUnknownImages();
  void UpdateUnknownImages();
  void UpdateCorners();
  void FormImage(const QImage& image, QPixmap& pixels);
  void ShowCornerImages();
  void StopGame();
  void RestartGame();
  void SaveStep(StepTypeForSave step_type, const StepInfo& info);
  void SaveWrongMine(unsigned int row, unsigned int col);
  void StartPointing(PointingTarget target);
  void Gaming(); // Thread for gaming procedure
  void InformGameStopper(bool no_screen, bool no_field, bool unknown_images);
  void LoadSettings();
  void SaveSettings();
  void UnhookMouseByTimeout();
  bool IsMouseIdle();
  void UpdateGamingByLevel(); // Update gaming parameters (rows, columns, mines) by level information
  void SetLevelButtonsStates();
  bool FieldHasWrongMine(const Field& field, unsigned int& row, unsigned int& col);

 private slots:
  void PointingTick();
  void UpdateTick();
  void OnClickPosition(int xpos, int ypos);
  void OnGameStopped(bool no_screen, bool no_field, bool unknown_images);
  void OnGameOver();
  void OnGameComplete();
  void OnStartUpdate();
  void OnGameStoppedByUser();
};

#endif // BOTDIALOG_H
