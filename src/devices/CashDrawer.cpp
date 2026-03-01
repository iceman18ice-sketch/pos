#include "CashDrawer.h"
#include <QPrinter>
#include <QPrinterInfo>
#include <QFile>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

CashDrawer::CashDrawer(QObject *parent) : QObject(parent) {}

QStringList CashDrawer::availablePrinters() {
    QStringList names;
    for (const auto &info : QPrinterInfo::availablePrinters()) {
        names.append(info.printerName());
    }
    return names;
}

void CashDrawer::setPulseTiming(int onTimeMs, int offTimeMs) {
    // ESC/POS timing is in units of 2ms, max 255
    m_onTime = qBound(2, onTimeMs, 510);
    m_offTime = qBound(2, offTimeMs, 510);
}

bool CashDrawer::openDrawer(const QString &printerName) {
    if (printerName.isEmpty()) {
        m_lastError = "لم يتم تحديد اسم الطابعة";
        emit errorOccurred(m_lastError);
        return false;
    }
    return sendESCPOSCommand(printerName);
}

bool CashDrawer::openDrawerDefault() {
    QPrinterInfo defaultPrinter = QPrinterInfo::defaultPrinter();
    if (defaultPrinter.isNull()) {
        m_lastError = "لا توجد طابعة افتراضية";
        emit errorOccurred(m_lastError);
        return false;
    }
    return sendESCPOSCommand(defaultPrinter.printerName());
}

bool CashDrawer::sendESCPOSCommand(const QString &printerName) {
    // ESC p m t1 t2
    // ESC = 0x1B, p = 0x70
    // m = pin connector (0x00 = pin2, 0x01 = pin5)
    // t1 = ON time  (units of 2ms)
    // t2 = OFF time (units of 2ms)
    unsigned char t1 = static_cast<unsigned char>(qBound(1, m_onTime / 2, 255));
    unsigned char t2 = static_cast<unsigned char>(qBound(1, m_offTime / 2, 255));

    char command[5];
    command[0] = 0x1B; // ESC
    command[1] = 0x70; // p
    command[2] = static_cast<char>(m_pin); // m (pin selector)
    command[3] = static_cast<char>(t1);    // ON time
    command[4] = static_cast<char>(t2);    // OFF time

#ifdef Q_OS_WIN
    // On Windows, send raw data directly to printer via spooler
    HANDLE hPrinter;
    DOC_INFO_1W docInfo;
    DWORD bytesWritten;

    std::wstring wPrinterName = printerName.toStdWString();

    if (!OpenPrinterW(const_cast<LPWSTR>(wPrinterName.c_str()), &hPrinter, nullptr)) {
        m_lastError = "فشل فتح الطابعة: " + printerName;
        emit errorOccurred(m_lastError);
        return false;
    }

    docInfo.pDocName = const_cast<LPWSTR>(L"CashDrawer_Kick");
    docInfo.pOutputFile = nullptr;
    docInfo.pDatatype = const_cast<LPWSTR>(L"RAW");

    if (StartDocPrinterW(hPrinter, 1, reinterpret_cast<LPBYTE>(&docInfo)) == 0) {
        ClosePrinter(hPrinter);
        m_lastError = "فشل بدء الطباعة";
        emit errorOccurred(m_lastError);
        return false;
    }

    if (!StartPagePrinter(hPrinter)) {
        EndDocPrinter(hPrinter);
        ClosePrinter(hPrinter);
        m_lastError = "فشل بدء الصفحة";
        emit errorOccurred(m_lastError);
        return false;
    }

    WritePrinter(hPrinter, command, 5, &bytesWritten);

    EndPagePrinter(hPrinter);
    EndDocPrinter(hPrinter);
    ClosePrinter(hPrinter);

    if (bytesWritten == 5) {
        emit drawerOpened();
        return true;
    } else {
        m_lastError = "فشل إرسال الأمر للطابعة";
        emit errorOccurred(m_lastError);
        return false;
    }
#else
    // On Linux/Mac, write directly to device file
    // Typical: /dev/usb/lp0 or through lp command
    QFile printer("/dev/usb/lp0");
    if (!printer.open(QIODevice::WriteOnly)) {
        m_lastError = "فشل فتح الطابعة: " + printer.errorString();
        emit errorOccurred(m_lastError);
        return false;
    }
    printer.write(command, 5);
    printer.close();
    emit drawerOpened();
    return true;
#endif
}
