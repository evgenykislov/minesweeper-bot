#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit SettingsDialog(QWidget *parent = nullptr);
  ~SettingsDialog();

  struct Parameters {
    // Gaming
    bool auto_restart_game_;
    // Training steps
    bool save_all_steps_;
    QString steps_save_folder_;
    // Error Tracking
    bool save_unexpected_error_steps_;
    bool save_steps_before_wrong_mine_;
    bool save_probability_steps_;
    bool save_fully_closed_steps_;
  };

  void Set(const Parameters& params);
  void Get(Parameters& params);

 public slots:
  void OnAccept();
  void OnProbabilityChanged(int);

 private:
  Ui::SettingsDialog *ui_;
  Parameters params_;

  void UpdateChecks();
};

#endif // SETTINGSDIALOG_H
