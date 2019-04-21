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
  explicit SettingsDialog(QWidget *parent = 0);
  ~SettingsDialog();

  void Set(bool auto_restart_game_, bool save_steps_, const QString& save_folder_, unsigned int start_index_);
  void Get(bool& auto_restart_game_, bool& save_steps_, QString& save_folder_, unsigned int& start_index_);

 public slots:
  void OnAccept();

 private:
  Ui::SettingsDialog *ui_;
  bool auto_restart_game_;
  bool save_steps_;
  QString save_folder_;
  unsigned int start_index_;
};

#endif // SETTINGSDIALOG_H
