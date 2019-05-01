#include "botdialog.h"
#include <QApplication>

#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char *argv[])
{
  el::Configurations default_log_conf;
  default_log_conf.setToDefault();
  default_log_conf.setGlobally(el::ConfigurationType::ToFile, "true");
  default_log_conf.setGlobally(el::ConfigurationType::Filename, "log.txt");
  el::Loggers::reconfigureAllLoggers(default_log_conf);

  QApplication a(argc, argv);
  BotDialog w;
  w.show();

  return a.exec();
}
