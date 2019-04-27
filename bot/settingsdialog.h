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

  void Set(bool auto_restart_game, bool save_steps, const QString& save_folder, unsigned int start_index, unsigned int finish_index);
  void Get(bool& auto_restart_game, bool& save_steps, QString& save_folder, unsigned int& start_index, unsigned int& finish_index);

 public slots:
  void OnAccept();

 private:
  Ui::SettingsDialog *ui_;
  bool auto_restart_game_;
  bool save_steps_;
  QString save_folder_;
  unsigned int start_index_;
  unsigned int finish_index_;
};

#endif // SETTINGSDIALOG_H
