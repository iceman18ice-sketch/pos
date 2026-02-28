#include "SalaryWidget.h"
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

SalaryWidget::SalaryWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("💰  الرواتب");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  auto *topBar = new QHBoxLayout;
  auto *monthLabel = new QLabel("الشهر:");
  monthLabel->setStyleSheet("background: transparent;");
  topBar->addWidget(monthLabel);
  m_monthSpin = new QSpinBox;
  m_monthSpin->setRange(1, 12);
  m_monthSpin->setValue(QDate::currentDate().month());
  topBar->addWidget(m_monthSpin);
  auto *yearLabel = new QLabel("السنة:");
  yearLabel->setStyleSheet("background: transparent;");
  topBar->addWidget(yearLabel);
  m_yearSpin = new QSpinBox;
  m_yearSpin->setRange(2020, 2040);
  m_yearSpin->setValue(QDate::currentDate().year());
  topBar->addWidget(m_yearSpin);
  auto *filterBtn = new QPushButton("🔍  عرض");
  filterBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(filterBtn);
  topBar->addStretch();
  auto *addBtn = new QPushButton("➕  صرف راتب");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(addBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(8);
  m_table->setHorizontalHeaderLabels({"#", "الموظف", "المنصب", "الراتب الأساسي",
                                      "البدلات", "الخصومات", "صافي الراتب",
                                      "ملاحظات"});
  m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(filterBtn, &QPushButton::clicked, this, &SalaryWidget::loadData);
  connect(addBtn, &QPushButton::clicked, this, &SalaryWidget::onAdd);
  loadData();
}

void SalaryWidget::refresh() { loadData(); }

void SalaryWidget::loadData() {
  m_table->setRowCount(0);
  auto q = DatabaseManager::instance().getSalaries(m_monthSpin->value(),
                                                   m_yearSpin->value());
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
    m_table->setItem(r, 2, mi(q.value("position").toString()));
    m_table->setItem(
        r, 3, mi(QString::number(q.value("basic_salary").toDouble(), 'f', 2)));
    m_table->setItem(
        r, 4, mi(QString::number(q.value("additions").toDouble(), 'f', 2)));
    auto *dedItem =
        mi(QString::number(q.value("deductions").toDouble(), 'f', 2));
    dedItem->setForeground(QColor("#EF4444"));
    m_table->setItem(r, 5, dedItem);
    auto *netItem =
        mi(QString::number(q.value("net_salary").toDouble(), 'f', 2));
    netItem->setForeground(QColor("#22C55E"));
    m_table->setItem(r, 6, netItem);
    m_table->setItem(r, 7, mi(q.value("notes").toString()));
  }
}

void SalaryWidget::onAdd() {
  QDialog dlg(this);
  dlg.setWindowTitle("صرف راتب");
  dlg.setMinimumSize(450, 380);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *empCombo = new QComboBox;
  auto eq = DatabaseManager::instance().getEmployees();
  while (eq.next())
    empCombo->addItem(eq.value("name").toString() + " - " +
                          eq.value("position").toString(),
                      eq.value("id").toInt());

  auto *basicSpin = new QDoubleSpinBox;
  basicSpin->setMaximum(999999);
  basicSpin->setDecimals(2);
  auto *addSpin = new QDoubleSpinBox;
  addSpin->setMaximum(999999);
  addSpin->setDecimals(2);
  auto *dedSpin = new QDoubleSpinBox;
  dedSpin->setMaximum(999999);
  dedSpin->setDecimals(2);
  auto *netLabel = new QLabel("0.00");
  netLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #22C55E; "
                          "background: transparent;");
  auto *notesEdit = new QLineEdit;

  auto updateNet = [&]() {
    double net = basicSpin->value() + addSpin->value() - dedSpin->value();
    netLabel->setText(QString::number(net, 'f', 2));
  };
  connect(basicSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          updateNet);
  connect(addSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          updateNet);
  connect(dedSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          updateNet);

  // Auto-fill salary from employee data
  connect(empCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [&]() {
    auto q2 = DatabaseManager::instance().getEmployees();
    while (q2.next()) {
      if (q2.value("id").toInt() == empCombo->currentData().toInt()) {
        basicSpin->setValue(q2.value("salary").toDouble());
        break;
      }
    }
  });
  if (empCombo->count() > 0)
    emit empCombo->currentIndexChanged(0);

  form->addRow("👤 الموظف:", empCombo);
  form->addRow("💰 الراتب الأساسي:", basicSpin);
  form->addRow("➕ البدلات:", addSpin);
  form->addRow("➖ الخصومات:", dedSpin);
  form->addRow("💵 صافي الراتب:", netLabel);
  form->addRow("📝 ملاحظات:", notesEdit);

  auto *btns = new QHBoxLayout;
  auto *saveBtn = new QPushButton("💾  صرف");
  saveBtn->setObjectName("btnSuccess");
  auto *cancelBtn = new QPushButton("❌  إلغاء");
  cancelBtn->setObjectName("btnDanger");
  btns->addWidget(saveBtn);
  btns->addWidget(cancelBtn);
  form->addRow(btns);

  connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
  connect(saveBtn, &QPushButton::clicked, [&]() {
    double net = basicSpin->value() + addSpin->value() - dedSpin->value();
    DatabaseManager::instance().addSalary(
        empCombo->currentData().toInt(), m_monthSpin->value(),
        m_yearSpin->value(), basicSpin->value(), addSpin->value(),
        dedSpin->value(), net, notesEdit->text());
    // Record treasury transaction
    DatabaseManager::instance().addTreasuryTransaction(
        "out", net, "راتب " + empCombo->currentText());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}
