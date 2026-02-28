#include "EmployeesWidget.h"
#include "database/DatabaseManager.h"
#include "widgets/SearchBar.h"
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

EmployeesWidget::EmployeesWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("👨‍💼  الموظفين");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  auto *topBar = new QHBoxLayout;
  auto *search = new SearchBar("🔍  بحث...");
  search->setMinimumWidth(300);
  topBar->addWidget(search);
  topBar->addStretch();

  auto *addBtn = new QPushButton("➕  إضافة موظف");
  addBtn->setObjectName("btnSuccess");
  addBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(addBtn);
  auto *editBtn = new QPushButton("✏️  تعديل");
  editBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(editBtn);
  auto *delBtn = new QPushButton("🗑️  حذف");
  delBtn->setObjectName("btnDanger");
  delBtn->setCursor(Qt::PointingHandCursor);
  topBar->addWidget(delBtn);
  layout->addLayout(topBar);

  m_table = new QTableWidget;
  m_table->setColumnCount(6);
  m_table->setHorizontalHeaderLabels(
      {"#", "الاسم", "الهاتف", "المنصب", "الراتب", "تاريخ التعيين"});
  m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(search, &QLineEdit::textChanged, [this](const QString &text) {
    m_table->setRowCount(0);
    auto q = DatabaseManager::instance().getEmployees(text);
    while (q.next()) {
      int r = m_table->rowCount();
      m_table->insertRow(r);
      auto mi = [](const QString &t) {
        auto *i = new QTableWidgetItem(t);
        i->setTextAlignment(Qt::AlignCenter);
        return i;
      };
      m_table->setItem(r, 0, mi(q.value("id").toString()));
      m_table->setItem(r, 1, mi(q.value("name").toString()));
      m_table->setItem(r, 2, mi(q.value("phone").toString()));
      m_table->setItem(r, 3, mi(q.value("position").toString()));
      m_table->setItem(
          r, 4, mi(QString::number(q.value("salary").toDouble(), 'f', 2)));
      m_table->setItem(r, 5, mi(q.value("start_date").toString()));
    }
  });
  connect(addBtn, &QPushButton::clicked, this, &EmployeesWidget::onAdd);
  connect(editBtn, &QPushButton::clicked, this, &EmployeesWidget::onEdit);
  connect(delBtn, &QPushButton::clicked, this, &EmployeesWidget::onDelete);
  connect(m_table, &QTableWidget::doubleClicked, this,
          &EmployeesWidget::onEdit);
  loadData();
}

void EmployeesWidget::refresh() { loadData(); }

void EmployeesWidget::loadData() {
  m_table->setRowCount(0);
  auto q = DatabaseManager::instance().getEmployees();
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("id").toString()));
    m_table->setItem(r, 1, mi(q.value("name").toString()));
    m_table->setItem(r, 2, mi(q.value("phone").toString()));
    m_table->setItem(r, 3, mi(q.value("position").toString()));
    m_table->setItem(r, 4,
                     mi(QString::number(q.value("salary").toDouble(), 'f', 2)));
    m_table->setItem(r, 5, mi(q.value("start_date").toString()));
  }
}

void EmployeesWidget::showDialog(int id) {
  QDialog dlg(this);
  dlg.setWindowTitle(id > 0 ? "تعديل موظف" : "إضافة موظف جديد");
  dlg.setMinimumSize(400, 320);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *nameEdit = new QLineEdit;
  auto *phoneEdit = new QLineEdit;
  auto *posEdit = new QLineEdit;
  auto *salarySpin = new QDoubleSpinBox;
  salarySpin->setMaximum(999999);
  salarySpin->setDecimals(2);
  auto *dateEdit = new QDateEdit(QDate::currentDate());
  dateEdit->setCalendarPopup(true);
  dateEdit->setDisplayFormat("yyyy/MM/dd");

  if (id > 0) {
    auto q = DatabaseManager::instance().getEmployees();
    while (q.next()) {
      if (q.value("id").toInt() == id) {
        nameEdit->setText(q.value("name").toString());
        phoneEdit->setText(q.value("phone").toString());
        posEdit->setText(q.value("position").toString());
        salarySpin->setValue(q.value("salary").toDouble());
        break;
      }
    }
  }

  form->addRow("👤 الاسم:", nameEdit);
  form->addRow("📞 الهاتف:", phoneEdit);
  form->addRow("💼 المنصب:", posEdit);
  form->addRow("💰 الراتب:", salarySpin);
  if (id <= 0)
    form->addRow("📅 تاريخ التعيين:", dateEdit);

  auto *btns = new QHBoxLayout;
  auto *saveBtn = new QPushButton(id > 0 ? "تحديث" : "حفظ");
  saveBtn->setObjectName("btnSuccess");
  auto *cancelBtn = new QPushButton("إلغاء");
  cancelBtn->setObjectName("btnDanger");
  btns->addWidget(saveBtn);
  btns->addWidget(cancelBtn);
  form->addRow(btns);

  connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
  connect(saveBtn, &QPushButton::clicked, [&]() {
    if (nameEdit->text().isEmpty()) {
      QMessageBox::warning(&dlg, "تنبيه", "أدخل الاسم");
      return;
    }
    auto &db = DatabaseManager::instance();
    if (id > 0)
      db.updateEmployee(id, nameEdit->text(), phoneEdit->text(),
                        posEdit->text(), salarySpin->value());
    else
      db.addEmployee(nameEdit->text(), phoneEdit->text(), posEdit->text(),
                     salarySpin->value(),
                     dateEdit->date().toString("yyyy-MM-dd"));
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadData();
}

void EmployeesWidget::onAdd() { showDialog(); }
void EmployeesWidget::onEdit() {
  if (m_table->currentRow() < 0)
    return;
  showDialog(m_table->item(m_table->currentRow(), 0)->text().toInt());
}
void EmployeesWidget::onDelete() {
  if (m_table->currentRow() < 0)
    return;
  if (QMessageBox::question(this, "تأكيد", "هل أنت متأكد من حذف هذا الموظف؟") ==
      QMessageBox::Yes) {
    DatabaseManager::instance().deleteEmployee(
        m_table->item(m_table->currentRow(), 0)->text().toInt());
    loadData();
  }
}
