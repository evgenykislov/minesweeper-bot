#include "botdialog.h"
#include "ui_botdialog.h"

BotDialog::BotDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::BotDialog)
{
  ui->setupUi(this);
}

BotDialog::~BotDialog()
{
  delete ui;
}
