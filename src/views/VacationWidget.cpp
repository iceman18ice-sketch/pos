#include "VacationWidget.h"
#include "database/DatabaseManager.h"
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

VacationWidget::VacationWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("🏖️  الإجازات");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  auto *topBar = new QHBoxLayout;
  m_empCombo = new QComboBox;
  m_empCombo->addItem("كل الموظفين", -1);
  auto eq = DatabaseManager::instance().getEmployees();
  while (eq.next())
    m_empCombo->addItem(eq.value("name").toString(), eq.value("id").toInt());
  topBar->addWidget(m_empCombo);
  auto *filterBtn = new QPushButton("🔍  عرض");
  filterBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(filterBtn);
  topBar->addStretch();
  auto *addBtn = new QPushButton("➕  إضافة إجازة");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(addBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(6);
  m_table->setHorizontalHeaderLabels(
      {"#", "الموظف", "النوع", "من", "إلى", "ملاحظات"});
  m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(filterBtn, &QPushButton::clicked, this, &VacationWidget::loadData);
  connect(addBtn, &QPushButton::clicked, this, &VacationWidget::onAdd);
  loadData();
}

void VacationWidget::refresh() { loadData(); }

void VacationWidget::loadData() {
  m_table->setRowCount(0);
  int empId = m_empCombo->currentData().toInt();
  auto q = DatabaseManager::instance().getVacations(empId);
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("id").toString()));
    m_table->setItem(r, 1, mi(q.value("employee_name").toString()));
    QString type = q.value("type").toString();
    m_table->setItem(r, 2,
                     mi(type == "annual"      ? "🏖️ سنوية"
                        : type == "sick"      ? "🏥 مرضية"
                        : type == "emergency" ? "⚡ طارئة"
                                              : "📋 " + type));
    m_table->setItem(r, 3, mi(q.value("date_from").toString()));
    m_table->setItem(r, 4, mi(q.value("date_to").toString()));
    m_table->setItem(r, 5, mi(q.value("notes").toString()));
  }
}

void VacationWidget::onAdd() {
  QDialog dlg(this);
  dlg.setWindowTitle("إضافة إجازة");
  dlg.setMinimumSize(420, 320);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *empCombo = new QComboBox;
  auto eq = DatabaseManager::instance().getEmployees();
  while (eq.next())
    empCombo->addItem(eq.value("name").toString(), eq.value("id").toInt());

  auto *typeCombo = new QComboBox;
  typeCombo->addItem("🏖️ سنوية", "annual");
  typeCombo->addItem("🏥 مرضية", "sick");
  typeCombo->addItem("⚡ طارئة", "emergency");
  typeCombo->addItem("📋 بدون راتب", "unpaid");

  auto *dateFrom = new QDateEdit(QDate::currentDate());
  dateFrom->setCalendarPopup(true);
  dateFrom->setDisplayFormat("yyyy/MM/dd");
  auto *dateTo = new QDateEdit(QDate::currentDate().addDays(1));
  dateTo->setCalendarPopup(true);
  dateTo->setDisplayFormat("yyyy/MM/dd");
  auto *notesEdit = new QLineEdit;

  form->addRow("👤 الموظف:", empCombo);
  form->addRow("📋 نوع الإجازة:", typeCombo);
  form->addRow("📅 من:", dateFrom);
  form->addRow("📅 إلى:", dateTo);
  form->addRow("📝 ملاحظات:", notesEdit);

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
    DatabaseManager::instance().addVacation(
        empCombo->currentData().toInt(), typeCombo->currentData().toString(),
        dateFrom->date().toString("yyyy-MM-dd"),
        dateTo->date().toString("yyyy-MM-dd"), notesEdit->text());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}
