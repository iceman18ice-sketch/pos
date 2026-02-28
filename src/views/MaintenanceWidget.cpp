#include "MaintenanceWidget.h"
#include "database/DatabaseManager.h"
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

MaintenanceWidget::MaintenanceWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("🔧  الصيانة");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  auto *topBar = new QHBoxLayout;
  auto *statusLabel = new QLabel("الحالة:");
  statusLabel->setStyleSheet("background: transparent;");
  topBar->addWidget(statusLabel);
  m_statusFilter = new QComboBox;
  m_statusFilter->addItem("الكل", "");
  m_statusFilter->addItem("⏳ قيد الانتظار", "pending");
  m_statusFilter->addItem("🔧 جاري الإصلاح", "in_progress");
  m_statusFilter->addItem("✅ تم الإصلاح", "completed");
  m_statusFilter->addItem("📦 تم التسليم", "delivered");
  topBar->addWidget(m_statusFilter);
  auto *filterBtn = new QPushButton("🔍  عرض");
  filterBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(filterBtn);
  topBar->addStretch();
  auto *addBtn = new QPushButton("➕  طلب صيانة جديد");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(addBtn);
  auto *statusBtn = new QPushButton("🔄  تحديث الحالة");
  statusBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(statusBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(8);
  m_table->setHorizontalHeaderLabels({"#", "التاريخ", "اسم العميل", "الهاتف",
                                      "نوع الجهاز", "المشكلة", "التكلفة",
                                      "الحالة"});
  m_table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(filterBtn, &QPushButton::clicked, this, &MaintenanceWidget::loadData);
  connect(addBtn, &QPushButton::clicked, this, &MaintenanceWidget::onAdd);
  connect(statusBtn, &QPushButton::clicked, this,
          &MaintenanceWidget::onUpdateStatus);
  loadData();
}

void MaintenanceWidget::refresh() { loadData(); }

void MaintenanceWidget::loadData() {
  m_table->setRowCount(0);
  auto q = DatabaseManager::instance().getMaintenanceRequests(
      m_statusFilter->currentData().toString());
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("id").toString()));
    m_table->setItem(r, 1, mi(q.value("date").toString().left(10)));
    m_table->setItem(r, 2, mi(q.value("customer_name").toString()));
    m_table->setItem(r, 3, mi(q.value("phone").toString()));
    m_table->setItem(r, 4, mi(q.value("device_type").toString()));
    m_table->setItem(r, 5, mi(q.value("problem").toString()));
    m_table->setItem(r, 6,
                     mi(QString::number(q.value("cost").toDouble(), 'f', 2)));
    QString status = q.value("status").toString();
    QString statusText = status == "pending"       ? "⏳ قيد الانتظار"
                         : status == "in_progress" ? "🔧 جاري الإصلاح"
                         : status == "completed"   ? "✅ تم الإصلاح"
                                                   : "📦 تم التسليم";
    auto *si = mi(statusText);
    si->setForeground(status == "pending"       ? QColor("#F59E0B")
                      : status == "in_progress" ? QColor("#3B82F6")
                      : status == "completed"   ? QColor("#22C55E")
                                                : QColor("#8B5CF6"));
    m_table->setItem(r, 7, si);
  }
}

void MaintenanceWidget::onAdd() {
  QDialog dlg(this);
  dlg.setWindowTitle("طلب صيانة جديد");
  dlg.setMinimumSize(450, 380);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *nameEdit = new QLineEdit;
  auto *phoneEdit = new QLineEdit;
  auto *deviceEdit = new QLineEdit;
  auto *problemEdit = new QLineEdit;
  auto *costSpin = new QDoubleSpinBox;
  costSpin->setMaximum(999999);
  costSpin->setDecimals(2);

  form->addRow("👤 اسم العميل:", nameEdit);
  form->addRow("📞 الهاتف:", phoneEdit);
  form->addRow("💻 نوع الجهاز:", deviceEdit);
  form->addRow("🔧 المشكلة:", problemEdit);
  form->addRow("💰 التكلفة:", costSpin);

  auto *btns = new QHBoxLayout;
  auto *saveBtn = new QPushButton("💾  حفظ");
  saveBtn->setObjectName("btnSuccess");
  auto *cancelBtn = new QPushButton("❌  إلغاء");
  cancelBtn->setObjectName("btnDanger");
  btns->addWidget(saveBtn);
  btns->addWidget(cancelBtn);
  form->addRow(btns);

  connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
  connect(saveBtn, &QPushButton::clicked, [&]() {
    if (nameEdit->text().isEmpty()) {
      QMessageBox::warning(&dlg, "تنبيه", "أدخل اسم العميل");
      return;
    }
    DatabaseManager::instance().addMaintenanceRequest(
        nameEdit->text(), phoneEdit->text(), deviceEdit->text(),
        problemEdit->text(), costSpin->value(), "pending");
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}

void MaintenanceWidget::onUpdateStatus() {
  if (m_table->currentRow() < 0) {
    QMessageBox::warning(this, "تنبيه", "اختر طلب صيانة");
    return;
  }
  int id = m_table->item(m_table->currentRow(), 0)->text().toInt();
  QDialog dlg(this);
  dlg.setWindowTitle("تحديث حالة الصيانة");
  dlg.setMinimumSize(300, 150);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  auto *statusCombo = new QComboBox;
  statusCombo->addItem("⏳ قيد الانتظار", "pending");
  statusCombo->addItem("🔧 جاري الإصلاح", "in_progress");
  statusCombo->addItem("✅ تم الإصلاح", "completed");
  statusCombo->addItem("📦 تم التسليم", "delivered");
  form->addRow("🔄 الحالة الجديدة:", statusCombo);
  auto *btns = new QHBoxLayout;
  auto *saveBtn = new QPushButton("💾  تحديث");
  saveBtn->setObjectName("btnSuccess");
  auto *cancelBtn = new QPushButton("❌  إلغاء");
  cancelBtn->setObjectName("btnDanger");
  btns->addWidget(saveBtn);
  btns->addWidget(cancelBtn);
  form->addRow(btns);
  connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
  connect(saveBtn, &QPushButton::clicked, [&]() {
    DatabaseManager::instance().updateMaintenanceStatus(
        id, statusCombo->currentData().toString());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}
