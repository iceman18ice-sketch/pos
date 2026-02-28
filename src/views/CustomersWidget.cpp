#include "CustomersWidget.h"
#include "database/DatabaseManager.h"
#include "widgets/SearchBar.h"
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


CustomersWidget::CustomersWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("👥  العملاء والموردين");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  auto *topBar = new QHBoxLayout;
  auto *search = new SearchBar("🔍  بحث...");
  search->setMinimumWidth(300);
  topBar->addWidget(search);

  m_typeCombo = new QComboBox;
  m_typeCombo->addItem("الكل", -1);
  m_typeCombo->addItem("عملاء", 0);
  m_typeCombo->addItem("موردين", 1);
  topBar->addWidget(m_typeCombo);
  topBar->addStretch();

  auto *addBtn = new QPushButton("➕  إضافة");
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
  m_table->setColumnCount(7);
  m_table->setHorizontalHeaderLabels({"#", "الاسم", "الهاتف", "النوع", "الرصيد",
                                      "حد الائتمان", "الرقم الضريبي"});
  m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  layout->addWidget(m_table, 1);

  connect(search, &QLineEdit::textChanged, this, &CustomersWidget::onSearch);
  connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this]() { loadCustomers("", m_typeCombo->currentData().toInt()); });
  connect(addBtn, &QPushButton::clicked, this, &CustomersWidget::onAdd);
  connect(editBtn, &QPushButton::clicked, this, &CustomersWidget::onEdit);
  connect(delBtn, &QPushButton::clicked, this, &CustomersWidget::onDelete);
  connect(m_table, &QTableWidget::doubleClicked, this,
          &CustomersWidget::onEdit);

  loadCustomers();
}

void CustomersWidget::refresh() { loadCustomers(); }

void CustomersWidget::loadCustomers(const QString &search, int type) {
  m_table->setRowCount(0);
  auto q = DatabaseManager::instance().getCustomers(type, search);
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
    int tp = q.value("type").toInt();
    m_table->setItem(r, 3, mi(tp == 0 ? "عميل" : tp == 1 ? "مورد" : "كلاهما"));
    auto *balItem = mi(QString::number(q.value("balance").toDouble(), 'f', 2));
    if (q.value("balance").toDouble() > 0)
      balItem->setForeground(QColor("#EF4444"));
    m_table->setItem(r, 4, balItem);
    m_table->setItem(
        r, 5, mi(QString::number(q.value("credit_limit").toDouble(), 'f', 2)));
    m_table->setItem(r, 6, mi(q.value("tax_number").toString()));
  }
}

void CustomersWidget::onSearch(const QString &text) {
  loadCustomers(text, m_typeCombo->currentData().toInt());
}

void CustomersWidget::showDialog(int id) {
  QDialog dlg(this);
  dlg.setWindowTitle(id > 0 ? "تعديل" : "إضافة جديد");
  dlg.setMinimumSize(400, 350);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(12);

  auto *nameEdit = new QLineEdit;
  auto *phoneEdit = new QLineEdit;
  auto *addressEdit = new QLineEdit;
  auto *typeCombo = new QComboBox;
  typeCombo->addItem("عميل", 0);
  typeCombo->addItem("مورد", 1);
  typeCombo->addItem("كلاهما", 2);
  auto *creditSpin = new QDoubleSpinBox;
  creditSpin->setMaximum(9999999);
  auto *taxEdit = new QLineEdit;

  if (id > 0) {
    auto q = DatabaseManager::instance().getCustomerById(id);
    if (q.next()) {
      nameEdit->setText(q.value("name").toString());
      phoneEdit->setText(q.value("phone").toString());
      addressEdit->setText(q.value("address").toString());
      typeCombo->setCurrentIndex(q.value("type").toInt());
      creditSpin->setValue(q.value("credit_limit").toDouble());
      taxEdit->setText(q.value("tax_number").toString());
    }
  }

  form->addRow("الاسم:", nameEdit);
  form->addRow("الهاتف:", phoneEdit);
  form->addRow("العنوان:", addressEdit);
  form->addRow("النوع:", typeCombo);
  form->addRow("حد الائتمان:", creditSpin);
  form->addRow("الرقم الضريبي:", taxEdit);

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
      db.updateCustomer(id, nameEdit->text(), phoneEdit->text(),
                        addressEdit->text(), typeCombo->currentData().toInt(),
                        creditSpin->value(), taxEdit->text());
    else
      db.addCustomer(nameEdit->text(), phoneEdit->text(), addressEdit->text(),
                     typeCombo->currentData().toInt(), creditSpin->value(),
                     taxEdit->text());
    dlg.accept();
  });
  if (dlg.exec() == QDialog::Accepted)
    loadCustomers();
}

void CustomersWidget::onAdd() { showDialog(); }
void CustomersWidget::onEdit() {
  if (m_table->currentRow() < 0)
    return;
  showDialog(m_table->item(m_table->currentRow(), 0)->text().toInt());
}
void CustomersWidget::onDelete() {
  if (m_table->currentRow() < 0)
    return;
  if (QMessageBox::question(this, "تأكيد", "هل أنت متأكد من الحذف؟") ==
      QMessageBox::Yes) {
    DatabaseManager::instance().deleteCustomer(
        m_table->item(m_table->currentRow(), 0)->text().toInt());
    loadCustomers();
  }
}
