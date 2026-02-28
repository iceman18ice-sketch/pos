#include "SettingsWidget.h"
#include "database/DatabaseManager.h"
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPrinterInfo>
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

  // === Company Settings Tab ===
  auto *companyTab = new QWidget;
  auto *compForm = new QFormLayout(companyTab);
  compForm->setSpacing(14);
  compForm->setContentsMargins(20, 20, 20, 20);

  m_companyName = new QLineEdit;
  m_companyPhone = new QLineEdit;
  m_companyAddress = new QLineEdit;
  m_taxNumber = new QLineEdit;
  m_taxRate = new QLineEdit;
  m_currency = new QLineEdit;

  compForm->addRow("🏪 اسم الشركة:", m_companyName);
  compForm->addRow("📞 هاتف الشركة:", m_companyPhone);
  compForm->addRow("📍 عنوان الشركة:", m_companyAddress);
  compForm->addRow("🔢 الرقم الضريبي:", m_taxNumber);
  compForm->addRow("💰 نسبة الضريبة:", m_taxRate);
  compForm->addRow("💵 العملة:", m_currency);

  m_defaultPrinter = new QComboBox;
  m_defaultPrinter->addItem("اختر عند الطباعة", "");
  for (const auto &pi : QPrinterInfo::availablePrinters()) {
    m_defaultPrinter->addItem(pi.printerName(), pi.printerName());
  }
  compForm->addRow("🖨️ الطابعة الافتراضية:", m_defaultPrinter);

  auto *saveBtn = new QPushButton("💾  حفظ الإعدادات");
  saveBtn->setObjectName("btnSuccess");
  saveBtn->setCursor(Qt::PointingHandCursor);
  saveBtn->setMinimumHeight(45);
  compForm->addRow(saveBtn);

  tabs->addTab(companyTab, "🏪 الشركة");

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

  layout->addWidget(tabs);

  connect(saveBtn, &QPushButton::clicked, this, &SettingsWidget::onSave);
  loadSettings();
}

void SettingsWidget::loadSettings() {
  auto &db = DatabaseManager::instance();
  m_companyName->setText(db.getSetting("company_name", "متجري"));
  m_companyPhone->setText(db.getSetting("company_phone"));
  m_companyAddress->setText(db.getSetting("company_address"));
  m_taxNumber->setText(db.getSetting("tax_number"));
  m_taxRate->setText(db.getSetting("tax_rate", "15"));
  m_currency->setText(db.getSetting("currency", "ريال"));
  QString defaultPrinter = db.getSetting("default_printer");
  int idx = m_defaultPrinter->findData(defaultPrinter);
  if (idx >= 0)
    m_defaultPrinter->setCurrentIndex(idx);
}

void SettingsWidget::onSave() {
  auto &db = DatabaseManager::instance();
  db.setSetting("company_name", m_companyName->text());
  db.setSetting("company_phone", m_companyPhone->text());
  db.setSetting("company_address", m_companyAddress->text());
  db.setSetting("tax_number", m_taxNumber->text());
  db.setSetting("tax_rate", m_taxRate->text());
  db.setSetting("currency", m_currency->text());
  db.setSetting("default_printer", m_defaultPrinter->currentData().toString());
  QMessageBox::information(this, "تم", "تم حفظ الإعدادات بنجاح! ✅");
}
