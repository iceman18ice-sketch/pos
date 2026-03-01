#include "SettingsWidget.h"
#include "database/DatabaseManager.h"
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPrinterInfo>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QSqlQuery>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("⚙️  الإعدادات");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  auto *tabs = new QTabWidget;

  // Company tab removed — all data in ZATCA E-Invoicing tab

  // === Categories Tab ===
  auto *catTab = new QWidget;
  auto *catLayout = new QVBoxLayout(catTab);
  auto *catTopBar = new QHBoxLayout;
  auto *catInput = new QLineEdit;
  catInput->setPlaceholderText("اسم التصنيف الجديد...");
  catTopBar->addWidget(catInput);
  auto *catAddBtn = new QPushButton("➕ إضافة تصنيف");
  catAddBtn->setObjectName("btnSuccess");
  catTopBar->addWidget(catAddBtn);
  catLayout->addLayout(catTopBar);

  auto *catTable = new QTableWidget;
  catTable->setColumnCount(2);
  catTable->setHorizontalHeaderLabels({"#", "الاسم"});
  catTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  catTable->setAlternatingRowColors(true);
  catTable->verticalHeader()->hide();
  catTable->setLayoutDirection(Qt::RightToLeft);
  catLayout->addWidget(catTable);
  tabs->addTab(catTab, "📂 التصنيفات");

  // Load categories
  auto loadCats = [catTable]() {
    catTable->setRowCount(0);
    auto q = DatabaseManager::instance().getCategories();
    while (q.next()) {
      int r = catTable->rowCount();
      catTable->insertRow(r);
      auto *id = new QTableWidgetItem(q.value("id").toString());
      id->setTextAlignment(Qt::AlignCenter);
      auto *name = new QTableWidgetItem(q.value("name").toString());
      name->setTextAlignment(Qt::AlignCenter);
      catTable->setItem(r, 0, id);
      catTable->setItem(r, 1, name);
    }
  };
  loadCats();

  connect(catAddBtn, &QPushButton::clicked, [catInput, loadCats]() {
    if (catInput->text().isEmpty())
      return;
    DatabaseManager::instance().addCategory(catInput->text());
    catInput->clear();
    loadCats();
  });

  // === Users Tab ===
  auto *usersTab = new QWidget;
  auto *usersLayout = new QVBoxLayout(usersTab);

  auto *usersTable = new QTableWidget;
  usersTable->setColumnCount(4);
  usersTable->setHorizontalHeaderLabels(
      {"#", "اسم المستخدم", "الاسم الكامل", "الدور"});
  usersTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  usersTable->setAlternatingRowColors(true);
  usersTable->verticalHeader()->hide();
  usersTable->setLayoutDirection(Qt::RightToLeft);
  usersLayout->addWidget(usersTable);

  // Load users
  {
    QSqlQuery q(DatabaseManager::instance().database());
    q.exec("SELECT * FROM Users WHERE active = 1");
    while (q.next()) {
      int r = usersTable->rowCount();
      usersTable->insertRow(r);
      auto mi = [](const QString &t) {
        auto *i = new QTableWidgetItem(t);
        i->setTextAlignment(Qt::AlignCenter);
        return i;
      };
      usersTable->setItem(r, 0, mi(q.value("id").toString()));
      usersTable->setItem(r, 1, mi(q.value("username").toString()));
      usersTable->setItem(r, 2, mi(q.value("full_name").toString()));
      QString role = q.value("role").toString();
      usersTable->setItem(r, 3,
                          mi(role == "admin"     ? "مدير"
                             : role == "cashier" ? "كاشير"
                                                 : "محاسب"));
    }
  }

  auto *addUserBtn = new QPushButton("➕ إضافة مستخدم");
  addUserBtn->setObjectName("btnSuccess");
  usersLayout->addWidget(addUserBtn);

  connect(addUserBtn, &QPushButton::clicked, [this, usersTable]() {
    QDialog dlg(this);
    dlg.setWindowTitle("إضافة مستخدم");
    dlg.setLayoutDirection(Qt::RightToLeft);
    auto *form = new QFormLayout(&dlg);
    auto *unEdit = new QLineEdit;
    auto *pwEdit = new QLineEdit;
    pwEdit->setEchoMode(QLineEdit::Password);
    auto *fnEdit = new QLineEdit;
    auto *roleCombo = new QComboBox;
    roleCombo->addItem("مدير", "admin");
    roleCombo->addItem("كاشير", "cashier");
    roleCombo->addItem("محاسب", "accountant");
    form->addRow("اسم المستخدم:", unEdit);
    form->addRow("كلمة المرور:", pwEdit);
    form->addRow("الاسم الكامل:", fnEdit);
    form->addRow("الدور:", roleCombo);
    auto *saveBtn2 = new QPushButton("حفظ");
    saveBtn2->setObjectName("btnSuccess");
    form->addRow(saveBtn2);
    connect(saveBtn2, &QPushButton::clicked, [&]() {
      if (unEdit->text().isEmpty() || pwEdit->text().isEmpty())
        return;
      QSqlQuery q(DatabaseManager::instance().database());
      q.prepare("INSERT INTO Users (username, password_hash, full_name, role) "
                "VALUES (?, ?, ?, ?)");
      q.addBindValue(unEdit->text());
      q.addBindValue(pwEdit->text());
      q.addBindValue(fnEdit->text());
      q.addBindValue(roleCombo->currentData().toString());
      if (q.exec())
        dlg.accept();
    });
    dlg.exec();
  });

  tabs->addTab(usersTab, "👤 المستخدمين");

  // === ZATCA E-Invoicing Tab ===
  auto *zatcaScroll = new QScrollArea;
  zatcaScroll->setWidgetResizable(true);
  auto *zatcaTab = new QWidget;
  auto *zatcaLayout = new QVBoxLayout(zatcaTab);
  zatcaLayout->setSpacing(12);
  zatcaLayout->setContentsMargins(20, 20, 20, 20);

  // --- Company Info Group ---
  auto *compGroup = new QGroupBox("📋 بيانات المنشأة");
  compGroup->setStyleSheet(
      "QGroupBox { font-weight: bold; color: #E8EAED; border: 1px solid "
      "#374151; border-radius: 8px; margin-top: 10px; padding-top: 16px; } "
      "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }");
  auto *compInfoForm = new QFormLayout(compGroup);
  compInfoForm->setSpacing(10);

  m_zatcaVat = new QLineEdit;
  m_zatcaVat->setPlaceholderText("مثال: 300000000000003 (15 رقم)");
  m_zatcaVat->setMaxLength(15);
  compInfoForm->addRow("🔢 الرقم الضريبي (VAT):", m_zatcaVat);

  m_zatcaCRN = new QLineEdit;
  m_zatcaCRN->setPlaceholderText("رقم السجل التجاري");
  compInfoForm->addRow("📄 السجل التجاري (CRN):", m_zatcaCRN);

  m_zatcaCompanyAr = new QLineEdit;
  m_zatcaCompanyAr->setPlaceholderText("اسم المنشأة بالعربي");
  compInfoForm->addRow("🏪 اسم المنشأة (عربي):", m_zatcaCompanyAr);

  m_zatcaCompanyEn = new QLineEdit;
  m_zatcaCompanyEn->setPlaceholderText("Company Name in English");
  compInfoForm->addRow("🏪 اسم المنشأة (إنجليزي):", m_zatcaCompanyEn);

  m_companyPhone = new QLineEdit;
  m_companyPhone->setPlaceholderText("05XXXXXXXX");
  compInfoForm->addRow("📞 هاتف المنشأة:", m_companyPhone);

  m_taxRate = new QLineEdit;
  m_taxRate->setPlaceholderText("15");
  compInfoForm->addRow("💰 نسبة الضريبة %:", m_taxRate);

  m_currency = new QLineEdit;
  m_currency->setPlaceholderText("ريال");
  compInfoForm->addRow("💵 العملة:", m_currency);

  m_defaultPrinter = new QComboBox;
  m_defaultPrinter->addItem("اختر عند الطباعة", "");
  for (const auto &pi : QPrinterInfo::availablePrinters()) {
    m_defaultPrinter->addItem(pi.printerName(), pi.printerName());
  }
  compInfoForm->addRow("🖨️ الطابعة الافتراضية:", m_defaultPrinter);

  zatcaLayout->addWidget(compGroup);

  // --- Address Group ---
  auto *addrGroup = new QGroupBox("📍 عنوان المنشأة");
  addrGroup->setStyleSheet(compGroup->styleSheet());
  auto *addrForm = new QFormLayout(addrGroup);
  addrForm->setSpacing(10);

  m_zatcaStreet = new QLineEdit;
  m_zatcaStreet->setPlaceholderText("اسم الشارع");
  addrForm->addRow("🛤️ الشارع:", m_zatcaStreet);

  m_zatcaBuildingNo = new QLineEdit;
  m_zatcaBuildingNo->setPlaceholderText("رقم المبنى");
  m_zatcaBuildingNo->setMaxLength(10);
  addrForm->addRow("🏢 رقم المبنى:", m_zatcaBuildingNo);

  m_zatcaDistrict = new QLineEdit;
  m_zatcaDistrict->setPlaceholderText("اسم الحي");
  addrForm->addRow("🏘️ الحي:", m_zatcaDistrict);

  m_zatcaCity = new QLineEdit;
  m_zatcaCity->setPlaceholderText("اسم المدينة");
  addrForm->addRow("🏙️ المدينة:", m_zatcaCity);

  m_zatcaPostalCode = new QLineEdit;
  m_zatcaPostalCode->setPlaceholderText("الرمز البريدي (5 أرقام)");
  m_zatcaPostalCode->setMaxLength(5);
  addrForm->addRow("📮 الرمز البريدي:", m_zatcaPostalCode);

  m_zatcaCountry = new QLineEdit("SA");
  m_zatcaCountry->setMaxLength(2);
  addrForm->addRow("🌍 رمز الدولة:", m_zatcaCountry);

  zatcaLayout->addWidget(addrGroup);

  // --- Invoice Settings Group ---
  auto *invGroup = new QGroupBox("🧾 إعدادات الفاتورة الإلكترونية");
  invGroup->setStyleSheet(compGroup->styleSheet());
  auto *invForm = new QFormLayout(invGroup);
  invForm->setSpacing(10);

  m_zatcaInvoiceType = new QComboBox;
  m_zatcaInvoiceType->addItem("فواتير مبسطة فقط (B2C)", "0100");
  m_zatcaInvoiceType->addItem("فواتير ضريبية فقط (B2B)", "1000");
  m_zatcaInvoiceType->addItem("مبسطة + ضريبية (B2C + B2B)", "1100");
  invForm->addRow("📋 نوع الفواتير:", m_zatcaInvoiceType);

  m_zatcaEnvironment = new QComboBox;
  m_zatcaEnvironment->addItem("🧪 بيئة الاختبار (Sandbox)", "sandbox");
  m_zatcaEnvironment->addItem("🏭 بيئة الإنتاج (Production)", "production");
  invForm->addRow("🌐 البيئة:", m_zatcaEnvironment);

  zatcaLayout->addWidget(invGroup);

  // --- SDK & Certificates Group ---
  auto *sdkGroup = new QGroupBox("🔐 SDK والشهادات");
  sdkGroup->setStyleSheet(compGroup->styleSheet());
  auto *sdkForm = new QFormLayout(sdkGroup);
  sdkForm->setSpacing(10);

  // SDK Path
  auto *sdkRow = new QHBoxLayout;
  m_zatcaSdkPath = new QLineEdit;
  m_zatcaSdkPath->setPlaceholderText("مسار fatooraNet.exe");
  sdkRow->addWidget(m_zatcaSdkPath);
  auto *sdkBrowse = new QPushButton("📂 استعراض");
  sdkBrowse->setCursor(Qt::PointingHandCursor);
  connect(sdkBrowse, &QPushButton::clicked, [this]() {
    QString f = QFileDialog::getOpenFileName(this, "اختر fatooraNet.exe",
        QDir::homePath(), "Executable (fatooraNet.exe)");
    if (!f.isEmpty()) m_zatcaSdkPath->setText(f);
  });
  sdkRow->addWidget(sdkBrowse);
  sdkForm->addRow("🛠️ مسار SDK:", sdkRow);

  // Certificate Path
  auto *certRow = new QHBoxLayout;
  m_zatcaCertPath = new QLineEdit;
  m_zatcaCertPath->setPlaceholderText("مسار ملف cert.pem");
  certRow->addWidget(m_zatcaCertPath);
  auto *certBrowse = new QPushButton("📂");
  certBrowse->setCursor(Qt::PointingHandCursor);
  connect(certBrowse, &QPushButton::clicked, [this]() {
    QString f = QFileDialog::getOpenFileName(this, "اختر الشهادة",
        QDir::homePath(), "PEM Files (*.pem);;All (*)");
    if (!f.isEmpty()) m_zatcaCertPath->setText(f);
  });
  certRow->addWidget(certBrowse);
  sdkForm->addRow("📜 الشهادة (cert.pem):", certRow);

  // Private Key Path
  auto *keyRow = new QHBoxLayout;
  m_zatcaPrivKeyPath = new QLineEdit;
  m_zatcaPrivKeyPath->setPlaceholderText("مسار ملف المفتاح الخاص");
  keyRow->addWidget(m_zatcaPrivKeyPath);
  auto *keyBrowse = new QPushButton("📂");
  keyBrowse->setCursor(Qt::PointingHandCursor);
  connect(keyBrowse, &QPushButton::clicked, [this]() {
    QString f = QFileDialog::getOpenFileName(this, "اختر المفتاح الخاص",
        QDir::homePath(), "PEM/Key Files (*.pem *.key);;All (*)");
    if (!f.isEmpty()) m_zatcaPrivKeyPath->setText(f);
  });
  keyRow->addWidget(keyBrowse);
  sdkForm->addRow("🔑 المفتاح الخاص:", keyRow);

  zatcaLayout->addWidget(sdkGroup);

  // --- Developer Portal Group ---
  auto *portalGroup = new QGroupBox("🌐 بوابة فاتورة (ZATCA Fatoora)");
  portalGroup->setStyleSheet(compGroup->styleSheet());
  auto *portalLayout = new QVBoxLayout(portalGroup);
  portalLayout->setSpacing(8);

  auto *portalInfo = new QLabel(
      "⚠️ سجّل في بوابة فاتورة وأضف جهاز للحصول على رمز OTP للربط");
  portalInfo->setStyleSheet("color: #F59E0B; font-size: 13px; padding: 5px;");
  portalInfo->setWordWrap(true);
  portalLayout->addWidget(portalInfo);

  auto *portalBtnLayout = new QHBoxLayout;

  auto *registerBtn = new QPushButton("📝 إضافة جهاز (OTP)");
  registerBtn->setCursor(Qt::PointingHandCursor);
  registerBtn->setMinimumHeight(40);
  registerBtn->setStyleSheet(
      "font-size: 14px; font-weight: bold; "
      "background: #3B82F6; color: white; "
      "border-radius: 8px; border: none; padding: 8px 16px;");
  connect(registerBtn, &QPushButton::clicked, []() {
    QDesktopServices::openUrl(QUrl("https://fatoora.zatca.gov.sa/onboard-solution"));
  });
  portalBtnLayout->addWidget(registerBtn);

  auto *loginBtn = new QPushButton("🔑 تسجيل الدخول");
  loginBtn->setCursor(Qt::PointingHandCursor);
  loginBtn->setMinimumHeight(40);
  loginBtn->setStyleSheet(
      "font-size: 14px; font-weight: bold; "
      "background: #6366F1; color: white; "
      "border-radius: 8px; border: none; padding: 8px 16px;");
  connect(loginBtn, &QPushButton::clicked, []() {
    QDesktopServices::openUrl(QUrl("https://fatoora.zatca.gov.sa/"));
  });
  portalBtnLayout->addWidget(loginBtn);

  auto *docsBtn = new QPushButton("📖 وثائق API");
  docsBtn->setCursor(Qt::PointingHandCursor);
  docsBtn->setMinimumHeight(40);
  docsBtn->setStyleSheet(
      "font-size: 14px; font-weight: bold; "
      "background: #8B5CF6; color: white; "
      "border-radius: 8px; border: none; padding: 8px 16px;");
  connect(docsBtn, &QPushButton::clicked, []() {
    QDesktopServices::openUrl(QUrl("https://zatca.gov.sa/en/E-Invoicing/Introduction/Guidelines/Pages/default.aspx"));
  });
  portalBtnLayout->addWidget(docsBtn);

  portalLayout->addLayout(portalBtnLayout);
  zatcaLayout->addWidget(portalGroup);

  // --- Connection Status ---
  m_zatcaStatus = new QLabel("⚪ غير مربوط");
  m_zatcaStatus->setStyleSheet(
      "font-size: 15px; font-weight: bold; color: #F59E0B; "
      "padding: 10px; background: rgba(245, 158, 11, 0.1); "
      "border-radius: 8px; border: 1px solid #F59E0B;");
  m_zatcaStatus->setAlignment(Qt::AlignCenter);
  zatcaLayout->addWidget(m_zatcaStatus);

  // --- Connect to ZATCA Button ---
  auto *connectBtn = new QPushButton("🔗  ربط مع هيئة الزكاة والضريبة والجمارك (ZATCA)");
  connectBtn->setObjectName("btnPrimary");
  connectBtn->setCursor(Qt::PointingHandCursor);
  connectBtn->setMinimumHeight(55);
  connectBtn->setStyleSheet(
      "font-size: 16px; font-weight: bold; "
      "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
      "stop:0 #10B981, stop:1 #059669); color: white; "
      "border-radius: 10px; border: none; padding: 12px;");
  connect(connectBtn, &QPushButton::clicked, [this]() {
    // Validate required fields
    if (m_zatcaVat->text().trimmed().length() != 15) {
      QMessageBox::warning(this, "تنبيه",
          "الرقم الضريبي (VAT) يجب أن يكون 15 رقم!");
      return;
    }
    if (m_zatcaCompanyAr->text().trimmed().isEmpty()) {
      QMessageBox::warning(this, "تنبيه", "يرجى إدخال اسم المنشأة بالعربي!");
      return;
    }
    if (m_zatcaSdkPath->text().trimmed().isEmpty() ||
        !QFile::exists(m_zatcaSdkPath->text().trimmed())) {
      QMessageBox::warning(this, "تنبيه",
          "يرجى تحديد مسار fatooraNet.exe صحيح!");
      return;
    }

    // Ask for OTP
    bool ok;
    QString otp = QInputDialog::getText(this, "رمز التحقق (OTP)",
        "أدخل رمز OTP المرسل من ZATCA:\n\n"
        "ملاحظة: في بيئة الاختبار (Sandbox) استخدم: 123456",
        QLineEdit::Normal, "123456", &ok);
    if (!ok || otp.isEmpty()) return;

    // Save settings first
    onSave();

    // Run onboarding script
    QString script = QCoreApplication::applicationDirPath() + "/zatca_onboard.py";
    QString dbPath = QCoreApplication::applicationDirPath() + "/data/pos_database.db";
    QString sdkPath = m_zatcaSdkPath->text().trimmed();

    if (!QFile::exists(script)) {
      QMessageBox::critical(this, "خطأ",
          "ملف zatca_onboard.py غير موجود في مجلد البرنامج!");
      return;
    }

    QProcess *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    // Progress dialog
    QProgressDialog *progress = new QProgressDialog(
        "⏳ جاري الربط مع ZATCA...", "إلغاء", 0, 3, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setLayoutDirection(Qt::RightToLeft);
    progress->setMinimumWidth(400);
    progress->show();

    // Accumulate all output
    auto *allOutput = new QStringList;

    connect(proc, &QProcess::readyReadStandardOutput, [proc, progress, allOutput]() {
      QString out = QString::fromUtf8(proc->readAll());
      allOutput->append(out);
      for (const QString &line : out.split('\n')) {
        if (line.startsWith("STEP:")) {
          QStringList parts = line.split(':');
          if (parts.size() >= 3) {
            int step = parts[1].toInt();
            progress->setValue(step);
            progress->setLabelText("⏳ " + parts[2]);
          }
        }
      }
    });

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int code, QProcess::ExitStatus) {
      progress->close();
      progress->deleteLater();

      // Get any remaining output + accumulated
      QString remaining = QString::fromUtf8(proc->readAll());
      if (!remaining.isEmpty()) allOutput->append(remaining);
      QString output = allOutput->join("");
      delete allOutput;

      if (code == 0 && !output.contains("FAILED:") && !output.contains("ERROR:")) {
        m_zatcaStatus->setText("🟢 مربوط بنجاح مع ZATCA ✅");
        m_zatcaStatus->setStyleSheet(
            "font-size: 15px; font-weight: bold; color: #10B981; "
            "padding: 10px; background: rgba(16, 185, 129, 0.1); "
            "border-radius: 8px; border: 1px solid #10B981;");
        QMessageBox::information(this, "تم الربط بنجاح! ✅",
            "🎉 تم ربط المنشأة مع هيئة الزكاة والضريبة والجمارك بنجاح!\n\n"
            "الآن جميع الفواتير ستُرسل تلقائياً إلى ZATCA.\n\n"
            "الرقم الضريبي: " + m_zatcaVat->text());
        loadSettings(); // Reload to show new cert paths
      } else {
        m_zatcaStatus->setText("🔴 فشل الربط");
        m_zatcaStatus->setStyleSheet(
            "font-size: 15px; font-weight: bold; color: #EF4444; "
            "padding: 10px; background: rgba(239, 68, 68, 0.1); "
            "border-radius: 8px; border: 1px solid #EF4444;");
        QMessageBox::critical(this, "فشل الربط",
            "❌ فشل الربط مع ZATCA:\n\n" + output);
      }
      proc->deleteLater();
    });

    proc->start("python", {"-u", script, dbPath, sdkPath, otp});
  });
  zatcaLayout->addWidget(connectBtn);

  // --- Save ZATCA Settings ---
  auto *saveZatcaBtn = new QPushButton("💾  حفظ إعدادات ZATCA");
  saveZatcaBtn->setObjectName("btnSuccess");
  saveZatcaBtn->setCursor(Qt::PointingHandCursor);
  saveZatcaBtn->setMinimumHeight(45);
  connect(saveZatcaBtn, &QPushButton::clicked, this, &SettingsWidget::onSave);
  zatcaLayout->addWidget(saveZatcaBtn);

  zatcaLayout->addStretch();
  zatcaScroll->setWidget(zatcaTab);
  tabs->addTab(zatcaScroll, "🧾 الفوترة الإلكترونية");

  // ============================================================
  // TAB 2: Database (قاعدة البيانات)
  // ============================================================
  auto *dbScroll = new QScrollArea;
  dbScroll->setWidgetResizable(true);
  auto *dbTab = new QWidget;
  auto *dbLayout = new QVBoxLayout(dbTab);
  dbLayout->setSpacing(12);
  dbLayout->setContentsMargins(15, 15, 15, 15);

  // --- Database Status ---
  auto &db = DatabaseManager::instance();
  auto *statusGroup = new QGroupBox("🗄️ حالة قاعدة البيانات");
  statusGroup->setStyleSheet(compGroup->styleSheet());
  auto *statusLayout = new QFormLayout(statusGroup);
  auto *dbTypeLabel = new QLabel(db.isMySQL() ? "🟢 MySQL (متصل)" : "🔵 SQLite (محلي)");
  dbTypeLabel->setStyleSheet("font-size: 15px; font-weight: bold; padding: 8px;");
  statusLayout->addRow("نوع القاعدة:", dbTypeLabel);
  dbLayout->addWidget(statusGroup);

  // --- MySQL Connection ---
  auto *mysqlGroup = new QGroupBox("🐬 اتصال MySQL (اختياري)");
  mysqlGroup->setStyleSheet(compGroup->styleSheet());
  auto *mysqlLayout = new QFormLayout(mysqlGroup);
  mysqlLayout->setSpacing(8);

  auto *mysqlHost = new QLineEdit("localhost");
  mysqlHost->setPlaceholderText("مثال: localhost أو 192.168.1.100");
  auto *mysqlPort = new QLineEdit("3306");
  auto *mysqlDbName = new QLineEdit("pos_database");
  auto *mysqlUser = new QLineEdit("root");
  auto *mysqlPass = new QLineEdit;
  mysqlPass->setEchoMode(QLineEdit::Password);
  mysqlPass->setPlaceholderText("كلمة مرور MySQL");

  mysqlLayout->addRow("🖥️ Host:", mysqlHost);
  mysqlLayout->addRow("🔌 Port:", mysqlPort);
  mysqlLayout->addRow("📁 اسم القاعدة:", mysqlDbName);
  mysqlLayout->addRow("👤 المستخدم:", mysqlUser);
  mysqlLayout->addRow("🔒 كلمة السر:", mysqlPass);

  auto *connectMySqlBtn = new QPushButton("🐬 اتصال بـ MySQL");
  connectMySqlBtn->setCursor(Qt::PointingHandCursor);
  connectMySqlBtn->setMinimumHeight(45);
  connectMySqlBtn->setStyleSheet(
      "font-size: 15px; font-weight: bold; "
      "background: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
      "stop:0 #0078D7, stop:1 #00BCF2); "
      "color: white; border-radius: 10px; border: none; padding: 10px;");
  connect(connectMySqlBtn, &QPushButton::clicked,
      [=, &db]() {
    QString h = mysqlHost->text().trimmed();
    int p = mysqlPort->text().toInt();
    QString d = mysqlDbName->text().trimmed();
    QString u = mysqlUser->text().trimmed();
    QString pw = mysqlPass->text();

    if (h.isEmpty() || d.isEmpty() || u.isEmpty()) {
      QMessageBox::warning(this, "خطأ", "الرجاء تعبئة جميع الحقول!");
      return;
    }

    connectMySqlBtn->setText("⏳ جاري الاتصال...");
    connectMySqlBtn->setEnabled(false);

    if (db.initializeMySQL(h, p, d, u, pw)) {
      dbTypeLabel->setText("🟢 MySQL (متصل) ✅");
      QMessageBox::information(this, "نجح!", "تم الاتصال بـ MySQL بنجاح! 🎉\n\n"
          "تم إنشاء جميع الجداول تلقائياً.");
    } else {
      QMessageBox::critical(this, "فشل", "❌ فشل الاتصال بـ MySQL!\n\n"
          "تأكد من:\n"
          "- MySQL Server شغّال\n"
          "- البيانات صحيحة\n"
          "- MySQL ODBC Driver مثبّت\n\n"
          "البرنامج سيكمل على SQLite.");
    }
    connectMySqlBtn->setText("🐬 اتصال بـ MySQL");
    connectMySqlBtn->setEnabled(true);
  });
  mysqlLayout->addRow(connectMySqlBtn);
  dbLayout->addWidget(mysqlGroup);

  // --- Export Section ---
  auto *exportGroup = new QGroupBox("📤 تصدير البيانات");
  exportGroup->setStyleSheet(compGroup->styleSheet());
  auto *exportLayout = new QVBoxLayout(exportGroup);
  exportLayout->setSpacing(10);

  // Export Database
  auto *exportDbBtn = new QPushButton("💾 تصدير نسخة احتياطية من القاعدة");
  exportDbBtn->setCursor(Qt::PointingHandCursor);
  exportDbBtn->setMinimumHeight(45);
  exportDbBtn->setStyleSheet(
      "font-size: 14px; font-weight: bold; "
      "background: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
      "stop:0 #10B981, stop:1 #34D399); "
      "color: white; border-radius: 10px; border: none; padding: 10px;");
  connect(exportDbBtn, &QPushButton::clicked, [this, &db]() {
    QString ext = db.isMySQL() ? "SQL Files (*.sql)" : "Database Files (*.db)";
    QString defName = db.isMySQL() ? "pos_backup.sql" : "pos_backup.db";
    QString path = QFileDialog::getSaveFileName(this,
        "تصدير قاعدة البيانات", defName, ext);
    if (path.isEmpty()) return;
    if (db.exportDatabase(path)) {
      QMessageBox::information(this, "تم!", "✅ تم تصدير قاعدة البيانات بنجاح إلى:\n" + path);
    } else {
      QMessageBox::critical(this, "خطأ", "❌ فشل تصدير قاعدة البيانات!");
    }
  });
  exportLayout->addWidget(exportDbBtn);

  // Export Products
  auto *exportProductsBtn = new QPushButton("📋 تصدير قائمة المنتجات (CSV)");
  exportProductsBtn->setCursor(Qt::PointingHandCursor);
  exportProductsBtn->setMinimumHeight(45);
  exportProductsBtn->setStyleSheet(
      "font-size: 14px; font-weight: bold; "
      "background: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
      "stop:0 #6366F1, stop:1 #818CF8); "
      "color: white; border-radius: 10px; border: none; padding: 10px;");
  connect(exportProductsBtn, &QPushButton::clicked, [this, &db]() {
    QString path = QFileDialog::getSaveFileName(this,
        "تصدير قائمة المنتجات", "products.csv", "CSV Files (*.csv)");
    if (path.isEmpty()) return;
    if (db.exportProducts(path)) {
      QMessageBox::information(this, "تم!", "✅ تم تصدير قائمة المنتجات بنجاح!\n\n"
          "الملف: " + path + "\n\n"
          "يمكنك فتحه ببرنامج Excel.");
    } else {
      QMessageBox::critical(this, "خطأ", "❌ فشل تصدير قائمة المنتجات!");
    }
  });
  exportLayout->addWidget(exportProductsBtn);
  dbLayout->addWidget(exportGroup);

  dbLayout->addStretch();
  dbScroll->setWidget(dbTab);
  tabs->addTab(dbScroll, "🗄️ قاعدة البيانات");

  layout->addWidget(tabs);

  loadSettings();
}

void SettingsWidget::loadSettings() {
  auto &db = DatabaseManager::instance();

  // ZATCA / Company settings (merged)
  m_zatcaVat->setText(db.getSetting("zatca_vat"));
  m_zatcaCRN->setText(db.getSetting("zatca_crn"));
  m_zatcaCompanyAr->setText(db.getSetting("zatca_company_ar", db.getSetting("company_name", "متجري")));
  m_zatcaCompanyEn->setText(db.getSetting("zatca_company_en"));
  m_companyPhone->setText(db.getSetting("company_phone"));
  m_taxRate->setText(db.getSetting("tax_rate", "15"));
  m_currency->setText(db.getSetting("currency", "ريال"));
  QString defaultPrinter = db.getSetting("default_printer");
  int idx = m_defaultPrinter->findData(defaultPrinter);
  if (idx >= 0)
    m_defaultPrinter->setCurrentIndex(idx);

  m_zatcaStreet->setText(db.getSetting("zatca_street"));
  m_zatcaBuildingNo->setText(db.getSetting("zatca_building_no"));
  m_zatcaDistrict->setText(db.getSetting("zatca_district"));
  m_zatcaCity->setText(db.getSetting("zatca_city"));
  m_zatcaPostalCode->setText(db.getSetting("zatca_postal_code"));
  m_zatcaCountry->setText(db.getSetting("zatca_country", "SA"));
  int invIdx = m_zatcaInvoiceType->findData(db.getSetting("zatca_invoice_type", "0100"));
  if (invIdx >= 0) m_zatcaInvoiceType->setCurrentIndex(invIdx);
  int envIdx = m_zatcaEnvironment->findData(db.getSetting("zatca_environment", "sandbox"));
  if (envIdx >= 0) m_zatcaEnvironment->setCurrentIndex(envIdx);
  // Auto-detect SDK path if not set
  QString sdkPath = db.getSetting("zatca_sdk_path");
  QString appDir = QCoreApplication::applicationDirPath();
  if (sdkPath.isEmpty() || !QFile::exists(sdkPath)) {
    QString autoSdk = appDir + "/zatca-sdk/fatooraNet.exe";
    if (QFile::exists(autoSdk)) sdkPath = autoSdk;
  }
  m_zatcaSdkPath->setText(sdkPath);

  // Auto-detect cert/key paths if not set
  QString certPath = db.getSetting("zatca_cert_path");
  if (certPath.isEmpty() || !QFile::exists(certPath)) {
    QString autoCert = appDir + "/zatca-sdk/Data/Certificates/cert.pem";
    if (QFile::exists(autoCert)) certPath = autoCert;
  }
  m_zatcaCertPath->setText(certPath);

  QString keyPath = db.getSetting("zatca_privkey_path");
  if (keyPath.isEmpty() || !QFile::exists(keyPath)) {
    QString autoKey = appDir + "/zatca-sdk/Data/Certificates/ec-secp256k1-priv-key.pem";
    if (QFile::exists(autoKey)) keyPath = autoKey;
  }
  m_zatcaPrivKeyPath->setText(keyPath);

  // Check if already onboarded
  if (db.getSetting("zatca_onboarded") == "1") {
    m_zatcaStatus->setText("🟢 مربوط بنجاح مع ZATCA ✅");
    m_zatcaStatus->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #10B981; "
        "padding: 10px; background: rgba(16, 185, 129, 0.1); "
        "border-radius: 8px; border: 1px solid #10B981;");
  }
}

void SettingsWidget::onSave() {
  auto &db = DatabaseManager::instance();

  // Save all company + ZATCA settings (merged)
  db.setSetting("zatca_vat", m_zatcaVat->text());
  db.setSetting("zatca_crn", m_zatcaCRN->text());
  db.setSetting("zatca_company_ar", m_zatcaCompanyAr->text());
  db.setSetting("zatca_company_en", m_zatcaCompanyEn->text());
  db.setSetting("zatca_street", m_zatcaStreet->text());
  db.setSetting("zatca_building_no", m_zatcaBuildingNo->text());
  db.setSetting("zatca_district", m_zatcaDistrict->text());
  db.setSetting("zatca_city", m_zatcaCity->text());
  db.setSetting("zatca_postal_code", m_zatcaPostalCode->text());
  db.setSetting("zatca_country", m_zatcaCountry->text());
  db.setSetting("zatca_invoice_type", m_zatcaInvoiceType->currentData().toString());
  db.setSetting("zatca_environment", m_zatcaEnvironment->currentData().toString());
  db.setSetting("zatca_sdk_path", m_zatcaSdkPath->text());
  db.setSetting("zatca_cert_path", m_zatcaCertPath->text());
  db.setSetting("zatca_privkey_path", m_zatcaPrivKeyPath->text());

  // Save phone, tax, currency, printer
  db.setSetting("company_phone", m_companyPhone->text());
  db.setSetting("tax_rate", m_taxRate->text());
  db.setSetting("currency", m_currency->text());
  db.setSetting("default_printer", m_defaultPrinter->currentData().toString());

  // Backward compatibility — keep old keys updated
  db.setSetting("company_name", m_zatcaCompanyAr->text());
  db.setSetting("tax_number", m_zatcaVat->text());

  QMessageBox::information(this, "تم", "تم حفظ جميع الإعدادات بنجاح! ✅");
}
