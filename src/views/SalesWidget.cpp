#include "SalesWidget.h"
#include "database/DatabaseManager.h"
#include "devices/CashDrawer.h"
#include "utils/ZatcaQR.h"
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

SalesWidget::SalesWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  // ===== Top bar =====
  auto *topBar = new QHBoxLayout;

  m_invoiceNoLabel = new QLabel("فاتورة رقم: ---");
  m_invoiceNoLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: "
                                  "#6C63FF; background: transparent;");
  topBar->addWidget(m_invoiceNoLabel);
  topBar->addStretch();

  // Hold / Resume buttons
  m_holdBtn = new QPushButton("⏸️  تعليق الفاتورة");
  m_holdBtn->setObjectName("btnOutline");
  m_holdBtn->setCursor(Qt::PointingHandCursor);
  m_holdBtn->setMinimumHeight(38);
  m_holdBtn->setStyleSheet(
      "QPushButton { padding: 6px 16px; font-size: 13px; "
      "border: 2px solid #F59E0B; color: #F59E0B; "
      "border-radius: 6px; background: transparent; }"
      "QPushButton:hover { background: rgba(245,158,11,0.15); }");
  topBar->addWidget(m_holdBtn);

  m_resumeBtn = new QPushButton("▶️  استرجاع فاتورة (0)");
  m_resumeBtn->setCursor(Qt::PointingHandCursor);
  m_resumeBtn->setMinimumHeight(38);
  m_resumeBtn->setStyleSheet(
      "QPushButton { padding: 6px 16px; font-size: 13px; "
      "border: 2px solid #10B981; color: #10B981; "
      "border-radius: 6px; background: transparent; }"
      "QPushButton:hover { background: rgba(16,185,129,0.15); }");
  m_resumeBtn->setEnabled(false);
  topBar->addWidget(m_resumeBtn);

  // Previous invoice button
  auto *prevInvBtn = new QPushButton("🔍  الفاتورة السابقة (F5)");
  prevInvBtn->setCursor(Qt::PointingHandCursor);
  prevInvBtn->setMinimumHeight(38);
  prevInvBtn->setShortcut(QKeySequence(Qt::Key_F5));
  prevInvBtn->setStyleSheet(
      "QPushButton { padding: 6px 16px; font-size: 13px; "
      "border: 2px solid #6C63FF; color: #6C63FF; "
      "border-radius: 6px; background: transparent; }"
      "QPushButton:hover { background: rgba(108,99,255,0.15); }");
  topBar->addWidget(prevInvBtn);

  layout->addLayout(topBar);

  // ===== Second row: customer, payment method, warehouse, treasury =====
  auto *detailsBar = new QHBoxLayout;

  auto *custLabel = new QLabel("العميل:");
  custLabel->setStyleSheet("background: transparent; font-weight: bold;");
  detailsBar->addWidget(custLabel);
  m_customerCombo = new QComboBox;
  m_customerCombo->setMinimumWidth(180);
  detailsBar->addWidget(m_customerCombo);

  detailsBar->addSpacing(15);

  auto *payLabel = new QLabel("طريقة الدفع:");
  payLabel->setStyleSheet("background: transparent; font-weight: bold;");
  detailsBar->addWidget(payLabel);
  m_paymentMethodCombo = new QComboBox;
  m_paymentMethodCombo->addItems({"كاش", "شبكة", "تحويل بنكي", "آجل"});
  m_paymentMethodCombo->setMinimumWidth(120);
  detailsBar->addWidget(m_paymentMethodCombo);

  detailsBar->addSpacing(15);

  auto *whLabel = new QLabel("المخزن:");
  whLabel->setStyleSheet("background: transparent; font-weight: bold;");
  detailsBar->addWidget(whLabel);
  m_warehouseCombo = new QComboBox;
  m_warehouseCombo->addItems({"الرئيسية", "فرع 1", "فرع 2"});
  m_warehouseCombo->setMinimumWidth(120);
  detailsBar->addWidget(m_warehouseCombo);

  detailsBar->addSpacing(15);

  auto *trLabel = new QLabel("الخزينة:");
  trLabel->setStyleSheet("background: transparent; font-weight: bold;");
  detailsBar->addWidget(trLabel);
  m_treasuryCombo = new QComboBox;
  m_treasuryCombo->addItems({"الرئيسية", "خزينة 1", "خزينة 2"});
  m_treasuryCombo->setMinimumWidth(120);
  detailsBar->addWidget(m_treasuryCombo);

  detailsBar->addStretch();
  layout->addLayout(detailsBar);

  // ===== Search bar with cancel mode =====
  auto *searchLayout = new QHBoxLayout;
  m_searchEdit = new QLineEdit;
  m_searchEdit->setObjectName("searchBar");
  m_searchEdit->setPlaceholderText(
      "🔍  ابحث بالاسم أو الباركود واضغط Enter...");
  m_searchEdit->setAlignment(Qt::AlignRight);
  searchLayout->addWidget(m_searchEdit);

  m_cancelModeBtn = new QPushButton("🔴  وضع الإلغاء");
  m_cancelModeBtn->setCursor(Qt::PointingHandCursor);
  m_cancelModeBtn->setCheckable(true);
  m_cancelModeBtn->setMinimumHeight(38);
  m_cancelModeBtn->setMinimumWidth(140);
  m_cancelModeBtn->setStyleSheet(
      "QPushButton { padding: 6px 16px; font-size: 13px; "
      "border: 2px solid #EF4444; color: #EF4444; "
      "border-radius: 6px; background: transparent; }"
      "QPushButton:hover { background: rgba(239,68,68,0.15); }"
      "QPushButton:checked { background: #EF4444; color: white; "
      "font-weight: bold; }");
  searchLayout->addWidget(m_cancelModeBtn);

  layout->addLayout(searchLayout);

  // ===== Invoice table =====
  m_table = new InvoiceTable;
  layout->addWidget(m_table, 1);

  // ===== Bottom: summary + buttons =====
  auto *bottomLayout = new QHBoxLayout;

  // Summary panel
  auto *summaryWidget = new QWidget;
  summaryWidget->setObjectName("invoiceSummary");
  auto *summaryLayout = new QGridLayout(summaryWidget);
  summaryLayout->setSpacing(8);

  auto addSummaryRow = [&](int row, const QString &label, QLabel *&valueLabel,
                           const QString &color = "#E8EAED") {
    auto *lbl = new QLabel(label);
    lbl->setStyleSheet("font-size: 14px; font-weight: bold; color: #9CA3AF; "
                       "background: transparent;");
    summaryLayout->addWidget(lbl, row, 0);
    valueLabel = new QLabel("0.00 ريال");
    valueLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; "
                                      "color: %1; background: transparent;")
                                  .arg(color));
    valueLabel->setAlignment(Qt::AlignLeft);
    summaryLayout->addWidget(valueLabel, row, 1);
  };

  addSummaryRow(0, "المجموع:", m_subtotalLabel);

  // Discount row
  auto *discLabel = new QLabel("الخصم:");
  discLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: "
                           "#9CA3AF; background: transparent;");
  summaryLayout->addWidget(discLabel, 1, 0);
  m_discountEdit = new QLineEdit("0");
  m_discountEdit->setAlignment(Qt::AlignLeft);
  m_discountEdit->setMaximumWidth(150);
  summaryLayout->addWidget(m_discountEdit, 1, 1);

  addSummaryRow(2, "الضريبة (15%):", m_taxLabel, "#F59E0B");
  addSummaryRow(3, "الإجمالي:", m_totalLabel, "#10B981");
  m_totalLabel->setObjectName("invoiceTotalLabel");

  auto *paidLabel = new QLabel("المدفوع:");
  paidLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: "
                           "#9CA3AF; background: transparent;");
  summaryLayout->addWidget(paidLabel, 4, 0);
  m_paidEdit = new QLineEdit("0");
  m_paidEdit->setAlignment(Qt::AlignLeft);
  summaryLayout->addWidget(m_paidEdit, 4, 1);

  addSummaryRow(5, "المتبقي:", m_remainingLabel, "#EF4444");

  auto *notesLabel = new QLabel("ملاحظات:");
  notesLabel->setStyleSheet(
      "font-size: 12px; color: #6B7280; background: transparent;");
  summaryLayout->addWidget(notesLabel, 6, 0);
  m_notesEdit = new QLineEdit;
  m_notesEdit->setPlaceholderText("ملاحظات اختيارية...");
  summaryLayout->addWidget(m_notesEdit, 6, 1);

  bottomLayout->addWidget(summaryWidget);

  // Action buttons
  auto *btnLayout = new QVBoxLayout;
  btnLayout->setSpacing(8);

  auto *newBtn = new QPushButton("📄  فاتورة جديدة");
  newBtn->setObjectName("btnOutline");
  newBtn->setCursor(Qt::PointingHandCursor);
  newBtn->setMinimumHeight(42);
  btnLayout->addWidget(newBtn);

  auto *removeBtn = new QPushButton("❌  حذف صنف");
  removeBtn->setObjectName("btnDanger");
  removeBtn->setCursor(Qt::PointingHandCursor);
  removeBtn->setMinimumHeight(42);
  btnLayout->addWidget(removeBtn);

  auto *saveBtn = new QPushButton("💾  حفظ (F1)");
  saveBtn->setObjectName("btnSuccess");
  saveBtn->setCursor(Qt::PointingHandCursor);
  saveBtn->setMinimumHeight(50);
  saveBtn->setShortcut(QKeySequence(Qt::Key_F1));
  saveBtn->setStyleSheet(saveBtn->styleSheet() + " font-size: 16px;");
  btnLayout->addWidget(saveBtn);

  auto *savePrintBtn = new QPushButton("🖨️  حفظ و طباعة (F2)");
  savePrintBtn->setObjectName("btnWarning");
  savePrintBtn->setCursor(Qt::PointingHandCursor);
  savePrintBtn->setMinimumHeight(42);
  savePrintBtn->setShortcut(QKeySequence(Qt::Key_F2));
  btnLayout->addWidget(savePrintBtn);

  auto *savePreviewBtn = new QPushButton("👁️  حفظ و معاينة (F3)");
  savePreviewBtn->setCursor(Qt::PointingHandCursor);
  savePreviewBtn->setMinimumHeight(42);
  savePreviewBtn->setShortcut(QKeySequence(Qt::Key_F3));
  btnLayout->addWidget(savePreviewBtn);

  auto *pdfBtn = new QPushButton("📄  تصدير PDF (F4)");
  pdfBtn->setCursor(Qt::PointingHandCursor);
  pdfBtn->setMinimumHeight(42);
  pdfBtn->setShortcut(QKeySequence(Qt::Key_F4));
  pdfBtn->setStyleSheet(
      "QPushButton { padding: 6px 16px; font-size: 13px; "
      "border: 2px solid #8B5CF6; color: #8B5CF6; "
      "border-radius: 6px; background: transparent; }"
      "QPushButton:hover { background: rgba(139,92,246,0.15); }");
  btnLayout->addWidget(pdfBtn);

  btnLayout->addStretch();
  bottomLayout->addLayout(btnLayout);

  layout->addLayout(bottomLayout);

  // Load data
  loadCustomers();
  onNewInvoice();

  // Connections
  connect(m_searchEdit, &QLineEdit::returnPressed, this,
          &SalesWidget::onSearchProduct);
  connect(saveBtn, &QPushButton::clicked, this, &SalesWidget::onSaveInvoice);
  connect(savePrintBtn, &QPushButton::clicked, this,
          &SalesWidget::onSaveAndPrint);
  connect(savePreviewBtn, &QPushButton::clicked, this,
          &SalesWidget::onSaveAndPreview);
  connect(removeBtn, &QPushButton::clicked, this, &SalesWidget::onRemoveItem);
  connect(newBtn, &QPushButton::clicked, this, &SalesWidget::onNewInvoice);
  connect(m_holdBtn, &QPushButton::clicked, this, &SalesWidget::onHoldInvoice);
  connect(m_resumeBtn, &QPushButton::clicked, this,
          &SalesWidget::onResumeInvoice);
  connect(m_cancelModeBtn, &QPushButton::toggled, this,
          &SalesWidget::onToggleCancelMode);
  connect(m_paidEdit, &QLineEdit::textChanged, this,
          &SalesWidget::updateTotals);
  connect(m_discountEdit, &QLineEdit::textChanged, this,
          &SalesWidget::updateTotals);
  connect(pdfBtn, &QPushButton::clicked, this, &SalesWidget::onExportPDF);
  connect(prevInvBtn, &QPushButton::clicked, this,
          &SalesWidget::onBrowsePreviousInvoice);
}

void SalesWidget::loadCustomers() {
  m_customerCombo->clear();
  auto q = DatabaseManager::instance().getCustomers(0);
  while (q.next()) {
    m_customerCombo->addItem(q.value("name").toString(), q.value("id").toInt());
  }
}

void SalesWidget::onSearchProduct() {
  QString search = m_searchEdit->text().trimmed();
  if (search.isEmpty())
    return;

  auto &db = DatabaseManager::instance();

  // Cancel mode: remove item from invoice instead of adding
  if (m_cancelMode) {
    auto q = db.getProductByBarcode(search);
    if (q.next()) {
      int productId = q.value("id").toInt();
      if (m_table->removeItemByProductId(productId)) {
        m_searchEdit->clear();
        updateTotals();
        return;
      }
    }
    q = db.getProducts(search);
    if (q.next()) {
      int productId = q.value("id").toInt();
      if (m_table->removeItemByProductId(productId)) {
        m_searchEdit->clear();
        updateTotals();
        return;
      }
    }
    m_searchEdit->setStyleSheet("border-color: #EF4444;");
    QTimer::singleShot(1000, [this]() { m_searchEdit->setStyleSheet(""); });
    return;
  }

  // Normal mode: add item to invoice
  auto q = db.getProductByBarcode(search);
  if (q.next()) {
    m_table->addItem(q.value("id").toInt(), q.value("name").toString(), 1,
                     q.value("sell_price").toDouble(),
                     q.value("tax_rate").toDouble());
    m_searchEdit->clear();
    updateTotals();
    return;
  }

  q = db.getProducts(search);
  if (q.next()) {
    m_table->addItem(q.value("id").toInt(), q.value("name").toString(), 1,
                     q.value("sell_price").toDouble(),
                     q.value("tax_rate").toDouble());
    m_searchEdit->clear();
    updateTotals();
  } else {
    m_searchEdit->setStyleSheet("border-color: #EF4444;");
    QTimer::singleShot(1000, [this]() { m_searchEdit->setStyleSheet(""); });
  }
}

void SalesWidget::onSaveInvoice() {
  if (m_table->rowCount() == 0) {
    QMessageBox::warning(this, "تنبيه", "لا توجد أصناف في الفاتورة!");
    return;
  }

  auto &db = DatabaseManager::instance();
  double subtotal = m_table->getSubtotal();
  double tax = m_table->getTaxTotal();
  double discount = m_discountEdit->text().toDouble();
  double total = subtotal - discount + tax;
  double paid = m_paidEdit->text().toDouble();
  double remaining = total - paid;
  int customerId = m_customerCombo->currentData().toInt();

  // Map payment method
  QStringList payMethods = {"cash", "card", "transfer", "credit"};
  QString payMethod =
      payMethods.value(m_paymentMethodCombo->currentIndex(), "cash");

  int invoiceId =
      db.createSalesInvoice(customerId, 1, subtotal, discount, 0, tax, total,
                            paid, remaining, payMethod, m_notesEdit->text());

  if (invoiceId > 0) {
    auto items = m_table->getAllItems();
    for (const auto &item : items) {
      db.addSalesInvoiceDetail(invoiceId, item.productId, item.name, item.qty,
                               item.price, 0, item.discount, item.taxRate,
                               item.taxValue, item.total);
      db.updateProductStock(item.productId, item.qty, false);
      db.addStockMovement(item.productId, 1, "out", item.qty, "sale", invoiceId,
                          "");
    }

    if (remaining > 0) {
      db.updateCustomerBalance(customerId, remaining, true);
    }

    QMessageBox::information(this, "تم",
                             "تم حفظ الفاتورة بنجاح!\nرقم الفاتورة: " +
                                 QString::number(m_invoiceNo));

    // Auto-open cash drawer if enabled
    QString drawerPrinter = db.getSetting("cash_drawer_printer", "");
    if (!drawerPrinter.isEmpty()) {
      CashDrawer drawer;
      drawer.openDrawer(drawerPrinter);
    }

    onNewInvoice();
  } else {
    QMessageBox::critical(this, "خطأ", "حدث خطأ أثناء حفظ الفاتورة!");
  }
}

void SalesWidget::onSaveAndPrint() {
  onSaveInvoice();
  QMessageBox::information(this, "طباعة", "تم إرسال الفاتورة للطباعة! 🖨️");
}

void SalesWidget::onSaveAndPreview() {
  if (m_table->rowCount() == 0) {
    QMessageBox::warning(this, "تنبيه", "لا توجد أصناف في الفاتورة!");
    return;
  }

  // Show preview dialog
  double subtotal = m_table->getSubtotal();
  double discount = m_discountEdit->text().toDouble();
  double tax = m_table->getTaxTotal();
  double total = subtotal - discount + tax;
  double paid = m_paidEdit->text().toDouble();

  QString preview = "═══════════════════════════\n"
                    "         معاينة الفاتورة\n"
                    "═══════════════════════════\n"
                    "فاتورة رقم: " +
                    QString::number(m_invoiceNo) +
                    "\n"
                    "العميل: " +
                    m_customerCombo->currentText() +
                    "\n"
                    "طريقة الدفع: " +
                    m_paymentMethodCombo->currentText() +
                    "\n"
                    "المخزن: " +
                    m_warehouseCombo->currentText() +
                    "\n"
                    "═══════════════════════════\n"
                    "عدد الأصناف: " +
                    QString::number(m_table->rowCount()) +
                    "\n"
                    "المجموع: " +
                    QString::number(subtotal, 'f', 2) +
                    " ريال\n"
                    "الخصم: " +
                    QString::number(discount, 'f', 2) +
                    " ريال\n"
                    "الضريبة: " +
                    QString::number(tax, 'f', 2) +
                    " ريال\n"
                    "الإجمالي: " +
                    QString::number(total, 'f', 2) +
                    " ريال\n"
                    "المدفوع: " +
                    QString::number(paid, 'f', 2) +
                    " ريال\n"
                    "المتبقي: " +
                    QString::number(total - paid, 'f', 2) +
                    " ريال\n"
                    "═══════════════════════════";

  QMessageBox::information(this, "معاينة الفاتورة", preview);
}

void SalesWidget::onRemoveItem() {
  m_table->removeSelectedItem();
  updateTotals();
}

void SalesWidget::onNewInvoice() {
  m_table->clearAll();
  m_invoiceNo = DatabaseManager::instance().getNextSalesInvoiceNo();
  m_invoiceNoLabel->setText("فاتورة رقم: " + QString::number(m_invoiceNo));
  m_paidEdit->clear();
  m_notesEdit->clear();
  m_discountEdit->setText("0");
  m_searchEdit->clear();
  m_searchEdit->setFocus();
  m_cancelMode = false;
  m_cancelModeBtn->setChecked(false);
  updateTotals();
}

void SalesWidget::updateTotals() {
  double subtotal = m_table->getSubtotal();
  double discount = m_discountEdit->text().toDouble();
  double tax = m_table->getTaxTotal();
  double total = subtotal - discount + tax;
  double paid = m_paidEdit->text().toDouble();
  double remaining = total - paid;

  m_subtotalLabel->setText(QString::number(subtotal, 'f', 2) + " ريال");
  m_taxLabel->setText(QString::number(tax, 'f', 2) + " ريال");
  m_totalLabel->setText(QString::number(total, 'f', 2) + " ريال");
  m_remainingLabel->setText(QString::number(remaining, 'f', 2) + " ريال");

  // Color the remaining label
  if (remaining > 0) {
    m_remainingLabel->setStyleSheet("font-size: 16px; font-weight: bold; "
                                    "color: #EF4444; background: transparent;");
  } else {
    m_remainingLabel->setStyleSheet("font-size: 16px; font-weight: bold; "
                                    "color: #10B981; background: transparent;");
  }
}

// ============ Hold Invoice ============
void SalesWidget::onHoldInvoice() {
  if (m_table->rowCount() == 0) {
    QMessageBox::warning(this, "تنبيه", "لا توجد أصناف لتعليق الفاتورة!");
    return;
  }

  HeldInvoice held;
  held.invoiceNo = m_invoiceNo;
  held.customerId = m_customerCombo->currentData().toInt();
  held.notes = m_notesEdit->text();
  held.items = m_table->getAllItems();

  m_heldInvoices.append(held);
  m_resumeBtn->setText("▶️  استرجاع فاتورة (" +
                       QString::number(m_heldInvoices.size()) + ")");
  m_resumeBtn->setEnabled(true);

  QMessageBox::information(
      this, "تم التعليق",
      "تم تعليق الفاتورة رقم " + QString::number(m_invoiceNo) +
          "\nعدد الفواتير المعلقة: " + QString::number(m_heldInvoices.size()));

  onNewInvoice();
}

void SalesWidget::onResumeInvoice() {
  if (m_heldInvoices.isEmpty()) {
    QMessageBox::information(this, "تنبيه", "لا توجد فواتير معلقة!");
    return;
  }

  if (m_table->rowCount() > 0) {
    auto answer = QMessageBox::question(
        this, "تأكيد",
        "الفاتورة الحالية بها أصناف.\nهل تريد تعليقها واسترجاع فاتورة أخرى؟",
        QMessageBox::Yes | QMessageBox::No);
    if (answer == QMessageBox::Yes) {
      onHoldInvoice();
    } else {
      return;
    }
  }

  if (m_heldInvoices.size() == 1) {
    HeldInvoice held = m_heldInvoices.takeFirst();
    m_table->clearAll();
    m_invoiceNo = held.invoiceNo;
    m_invoiceNoLabel->setText("فاتورة رقم: " + QString::number(m_invoiceNo));
    m_notesEdit->setText(held.notes);

    int ci = m_customerCombo->findData(held.customerId);
    if (ci >= 0)
      m_customerCombo->setCurrentIndex(ci);

    for (const auto &item : held.items) {
      m_table->addItem(item.productId, item.name, item.qty, item.price,
                       item.taxRate);
    }
    updateTotals();
  } else {
    QDialog dlg(this);
    dlg.setWindowTitle("اختر الفاتورة المعلقة");
    dlg.setMinimumSize(400, 300);
    dlg.setLayoutDirection(Qt::RightToLeft);
    auto *dlgLayout = new QVBoxLayout(&dlg);

    auto *listLabel = new QLabel("الفواتير المعلقة:");
    listLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; background: transparent;");
    dlgLayout->addWidget(listLabel);

    auto *listWidget = new QListWidget;
    listWidget->setStyleSheet("font-size: 14px; padding: 4px;");
    for (int i = 0; i < m_heldInvoices.size(); i++) {
      const auto &h = m_heldInvoices[i];
      double total = 0;
      for (const auto &item : h.items)
        total += item.total;
      QString text = "فاتورة #" + QString::number(h.invoiceNo) + "  |  " +
                     QString::number(h.items.size()) + " أصناف" + "  |  " +
                     QString::number(total, 'f', 2) + " ريال";
      listWidget->addItem(text);
    }
    listWidget->setCurrentRow(0);
    dlgLayout->addWidget(listWidget);

    auto *btnLay = new QHBoxLayout;
    auto *okBtn = new QPushButton("استرجاع");
    okBtn->setObjectName("btnSuccess");
    auto *cancelBtn = new QPushButton("إلغاء");
    cancelBtn->setObjectName("btnDanger");
    btnLay->addWidget(okBtn);
    btnLay->addWidget(cancelBtn);
    dlgLayout->addLayout(btnLay);

    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(listWidget, &QListWidget::doubleClicked, &dlg, &QDialog::accept);

    if (dlg.exec() == QDialog::Accepted && listWidget->currentRow() >= 0) {
      int idx = listWidget->currentRow();
      HeldInvoice held = m_heldInvoices.takeAt(idx);
      m_table->clearAll();
      m_invoiceNo = held.invoiceNo;
      m_invoiceNoLabel->setText("فاتورة رقم: " + QString::number(m_invoiceNo));
      m_notesEdit->setText(held.notes);

      int ci = m_customerCombo->findData(held.customerId);
      if (ci >= 0)
        m_customerCombo->setCurrentIndex(ci);

      for (const auto &item : held.items) {
        m_table->addItem(item.productId, item.name, item.qty, item.price,
                         item.taxRate);
      }
      updateTotals();
    }
  }

  m_resumeBtn->setText("▶️  استرجاع فاتورة (" +
                       QString::number(m_heldInvoices.size()) + ")");
  m_resumeBtn->setEnabled(!m_heldInvoices.isEmpty());
}

// ============ Cancel Mode ============
void SalesWidget::onToggleCancelMode() {
  m_cancelMode = m_cancelModeBtn->isChecked();
  if (m_cancelMode) {
    m_searchEdit->setPlaceholderText(
        "🔴  وضع الإلغاء: اسحب الباركود لإزالة صنف...");
    m_searchEdit->setStyleSheet(
        "border: 2px solid #EF4444; background: rgba(239,68,68,0.08);");
  } else {
    m_searchEdit->setPlaceholderText(
        "🔍  ابحث بالاسم أو الباركود واضغط Enter...");
    m_searchEdit->setStyleSheet("");
  }
  m_searchEdit->setFocus();
}

// ============ PDF Export ============
void SalesWidget::onExportPDF() {
  if (m_table->rowCount() == 0) {
    QMessageBox::warning(this, "تنبيه", "لا توجد أصناف في الفاتورة!");
    return;
  }

  QString defaultName =
      "فاتورة_" + QString::number(m_invoiceNo) + "_" +
      QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".pdf";
  QString filePath = QFileDialog::getSaveFileName(
      this, "حفظ الفاتورة كـ PDF", QDir::homePath() + "/Desktop/" + defaultName,
      "PDF Files (*.pdf)");

  if (filePath.isEmpty())
    return;

  generateInvoicePDF(filePath);

  QMessageBox::StandardButton openFile = QMessageBox::question(
      this, "تم التصدير",
      "تم تصدير الفاتورة بنجاح!\n\n" + filePath + "\n\nهل تريد فتح الملف؟",
      QMessageBox::Yes | QMessageBox::No);
  if (openFile == QMessageBox::Yes) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
  }
}

void SalesWidget::generateInvoicePDF(const QString &filePath) {
  QPdfWriter writer(filePath);
  writer.setPageSize(QPageSize(QPageSize::A4));
  writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
  int dpi = writer.resolution();

  QPainter painter(&writer);
  painter.setLayoutDirection(Qt::RightToLeft);

  int pageW = writer.width();
  int pageH = writer.height();
  int margin = dpi * 0.2;
  int y = margin;

  // Helper: mm to device units
  auto mm = [dpi](double millimeters) -> int {
    return static_cast<int>(millimeters * dpi / 25.4);
  };

  // ===== Company Header =====
  QFont titleFont("Arial", 18, QFont::Bold);
  QFont headerFont("Arial", 12, QFont::Bold);
  QFont normalFont("Arial", 10);
  QFont smallFont("Arial", 8);

  // Load company info from settings
  auto &db = DatabaseManager::instance();
  QString companyName = db.getSetting("company_name", "متجري");
  QString taxNumber = db.getSetting("tax_number", "300000000000003");

  painter.setFont(titleFont);
  painter.setPen(QColor("#1E40AF"));
  painter.drawText(QRect(0, y, pageW, mm(10)), Qt::AlignCenter, companyName);
  y += mm(12);

  painter.setFont(headerFont);
  painter.setPen(QColor("#374151"));
  painter.drawText(QRect(0, y, pageW, mm(8)), Qt::AlignCenter,
                   "فاتورة مبيعات ضريبية");
  y += mm(10);

  // Horizontal line
  painter.setPen(QPen(QColor("#3B82F6"), 3));
  painter.drawLine(margin, y, pageW - margin, y);
  y += mm(5);

  // ===== Invoice Info (2 columns) =====
  painter.setFont(normalFont);
  painter.setPen(Qt::black);

  int colW = (pageW - 2 * margin) / 2;
  int leftCol = margin;
  int rightCol = margin + colW;

  // Right column (Arabic RTL)
  auto drawField = [&](int x, int &yPos, const QString &label,
                       const QString &value) {
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(QRect(x, yPos, colW, mm(5)), Qt::AlignRight, label);
    painter.setFont(normalFont);
    painter.drawText(QRect(x, yPos, colW - mm(30), mm(5)), Qt::AlignRight,
                     value);
    yPos += mm(6);
  };

  int y1 = y, y2 = y;
  drawField(rightCol, y1, "رقم الفاتورة:", QString::number(m_invoiceNo));
  drawField(rightCol, y1, "التاريخ:",
            QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm"));
  drawField(rightCol, y1, "العميل:", m_customerCombo->currentText());

  drawField(leftCol, y2, "طريقة الدفع:", m_paymentMethodCombo->currentText());
  drawField(leftCol, y2, "المخزن:", m_warehouseCombo->currentText());
  drawField(leftCol, y2, "الرقم الضريبي:", taxNumber);

  y = qMax(y1, y2) + mm(3);

  // Horizontal line
  painter.setPen(QPen(QColor("#D1D5DB"), 1));
  painter.drawLine(margin, y, pageW - margin, y);
  y += mm(4);

  // ===== Items Table =====
  auto items = m_table->getAllItems();
  int cols[] = {0, 0, 0, 0,
                0, 0, 0}; // م, الاسم, الكمية, السعر, الخصم, الضريبة, الإجمالي
  int colWidths[] = {mm(8), 0, mm(18), mm(22), mm(18), mm(22), mm(25)};
  int usedW = 0;
  for (int i = 0; i < 7; i++)
    if (i != 1)
      usedW += colWidths[i];
  colWidths[1] = pageW - 2 * margin - usedW; // Name gets remaining space

  // Column positions (RTL)
  int cx = pageW - margin;
  for (int i = 0; i < 7; i++) {
    cx -= colWidths[i];
    cols[i] = cx;
  }

  // Table header
  painter.setFont(QFont("Arial", 9, QFont::Bold));
  painter.setPen(Qt::white);
  painter.setBrush(QColor("#1E40AF"));
  painter.drawRect(margin, y, pageW - 2 * margin, mm(7));

  QStringList headers = {"م",     "اسم الصنف", "الكمية",  "السعر",
                         "الخصم", "الضريبة",   "الإجمالي"};
  for (int i = 0; i < 7; i++) {
    painter.drawText(QRect(cols[i], y, colWidths[i], mm(7)), Qt::AlignCenter,
                     headers[i]);
  }
  y += mm(7);

  // Table rows
  painter.setFont(normalFont);
  painter.setPen(Qt::black);
  int rowIdx = 1;
  for (const auto &item : items) {
    bool alt = (rowIdx % 2 == 0);
    if (alt) {
      painter.setBrush(QColor("#F3F4F6"));
      painter.setPen(Qt::NoPen);
      painter.drawRect(margin, y, pageW - 2 * margin, mm(6));
    }
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::black);

    QStringList rowData = {QString::number(rowIdx),
                           item.name,
                           QString::number(item.qty),
                           QString::number(item.price, 'f', 2),
                           QString::number(item.discount, 'f', 2),
                           QString::number(item.taxValue, 'f', 2),
                           QString::number(item.total, 'f', 2)};

    for (int i = 0; i < 7; i++) {
      painter.drawText(QRect(cols[i], y, colWidths[i], mm(6)), Qt::AlignCenter,
                       rowData[i]);
    }
    y += mm(6);
    rowIdx++;

    // Page break check
    if (y > pageH - mm(80)) {
      writer.newPage();
      y = margin;
    }
  }

  // Table bottom line
  painter.setPen(QPen(QColor("#1E40AF"), 2));
  painter.drawLine(margin, y, pageW - margin, y);
  y += mm(8);

  // ===== Totals Section =====
  double subtotal = m_table->getSubtotal();
  double discount = m_discountEdit->text().toDouble();
  double tax = m_table->getTaxTotal();
  double total = subtotal - discount + tax;
  double paid = m_paidEdit->text().toDouble();
  double remaining = total - paid;

  int totalsX = pageW - margin - mm(80);
  int totalsW = mm(80);

  auto drawTotalRow = [&](const QString &label, double value,
                          const QColor &color = Qt::black, bool bold = false) {
    if (bold)
      painter.setFont(QFont("Arial", 11, QFont::Bold));
    else
      painter.setFont(normalFont);
    painter.setPen(color);
    painter.drawText(QRect(totalsX, y, mm(40), mm(6)), Qt::AlignRight, label);
    painter.drawText(QRect(totalsX + mm(40), y, mm(40), mm(6)), Qt::AlignCenter,
                     QString::number(value, 'f', 2) + " ريال");
    y += mm(6);
  };

  drawTotalRow("المجموع:", subtotal);
  if (discount > 0)
    drawTotalRow("الخصم:", discount, QColor("#EF4444"));
  drawTotalRow("ضريبة القيمة المضافة (15%):", tax, QColor("#F59E0B"));
  painter.setPen(QPen(QColor("#1E40AF"), 1));
  painter.drawLine(totalsX, y, totalsX + totalsW, y);
  y += mm(2);
  drawTotalRow("الإجمالي:", total, QColor("#1E40AF"), true);
  drawTotalRow("المدفوع:", paid, QColor("#10B981"));
  if (remaining > 0)
    drawTotalRow("المتبقي:", remaining, QColor("#EF4444"), true);

  y += mm(10);

  // ===== ZATCA QR Code =====
  QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
  QImage qrImage = ZatcaQR::generateZatcaQR(companyName, taxNumber, timestamp,
                                            total, tax, 3);



  if (!qrImage.isNull()) {
    int qrSize = mm(35);
    int qrX = margin;
    int qrY = y;

    painter.drawImage(QRect(qrX, qrY, qrSize, qrSize), qrImage);

    painter.setFont(smallFont);
    painter.setPen(QColor("#6B7280"));
    painter.drawText(QRect(qrX, qrY + qrSize + mm(1), qrSize, mm(5)),
                     Qt::AlignCenter, "باركود الزكاة والدخل");
    painter.drawText(QRect(qrX, qrY + qrSize + mm(5), qrSize, mm(5)),
                     Qt::AlignCenter, "ZATCA E-Invoice QR");
  }

  // ===== Footer =====
  painter.setFont(smallFont);
  painter.setPen(QColor("#9CA3AF"));
  int footerY = pageH - mm(15);
  painter.drawLine(margin, footerY, pageW - margin, footerY);
  footerY += mm(3);
  painter.drawText(QRect(0, footerY, pageW, mm(5)), Qt::AlignCenter,
                   "هذه الفاتورة صادرة عن " + companyName +
                       "  |  الرقم الضريبي: " + taxNumber);
  footerY += mm(5);
  painter.drawText(QRect(0, footerY, pageW, mm(5)), Qt::AlignCenter,
                   "This is a computer-generated invoice - "
                   "لا تحتاج إلى توقيع أو ختم");

  painter.end();
}

// ============ Browse Previous Invoices ============
void SalesWidget::onBrowsePreviousInvoice() {
  QDialog dlg(this);
  dlg.setWindowTitle("الفواتير السابقة");
  dlg.setMinimumSize(800, 500);
  dlg.setLayoutDirection(Qt::RightToLeft);
  auto *dlgLayout = new QVBoxLayout(&dlg);

  auto *titleLbl = new QLabel("🔍 البحث في الفواتير السابقة");
  titleLbl->setStyleSheet(
      "font-size: 16px; font-weight: bold; background: transparent;");
  dlgLayout->addWidget(titleLbl);

  // Invoice table
  auto *invTable = new QTableWidget;
  invTable->setAlternatingRowColors(true);
  invTable->verticalHeader()->hide();
  invTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  invTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  invTable->setSelectionMode(QAbstractItemView::SingleSelection);
  invTable->setLayoutDirection(Qt::RightToLeft);
  invTable->setColumnCount(7);
  invTable->setHorizontalHeaderLabels(
      {"رقم", "التاريخ", "العميل", "المجموع", "الضريبة", "الإجمالي", "الحالة"});
  invTable->horizontalHeader()->setStretchLastSection(true);
  invTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

  // Load invoices
  auto q = DatabaseManager::instance().getSalesInvoices("", "");
  while (q.next()) {
    int r = invTable->rowCount();
    invTable->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    auto *idItem = mi(q.value("invoice_no").toString());
    idItem->setData(Qt::UserRole, q.value("id").toInt());
    invTable->setItem(r, 0, idItem);
    invTable->setItem(
        r, 1, mi(q.value("date").toDateTime().toString("yyyy-MM-dd hh:mm")));
    invTable->setItem(r, 2, mi(q.value("customer_name").toString()));
    invTable->setItem(
        r, 3, mi(QString::number(q.value("subtotal").toDouble(), 'f', 2)));
    invTable->setItem(
        r, 4, mi(QString::number(q.value("tax_value").toDouble(), 'f', 2)));
    invTable->setItem(r, 5,
                      mi(QString::number(q.value("total").toDouble(), 'f', 2)));
    QString status =
        q.value("status").toString() == "completed" ? "مكتملة" : "معلقة";
    auto *statusItem = mi(status);
    statusItem->setForeground(status == "مكتملة" ? QColor("#10B981")
                                                 : QColor("#F59E0B"));
    invTable->setItem(r, 6, statusItem);
  }
  dlgLayout->addWidget(invTable, 1);

  // Detail section
  auto *detailLbl = new QLabel("📋 تفاصيل الفاتورة المحددة:");
  detailLbl->setStyleSheet(
      "font-size: 14px; font-weight: bold; background: transparent; "
      "margin-top: 8px;");
  dlgLayout->addWidget(detailLbl);

  auto *detailTable = new QTableWidget;
  detailTable->setAlternatingRowColors(true);
  detailTable->verticalHeader()->hide();
  detailTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  detailTable->setLayoutDirection(Qt::RightToLeft);
  detailTable->setColumnCount(6);
  detailTable->setHorizontalHeaderLabels(
      {"الصنف", "الكمية", "السعر", "الخصم", "الضريبة", "الإجمالي"});
  detailTable->horizontalHeader()->setStretchLastSection(true);
  detailTable->horizontalHeader()->setSectionResizeMode(0,
                                                        QHeaderView::Stretch);
  detailTable->setMaximumHeight(180);
  dlgLayout->addWidget(detailTable);

  // Load details when invoice selected
  connect(
      invTable, &QTableWidget::currentCellChanged, [&](int row, int, int, int) {
        if (row < 0)
          return;
        detailTable->setRowCount(0);
        auto *item = invTable->item(row, 0);
        if (!item)
          return;
        int invoiceId = item->data(Qt::UserRole).toInt();
        auto dq = DatabaseManager::instance().getSalesInvoiceDetails(invoiceId);
        while (dq.next()) {
          int dr = detailTable->rowCount();
          detailTable->insertRow(dr);
          auto mi2 = [](const QString &t) {
            auto *i = new QTableWidgetItem(t);
            i->setTextAlignment(Qt::AlignCenter);
            return i;
          };
          detailTable->setItem(dr, 0, mi2(dq.value("product_name").toString()));
          detailTable->setItem(dr, 1,
                               mi2(QString::number(dq.value("qty").toInt())));
          detailTable->setItem(
              dr, 2,
              mi2(QString::number(dq.value("price").toDouble(), 'f', 2)));
          detailTable->setItem(
              dr, 3,
              mi2(QString::number(dq.value("discount").toDouble(), 'f', 2)));
          detailTable->setItem(
              dr, 4,
              mi2(QString::number(dq.value("tax_value").toDouble(), 'f', 2)));
          detailTable->setItem(
              dr, 5,
              mi2(QString::number(dq.value("total").toDouble(), 'f', 2)));
        }
      });

  // Buttons
  auto *btnLay = new QHBoxLayout;
  auto *closeBtn = new QPushButton("إغلاق");
  closeBtn->setObjectName("btnDanger");
  closeBtn->setMinimumHeight(40);
  btnLay->addWidget(closeBtn);

  auto *printPdfBtn = new QPushButton("🖨️  طباعة الفاتورة");
  printPdfBtn->setObjectName("btnSuccess");
  printPdfBtn->setMinimumHeight(40);
  btnLay->addWidget(printPdfBtn);

  auto *exportPdfBtn = new QPushButton("📄  تصدير PDF");
  exportPdfBtn->setObjectName("btnOutline");
  exportPdfBtn->setMinimumHeight(40);
  btnLay->addWidget(exportPdfBtn);

  btnLay->addStretch();
  dlgLayout->addLayout(btnLay);

  connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

  // Print invoice
  connect(printPdfBtn, &QPushButton::clicked, [&]() {
    int row = invTable->currentRow();
    if (row < 0) {
      QMessageBox::warning(&dlg, "تنبيه", "اختر فاتورة أولاً!");
      return;
    }
    // Check for default printer
    QString defaultPrinter =
        DatabaseManager::instance().getSetting("default_printer");
    QPrinter printer;
    if (!defaultPrinter.isEmpty()) {
      printer.setPrinterName(defaultPrinter);
    } else {
      QPrintDialog printDlg(&printer, &dlg);
      printDlg.setWindowTitle("اختيار الطابعة");
      if (printDlg.exec() != QDialog::Accepted)
        return;
    }
    // Professional invoice printing with DPI-aware coordinates
    QPainter painter(&printer);
    int dpi = printer.resolution();
    auto mm = [dpi](double millimeters) -> int {
      return static_cast<int>(millimeters * dpi / 25.4);
    };

    int pageW = printer.width();
    int margin = mm(15);
    int contentW = pageW - 2 * margin;
    int y = mm(10);

    // Load company info
    auto &db2 = DatabaseManager::instance();
    QString compName2 = db2.getSetting("company_name", "متجري");
    QString compPhone2 = db2.getSetting("company_phone");
    QString compAddr2 = db2.getSetting("company_address");
    QString taxNum2 = db2.getSetting("tax_number");

    // ===== Company Header =====
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.setPen(QColor("#1E40AF"));
    painter.drawText(QRect(0, y, pageW, mm(8)), Qt::AlignCenter, compName2);
    y += mm(10);

    painter.setFont(QFont("Arial", 9));
    painter.setPen(QColor("#6B7280"));
    if (!compPhone2.isEmpty() || !compAddr2.isEmpty()) {
      painter.drawText(QRect(0, y, pageW, mm(5)), Qt::AlignCenter,
                       compPhone2 + "  |  " + compAddr2);
      y += mm(6);
    }
    if (!taxNum2.isEmpty()) {
      painter.drawText(QRect(0, y, pageW, mm(5)), Qt::AlignCenter,
                       "الرقم الضريبي: " + taxNum2);
      y += mm(6);
    }

    // Title
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.setPen(QColor("#374151"));
    painter.drawText(QRect(0, y, pageW, mm(7)), Qt::AlignCenter,
                     "فاتورة مبيعات ضريبية");
    y += mm(9);

    // Blue separator
    painter.setPen(QPen(QColor("#3B82F6"), 2));
    painter.drawLine(margin, y, pageW - margin, y);
    y += mm(5);

    // ===== Invoice Info =====
    painter.setFont(QFont("Arial", 9));
    painter.setPen(Qt::black);
    int colW2 = contentW / 2;

    auto drawInfo = [&](int x, int &yp, const QString &label,
                        const QString &val) {
      painter.setFont(QFont("Arial", 9, QFont::Bold));
      painter.drawText(QRect(x, yp, colW2, mm(5)),
                       Qt::AlignRight | Qt::AlignVCenter, label);
      painter.setFont(QFont("Arial", 9));
      painter.drawText(QRect(x, yp, colW2 - mm(25), mm(5)),
                       Qt::AlignRight | Qt::AlignVCenter, val);
      yp += mm(6);
    };

    int y1p = y, y2p = y;
    drawInfo(margin + colW2, y1p, "رقم الفاتورة:",
             invTable->item(row, 0)->text());
    drawInfo(margin + colW2, y1p, "التاريخ:",
             invTable->item(row, 1)->text());
    drawInfo(margin + colW2, y1p, "العميل:",
             invTable->item(row, 2)->text());

    drawInfo(margin, y2p, "الحالة:",
             invTable->item(row, 6)->text());
    drawInfo(margin, y2p, "الرقم الضريبي:", taxNum2);

    y = qMax(y1p, y2p) + mm(3);

    // ===== Items Table =====
    int colWidths2[] = {mm(8), 0, mm(15), mm(20), mm(15), mm(18), mm(22)};
    int usedW2 = 0;
    for (int i = 0; i < 7; i++)
      if (i != 1) usedW2 += colWidths2[i];
    colWidths2[1] = contentW - usedW2;

    // Table header background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#1E40AF"));
    painter.drawRect(margin, y, contentW, mm(7));

    // Header text
    painter.setFont(QFont("Arial", 8, QFont::Bold));
    painter.setPen(Qt::white);
    int cx2 = pageW - margin;
    int cols2[7];
    for (int i = 0; i < 7; i++) {
      cx2 -= colWidths2[i];
      cols2[i] = cx2;
    }
    QString headers2[] = {"م", "الصنف", "الكمية", "السعر",
                          "الخصم", "الضريبة", "الإجمالي"};
    for (int i = 0; i < 7; i++) {
      painter.drawText(QRect(cols2[i], y, colWidths2[i], mm(7)),
                       Qt::AlignCenter, headers2[i]);
    }
    y += mm(7);

    // Table rows
    painter.setFont(QFont("Arial", 8));
    for (int r = 0; r < detailTable->rowCount(); r++) {
      if (r % 2 == 0) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#F3F4F6"));
        painter.drawRect(margin, y, contentW, mm(6));
      }
      painter.setPen(Qt::black);
      painter.setBrush(Qt::NoBrush);
      painter.drawText(QRect(cols2[0], y, colWidths2[0], mm(6)),
                       Qt::AlignCenter, QString::number(r + 1));
      for (int c = 0; c < 6; c++) {
        if (detailTable->item(r, c)) {
          painter.drawText(QRect(cols2[c + 1], y, colWidths2[c + 1], mm(6)),
                           Qt::AlignCenter, detailTable->item(r, c)->text());
        }
      }
      y += mm(6);
    }

    // Bottom line
    painter.setPen(QPen(QColor("#D1D5DB"), 1));
    painter.drawLine(margin, y, pageW - margin, y);
    y += mm(4);

    // ===== Totals =====
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.setPen(QColor("#1E40AF"));
    painter.drawText(QRect(margin, y, contentW, mm(7)), Qt::AlignCenter,
                     "المجموع: " + invTable->item(row, 3)->text() +
                         "  |  الضريبة: " + invTable->item(row, 4)->text() +
                         "  |  الإجمالي: " + invTable->item(row, 5)->text() +
                         " ريال");
    y += mm(10);

    // ===== ZATCA QR Code =====
    QString ts2 = invTable->item(row, 1)->text();
    QImage qr2 = ZatcaQR::generateZatcaQR(
        compName2, taxNum2, ts2,
        invTable->item(row, 5)->text().toDouble(),
        invTable->item(row, 4)->text().toDouble(), 3);
    if (!qr2.isNull()) {
      int qrSz = mm(25);
      painter.drawImage(QRect(margin, y, qrSz, qrSz), qr2);
    }

    painter.end();
    QMessageBox::information(&dlg, "تم", "تمت الطباعة بنجاح! ✅");
  });

  // Export PDF for selected invoice
  connect(exportPdfBtn, &QPushButton::clicked, [&]() {
    int row = invTable->currentRow();
    if (row < 0) {
      QMessageBox::warning(&dlg, "تنبيه", "اختر فاتورة أولاً!");
      return;
    }
    QString invoiceNo = invTable->item(row, 0)->text();
    QString filePath = QFileDialog::getSaveFileName(
        &dlg, "حفظ الفاتورة PDF",
        QDir::homePath() + "/Desktop/فاتورة_" + invoiceNo + ".pdf",
        "PDF Files (*.pdf)");
    if (filePath.isEmpty())
      return;

    // Generate PDF with selected invoice data (not current invoice)
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);
    QPainter painter(&writer);

    int dpi = writer.resolution();
    auto mm = [dpi](double millimeters) -> int {
      return static_cast<int>(millimeters * dpi / 25.4);
    };

    int pageW = writer.width();
    int pageH = writer.height();
    int margin = mm(15);
    int contentW = pageW - 2 * margin;
    int y = mm(10);

    QFont titleFont("Arial", 16, QFont::Bold);
    QFont headerFont("Arial", 12, QFont::Bold);
    QFont normalFont("Arial", 10);
    QFont smallFont("Arial", 8);

    // Load company info
    auto &db3 = DatabaseManager::instance();
    QString compName3 = db3.getSetting("company_name", "متجري");
    QString compPhone3 = db3.getSetting("company_phone");
    QString compAddr3 = db3.getSetting("company_address");
    QString taxNum3 = db3.getSetting("tax_number");

    // ===== Company Header =====
    painter.setFont(titleFont);
    painter.setPen(QColor("#1E40AF"));
    painter.drawText(QRect(0, y, pageW, mm(10)), Qt::AlignCenter, compName3);
    y += mm(12);

    painter.setFont(normalFont);
    painter.setPen(QColor("#6B7280"));
    if (!compPhone3.isEmpty() || !compAddr3.isEmpty()) {
      painter.drawText(QRect(0, y, pageW, mm(5)), Qt::AlignCenter,
                       compPhone3 + "  |  " + compAddr3);
      y += mm(6);
    }

    painter.setFont(headerFont);
    painter.setPen(QColor("#374151"));
    painter.drawText(QRect(0, y, pageW, mm(8)), Qt::AlignCenter,
                     "فاتورة مبيعات ضريبية");
    y += mm(10);

    // Blue separator
    painter.setPen(QPen(QColor("#3B82F6"), 3));
    painter.drawLine(margin, y, pageW - margin, y);
    y += mm(5);

    // ===== Invoice Info =====
    painter.setFont(normalFont);
    painter.setPen(Qt::black);
    int colW3 = contentW / 2;

    auto drawField3 = [&](int x, int &yPos, const QString &label,
                          const QString &value) {
      painter.setFont(QFont("Arial", 9, QFont::Bold));
      painter.drawText(QRect(x, yPos, colW3, mm(5)), Qt::AlignRight, label);
      painter.setFont(normalFont);
      painter.drawText(QRect(x, yPos, colW3 - mm(30), mm(5)), Qt::AlignRight, value);
      yPos += mm(6);
    };

    int y1 = y, y2 = y;
    drawField3(margin + colW3, y1, "رقم الفاتورة:", invTable->item(row, 0)->text());
    drawField3(margin + colW3, y1, "التاريخ:", invTable->item(row, 1)->text());
    drawField3(margin + colW3, y1, "العميل:", invTable->item(row, 2)->text());

    drawField3(margin, y2, "الحالة:", invTable->item(row, 6)->text());
    drawField3(margin, y2, "الرقم الضريبي:", taxNum3);

    y = qMax(y1, y2) + mm(3);

    // ===== Items Table =====
    QString hdr3[] = {"م", "الصنف", "الكمية", "السعر",
                      "الخصم", "الضريبة", "الإجمالي"};
    int cw3[] = {mm(8), 0, mm(18), mm(22), mm(18), mm(22), mm(25)};
    int uw3 = 0;
    for (int i = 0; i < 7; i++)
      if (i != 1) uw3 += cw3[i];
    cw3[1] = contentW - uw3;

    // Column positions (RTL)
    int cx3 = pageW - margin;
    int co3[7];
    for (int i = 0; i < 7; i++) {
      cx3 -= cw3[i];
      co3[i] = cx3;
    }

    // Table header
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.setPen(Qt::white);
    painter.setBrush(QColor("#1E40AF"));
    painter.drawRect(margin, y, contentW, mm(7));
    for (int i = 0; i < 7; i++)
      painter.drawText(QRect(co3[i], y, cw3[i], mm(7)), Qt::AlignCenter, hdr3[i]);
    y += mm(7);

    // Table rows
    painter.setFont(QFont("Arial", 9));
    for (int r = 0; r < detailTable->rowCount(); r++) {
      // Alternating colors
      painter.setPen(Qt::NoPen);
      painter.setBrush(r % 2 == 0 ? QColor("#F9FAFB") : Qt::white);
      painter.drawRect(margin, y, contentW, mm(6));
      painter.setPen(Qt::black);
      painter.setBrush(Qt::NoBrush);

      // Row number
      painter.drawText(QRect(co3[0], y, cw3[0], mm(6)),
                       Qt::AlignCenter, QString::number(r + 1));
      // Data columns
      for (int c = 0; c < 6; c++) {
        if (detailTable->item(r, c))
          painter.drawText(QRect(co3[c + 1], y, cw3[c + 1], mm(6)),
                           Qt::AlignCenter, detailTable->item(r, c)->text());
      }
      y += mm(6);
    }

    // Bottom border
    painter.setPen(QPen(QColor("#D1D5DB"), 1));
    painter.drawLine(margin, y, pageW - margin, y);
    y += mm(2);

    // ===== Totals =====
    auto drawTotal3 = [&](const QString &label, const QString &val,
                          const QColor &color, bool bold = false) {
      if (bold) painter.setFont(QFont("Arial", 10, QFont::Bold));
      else painter.setFont(normalFont);
      painter.setPen(color);
      int totalW = mm(60);
      int totalX = pageW - margin - totalW;
      painter.drawText(QRect(totalX, y, mm(35), mm(6)), Qt::AlignRight, label);
      painter.drawText(QRect(totalX + mm(36), y, mm(24), mm(6)),
                       Qt::AlignRight, val);
      y += mm(7);
    };

    drawTotal3("المجموع:", invTable->item(row, 3)->text(), Qt::black);
    drawTotal3("الضريبة:", invTable->item(row, 4)->text(), Qt::black);
    drawTotal3("الإجمالي:", invTable->item(row, 5)->text(), QColor("#1E40AF"), true);

    y += mm(10);

    // ===== ZATCA QR Code =====
    QString ts3 = invTable->item(row, 1)->text();
    double total3 = invTable->item(row, 5)->text().toDouble();
    double tax3 = invTable->item(row, 4)->text().toDouble();
    QImage qr3 = ZatcaQR::generateZatcaQR(compName3, taxNum3, ts3, total3, tax3, 3);
    if (!qr3.isNull()) {
      int qrSz = mm(35);
      painter.drawImage(QRect(margin, y, qrSz, qrSz), qr3);
      painter.setFont(smallFont);
      painter.setPen(QColor("#6B7280"));
      painter.drawText(QRect(margin + qrSz + mm(5), y + mm(5), contentW, mm(5)),
                       Qt::AlignVCenter, "ZATCA E-Invoice QR Code");
    }

    // ===== Footer =====
    painter.setFont(smallFont);
    painter.setPen(QColor("#9CA3AF"));
    int footerY = pageH - mm(15);
    painter.drawLine(margin, footerY, pageW - margin, footerY);
    footerY += mm(3);
    painter.drawText(QRect(0, footerY, pageW, mm(5)), Qt::AlignCenter,
                     "هذه الفاتورة صادرة عن " + compName3 +
                         "  |  الرقم الضريبي: " + taxNum3);

    painter.end();
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
  });

  // Select first row
  if (invTable->rowCount() > 0)
    invTable->selectRow(0);

  dlg.exec();
}
