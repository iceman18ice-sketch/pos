#include "database/DatabaseManager.h"
#include "views/LoginWindow.h"
#include "views/MainWindow.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QSplashScreen>
#include <QPainter>
#include <QTimer>
#include <QScreen>


// Create a premium splash screen programmatically
static QPixmap createSplashPixmap() {
    QPixmap pixmap(600, 380);
    pixmap.fill(QColor("#0F1117"));

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);

    // Background gradient
    QLinearGradient bg(0, 0, 600, 380);
    bg.setColorAt(0, QColor("#0F1117"));
    bg.setColorAt(0.5, QColor("#151922"));
    bg.setColorAt(1, QColor("#1A1D23"));
    p.fillRect(pixmap.rect(), bg);

    // Decorative circle
    QRadialGradient circle(300, 150, 200);
    circle.setColorAt(0, QColor(108, 99, 255, 40));
    circle.setColorAt(1, QColor(108, 99, 255, 0));
    p.setBrush(circle);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPoint(300, 150), 200, 200);

    // Icon
    QFont iconFont("Segoe UI Emoji", 52);
    p.setFont(iconFont);
    p.setPen(QColor("#E8EAED"));
    p.drawText(QRect(0, 40, 600, 80), Qt::AlignCenter, "🏪");

    // Title
    QFont titleFont("Cairo", 28, QFont::Bold);
    if (!QFontDatabase::families().contains("Cairo"))
        titleFont = QFont("Segoe UI", 28, QFont::Bold);
    p.setFont(titleFont);

    // Title gradient text (simulate with solid color)
    p.setPen(QColor("#6C63FF"));
    p.drawText(QRect(0, 130, 600, 50), Qt::AlignCenter, "نظام المحاسبة");

    // Subtitle
    QFont subFont("Cairo", 14);
    if (!QFontDatabase::families().contains("Cairo"))
        subFont = QFont("Segoe UI", 14);
    p.setFont(subFont);
    p.setPen(QColor("#6B7280"));
    p.drawText(QRect(0, 185, 600, 30), Qt::AlignCenter, "POS System v3.0.0 — Professional Edition");

    // Loading bar background
    p.setBrush(QColor("#252A35"));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(150, 260, 300, 6, 3, 3);

    // Loading bar fill
    QLinearGradient barGrad(150, 0, 450, 0);
    barGrad.setColorAt(0, QColor("#6C63FF"));
    barGrad.setColorAt(1, QColor("#10B981"));
    p.setBrush(barGrad);
    p.drawRoundedRect(150, 260, 220, 6, 3, 3);

    // Loading text
    QFont loadFont("Cairo", 11);
    if (!QFontDatabase::families().contains("Cairo"))
        loadFont = QFont("Segoe UI", 11);
    p.setFont(loadFont);
    p.setPen(QColor("#4B5563"));
    p.drawText(QRect(0, 280, 600, 30), Qt::AlignCenter, "جاري تحميل النظام...");

    // Version / Copyright
    QFont verFont("Segoe UI", 9);
    p.setFont(verFont);
    p.setPen(QColor("#374151"));
    p.drawText(QRect(0, 345, 600, 20), Qt::AlignCenter, "© 2026 POS System — All Rights Reserved");

    // Border
    p.setPen(QPen(QColor("#2D3139"), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(0, 0, 599, 379, 12, 12);

    p.end();
    return pixmap;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set RTL layout for Arabic
    app.setLayoutDirection(Qt::RightToLeft);

    // Set application info
    app.setApplicationName("نظام المحاسبة - POS System");
    app.setApplicationVersion("3.0.0");
    app.setOrganizationName("POS System");

    // Load Arabic font (Cairo) if available
    QStringList fontPaths = {
        ":/fonts/Cairo-Regular.ttf",
        ":/fonts/Cairo-Bold.ttf",
        QCoreApplication::applicationDirPath() + "/fonts/Cairo-Regular.ttf",
    };
    for (const auto &fp : fontPaths) {
        if (QFile::exists(fp))
            QFontDatabase::addApplicationFont(fp);
    }

    // Load stylesheet
    QFile styleFile(":/theme.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    // Show splash screen
    QPixmap splashPix = createSplashPixmap();
    QSplashScreen splash(splashPix, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    splash.show();
    app.processEvents();

    splash.showMessage("  جاري تحميل قاعدة البيانات...",
                       Qt::AlignBottom | Qt::AlignHCenter, QColor("#6B7280"));
    app.processEvents();

    // Initialize database
    if (!DatabaseManager::instance().initialize()) {
        qCritical() << "Failed to initialize database!";
        return -1;
    }

    splash.showMessage("  جاري تحضير الواجهة...",
                       Qt::AlignBottom | Qt::AlignHCenter, QColor("#6B7280"));
    app.processEvents();

    int result = 0;
    do {
        // Show login window
        LoginWindow loginWin;

        // Close splash after a short delay
        QTimer::singleShot(1500, &splash, &QSplashScreen::close);

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
