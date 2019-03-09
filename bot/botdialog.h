#ifndef BOTDIALOG_H
#define BOTDIALOG_H

#include <QDialog>

namespace Ui {
class BotDialog;
}

class BotDialog : public QDialog
{
  Q_OBJECT

public:
  explicit BotDialog(QWidget *parent = 0);
  ~BotDialog();

private:
  Ui::BotDialog *ui;
};

#endif // BOTDIALOG_H
