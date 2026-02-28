#include "StockTransferWidget.h"
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
#include <QSpinBox>
#include <QVBoxLayout>

StockTransferWidget::StockTransferWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("🔀  تحويل أصناف بين المخازن");
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
  auto *addBtn = new QPushButton("➕  تحويل جديد");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(addBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(5);
  m_table->setHorizontalHeaderLabels(
      {"#", "رقم التحويل", "التاريخ", "من مخزن", "إلى مخزن"});
  m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(filterBtn, &QPushButton::clicked, this,
          &StockTransferWidget::loadData);
  connect(addBtn, &QPushButton::clicked, this, &StockTransferWidget::onAdd);
  loadData();
}

void StockTransferWidget::refresh() { loadData(); }

void StockTransferWidget::loadData() {
  m_table->setRowCount(0);
  auto q = DatabaseManager::instance().getStockTransfers(
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
    m_table->setItem(r, 1, mi(q.value("transfer_no").toString()));
    m_table->setItem(r, 2, mi(q.value("date").toString().left(10)));
    m_table->setItem(r, 3,
                     mi(QString::number(q.value("from_stock_id").toInt())));
    m_table->setItem(r, 4, mi(QString::number(q.value("to_stock_id").toInt())));
  }
}

void StockTransferWidget::onAdd() {
  QDialog dlg(this);
  dlg.setWindowTitle("تحويل أصناف بين المخازن");
  dlg.setMinimumSize(450, 350);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *fromCombo = new QComboBox;
  auto *toCombo = new QComboBox;
  auto sq = DatabaseManager::instance().getStocks();
  while (sq.next()) {
    fromCombo->addItem(sq.value("name").toString(), sq.value("id").toInt());
    toCombo->addItem(sq.value("name").toString(), sq.value("id").toInt());
  }
  if (fromCombo->count() == 0) {
    fromCombo->addItem("المخزن الرئيسي", 1);
    toCombo->addItem("المخزن الرئيسي", 1);
  }

  auto *prodSearch = new QLineEdit;
  prodSearch->setPlaceholderText("ابحث عن صنف...");
  auto *qtySpin = new QDoubleSpinBox;
  qtySpin->setMaximum(99999);
  qtySpin->setDecimals(2);
  qtySpin->setValue(1);
  auto *notesEdit = new QLineEdit;

  form->addRow("📦 من مخزن:", fromCombo);
  form->addRow("📦 إلى مخزن:", toCombo);
  form->addRow("🔍 الصنف:", prodSearch);
  form->addRow("📊 الكمية:", qtySpin);
  form->addRow("📝 ملاحظات:", notesEdit);

  auto *btns = new QHBoxLayout;
  auto *saveBtn = new QPushButton("💾  تحويل");
  saveBtn->setObjectName("btnSuccess");
  auto *cancelBtn = new QPushButton("❌  إلغاء");
  cancelBtn->setObjectName("btnDanger");
  btns->addWidget(saveBtn);
  btns->addWidget(cancelBtn);
  form->addRow(btns);

  connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
  connect(saveBtn, &QPushButton::clicked, [&]() {
    if (fromCombo->currentData() == toCombo->currentData()) {
      QMessageBox::warning(&dlg, "تنبيه", "لا يمكن التحويل لنفس المخزن");
      return;
    }
    auto &db = DatabaseManager::instance();
    auto pq = db.getProducts(prodSearch->text());
    if (pq.next()) {
      int tid = db.createStockTransfer(fromCombo->currentData().toInt(),
                                       toCombo->currentData().toInt(),
                                       notesEdit->text());
      if (tid > 0) {
        db.addStockTransferDetail(tid, pq.value("id").toInt(),
                                  pq.value("name").toString(),
                                  qtySpin->value());
        db.updateProductStock(pq.value("id").toInt(), qtySpin->value(), false);
        db.updateProductStock(pq.value("id").toInt(), qtySpin->value(), true);
        QMessageBox::information(&dlg, "✅ تم", "تم التحويل بنجاح");
        dlg.accept();
      }
    } else {
      QMessageBox::warning(&dlg, "تنبيه", "لم يتم العثور على الصنف");
    }
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}
