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

  bool auto_restart_game_;
  bool save_steps_;
  QString save_folder_;
  unsigned int start_index_;

 public slots:
  void OnAccept();

 private:
  Ui::SettingsDialog *ui_;



};

#endif // SETTINGSDIALOG_H
