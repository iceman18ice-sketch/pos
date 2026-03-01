#include "ZKDevice.h"
#include <QDataStream>
#include <QThread>
#include <QtEndian>

// ============================================================
//  ZKTeco TCP Protocol Implementation
//  بروتوكول الاتصال: حزم 8 بايت header + بيانات
//  Header: [command(2)] [checksum(2)] [sessionId(2)] [replyId(2)]
// ============================================================

ZKDevice::ZKDevice(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this)),
      m_port(4370), m_sessionId(0), m_replyId(0), m_connected(false) {}

ZKDevice::~ZKDevice() {
    if (m_connected)
        disconnect();
}

// ============ Packet Creation ============
QByteArray ZKDevice::createPacket(quint16 command, quint16 sessionId,
                                   quint16 replyId, const QByteArray &data) {
    QByteArray packet;
    int dataLen = 8 + data.size(); // header(8) + data

    // Start tag
    packet.append(static_cast<char>(0x50));
    packet.append(static_cast<char>(0x50));
    packet.append(static_cast<char>(0x82));
    packet.append(static_cast<char>(0x7D));

    // Data length (little-endian)
    packet.append(static_cast<char>(dataLen & 0xFF));
    packet.append(static_cast<char>((dataLen >> 8) & 0xFF));

    // Command (little-endian)
    packet.append(static_cast<char>(command & 0xFF));
    packet.append(static_cast<char>((command >> 8) & 0xFF));

    // Checksum placeholder
    packet.append(static_cast<char>(0x00));
    packet.append(static_cast<char>(0x00));

    // Session ID (little-endian)
    packet.append(static_cast<char>(sessionId & 0xFF));
    packet.append(static_cast<char>((sessionId >> 8) & 0xFF));

    // Reply ID (little-endian)
    packet.append(static_cast<char>(replyId & 0xFF));
    packet.append(static_cast<char>((replyId >> 8) & 0xFF));

    // Data payload
    packet.append(data);

    // Calculate and fill checksum (bytes 8-9 of the full packet)
    QByteArray checksumData = packet.mid(6); // from command onwards
    quint16 chk = calcChecksum(checksumData);
    packet[8] = static_cast<char>(chk & 0xFF);
    packet[9] = static_cast<char>((chk >> 8) & 0xFF);

    return packet;
}

quint16 ZKDevice::calcChecksum(const QByteArray &data) {
    quint32 chksum = 0;
    int i = 0;
    while (i < data.size() - 1) {
        quint16 val = static_cast<quint8>(data[i]) |
                      (static_cast<quint8>(data[i + 1]) << 8);
        chksum += val;
        i += 2;
    }
    if (i < data.size()) {
        chksum += static_cast<quint8>(data[i]);
    }
    while (chksum > 0xFFFF) {
        chksum = (chksum & 0xFFFF) + (chksum >> 16);
    }
    return static_cast<quint16>(~chksum & 0xFFFF);
}

// ============ Connection ============
bool ZKDevice::connectDevice(const QString &ip, quint16 port) {
    m_ip = ip;
    m_port = port;
    m_replyId = 0;
    m_sessionId = 0;

    emit statusChanged("جاري الاتصال بـ " + ip + ":" + QString::number(port) + "...");

    m_socket->connectToHost(ip, port);
    if (!m_socket->waitForConnected(5000)) {
        m_lastError = "فشل الاتصال: " + m_socket->errorString();
        emit errorOccurred(m_lastError);
        return false;
    }

    // Send connect command
    QByteArray packet = createPacket(CMD_CONNECT, m_sessionId, m_replyId);
    m_socket->write(packet);
    m_socket->flush();

    QByteArray response = receiveResponse(5000);
    if (response.isEmpty()) {
        m_lastError = "لم يتم استلام رد من الجهاز";
        emit errorOccurred(m_lastError);
        m_socket->disconnectFromHost();
        return false;
    }

    // Parse response - check for ACK_OK
    if (response.size() >= 14) {
        quint16 respCmd = static_cast<quint8>(response[6]) |
                          (static_cast<quint8>(response[7]) << 8);
        m_sessionId = static_cast<quint8>(response[10]) |
                      (static_cast<quint8>(response[11]) << 8);
        m_replyId = 1;

        if (respCmd == CMD_ACK_OK) {
            m_connected = true;
            emit statusChanged("✅ متصل بالجهاز بنجاح");
            emit connected();
            return true;
        }
    }

    m_lastError = "رد غير صالح من الجهاز";
    emit errorOccurred(m_lastError);
    m_socket->disconnectFromHost();
    return false;
}

void ZKDevice::disconnect() {
    if (m_connected) {
        sendCommand(CMD_EXIT);
        m_socket->disconnectFromHost();
        m_connected = false;
        m_sessionId = 0;
        m_replyId = 0;
        emit statusChanged("❌ غير متصل");
        emit disconnected();
    }
}

bool ZKDevice::isConnected() const {
    return m_connected && m_socket->state() == QAbstractSocket::ConnectedState;
}

// ============ Send / Receive ============
bool ZKDevice::sendCommand(quint16 command, const QByteArray &data) {
    if (!isConnected() && command != CMD_CONNECT) return false;

    QByteArray packet = createPacket(command, m_sessionId, m_replyId, data);
    m_socket->write(packet);
    m_socket->flush();
    m_replyId++;
    return true;
}

QByteArray ZKDevice::receiveResponse(int timeoutMs) {
    if (m_socket->waitForReadyRead(timeoutMs)) {
        return m_socket->readAll();
    }
    return {};
}

QByteArray ZKDevice::receiveLargeData(int timeoutMs) {
    QByteArray allData;
    while (m_socket->waitForReadyRead(timeoutMs)) {
        allData.append(m_socket->readAll());
        // Check if we've received all data
        if (m_socket->bytesAvailable() == 0) {
            QThread::msleep(100);
            if (m_socket->bytesAvailable() == 0) break;
        }
    }
    return allData;
}

// ============ Device Info ============
bool ZKDevice::getDeviceInfo(ZKDeviceInfo &info) {
    if (!isConnected()) {
        m_lastError = "الجهاز غير متصل";
        return false;
    }

    emit statusChanged("جاري جلب معلومات الجهاز...");

    // Request device info
    sendCommand(CMD_DEVICE, QByteArray("~SerialNumber\0", 14));
    QByteArray resp = receiveResponse(3000);

    info.serialNumber = "غير معروف";
    info.deviceName = "ZKTeco Device";
    info.platform = "ZEM";
    info.firmwareVersion = "غير معروف";
    info.userCount = 0;
    info.logCount = 0;
    info.fingerprintCount = 0;

    if (resp.size() > 14) {
        // Parse the key=value response
        QByteArray payload = resp.mid(14);
        QString dataStr = QString::fromUtf8(payload).trimmed();
        if (dataStr.contains("=")) {
            info.serialNumber = dataStr.section("=", 1).trimmed();
        }
    }

    emit statusChanged("✅ تم جلب معلومات الجهاز");
    return true;
}

// ============ Attendance Logs ============
QVector<ZKAttendanceLog> ZKDevice::getAttendanceLogs() {
    QVector<ZKAttendanceLog> logs;

    if (!isConnected()) {
        m_lastError = "الجهاز غير متصل";
        emit errorOccurred(m_lastError);
        return logs;
    }

    emit statusChanged("جاري سحب سجلات الحضور...");

    // Disable device during read
    sendCommand(CMD_DISABLEDEVICE);
    receiveResponse(2000);
    m_replyId++;

    // Request attendance logs
    sendCommand(CMD_ATTLOG_RRQ);
    QByteArray response = receiveResponse(5000);

    if (response.isEmpty()) {
        sendCommand(CMD_ENABLEDEVICE);
        receiveResponse(2000);
        m_lastError = "لم يتم استلام بيانات الحضور";
        emit errorOccurred(m_lastError);
        return logs;
    }

    // Check if PREPARE_DATA response (large data)
    QByteArray allData;
    if (response.size() >= 14) {
        quint16 respCmd = static_cast<quint8>(response[6]) |
                          (static_cast<quint8>(response[7]) << 8);

        if (respCmd == CMD_PREPARE_DATA) {
            // Large data - need to receive in chunks
            quint32 totalSize = 0;
            if (response.size() >= 18) {
                totalSize = static_cast<quint8>(response[14]) |
                            (static_cast<quint8>(response[15]) << 8) |
                            (static_cast<quint8>(response[16]) << 16) |
                            (static_cast<quint8>(response[17]) << 24);
            }

            m_replyId++;
            allData = receiveLargeData(15000);

            // Send free data command
            sendCommand(CMD_FREE_DATA);
            receiveResponse(2000);
            m_replyId++;

        } else if (respCmd == CMD_ACK_DATA) {
            allData = response.mid(14);
        }
    }

    // Parse attendance records
    // Each record is typically 40 bytes:
    // userId(24 bytes string) + timestamp(4 bytes) + state(1 byte) + verify(1 byte) + ...
    // Or newline-separated text format depending on firmware
    if (!allData.isEmpty()) {
        // Try text-based parsing first (many devices use this)
        QString dataStr = QString::fromUtf8(allData);
        QStringList lines = dataStr.split('\n', Qt::SkipEmptyParts);

        for (const QString &line : lines) {
            // Format: "userId\ttimestamp\tstate\tverify"
            // or: "userId\t2024-01-15 08:30:00\t0\t1"
            QStringList parts = line.split('\t');
            if (parts.size() >= 2) {
                ZKAttendanceLog log;
                log.userId = parts[0].trimmed().toInt();
                log.timestamp = QDateTime::fromString(parts[1].trimmed(),
                                                       "yyyy-MM-dd hh:mm:ss");
                if (!log.timestamp.isValid()) {
                    log.timestamp = QDateTime::fromString(parts[1].trimmed(),
                                                           "yyyy-M-d hh:mm:ss");
                }
                log.inOutState = (parts.size() > 2) ? parts[2].trimmed().toInt() : 0;
                log.verifyType = (parts.size() > 3) ? parts[3].trimmed().toInt() : 1;
                log.userName = "";

                if (log.timestamp.isValid()) {
                    logs.append(log);
                }
            }
        }

        // If text parsing didn't work, try binary parsing
        if (logs.isEmpty() && allData.size() >= 40) {
            int recordSize = 40;
            int offset = 0;

            // Skip initial packet headers if present
            if (allData.size() > 14 && static_cast<quint8>(allData[0]) == 0x50) {
                offset = 14; // skip header
            }

            while (offset + recordSize <= allData.size()) {
                ZKAttendanceLog log;

                // User ID: first 9 bytes (string, null-terminated)
                QByteArray userIdBytes = allData.mid(offset, 9);
                log.userId = QString::fromLatin1(userIdBytes).trimmed()
                                 .replace(QChar('\0'), "").toInt();

                // Timestamp: 4 bytes at offset 24 (little-endian, seconds since 2000-01-01)
                if (offset + 27 < allData.size()) {
                    quint32 ts = static_cast<quint8>(allData[offset + 24]) |
                                 (static_cast<quint8>(allData[offset + 25]) << 8) |
                                 (static_cast<quint8>(allData[offset + 26]) << 16) |
                                 (static_cast<quint8>(allData[offset + 27]) << 24);

                    // Decode: seconds since 2000-01-01 00:00:00
                    int second = ts % 60; ts /= 60;
                    int minute = ts % 60; ts /= 60;
                    int hour = ts % 24; ts /= 24;
                    int day = ts % 31 + 1; ts /= 31;
                    int month = ts % 12 + 1; ts /= 12;
                    int year = ts + 2000;

                    log.timestamp = QDateTime(QDate(year, month, day),
                                              QTime(hour, minute, second));
                }

                // State and verify type
                if (offset + 29 < allData.size()) {
                    log.inOutState = static_cast<quint8>(allData[offset + 28]);
                    log.verifyType = static_cast<quint8>(allData[offset + 29]);
                }

                log.userName = "";

                if (log.timestamp.isValid() && log.userId > 0) {
                    logs.append(log);
                }

                offset += recordSize;
            }
        }
    }

    // Re-enable device
    sendCommand(CMD_ENABLEDEVICE);
    receiveResponse(2000);
    m_replyId++;

    emit statusChanged("✅ تم سحب " + QString::number(logs.size()) + " سجل حضور");
    emit logsReceived(logs);
    return logs;
}

// ============ Clear Logs ============
bool ZKDevice::clearAttendanceLogs() {
    if (!isConnected()) {
        m_lastError = "الجهاز غير متصل";
        return false;
    }

    sendCommand(CMD_CLEAR_ATTLOG);
    QByteArray resp = receiveResponse(5000);

    if (resp.size() >= 14) {
        quint16 respCmd = static_cast<quint8>(resp[6]) |
                          (static_cast<quint8>(resp[7]) << 8);
        if (respCmd == CMD_ACK_OK) {
            m_replyId++;
            emit statusChanged("✅ تم مسح سجلات الحضور من الجهاز");
            return true;
        }
    }

    m_lastError = "فشل مسح السجلات";
    return false;
}
