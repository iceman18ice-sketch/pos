#include "database/DatabaseManager.h"
#include "views/LoginWindow.h"
#include "views/MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Set RTL layout for Arabic
  app.setLayoutDirection(Qt::RightToLeft);

  // Load stylesheet
  QFile styleFile(":/theme.qss");
  if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
    app.setStyleSheet(styleFile.readAll());
    styleFile.close();
  }

  // Set application info
  app.setApplicationName("POS System");
  app.setApplicationVersion("1.0.0");
  app.setOrganizationName("POS");

  // Initialize database
  if (!DatabaseManager::instance().initialize()) {
    qCritical() << "Failed to initialize database!";
    return -1;
  }

  int result = 0;
  do {
    // Show login window
    LoginWindow loginWin;
    loginWin.show();

    MainWindow *mainWin = nullptr;
    QObject::connect(&loginWin, &LoginWindow::loginSuccessful, [&]() {
      loginWin.hide();
      mainWin = new MainWindow;
      mainWin->show();
    });

    result = app.exec();
    delete mainWin;
  } while (result == 1000); // Restart on logout

  return result;
}
