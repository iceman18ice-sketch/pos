#include "DeviceSettingsWidget.h"
#include "database/DatabaseManager.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDateTime>

DeviceSettingsWidget::DeviceSettingsWidget(QWidget *parent)
    : QWidget(parent),
      m_zkDevice(new ZKDevice(this)),
      m_cashDrawer(new CashDrawer(this)) {

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    setLayoutDirection(Qt::RightToLeft);

    auto *title = new QLabel("🖨️  إعدادات الأجهزة");
    title->setStyleSheet(
        "font-size: 22px; font-weight: bold; background: transparent;");
    mainLayout->addWidget(title);

    auto *splitter = new QSplitter(Qt::Vertical);

    // — Fingerprint section —
    auto *fpWidget = new QWidget;
    auto *fpLayout = new QVBoxLayout(fpWidget);
    fpLayout->setContentsMargins(0, 0, 0, 0);
    setupFingerprintSection(fpLayout);
    splitter->addWidget(fpWidget);

    // — Cash Drawer section —
    auto *cdWidget = new QWidget;
    auto *cdLayout = new QVBoxLayout(cdWidget);
    cdLayout->setContentsMargins(0, 0, 0, 0);
    setupCashDrawerSection(cdLayout);
    splitter->addWidget(cdWidget);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    mainLayout->addWidget(splitter, 1);

    // Signals
    connect(m_zkDevice, &ZKDevice::statusChanged,
            this, &DeviceSettingsWidget::onDeviceStatusChanged);
    connect(m_zkDevice, &ZKDevice::errorOccurred,
            this, &DeviceSettingsWidget::onDeviceError);
    connect(m_zkDevice, &ZKDevice::logsReceived,
            this, &DeviceSettingsWidget::onLogsReceived);
    connect(m_zkDevice, &ZKDevice::connected, this, [this]() {
        m_connectBtn->setEnabled(false);
        m_disconnectBtn->setEnabled(true);
        m_fetchLogsBtn->setEnabled(true);
    });
    connect(m_zkDevice, &ZKDevice::disconnected, this, [this]() {
        m_connectBtn->setEnabled(true);
        m_disconnectBtn->setEnabled(false);
        m_fetchLogsBtn->setEnabled(false);
        m_syncBtn->setEnabled(false);
    });

    // Load saved settings
    auto &db = DatabaseManager::instance();
    m_ipEdit->setText(db.getSetting("zk_device_ip", "192.168.1.201"));
    m_portSpin->setValue(db.getSetting("zk_device_port", "4370").toInt());
}

DeviceSettingsWidget::~DeviceSettingsWidget() {
    if (m_zkDevice->isConnected())
        m_zkDevice->disconnect();
}

// ============================================================
//  Fingerprint Device Section
// ============================================================
void DeviceSettingsWidget::setupFingerprintSection(QVBoxLayout *layout) {
    auto *group = new QGroupBox("👆 جهاز البصمة - ZKTeco");
    group->setStyleSheet(
        "QGroupBox { font-size: 16px; font-weight: bold; border: 2px solid "
        "#45475a; border-radius: 8px; margin-top: 12px; padding-top: 20px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 16px; "
        "padding: 0 8px; }");
    auto *gLayout = new QVBoxLayout(group);
    gLayout->setSpacing(10);

    // Connection row
    auto *connRow = new QHBoxLayout;

    auto *ipLabel = new QLabel("📡 عنوان IP:");
    ipLabel->setStyleSheet("background: transparent;");
    connRow->addWidget(ipLabel);

    m_ipEdit = new QLineEdit;
    m_ipEdit->setPlaceholderText("192.168.1.201");
    m_ipEdit->setFixedWidth(200);
    connRow->addWidget(m_ipEdit);

    auto *portLabel = new QLabel("المنفذ:");
    portLabel->setStyleSheet("background: transparent;");
    connRow->addWidget(portLabel);

    m_portSpin = new QSpinBox;
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(4370);
    m_portSpin->setFixedWidth(100);
    connRow->addWidget(m_portSpin);

    m_connectBtn = new QPushButton("🔗  اتصال");
    m_connectBtn->setObjectName("btnSuccess");
    m_connectBtn->setCursor(Qt::PointingHandCursor);
    m_connectBtn->setFixedWidth(140);
    connRow->addWidget(m_connectBtn);

    m_disconnectBtn = new QPushButton("🔌  قطع");
    m_disconnectBtn->setObjectName("btnDanger");
    m_disconnectBtn->setCursor(Qt::PointingHandCursor);
    m_disconnectBtn->setFixedWidth(120);
    m_disconnectBtn->setEnabled(false);
    connRow->addWidget(m_disconnectBtn);

    connRow->addStretch();
    gLayout->addLayout(connRow);

    // Status
    m_statusLabel = new QLabel("❌ غير متصل");
    m_statusLabel->setStyleSheet(
        "font-size: 14px; padding: 8px; border-radius: 4px; "
        "background: #1E1E2E; color: #F38BA8; border: 1px solid #313244;");
    gLayout->addWidget(m_statusLabel);

    // Action buttons row
    auto *actionRow = new QHBoxLayout;

    m_fetchLogsBtn = new QPushButton("📥  سحب سجلات الحضور");
    m_fetchLogsBtn->setObjectName("btnPrimary");
    m_fetchLogsBtn->setCursor(Qt::PointingHandCursor);
    m_fetchLogsBtn->setEnabled(false);
    actionRow->addWidget(m_fetchLogsBtn);

    m_syncBtn = new QPushButton("🔄  مزامنة مع قاعدة البيانات");
    m_syncBtn->setObjectName("btnSuccess");
    m_syncBtn->setCursor(Qt::PointingHandCursor);
    m_syncBtn->setEnabled(false);
    actionRow->addWidget(m_syncBtn);

    actionRow->addStretch();

    auto *supportLabel = new QLabel(
        "📋 الأجهزة المدعومة: ZKTeco K40/K50, iClock, UA760, U580, FingerTec R2/R3");
    supportLabel->setStyleSheet(
        "font-size: 11px; color: #6c7086; background: transparent;");
    actionRow->addWidget(supportLabel);

    gLayout->addLayout(actionRow);

    // Logs table
    m_logsTable = new QTableWidget;
    m_logsTable->setColumnCount(5);
    m_logsTable->setHorizontalHeaderLabels(
        {"رقم الموظف", "التاريخ", "الوقت", "النوع", "طريقة التحقق"});
    m_logsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_logsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logsTable->setAlternatingRowColors(true);
    m_logsTable->verticalHeader()->hide();
    m_logsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logsTable->setLayoutDirection(Qt::RightToLeft);
    gLayout->addWidget(m_logsTable, 1);

    // Log output
    m_logOutput = new QTextEdit;
    m_logOutput->setReadOnly(true);
    m_logOutput->setMaximumHeight(80);
    m_logOutput->setStyleSheet(
        "background: #11111b; color: #a6e3a1; font-family: 'Consolas'; "
        "font-size: 12px; border: 1px solid #313244; border-radius: 4px;");
    m_logOutput->setPlaceholderText("سجل العمليات...");
    gLayout->addWidget(m_logOutput);

    layout->addWidget(group);

    // Connections
    connect(m_connectBtn, &QPushButton::clicked, this,
            &DeviceSettingsWidget::onConnect);
    connect(m_disconnectBtn, &QPushButton::clicked, this,
            &DeviceSettingsWidget::onDisconnect);
    connect(m_fetchLogsBtn, &QPushButton::clicked, this,
            &DeviceSettingsWidget::onFetchLogs);
    connect(m_syncBtn, &QPushButton::clicked, this,
            &DeviceSettingsWidget::onSyncToDatabase);
}

// ============================================================
//  Cash Drawer Section
// ============================================================
void DeviceSettingsWidget::setupCashDrawerSection(QVBoxLayout *layout) {
    auto *group = new QGroupBox("🗃️ درج الكاش");
    group->setStyleSheet(
        "QGroupBox { font-size: 16px; font-weight: bold; border: 2px solid "
        "#45475a; border-radius: 8px; margin-top: 12px; padding-top: 20px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 16px; "
        "padding: 0 8px; }");
    auto *gLayout = new QVBoxLayout(group);
    gLayout->setSpacing(10);

    auto *row1 = new QHBoxLayout;

    auto *printerLabel = new QLabel("🖨️ الطابعة:");
    printerLabel->setStyleSheet("background: transparent;");
    row1->addWidget(printerLabel);

    m_printerCombo = new QComboBox;
    m_printerCombo->setMinimumWidth(300);
    for (const auto &name : CashDrawer::availablePrinters())
        m_printerCombo->addItem(name);
    row1->addWidget(m_printerCombo);

    m_refreshPrintersBtn = new QPushButton("🔄");
    m_refreshPrintersBtn->setCursor(Qt::PointingHandCursor);
    m_refreshPrintersBtn->setFixedWidth(40);
    row1->addWidget(m_refreshPrintersBtn);

    m_testDrawerBtn = new QPushButton("🗃️  اختبار فتح الدرج");
    m_testDrawerBtn->setObjectName("btnPrimary");
    m_testDrawerBtn->setCursor(Qt::PointingHandCursor);
    row1->addWidget(m_testDrawerBtn);

    row1->addStretch();
    gLayout->addLayout(row1);

    m_drawerStatusLabel = new QLabel(
        "💡 الدرج يتصل عبر طابعة POS بكيبل RJ11. "
        "اختر الطابعة واضغط 'اختبار فتح الدرج'.");
    m_drawerStatusLabel->setStyleSheet(
        "font-size: 12px; color: #a6adc8; padding: 8px; "
        "background: #1E1E2E; border-radius: 4px; border: 1px solid #313244;");
    m_drawerStatusLabel->setWordWrap(true);
    gLayout->addWidget(m_drawerStatusLabel);

    auto *autoOpenRow = new QHBoxLayout;
    auto *autoLabel = new QLabel(
        "⚙️ فتح الدرج تلقائياً عند حفظ فاتورة المبيعات — يمكن تفعيلها من الإعدادات");
    autoLabel->setStyleSheet(
        "font-size: 11px; color: #6c7086; background: transparent;");
    autoOpenRow->addWidget(autoLabel);
    autoOpenRow->addStretch();
    gLayout->addLayout(autoOpenRow);

    layout->addWidget(group);

    // Connections
    connect(m_testDrawerBtn, &QPushButton::clicked, this,
            &DeviceSettingsWidget::onTestDrawer);
    connect(m_refreshPrintersBtn, &QPushButton::clicked, this, [this]() {
        m_printerCombo->clear();
        for (const auto &name : CashDrawer::availablePrinters())
            m_printerCombo->addItem(name);
    });
}

// ============================================================
//  Slots
// ============================================================
void DeviceSettingsWidget::onConnect() {
    QString ip = m_ipEdit->text().trimmed();
    if (ip.isEmpty()) {
        QMessageBox::warning(this, "تنبيه", "أدخل عنوان IP للجهاز");
        return;
    }

    // Save settings
    auto &db = DatabaseManager::instance();
    db.setSetting("zk_device_ip", ip);
    db.setSetting("zk_device_port", QString::number(m_portSpin->value()));

    m_connectBtn->setEnabled(false);
    m_logOutput->append("🔗 جاري الاتصال بـ " + ip + ":" +
                        QString::number(m_portSpin->value()) + "...");

    bool ok = m_zkDevice->connectDevice(ip, m_portSpin->value());
    if (!ok) {
        m_connectBtn->setEnabled(true);
    }
}

void DeviceSettingsWidget::onDisconnect() {
    m_zkDevice->disconnect();
    m_logOutput->append("🔌 تم قطع الاتصال");
}

void DeviceSettingsWidget::onFetchLogs() {
    m_fetchLogsBtn->setEnabled(false);
    m_logOutput->append("📥 جاري سحب سجلات الحضور...");

    auto logs = m_zkDevice->getAttendanceLogs();
    m_currentLogs = logs;
    m_fetchLogsBtn->setEnabled(true);

    // Populate table
    m_logsTable->setRowCount(0);
    for (const auto &log : logs) {
        int r = m_logsTable->rowCount();
        m_logsTable->insertRow(r);

        auto mi = [](const QString &t) {
            auto *i = new QTableWidgetItem(t);
            i->setTextAlignment(Qt::AlignCenter);
            return i;
        };

        m_logsTable->setItem(r, 0, mi(QString::number(log.userId)));
        m_logsTable->setItem(r, 1, mi(log.timestamp.date().toString("yyyy/MM/dd")));
        m_logsTable->setItem(r, 2, mi(log.timestamp.time().toString("hh:mm:ss")));
        m_logsTable->setItem(r, 3,
            mi(log.inOutState == 0 ? "🟢 حضور" : "🔴 انصراف"));

        QString verifyStr;
        switch (log.verifyType) {
            case 0: verifyStr = "🔑 كلمة مرور"; break;
            case 1: verifyStr = "👆 بصمة"; break;
            case 2: verifyStr = "💳 بطاقة"; break;
            default: verifyStr = "❓ غير معروف"; break;
        }
        m_logsTable->setItem(r, 4, mi(verifyStr));
    }

    if (!logs.isEmpty())
        m_syncBtn->setEnabled(true);

    m_logOutput->append("✅ تم سحب " + QString::number(logs.size()) + " سجل");
}

void DeviceSettingsWidget::onSyncToDatabase() {
    if (m_currentLogs.isEmpty()) {
        QMessageBox::information(this, "تنبيه", "لا توجد سجلات للمزامنة");
        return;
    }

    int synced = 0;
    auto &db = DatabaseManager::instance();

    // Group logs by userId and date to pair check-in/check-out
    QMap<QString, QPair<QString, QString>> dayRecords; // key: "userId_date"

    for (const auto &log : m_currentLogs) {
        QString key = QString::number(log.userId) + "_" +
                      log.timestamp.date().toString("yyyy-MM-dd");
        QString time = log.timestamp.time().toString("hh:mm");

        if (!dayRecords.contains(key)) {
            dayRecords[key] = {"", ""};
        }

        if (log.inOutState == 0) {
            // Check-in: keep earliest
            if (dayRecords[key].first.isEmpty() || time < dayRecords[key].first)
                dayRecords[key].first = time;
        } else {
            // Check-out: keep latest
            if (dayRecords[key].second.isEmpty() || time > dayRecords[key].second)
                dayRecords[key].second = time;
        }
    }

    for (auto it = dayRecords.begin(); it != dayRecords.end(); ++it) {
        QStringList parts = it.key().split("_");
        int empId = parts[0].toInt();
        QString date = parts[1];
        QString checkIn = it.value().first;
        QString checkOut = it.value().second;

        if (db.addAttendance(empId, date, checkIn, checkOut, "بصمة - مزامنة تلقائية"))
            synced++;
    }

    m_logOutput->append("🔄 تم مزامنة " + QString::number(synced) + " سجل حضور");
    QMessageBox::information(this, "مزامنة",
        "تم مزامنة " + QString::number(synced) + " سجل حضور مع قاعدة البيانات بنجاح! ✅");
}

void DeviceSettingsWidget::onTestDrawer() {
    QString printer = m_printerCombo->currentText();
    if (printer.isEmpty()) {
        QMessageBox::warning(this, "تنبيه", "لا توجد طابعات متاحة");
        return;
    }

    // Save printer setting
    DatabaseManager::instance().setSetting("cash_drawer_printer", printer);

    m_drawerStatusLabel->setText("🔄 جاري فتح الدرج...");
    m_drawerStatusLabel->setStyleSheet(
        "font-size: 12px; color: #F9E2AF; padding: 8px; "
        "background: #1E1E2E; border-radius: 4px; border: 1px solid #313244;");

    bool ok = m_cashDrawer->openDrawer(printer);
    if (ok) {
        m_drawerStatusLabel->setText("✅ تم فتح الدرج بنجاح!");
        m_drawerStatusLabel->setStyleSheet(
            "font-size: 12px; color: #A6E3A1; padding: 8px; "
            "background: #1E1E2E; border-radius: 4px; border: 1px solid #313244;");
    } else {
        m_drawerStatusLabel->setText("❌ فشل: " + m_cashDrawer->lastError());
        m_drawerStatusLabel->setStyleSheet(
            "font-size: 12px; color: #F38BA8; padding: 8px; "
            "background: #1E1E2E; border-radius: 4px; border: 1px solid #313244;");
    }
}

void DeviceSettingsWidget::onDeviceStatusChanged(const QString &status) {
    m_statusLabel->setText(status);
    m_logOutput->append(status);

    if (status.contains("✅")) {
        m_statusLabel->setStyleSheet(
            "font-size: 14px; padding: 8px; border-radius: 4px; "
            "background: #1E1E2E; color: #A6E3A1; border: 1px solid #313244;");
    } else {
        m_statusLabel->setStyleSheet(
            "font-size: 14px; padding: 8px; border-radius: 4px; "
            "background: #1E1E2E; color: #F9E2AF; border: 1px solid #313244;");
    }
}

void DeviceSettingsWidget::onDeviceError(const QString &error) {
    m_statusLabel->setText("❌ " + error);
    m_statusLabel->setStyleSheet(
        "font-size: 14px; padding: 8px; border-radius: 4px; "
        "background: #1E1E2E; color: #F38BA8; border: 1px solid #313244;");
    m_logOutput->append("❌ خطأ: " + error);
}

void DeviceSettingsWidget::onLogsReceived(const QVector<ZKAttendanceLog> &logs) {
    Q_UNUSED(logs);
    // Already handled in onFetchLogs
}
