#include "PurchaseReturnsWidget.h"
#include "database/DatabaseManager.h"
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

PurchaseReturnsWidget::PurchaseReturnsWidget(QWidget *parent)
    : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("🔄  مرتجعات المشتريات");
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
  auto *addBtn = new QPushButton("➕  إضافة مرتجع");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(addBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(7);
  m_table->setHorizontalHeaderLabels({"#", "رقم المرتجع", "التاريخ", "المورد",
                                      "الإجمالي", "الضريبة", "الصافي"});
  m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(filterBtn, &QPushButton::clicked, this,
          &PurchaseReturnsWidget::loadData);
  connect(addBtn, &QPushButton::clicked, this, &PurchaseReturnsWidget::onAdd);
  loadData();
}

void PurchaseReturnsWidget::refresh() { loadData(); }

void PurchaseReturnsWidget::loadData() {
  m_table->setRowCount(0);
  auto q = DatabaseManager::instance().getPurchaseReturns(
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
    m_table->setItem(r, 1, mi(q.value("return_no").toString()));
    m_table->setItem(r, 2, mi(q.value("date").toString().left(10)));
    m_table->setItem(r, 3, mi(q.value("supplier_name").toString()));
    m_table->setItem(
        r, 4, mi(QString::number(q.value("subtotal").toDouble(), 'f', 2)));
    m_table->setItem(
        r, 5, mi(QString::number(q.value("tax_value").toDouble(), 'f', 2)));
    auto *totalItem = mi(QString::number(q.value("total").toDouble(), 'f', 2));
    totalItem->setForeground(QColor("#EF4444"));
    m_table->setItem(r, 6, totalItem);
  }
}

void PurchaseReturnsWidget::onAdd() {
  QDialog dlg(this);
  dlg.setWindowTitle("إضافة مرتجع مشتريات");
  dlg.setMinimumSize(420, 350);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *invSpin = new QSpinBox;
  invSpin->setMaximum(999999);
  auto *suppCombo = new QComboBox;
  auto sq = DatabaseManager::instance().getCustomers(1);
  while (sq.next())
    suppCombo->addItem(sq.value("name").toString(), sq.value("id").toInt());

  auto *subtotalSpin = new QDoubleSpinBox;
  subtotalSpin->setMaximum(9999999);
  subtotalSpin->setDecimals(2);
  auto *taxSpin = new QDoubleSpinBox;
  taxSpin->setMaximum(9999999);
  taxSpin->setDecimals(2);
  auto *totalSpin = new QDoubleSpinBox;
  totalSpin->setMaximum(9999999);
  totalSpin->setDecimals(2);
  auto *notesEdit = new QLineEdit;

  form->addRow("رقم فاتورة الشراء:", invSpin);
  form->addRow("المورد:", suppCombo);
  form->addRow("المجموع:", subtotalSpin);
  form->addRow("الضريبة:", taxSpin);
  form->addRow("الإجمالي:", totalSpin);
  form->addRow("ملاحظات:", notesEdit);

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
    DatabaseManager::instance().createPurchaseReturn(
        invSpin->value(), suppCombo->currentData().toInt(),
        subtotalSpin->value(), taxSpin->value(), totalSpin->value(),
        notesEdit->text());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}
