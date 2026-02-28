#include "AttendanceWidget.h"
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
#include <QTimeEdit>
#include <QVBoxLayout>

AttendanceWidget::AttendanceWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("🕐  الحضور والانصراف");
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

  auto *fromLabel = new QLabel("من:");
  fromLabel->setStyleSheet("background: transparent;");
  topBar->addWidget(fromLabel);
  m_dateFrom = new QDateEdit(QDate::currentDate().addDays(-30));
  m_dateFrom->setCalendarPopup(true);
  m_dateFrom->setDisplayFormat("yyyy/MM/dd");
  topBar->addWidget(m_dateFrom);
  auto *toLabel = new QLabel("إلى:");
  toLabel->setStyleSheet("background: transparent;");
  topBar->addWidget(toLabel);
  m_dateTo = new QDateEdit(QDate::currentDate());
  m_dateTo->setCalendarPopup(true);
  m_dateTo->setDisplayFormat("yyyy/MM/dd");
  topBar->addWidget(m_dateTo);
  auto *filterBtn = new QPushButton("🔍  عرض");
  filterBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(filterBtn);
  topBar->addStretch();

  auto *addBtn = new QPushButton("➕  تسجيل حضور");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(addBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(6);
  m_table->setHorizontalHeaderLabels(
      {"#", "الموظف", "التاريخ", "وقت الحضور", "وقت الانصراف", "ملاحظات"});
  m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(filterBtn, &QPushButton::clicked, this, &AttendanceWidget::loadData);
  connect(addBtn, &QPushButton::clicked, this, &AttendanceWidget::onAdd);
  loadData();
}

void AttendanceWidget::refresh() { loadData(); }

void AttendanceWidget::loadData() {
  m_table->setRowCount(0);
  int empId = m_empCombo->currentData().toInt();
  auto q = DatabaseManager::instance().getAttendance(
      m_dateFrom->date().toString("yyyy-MM-dd"),
      m_dateTo->date().toString("yyyy-MM-dd"), empId);
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
    m_table->setItem(r, 2, mi(q.value("date").toString()));
    m_table->setItem(r, 3, mi(q.value("check_in").toString()));
    m_table->setItem(r, 4, mi(q.value("check_out").toString()));
    m_table->setItem(r, 5, mi(q.value("notes").toString()));
  }
}

void AttendanceWidget::onAdd() {
  QDialog dlg(this);
  dlg.setWindowTitle("تسجيل حضور وانصراف");
  dlg.setMinimumSize(400, 300);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *empCombo = new QComboBox;
  auto eq = DatabaseManager::instance().getEmployees();
  while (eq.next())
    empCombo->addItem(eq.value("name").toString(), eq.value("id").toInt());

  auto *dateEdit = new QDateEdit(QDate::currentDate());
  dateEdit->setCalendarPopup(true);
  dateEdit->setDisplayFormat("yyyy/MM/dd");
  auto *inTime = new QTimeEdit(QTime(8, 0));
  inTime->setDisplayFormat("hh:mm AP");
  auto *outTime = new QTimeEdit(QTime(17, 0));
  outTime->setDisplayFormat("hh:mm AP");
  auto *notesEdit = new QLineEdit;

  form->addRow("👤 الموظف:", empCombo);
  form->addRow("📅 التاريخ:", dateEdit);
  form->addRow("🟢 وقت الحضور:", inTime);
  form->addRow("🔴 وقت الانصراف:", outTime);
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
    if (empCombo->count() == 0) {
      QMessageBox::warning(&dlg, "تنبيه", "أضف موظفين أولاً");
      return;
    }
    DatabaseManager::instance().addAttendance(
        empCombo->currentData().toInt(),
        dateEdit->date().toString("yyyy-MM-dd"),
        inTime->time().toString("hh:mm"), outTime->time().toString("hh:mm"),
        notesEdit->text());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}
