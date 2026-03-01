#include "PurchasesWidget.h"
#include "database/DatabaseManager.h"
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

PurchasesWidget::PurchasesWidget(QWidget *parent) : QWidget(parent) {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(8);
  mainLayout->setContentsMargins(12, 8, 12, 8);
  setLayoutDirection(Qt::RightToLeft);

  // ====== Header: فاتورة مشتريات ======
  auto *headerBar = new QWidget;
  headerBar->setStyleSheet(
      "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
      "stop:0 #DC2626, stop:1 #B91C1C); border-radius: 8px; padding: 8px;");
  auto *headerLayout = new QHBoxLayout(headerBar);
  auto *headerTitle = new QLabel("🛒  فاتورة مشتريات");
  headerTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: "
                             "white; background: transparent;");
  headerLayout->addWidget(headerTitle);
  headerLayout->addStretch();
  m_invoiceNoLabel = new QLabel("رقم الفاتورة: ---");
  m_invoiceNoLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: "
                                  "#FEE2E2; background: transparent;");
  headerLayout->addWidget(m_invoiceNoLabel);
  mainLayout->addWidget(headerBar);

  // ====== Supplier Info Row ======
  auto *supplierGroup = new QGroupBox("بيانات المورد");
  supplierGroup->setStyleSheet(
      "QGroupBox { font-weight: bold; color: #E8EAED; border: 1px solid "
      "#374151; "
      "border-radius: 8px; margin-top: 6px; padding-top: 14px; } "
      "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 "
      "5px; }");
  auto *suppGrid = new QGridLayout(supplierGroup);
  suppGrid->setSpacing(8);

  // Row 1
  auto *suppLabel = new QLabel("اسم المورد:");
  suppLabel->setStyleSheet("background: transparent; font-weight: bold;");
  suppGrid->addWidget(suppLabel, 0, 0);
  m_supplierCombo = new QComboBox;
  m_supplierCombo->setMinimumWidth(200);
  m_supplierCombo->setEditable(true);
  suppGrid->addWidget(m_supplierCombo, 0, 1);

  auto *siLabel = new QLabel("رقم فاتورة المورد:");
  siLabel->setStyleSheet("background: transparent; font-weight: bold;");
  suppGrid->addWidget(siLabel, 0, 2);
  m_supplierInvEdit = new QLineEdit;
  m_supplierInvEdit->setPlaceholderText("رقم فاتورة المورد...");
  suppGrid->addWidget(m_supplierInvEdit, 0, 3);

  auto *payLabel = new QLabel("طريقة الدفع:");
  payLabel->setStyleSheet("background: transparent; font-weight: bold;");
  suppGrid->addWidget(payLabel, 0, 4);
  m_paymentCombo = new QComboBox;
  m_paymentCombo->addItems({"نقدي", "آجل", "شبكة", "تحويل بنكي"});
  suppGrid->addWidget(m_paymentCombo, 0, 5);

  // Row 2
  auto *stockLabel = new QLabel("المخزن:");
  stockLabel->setStyleSheet("background: transparent; font-weight: bold;");
  suppGrid->addWidget(stockLabel, 1, 0);
  m_stockCombo = new QComboBox;
  suppGrid->addWidget(m_stockCombo, 1, 1);

  mainLayout->addWidget(supplierGroup);

  // ====== Search + Add Product ======
  auto *searchRow = new QHBoxLayout;
  m_searchEdit = new QLineEdit;
  m_searchEdit->setObjectName("searchBar");
  m_searchEdit->setPlaceholderText(
      "🔍  ابحث بالاسم أو الباركود واضغط Enter...");
  m_searchEdit->setAlignment(Qt::AlignRight);
  m_searchEdit->setMinimumHeight(40);
  m_searchEdit->setStyleSheet("font-size: 14px; padding: 8px 12px;");
  searchRow->addWidget(m_searchEdit, 1);

  auto *uploadBtn = new QPushButton("📄 رفع فاتورة");
  uploadBtn->setObjectName("btnPrimary");
  uploadBtn->setCursor(Qt::PointingHandCursor);
  uploadBtn->setMinimumHeight(40);
  uploadBtn->setToolTip("رفع فاتورة من ملف Excel أو CSV لإدخال الأصناف تلقائياً");
  searchRow->addWidget(uploadBtn);

  auto *addProdBtn = new QPushButton("➕ إضافة صنف جديد");
  addProdBtn->setObjectName("btnSuccess");
  addProdBtn->setCursor(Qt::PointingHandCursor);
  addProdBtn->setMinimumHeight(40);
  searchRow->addWidget(addProdBtn);
  mainLayout->addLayout(searchRow);

  // ====== Invoice Table ======
  m_table = new InvoiceTable;
  mainLayout->addWidget(m_table, 1);

  // ====== Bottom: Totals + Actions ======
  auto *bottomLayout = new QHBoxLayout;

  // Left: Summary
  auto *summaryGroup = new QGroupBox("ملخص الفاتورة");
  summaryGroup->setStyleSheet(
      "QGroupBox { font-weight: bold; color: #E8EAED; border: 1px solid "
      "#374151; "
      "border-radius: 8px; margin-top: 6px; padding-top: 14px; } "
      "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 "
      "5px; }");
  auto *summaryLayout = new QGridLayout(summaryGroup);
  summaryLayout->setSpacing(8);

  auto addRow = [&](int row, const QString &label, QLabel *&valueLabel,
                    const QString &color = "#E8EAED") {
    auto *lbl = new QLabel(label);
    lbl->setStyleSheet("font-size: 13px; font-weight: bold; color: #9CA3AF; "
                       "background: transparent;");
    summaryLayout->addWidget(lbl, row, 0);
    valueLabel = new QLabel("0.00 ريال");
    valueLabel->setStyleSheet(QString("font-size: 14px; font-weight: bold; "
                                      "color: %1; background: transparent;")
                                  .arg(color));
    valueLabel->setAlignment(Qt::AlignLeft);
    summaryLayout->addWidget(valueLabel, row, 1);
  };
  addRow(0, "الإجمالي قبل الخصم:", m_subtotalLabel);

  auto *discLabel = new QLabel("الخصم %:");
  discLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: "
                           "#9CA3AF; background: transparent;");
  summaryLayout->addWidget(discLabel, 1, 0);
  m_discountEdit = new QLineEdit("0");
  m_discountEdit->setMaximumWidth(100);
  summaryLayout->addWidget(m_discountEdit, 1, 1);

  addRow(2, "قيمة الخصم:", m_discountTotalLabel, "#F59E0B");
  addRow(3, "الإجمالي بعد الخصم:", m_afterDiscountLabel);
  addRow(4, "الضريبة (15%):", m_taxLabel, "#F59E0B");
  addRow(5, "الإجمالي بعد الضريبة:", m_totalLabel, "#10B981");

  auto *paidLabel = new QLabel("المدفوع:");
  paidLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: "
                           "#9CA3AF; background: transparent;");
  summaryLayout->addWidget(paidLabel, 6, 0);
  m_paidEdit = new QLineEdit("0");
  summaryLayout->addWidget(m_paidEdit, 6, 1);

  addRow(7, "المتبقي:", m_remainingLabel, "#EF4444");
  bottomLayout->addWidget(summaryGroup);

  // Middle: Notes
  auto *notesGroup = new QGroupBox("ملاحظات");
  notesGroup->setStyleSheet(
      "QGroupBox { font-weight: bold; color: #E8EAED; border: 1px solid "
      "#374151; "
      "border-radius: 8px; margin-top: 6px; padding-top: 14px; } "
      "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 "
      "5px; }");
  auto *notesLayout = new QVBoxLayout(notesGroup);
  m_notesEdit = new QLineEdit;
  m_notesEdit->setPlaceholderText("ملاحظات على الفاتورة...");
  m_notesEdit->setMinimumHeight(40);
  notesLayout->addWidget(m_notesEdit);

  auto *shortcutsLabel = new QLabel("⌨️ اختصارات:\n"
                                    "F1 = حفظ\n"
                                    "F2 = فاتورة جديدة\n"
                                    "F4 = البحث");
  shortcutsLabel->setStyleSheet("font-size: 11px; color: #6B7280; background: "
                                "transparent; line-height: 1.6;");
  notesLayout->addWidget(shortcutsLabel);
  notesLayout->addStretch();
  bottomLayout->addWidget(notesGroup);

  // Right: Buttons
  auto *btnLayout = new QVBoxLayout;
  btnLayout->setSpacing(8);

  auto *saveBtn = new QPushButton("✅ حفظ  (F1)");
  saveBtn->setObjectName("btnSuccess");
  saveBtn->setCursor(Qt::PointingHandCursor);
  saveBtn->setMinimumHeight(55);
  saveBtn->setStyleSheet("font-size: 16px; font-weight: bold;");
  btnLayout->addWidget(saveBtn);

  auto *savePrintBtn = new QPushButton("🖨️  حفظ وطباعة  (F2)");
  savePrintBtn->setObjectName("btnOutline");
  savePrintBtn->setCursor(Qt::PointingHandCursor);
  savePrintBtn->setMinimumHeight(45);
  btnLayout->addWidget(savePrintBtn);

  auto *newBtn = new QPushButton("📄  فاتورة جديدة");
  newBtn->setObjectName("btnOutline");
  newBtn->setCursor(Qt::PointingHandCursor);
  newBtn->setMinimumHeight(45);
  btnLayout->addWidget(newBtn);

  btnLayout->addStretch();
  bottomLayout->addLayout(btnLayout);
  mainLayout->addLayout(bottomLayout);

  // ====== Load Data ======
  loadSuppliers();
  loadStocks();
  onNewInvoice();

  // ====== Connections ======
  connect(m_searchEdit, &QLineEdit::returnPressed, this,
          &PurchasesWidget::onSearchProduct);
  connect(saveBtn, &QPushButton::clicked, this,
          &PurchasesWidget::onSaveInvoice);
  connect(savePrintBtn, &QPushButton::clicked, [this]() { onSaveInvoice(); });
  connect(newBtn, &QPushButton::clicked, this, &PurchasesWidget::onNewInvoice);
  connect(uploadBtn, &QPushButton::clicked, this,
          &PurchasesWidget::onUploadInvoice);
  connect(addProdBtn, &QPushButton::clicked, this,
          &PurchasesWidget::onAddNewProduct);
  connect(m_discountEdit, &QLineEdit::textChanged,
          [this]() { updateTotals(); });
  connect(m_paidEdit, &QLineEdit::textChanged, [this]() { updateTotals(); });
  connect(m_table, &QTableWidget::cellChanged, [this]() { updateTotals(); });
}

void PurchasesWidget::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_F1:
    onSaveInvoice();
    break;
  case Qt::Key_F2:
    onNewInvoice();
    break;
  case Qt::Key_F4:
    m_searchEdit->setFocus();
    m_searchEdit->selectAll();
    break;
  default:
    QWidget::keyPressEvent(event);
  }
}

void PurchasesWidget::loadSuppliers() {
  m_supplierCombo->clear();
  auto q = DatabaseManager::instance().getCustomers(1);
  while (q.next())
    m_supplierCombo->addItem(q.value("name").toString(), q.value("id").toInt());
  if (m_supplierCombo->count() == 0)
    m_supplierCombo->addItem("مورد افتراضي", 1);
}

void PurchasesWidget::loadStocks() {
  m_stockCombo->clear();
  auto q = DatabaseManager::instance().getStocks();
  while (q.next())
    m_stockCombo->addItem(q.value("name").toString(), q.value("id").toInt());
  if (m_stockCombo->count() == 0)
    m_stockCombo->addItem("المخزن الرئيسي", 1);
}

void PurchasesWidget::onSearchProduct() {
  QString s = m_searchEdit->text().trimmed();
  if (s.isEmpty())
    return;
  auto &db = DatabaseManager::instance();
  auto q = db.getProductByBarcode(s);
  if (!q.next()) {
    q = db.getProducts(s);
    q.next();
  }
  if (q.isValid()) {
    m_table->addItem(q.value("id").toInt(), q.value("name").toString(), 1,
                     q.value("buy_price").toDouble(),
                     q.value("tax_rate").toDouble());
    m_searchEdit->clear();
    updateTotals();
  } else {
    QMessageBox::warning(this, "تنبيه", "لم يتم العثور على الصنف: " + s);
  }
}

void PurchasesWidget::onSaveInvoice() {
  if (m_table->rowCount() == 0) {
    QMessageBox::warning(this, "تنبيه", "لا توجد أصناف في الفاتورة!");
    return;
  }
  auto &db = DatabaseManager::instance();
  double sub = m_table->getSubtotal();
  double discRate = m_discountEdit->text().toDouble();
  double discValue = sub * discRate / 100.0;
  double afterDisc = sub - discValue;
  double tax = m_table->getTaxTotal();
  double total = afterDisc + tax;
  double paid = m_paidEdit->text().toDouble();
  double rem = total - paid;
  int suppId = m_supplierCombo->currentData().toInt();
  int stockId = m_stockCombo->currentData().toInt();
  QString payType = m_paymentCombo->currentText();
  int invId = db.createPurchaseInvoice(
      suppId, stockId, sub, discRate, discValue, tax, total, paid, rem,
      m_supplierInvEdit->text(), payType, m_notesEdit->text());
  if (invId > 0) {
    for (const auto &item : m_table->getAllItems()) {
      db.addPurchaseInvoiceDetail(invId, item.productId, item.name, item.qty,
                                  item.price, 0, item.discount, item.taxRate,
                                  item.taxValue, item.total);
      db.updateProductStock(item.productId, item.qty, true);
      db.addStockMovement(item.productId, stockId, "in", item.qty, "purchase",
                          invId, "");
    }
    // Record treasury transaction
    db.addTreasuryTransaction("out", paid,
                              "فاتورة شراء رقم " + QString::number(m_invoiceNo),
                              "purchase", invId);
    QMessageBox::information(this, "✅ تم الحفظ",
                             "تم حفظ فاتورة الشراء رقم " +
                                 QString::number(m_invoiceNo) +
                                 " بنجاح!\n"
                                 "الإجمالي: " +
                                 QString::number(total, 'f', 2) + " ريال");
    onNewInvoice();
  }
}

void PurchasesWidget::onNewInvoice() {
  m_table->clearAll();
  m_invoiceNo = DatabaseManager::instance().getNextPurchaseInvoiceNo();
  m_invoiceNoLabel->setText("رقم الفاتورة: " + QString::number(m_invoiceNo));
  m_paidEdit->setText("0");
  m_discountEdit->setText("0");
  m_notesEdit->clear();
  m_supplierInvEdit->clear();
  m_searchEdit->clear();
  m_searchEdit->setFocus();
  updateTotals();
}

void PurchasesWidget::updateTotals() {
  double sub = m_table->getSubtotal();
  double discRate = m_discountEdit->text().toDouble();
  double discValue = sub * discRate / 100.0;
  double afterDisc = sub - discValue;
  double tax = m_table->getTaxTotal();
  double total = afterDisc + tax;
  double paid = m_paidEdit->text().toDouble();
  double rem = total - paid;

  QString curr = DatabaseManager::instance().getSetting("currency", "ريال");
  m_subtotalLabel->setText(QString::number(sub, 'f', 2) + " " + curr);
  m_discountTotalLabel->setText(QString::number(discValue, 'f', 2) + " " +
                                curr);
  m_afterDiscountLabel->setText(QString::number(afterDisc, 'f', 2) + " " +
                                curr);
  m_taxLabel->setText(QString::number(tax, 'f', 2) + " " + curr);
  m_totalLabel->setText(QString::number(total, 'f', 2) + " " + curr);
  m_remainingLabel->setText(QString::number(rem, 'f', 2) + " " + curr);
}

void PurchasesWidget::onAddNewProduct() {
  QDialog dlg(this);
  dlg.setWindowTitle("إضافة صنف جديد");
  dlg.setMinimumSize(400, 300);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *form = new QFormLayout(&dlg);
  form->setSpacing(10);

  auto *nameEdit = new QLineEdit;
  auto *barcodeEdit = new QLineEdit;
  auto *buyPriceSpin = new QDoubleSpinBox;
  buyPriceSpin->setMaximum(999999);
  buyPriceSpin->setDecimals(2);
  auto *sellPriceSpin = new QDoubleSpinBox;
  sellPriceSpin->setMaximum(999999);
  sellPriceSpin->setDecimals(2);

  form->addRow("📦 اسم الصنف:", nameEdit);
  form->addRow("🏷️ الباركود:", barcodeEdit);
  form->addRow("💰 سعر الشراء:", buyPriceSpin);
  form->addRow("💵 سعر البيع:", sellPriceSpin);

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
    if (nameEdit->text().isEmpty()) {
      QMessageBox::warning(&dlg, "تنبيه", "أدخل اسم الصنف");
      return;
    }
    auto &db = DatabaseManager::instance();
    if (db.addProduct(nameEdit->text(), barcodeEdit->text(), 1, 1,
                      buyPriceSpin->value(), sellPriceSpin->value(), 15, 0)) {
      QMessageBox::information(&dlg, "✅ تم", "تم إضافة الصنف بنجاح!");
      dlg.accept();
    }
  });
  dlg.exec();
}

void PurchasesWidget::onUploadInvoice() {
  QString file = QFileDialog::getOpenFileName(
      this, "اختر ملف فاتورة المشتريات", QDir::homePath(),
      "Excel & CSV (*.xlsx *.xls *.csv);;All Files (*)");
  if (file.isEmpty())
    return;

  // Run Python parser
  QString script =
      QCoreApplication::applicationDirPath() + "/parse_invoice.py";
  if (!QFile::exists(script)) {
    QMessageBox::critical(this, "خطأ",
                          "ملف parse_invoice.py غير موجود في مجلد البرنامج!");
    return;
  }

  QProcess proc;
  proc.setProcessChannelMode(QProcess::MergedChannels);
  proc.start("python", {"-u", script, file});

  QProgressDialog progress("⏳ جاري قراءة الفاتورة...", "إلغاء", 0, 0, this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setLayoutDirection(Qt::RightToLeft);
  progress.show();

  if (!proc.waitForFinished(60000)) {
    progress.close();
    QMessageBox::critical(this, "خطأ", "انتهت مهلة قراءة الملف!");
    return;
  }
  progress.close();

  QString output = QString::fromUtf8(proc.readAll());
  QStringList lines = output.split('\n', Qt::SkipEmptyParts);

  if (lines.isEmpty() || proc.exitCode() != 0) {
    QMessageBox::critical(this, "خطأ",
                          "فشل قراءة الملف!\n\n" + output);
    return;
  }

  auto &db = DatabaseManager::instance();
  int added = 0, notFound = 0;
  QStringList notFoundList;

  for (const QString &line : lines) {
    if (line.startsWith("ITEMS:") || line.startsWith("END") ||
        line.startsWith("ERROR"))
      continue;

    QStringList parts = line.trimmed().split('|');
    if (parts.size() < 4)
      continue;

    QString barcode = parts[0].trimmed();
    QString name = parts[1].trimmed();
    double qty = parts[2].trimmed().toDouble();
    double price = parts[3].trimmed().toDouble();
    if (qty <= 0)
      qty = 1;

    // Try to find product by barcode first, then by name
    bool found = false;
    QSqlQuery q;

    if (!barcode.isEmpty()) {
      q = db.getProductByBarcode(barcode);
      if (q.next()) {
        int pid = q.value("id").toInt();
        QString pname = q.value("name").toString();
        double buyPrice = price > 0 ? price : q.value("buy_price").toDouble();
        double tax = q.value("tax_rate").toDouble();
        m_table->addItem(pid, pname, qty, buyPrice, tax);
        added++;
        found = true;
      }
    }

    if (!found && !name.isEmpty()) {
      q = db.getProducts(name);
      if (q.next()) {
        int pid = q.value("id").toInt();
        QString pname = q.value("name").toString();
        double buyPrice = price > 0 ? price : q.value("buy_price").toDouble();
        double tax = q.value("tax_rate").toDouble();
        m_table->addItem(pid, pname, qty, buyPrice, tax);
        added++;
        found = true;
      }
    }

    if (!found) {
      notFound++;
      QString display = name.isEmpty() ? barcode : name;
      if (notFoundList.size() < 20)
        notFoundList << display;
    }
  }

  updateTotals();

  // Show result
  QString msg = QString("✅ تم إضافة %1 صنف من الفاتورة.").arg(added);
  if (notFound > 0) {
    msg += QString("\n\n⚠️ %1 صنف لم يتم العثور عليه في قاعدة البيانات:\n")
               .arg(notFound);
    msg += notFoundList.join("\n");
    if (notFound > 20)
      msg += "\n...";
  }

  QMessageBox::information(this, "نتيجة الرفع", msg);
}
