#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include "devices/ZKDevice.h"
#include "devices/CashDrawer.h"

class DeviceSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit DeviceSettingsWidget(QWidget *parent = nullptr);
    ~DeviceSettingsWidget();

private slots:
    void onConnect();
    void onDisconnect();
    void onFetchLogs();
    void onSyncToDatabase();
    void onTestDrawer();
    void onDeviceStatusChanged(const QString &status);
    void onDeviceError(const QString &error);
    void onLogsReceived(const QVector<ZKAttendanceLog> &logs);

private:
    void setupFingerprintSection(QVBoxLayout *layout);
    void setupCashDrawerSection(QVBoxLayout *layout);

    // Fingerprint device
    ZKDevice *m_zkDevice;
    QLineEdit *m_ipEdit;
    QSpinBox *m_portSpin;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QPushButton *m_fetchLogsBtn;
    QPushButton *m_syncBtn;
    QLabel *m_statusLabel;
    QTableWidget *m_logsTable;
    QTextEdit *m_logOutput;
    QVector<ZKAttendanceLog> m_currentLogs;

    // Cash drawer
    CashDrawer *m_cashDrawer;
    QComboBox *m_printerCombo;
    QPushButton *m_testDrawerBtn;
    QPushButton *m_refreshPrintersBtn;
    QLabel *m_drawerStatusLabel;
};
