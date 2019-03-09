#include "botdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  BotDialog w;
  w.show();

  return a.exec();
}
