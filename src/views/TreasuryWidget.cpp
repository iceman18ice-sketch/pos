#include "TreasuryWidget.h"
#include "database/DatabaseManager.h"
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
#include <QVBoxLayout>

TreasuryWidget::TreasuryWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("💰  الخزينة");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  // Balance card
  auto *balCard = new QWidget;
  balCard->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                         "stop:0 #6C63FF, stop:1 #3B82F6); "
                         "border-radius: 12px; padding: 16px;");
  auto *balLayout = new QVBoxLayout(balCard);
  auto *balTitle = new QLabel("💰 رصيد الخزينة الحالي");
  balTitle->setStyleSheet("font-size: 14px; color: rgba(255,255,255,0.8); "
                          "background: transparent;");
  balLayout->addWidget(balTitle);
  m_balanceLabel = new QLabel("0.00 ريال");
  m_balanceLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: "
                                "white; background: transparent;");
  balLayout->addWidget(m_balanceLabel);
  layout->addWidget(balCard);

  // Top bar
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

  auto *inBtn = new QPushButton("📥  استلام نقدية");
  inBtn->setObjectName("btnSuccess");
  inBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(inBtn);
  auto *outBtn = new QPushButton("📤  دفع نقدية");
  outBtn->setObjectName("btnDanger");
  outBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(outBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(5);
  m_table->setHorizontalHeaderLabels(
      {"#", "التاريخ", "النوع", "المبلغ", "الوصف"});
  m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(filterBtn, &QPushButton::clicked, this, &TreasuryWidget::loadData);
  connect(inBtn, &QPushButton::clicked, this, &TreasuryWidget::onCashIn);
  connect(outBtn, &QPushButton::clicked, this, &TreasuryWidget::onCashOut);
  loadData();
}

void TreasuryWidget::refresh() { loadData(); }

void TreasuryWidget::loadData() {
  m_table->setRowCount(0);
  QString curr = DatabaseManager::instance().getSetting("currency", "ريال");
  double bal = DatabaseManager::instance().getTreasuryBalance();
  m_balanceLabel->setText(QString::number(bal, 'f', 2) + " " + curr);

  auto q = DatabaseManager::instance().getTreasuryTransactions(
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
    m_table->setItem(r, 1, mi(q.value("date").toString().left(10)));
    QString type = q.value("type").toString();
    auto *typeItem = mi(type == "in" ? "📥 استلام" : "📤 دفع");
    typeItem->setForeground(type == "in" ? QColor("#22C55E")
                                         : QColor("#EF4444"));
    m_table->setItem(r, 2, typeItem);
    auto *amtItem = mi(QString::number(q.value("amount").toDouble(), 'f', 2));
    amtItem->setForeground(type == "in" ? QColor("#22C55E")
                                        : QColor("#EF4444"));
    m_table->setItem(r, 3, amtItem);
    m_table->setItem(r, 4, mi(q.value("description").toString()));
  }
}

void TreasuryWidget::onCashIn() {
  QDialog dlg(this);
  dlg.setWindowTitle("📥 استلام نقدية");
  dlg.setMinimumSize(380, 220);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);
  auto *amountSpin = new QDoubleSpinBox;
  amountSpin->setMaximum(9999999);
  amountSpin->setDecimals(2);
  amountSpin->setSuffix(
      " " + DatabaseManager::instance().getSetting("currency", "ريال"));
  auto *descEdit = new QLineEdit;
  descEdit->setPlaceholderText("وصف العملية...");
  form->addRow("💰 المبلغ:", amountSpin);
  form->addRow("📝 الوصف:", descEdit);
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
    if (amountSpin->value() <= 0) {
      QMessageBox::warning(&dlg, "تنبيه", "أدخل المبلغ");
      return;
    }
    DatabaseManager::instance().addTreasuryTransaction(
        "in", amountSpin->value(), descEdit->text());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}

void TreasuryWidget::onCashOut() {
  QDialog dlg(this);
  dlg.setWindowTitle("📤 دفع نقدية");
  dlg.setMinimumSize(380, 220);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);
  auto *amountSpin = new QDoubleSpinBox;
  amountSpin->setMaximum(9999999);
  amountSpin->setDecimals(2);
  amountSpin->setSuffix(
      " " + DatabaseManager::instance().getSetting("currency", "ريال"));
  auto *descEdit = new QLineEdit;
  descEdit->setPlaceholderText("وصف العملية...");
  form->addRow("💰 المبلغ:", amountSpin);
  form->addRow("📝 الوصف:", descEdit);
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
    if (amountSpin->value() <= 0) {
      QMessageBox::warning(&dlg, "تنبيه", "أدخل المبلغ");
      return;
    }
    DatabaseManager::instance().addTreasuryTransaction(
        "out", amountSpin->value(), descEdit->text());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}
