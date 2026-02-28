#include "BookingWidget.h"
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

BookingWidget::BookingWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("📋  فواتير الحجز");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  auto *topBar = new QHBoxLayout;
  auto *fromLabel = new QLabel("من:");
  fromLabel->setStyleSheet("background: transparent;");
  topBar->addWidget(fromLabel);
  m_dateFrom = new QDateEdit(QDate::currentDate().addMonths(-1));
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
  auto *addBtn = new QPushButton("➕  حجز جديد");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(addBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(7);
  m_table->setHorizontalHeaderLabels(
      {"#", "رقم الحجز", "التاريخ", "العميل", "الإجمالي", "العربون", "الحالة"});
  m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(filterBtn, &QPushButton::clicked, this, &BookingWidget::loadData);
  connect(addBtn, &QPushButton::clicked, this, &BookingWidget::onAdd);
  loadData();
}

void BookingWidget::refresh() { loadData(); }

void BookingWidget::loadData() {
  m_table->setRowCount(0);
  auto q = DatabaseManager::instance().getBookings(
      m_dateFrom->date().toString("yyyy-MM-dd"),
      m_dateTo->date().toString("yyyy-MM-dd"));
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("id").toString()));
    m_table->setItem(r, 1, mi(q.value("booking_no").toString()));
    m_table->setItem(r, 2, mi(q.value("date").toString().left(10)));
    m_table->setItem(r, 3, mi(q.value("customer_name").toString()));
    m_table->setItem(r, 4,
                     mi(QString::number(q.value("total").toDouble(), 'f', 2)));
    m_table->setItem(
        r, 5, mi(QString::number(q.value("deposit").toDouble(), 'f', 2)));
    QString status = q.value("status").toString();
    auto *si = mi(status == "pending"     ? "⏳ قيد الانتظار"
                  : status == "delivered" ? "✅ تم التسليم"
                                          : "❌ ملغي");
    si->setForeground(status == "pending"     ? QColor("#F59E0B")
                      : status == "delivered" ? QColor("#22C55E")
                                              : QColor("#EF4444"));
    m_table->setItem(r, 6, si);
  }
}

void BookingWidget::onAdd() {
  QDialog dlg(this);
  dlg.setWindowTitle("حجز جديد");
  dlg.setMinimumSize(420, 300);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *custCombo = new QComboBox;
  auto cq = DatabaseManager::instance().getCustomers();
  while (cq.next())
    custCombo->addItem(cq.value("name").toString(), cq.value("id").toInt());

  auto *totalSpin = new QDoubleSpinBox;
  totalSpin->setMaximum(9999999);
  totalSpin->setDecimals(2);
  auto *depositSpin = new QDoubleSpinBox;
  depositSpin->setMaximum(9999999);
  depositSpin->setDecimals(2);
  auto *notesEdit = new QLineEdit;

  form->addRow("👤 العميل:", custCombo);
  form->addRow("💰 الإجمالي:", totalSpin);
  form->addRow("💵 العربون:", depositSpin);
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
    DatabaseManager::instance().createBooking(
        custCombo->currentData().toInt(), totalSpin->value(),
        depositSpin->value(), "pending", notesEdit->text());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}
