#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QVector>

// ============================================================
//  ZKTeco Attendance Device - TCP Protocol (Port 4370)
//  يدعم: ZKTeco K40/K50, iClock, UA760, U580, FingerTec R2/R3
// ============================================================

// سجل حضور واحد من الجهاز
struct ZKAttendanceLog {
    int userId;
    QString userName;
    QDateTime timestamp;
    int verifyType;   // 0=Password, 1=Fingerprint, 2=Card
    int inOutState;   // 0=Check-In, 1=Check-Out
};

// معلومات الجهاز
struct ZKDeviceInfo {
    QString serialNumber;
    QString deviceName;
    QString platform;
    QString firmwareVersion;
    int userCount;
    int logCount;
    int fingerprintCount;
};

class ZKDevice : public QObject {
    Q_OBJECT
public:
    explicit ZKDevice(QObject *parent = nullptr);
    ~ZKDevice();

    // === Connection ===
    bool connectDevice(const QString &ip, quint16 port = 4370);
    void disconnect();
    bool isConnected() const;

    // === Device Commands ===
    bool getDeviceInfo(ZKDeviceInfo &info);
    QVector<ZKAttendanceLog> getAttendanceLogs();
    bool clearAttendanceLogs();

    // === Getters ===
    QString lastError() const { return m_lastError; }
    QString deviceIP() const { return m_ip; }
    quint16 devicePort() const { return m_port; }

signals:
    void connected();
    void disconnected();
    void logsReceived(const QVector<ZKAttendanceLog> &logs);
    void errorOccurred(const QString &error);
    void statusChanged(const QString &status);

private:
    // === Protocol ===
    static const quint16 CMD_CONNECT      = 1000;
    static const quint16 CMD_EXIT         = 1001;
    static const quint16 CMD_ENABLEDEVICE = 1002;
    static const quint16 CMD_DISABLEDEVICE= 1003;
    static const quint16 CMD_ACK_OK       = 2000;
    static const quint16 CMD_ACK_ERROR    = 2001;
    static const quint16 CMD_ACK_DATA     = 2002;
    static const quint16 CMD_PREPARE_DATA = 1500;
    static const quint16 CMD_DATA         = 1501;
    static const quint16 CMD_FREE_DATA    = 1502;
    static const quint16 CMD_ATTLOG_RRQ   = 13;
    static const quint16 CMD_DEVICE       = 11;
    static const quint16 CMD_CLEAR_ATTLOG = 15;

    QByteArray createPacket(quint16 command, quint16 sessionId,
                            quint16 replyId, const QByteArray &data = {});
    bool sendCommand(quint16 command, const QByteArray &data = {});
    QByteArray receiveResponse(int timeoutMs = 5000);
    QByteArray receiveLargeData(int timeoutMs = 10000);
    quint16 calcChecksum(const QByteArray &data);

    QTcpSocket *m_socket;
    QString m_ip;
    quint16 m_port;
    quint16 m_sessionId;
    quint16 m_replyId;
    QString m_lastError;
    bool m_connected;
};
