#include "ExpensesWidget.h"
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
#include <QVBoxLayout>

ExpensesWidget::ExpensesWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("💸  المصروفات");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  // Top bar - date filters
  auto *filterBar = new QHBoxLayout;

  auto *fromLabel = new QLabel("من:");
  fromLabel->setStyleSheet("background: transparent;");
  filterBar->addWidget(fromLabel);
  m_dateFrom = new QDateEdit(QDate::currentDate().addMonths(-1));
  m_dateFrom->setCalendarPopup(true);
  m_dateFrom->setDisplayFormat("yyyy/MM/dd");
  filterBar->addWidget(m_dateFrom);

  auto *toLabel = new QLabel("إلى:");
  toLabel->setStyleSheet("background: transparent;");
  filterBar->addWidget(toLabel);
  m_dateTo = new QDateEdit(QDate::currentDate());
  m_dateTo->setCalendarPopup(true);
  m_dateTo->setDisplayFormat("yyyy/MM/dd");
  filterBar->addWidget(m_dateTo);

  m_categoryCombo = new QComboBox;
  m_categoryCombo->addItem("كل التصنيفات", "");
  m_categoryCombo->addItem("إيجار", "إيجار");
  m_categoryCombo->addItem("رواتب", "رواتب");
  m_categoryCombo->addItem("كهرباء", "كهرباء");
  m_categoryCombo->addItem("ماء", "ماء");
  m_categoryCombo->addItem("هاتف وإنترنت", "هاتف وإنترنت");
  m_categoryCombo->addItem("صيانة", "صيانة");
  m_categoryCombo->addItem("نقل ومواصلات", "نقل ومواصلات");
  m_categoryCombo->addItem("مستلزمات مكتبية", "مستلزمات مكتبية");
  m_categoryCombo->addItem("تسويق وإعلان", "تسويق وإعلان");
  m_categoryCombo->addItem("أخرى", "أخرى");
  filterBar->addWidget(m_categoryCombo);

  auto *filterBtn = new QPushButton("🔍  عرض");
  filterBtn->setCursor(Qt::PointingHandCursor);
  filterBar->addWidget(filterBtn);

  filterBar->addStretch();

  auto *addBtn = new QPushButton("➕  إضافة مصروف");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  filterBar->addWidget(addBtn);

  auto *delBtn = new QPushButton("🗑️  حذف");
  delBtn->setObjectName("btnDanger");
  delBtn->setCursor(Qt::PointingHandCursor);
  filterBar->addWidget(delBtn);

  layout->addLayout(filterBar);

  // Table
  m_table = new QTableWidget;
  m_table->setColumnCount(6);
  m_table->setHorizontalHeaderLabels(
      {"#", "التاريخ", "التصنيف", "الوصف", "المبلغ", "ملاحظات"});
  m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  // Total bar
  auto *totalBar = new QHBoxLayout;
  totalBar->addStretch();
  auto *totalTitle = new QLabel("💰 إجمالي المصروفات:");
  totalTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; background: transparent;");
  totalBar->addWidget(totalTitle);
  m_totalLabel = new QLabel("0.00");
  m_totalLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: "
                              "#EF4444; background: transparent;");
  totalBar->addWidget(m_totalLabel);
  auto *currLabel =
      new QLabel(DatabaseManager::instance().getSetting("currency", "ريال"));
  currLabel->setStyleSheet("font-size: 16px; background: transparent;");
  totalBar->addWidget(currLabel);
  layout->addLayout(totalBar);

  // Connections
  connect(filterBtn, &QPushButton::clicked, this, &ExpensesWidget::onFilter);
  connect(m_dateFrom, &QDateEdit::dateChanged, this, &ExpensesWidget::onFilter);
  connect(m_dateTo, &QDateEdit::dateChanged, this, &ExpensesWidget::onFilter);
  connect(addBtn, &QPushButton::clicked, this, &ExpensesWidget::onAdd);
  connect(delBtn, &QPushButton::clicked, this, &ExpensesWidget::onDelete);

  loadExpenses();
}

void ExpensesWidget::refresh() { loadExpenses(); }

void ExpensesWidget::loadExpenses() {
  m_table->setRowCount(0);
  double total = 0;

  QString dateFrom = m_dateFrom->date().toString("yyyy-MM-dd");
  QString dateTo = m_dateTo->date().toString("yyyy-MM-dd");
  QString catFilter = m_categoryCombo->currentData().toString();

  auto q = DatabaseManager::instance().getExpenses(dateFrom, dateTo);
  while (q.next()) {
    QString cat = q.value("category").toString();
    // Apply category filter
    if (!catFilter.isEmpty() && cat != catFilter)
      continue;

    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("id").toString()));
    m_table->setItem(r, 1, mi(q.value("date").toString().left(10)));
    m_table->setItem(r, 2, mi(cat));
    m_table->setItem(r, 3, mi(q.value("description").toString()));

    double amount = q.value("amount").toDouble();
    total += amount;
    auto *amtItem = mi(QString::number(amount, 'f', 2));
    amtItem->setForeground(QColor("#EF4444"));
    m_table->setItem(r, 4, amtItem);
    m_table->setItem(r, 5, mi(q.value("notes").toString()));
  }
  m_totalLabel->setText(QString::number(total, 'f', 2));
}

void ExpensesWidget::onFilter() { loadExpenses(); }

void ExpensesWidget::onAdd() {
  QDialog dlg(this);
  dlg.setWindowTitle("إضافة مصروف جديد");
  dlg.setMinimumSize(420, 320);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *catCombo = new QComboBox;
  catCombo->addItem("إيجار");
  catCombo->addItem("رواتب");
  catCombo->addItem("كهرباء");
  catCombo->addItem("ماء");
  catCombo->addItem("هاتف وإنترنت");
  catCombo->addItem("صيانة");
  catCombo->addItem("نقل ومواصلات");
  catCombo->addItem("مستلزمات مكتبية");
  catCombo->addItem("تسويق وإعلان");
  catCombo->addItem("أخرى");
  catCombo->setEditable(true);

  auto *descEdit = new QLineEdit;
  descEdit->setPlaceholderText("وصف المصروف...");

  auto *amountSpin = new QDoubleSpinBox;
  amountSpin->setMaximum(9999999);
  amountSpin->setDecimals(2);
  amountSpin->setSuffix(
      " " + DatabaseManager::instance().getSetting("currency", "ريال"));

  auto *notesEdit = new QLineEdit;
  notesEdit->setPlaceholderText("ملاحظات (اختياري)...");

  form->addRow("📂 التصنيف:", catCombo);
  form->addRow("📝 الوصف:", descEdit);
  form->addRow("💰 المبلغ:", amountSpin);
  form->addRow("📌 ملاحظات:", notesEdit);

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
    if (descEdit->text().isEmpty()) {
      QMessageBox::warning(&dlg, "تنبيه", "أدخل وصف المصروف");
      return;
    }
    if (amountSpin->value() <= 0) {
      QMessageBox::warning(&dlg, "تنبيه", "أدخل المبلغ");
      return;
    }
    DatabaseManager::instance().addExpense(
        catCombo->currentText(), descEdit->text(), amountSpin->value(),
        notesEdit->text());
    dlg.accept();
  });

  if (dlg.exec() == QDialog::Accepted)
    loadExpenses();
}

void ExpensesWidget::onDelete() {
  if (m_table->currentRow() < 0)
    return;
  if (QMessageBox::question(this, "تأكيد",
                            "هل أنت متأكد من حذف هذا المصروف؟") ==
      QMessageBox::Yes) {
    int id = m_table->item(m_table->currentRow(), 0)->text().toInt();
    QSqlQuery q(DatabaseManager::instance().database());
    q.prepare("DELETE FROM Expenses WHERE id = ?");
    q.addBindValue(id);
    q.exec();
    loadExpenses();
  }
}
