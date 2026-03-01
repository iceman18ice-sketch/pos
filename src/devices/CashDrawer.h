#pragma once
#include <QObject>
#include <QString>
#include <QPrinterInfo>

// ============================================================
//  Cash Drawer Control via ESC/POS Printer
//  يرسل أمر فتح الدرج عبر طابعة POS (RJ11 كيبل)
//  يدعم: Epson, Star, Xprinter, Bixolon, وغيرها
// ============================================================

class CashDrawer : public QObject {
    Q_OBJECT
public:
    explicit CashDrawer(QObject *parent = nullptr);

    // فتح الدرج عبر طابعة محددة
    bool openDrawer(const QString &printerName);

    // فتح الدرج عبر الطابعة الافتراضية
    bool openDrawerDefault();

    // قائمة الطابعات المتاحة
    static QStringList availablePrinters();

    // آخر خطأ
    QString lastError() const { return m_lastError; }

    // إعدادات pin connector
    enum DrawerPin { Pin2 = 0, Pin5 = 1 };
    void setDrawerPin(DrawerPin pin) { m_pin = pin; }

    // إعدادات التوقيت
    void setPulseTiming(int onTimeMs, int offTimeMs);

signals:
    void drawerOpened();
    void errorOccurred(const QString &error);

private:
    bool sendESCPOSCommand(const QString &printerName);
    QString m_lastError;
    DrawerPin m_pin = Pin2;
    int m_onTime = 50;   // مللي ثانية
    int m_offTime = 500;  // مللي ثانية
};
