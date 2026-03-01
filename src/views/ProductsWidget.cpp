#include "ProductsWidget.h"
#include "database/DatabaseManager.h"
#include "widgets/SearchBar.h"
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>

ProductsWidget::ProductsWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  // Title
  auto *titleLabel = new QLabel("📦  إدارة المنتجات");
  titleLabel->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(titleLabel);

  // Top bar
  auto *topBar = new QHBoxLayout;

  auto *search = new SearchBar("🔍  بحث عن منتج...");
  search->setMinimumWidth(300);
  topBar->addWidget(search);

  auto *catLabel = new QLabel("التصنيف:");
  catLabel->setStyleSheet("background: transparent;");
  topBar->addWidget(catLabel);
  m_categoryCombo = new QComboBox;
  m_categoryCombo->setMinimumWidth(150);
  topBar->addWidget(m_categoryCombo);

  topBar->addStretch();

  auto *importBtn = new QPushButton("📥 استيراد من Excel");
  importBtn->setCursor(Qt::PointingHandCursor);
  importBtn->setStyleSheet(
      "font-size: 13px; font-weight: bold; "
      "background: #6366F1; color: white; "
      "border-radius: 8px; border: none; padding: 8px 14px;");
  topBar->addWidget(importBtn);

  auto *addBtn = new QPushButton("➕  إضافة منتج");
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

  auto *delAllBtn = new QPushButton("⚠️ حذف الكل");
  delAllBtn->setCursor(Qt::PointingHandCursor);
  delAllBtn->setStyleSheet(
      "font-size: 13px; font-weight: bold; "
      "background: #DC2626; color: white; "
      "border-radius: 8px; border: none; padding: 8px 14px;");
  topBar->addWidget(delAllBtn);

  layout->addLayout(topBar);

  // Table with image column
  m_table = new QTableWidget;
  m_table->setColumnCount(11);
  m_table->setHorizontalHeaderLabels(
      {"#", "الصورة", "الاسم", "الاسم EN", "الباركود", "التصنيف", "الماركة",
       "سعر البيع", "الضريبة%", "المخزون", "الحد الأدنى"});
  m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  m_table->setIconSize(QSize(48, 48));
  m_table->setColumnWidth(1, 60); // Image column width
  layout->addWidget(m_table, 1);

  // Connections
  connect(search, &QLineEdit::textChanged, this, &ProductsWidget::onSearch);
  connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ProductsWidget::onCategoryFilter);
  connect(addBtn, &QPushButton::clicked, this, &ProductsWidget::onAdd);
  connect(editBtn, &QPushButton::clicked, this, &ProductsWidget::onEdit);
  connect(delBtn, &QPushButton::clicked, this, &ProductsWidget::onDelete);
  connect(m_table, &QTableWidget::doubleClicked, this, &ProductsWidget::onEdit);

  // Delete All Products
  connect(delAllBtn, &QPushButton::clicked, [this]() {
    if (QMessageBox::warning(this, "⚠️ تحذير",
            "هل أنت متأكد أنك تريد حذف جميع المنتجات؟\n\n"
            "هذا الإجراء لا يمكن التراجع عنه!",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) != QMessageBox::Yes) return;
    if (QMessageBox::critical(this, "⚠️ تأكيد نهائي",
            "هل أنت متأكد 100%؟\n\nسيتم حذف جميع المنتجات نهائياً!",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) != QMessageBox::Yes) return;

    DatabaseManager::instance().deleteAllProducts();
    loadProducts();
    QMessageBox::information(this, "تم", "✅ تم حذف جميع المنتجات!");
  });

  // Import from Excel
  connect(importBtn, &QPushButton::clicked, [this]() {
    QString file = QFileDialog::getOpenFileName(this,
        "اختر ملف Excel", "",
        "Excel Files (*.xlsx *.xls);;CSV Files (*.csv)");
    if (file.isEmpty()) return;

    QString appDir = QCoreApplication::applicationDirPath();
    QString script = appDir + "/import_products.py";
    QString dbPath = appDir + "/data/pos_database.db";
    QString imagesDir = appDir + "/data/images";

    if (!QFile::exists(script)) {
      QMessageBox::critical(this, "خطأ",
          "ملف import_products.py غير موجود!");
      return;
    }

    QProcess *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    QProgressDialog *progress = new QProgressDialog(
        "⏳ جاري استيراد المنتجات...", "إلغاء", 0, 2, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setLayoutDirection(Qt::RightToLeft);
    progress->setMinimumWidth(400);
    progress->show();

    connect(proc, &QProcess::readyReadStandardOutput, [proc, progress]() {
      QString out = QString::fromUtf8(proc->readAll());
      for (const QString &line : out.split('\n')) {
        if (line.startsWith("STEP:")) {
          QStringList parts = line.split(':');
          if (parts.size() >= 3) {
            progress->setValue(parts[1].toInt());
            progress->setLabelText("⏳ " + parts[2]);
          }
        }
      }
    });

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int code, QProcess::ExitStatus) {
      progress->close();
      progress->deleteLater();

      QString output = QString::fromUtf8(proc->readAll());
      if (code == 0 && output.contains("DONE:")) {
        // Extract result message
        for (const QString &line : output.split('\n')) {
          if (line.startsWith("DONE:")) {
            QMessageBox::information(this, "تم الاستيراد! ✅",
                "🎉 " + line.mid(5));
            break;
          }
        }
      } else {
        QMessageBox::critical(this, "خطأ في الاستيراد",
            "❌ فشل استيراد المنتجات:\n\n" + output);
      }
      loadProducts();
      proc->deleteLater();
    });

    proc->start("python", {"-u", script, dbPath, file, imagesDir});
  });

  loadCategories();
  loadProducts();
}

void ProductsWidget::refresh() { loadProducts(); }

QString ProductsWidget::getImagesDir() const {
  return QCoreApplication::applicationDirPath() + "/data/images";
}

void ProductsWidget::loadCategories() {
  m_categoryCombo->clear();
  m_categoryCombo->addItem("الكل", -1);
  auto q = DatabaseManager::instance().getCategories();
  while (q.next()) {
    m_categoryCombo->addItem(q.value("name").toString(), q.value("id").toInt());
  }
}

void ProductsWidget::loadProducts(const QString &search, int categoryId) {
  m_table->setRowCount(0);
  QString imgDir = getImagesDir();
  auto q = DatabaseManager::instance().getProducts(search, categoryId);
  while (q.next()) {
    int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setRowHeight(row, 52);

    auto makeItem = [](const QString &text) {
      auto *item = new QTableWidgetItem(text);
      item->setTextAlignment(Qt::AlignCenter);
      return item;
    };

    m_table->setItem(row, 0, makeItem(q.value("id").toString()));

    // Image thumbnail
    QString imgPath = q.value("image_path").toString();
    auto *imgItem = new QTableWidgetItem();
    if (!imgPath.isEmpty()) {
      QString fullPath = imgDir + "/" + imgPath;
      if (QFileInfo::exists(fullPath)) {
        QPixmap pix(fullPath);
        if (!pix.isNull()) {
          imgItem->setIcon(QIcon(pix.scaled(48, 48, Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation)));
        }
      }
    }
    imgItem->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 1, imgItem);

    m_table->setItem(row, 2, makeItem(q.value("name").toString()));
    m_table->setItem(row, 3, makeItem(q.value("name_en").toString()));
    m_table->setItem(row, 4, makeItem(q.value("barcode").toString()));
    m_table->setItem(row, 5, makeItem(q.value("category_name").toString()));
    m_table->setItem(row, 6, makeItem(q.value("brand_ar").toString()));
    m_table->setItem(
        row, 7,
        makeItem(QString::number(q.value("sell_price").toDouble(), 'f', 2)));
    m_table->setItem(
        row, 8,
        makeItem(QString::number(q.value("tax_rate").toDouble(), 'f', 0) +
                 "%"));
    m_table->setItem(
        row, 9,
        makeItem(QString::number(q.value("current_stock").toDouble(), 'f', 0)));
    auto *minItem =
        makeItem(QString::number(q.value("min_quantity").toDouble(), 'f', 0));
    double stock = q.value("current_stock").toDouble();
    double minQ = q.value("min_quantity").toDouble();
    if (minQ > 0 && stock <= minQ) {
      m_table->item(row, 9)->setForeground(QColor("#EF4444"));
      minItem->setForeground(QColor("#EF4444"));
    }
    m_table->setItem(row, 10, minItem);
  }
}

void ProductsWidget::onSearch(const QString &text) {
  int catId = m_categoryCombo->currentData().toInt();
  loadProducts(text, catId);
}

void ProductsWidget::onCategoryFilter() {
  int catId = m_categoryCombo->currentData().toInt();
  loadProducts("", catId);
}

void ProductsWidget::showProductDialog(int productId) {
  QDialog dlg(this);
  dlg.setWindowTitle(productId > 0 ? "تعديل منتج" : "إضافة منتج جديد");
  dlg.setMinimumSize(550, 600);
  dlg.setLayoutDirection(Qt::RightToLeft);

  auto *mainLayout = new QVBoxLayout(&dlg);

  // Image preview at top
  auto *imageLabel = new QLabel;
  imageLabel->setFixedSize(150, 150);
  imageLabel->setAlignment(Qt::AlignCenter);
  imageLabel->setStyleSheet(
      "QLabel { border: 2px dashed #555; border-radius: 8px; "
      "background: rgba(255,255,255,0.05); }");
  imageLabel->setText("📷 لا توجد صورة");

  auto *imgContainer = new QHBoxLayout;
  imgContainer->addStretch();
  imgContainer->addWidget(imageLabel);
  imgContainer->addStretch();
  mainLayout->addLayout(imgContainer);

  auto *form = new QFormLayout;
  form->setSpacing(12);

  auto *nameEdit = new QLineEdit;
  auto *nameEnEdit = new QLineEdit;
  auto *barcodeEdit = new QLineEdit;
  auto *catCombo = new QComboBox;
  auto *unitCombo = new QComboBox;
  auto *brandArEdit = new QLineEdit;
  auto *brandEnEdit = new QLineEdit;
  auto *sizeEdit = new QLineEdit;
  auto *buyPriceSpin = new QDoubleSpinBox;
  buyPriceSpin->setMaximum(999999);
  buyPriceSpin->setDecimals(2);
  auto *sellPriceSpin = new QDoubleSpinBox;
  sellPriceSpin->setMaximum(999999);
  sellPriceSpin->setDecimals(2);
  auto *taxSpin = new QDoubleSpinBox;
  taxSpin->setMaximum(100);
  taxSpin->setValue(15);
  auto *minQtySpin = new QDoubleSpinBox;
  minQtySpin->setMaximum(999999);

  auto &db = DatabaseManager::instance();
  auto cq = db.getCategories();
  while (cq.next())
    catCombo->addItem(cq.value("name").toString(), cq.value("id").toInt());
  auto uq = db.getUnits();
  while (uq.next())
    unitCombo->addItem(uq.value("name").toString(), uq.value("id").toInt());

  QString currentImagePath;

  if (productId > 0) {
    auto pq = db.getProductById(productId);
    if (pq.next()) {
      nameEdit->setText(pq.value("name").toString());
      nameEnEdit->setText(pq.value("name_en").toString());
      barcodeEdit->setText(pq.value("barcode").toString());
      brandArEdit->setText(pq.value("brand_ar").toString());
      brandEnEdit->setText(pq.value("brand_en").toString());
      sizeEdit->setText(pq.value("size_info").toString());
      buyPriceSpin->setValue(pq.value("buy_price").toDouble());
      sellPriceSpin->setValue(pq.value("sell_price").toDouble());
      taxSpin->setValue(pq.value("tax_rate").toDouble());
      minQtySpin->setValue(pq.value("min_quantity").toDouble());
      int ci = catCombo->findData(pq.value("category_id").toInt());
      if (ci >= 0)
        catCombo->setCurrentIndex(ci);
      int ui = unitCombo->findData(pq.value("unit_id").toInt());
      if (ui >= 0)
        unitCombo->setCurrentIndex(ui);

      // Load image
      currentImagePath = pq.value("image_path").toString();
      if (!currentImagePath.isEmpty()) {
        QString fullPath = getImagesDir() + "/" + currentImagePath;
        if (QFileInfo::exists(fullPath)) {
          QPixmap pix(fullPath);
          if (!pix.isNull()) {
            imageLabel->setPixmap(pix.scaled(140, 140, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation));
            imageLabel->setText("");
          }
        }
      }
    }
  }

  form->addRow("اسم المنتج:", nameEdit);
  form->addRow("الاسم بالإنجليزي:", nameEnEdit);
  form->addRow("الباركود:", barcodeEdit);
  form->addRow("التصنيف:", catCombo);
  form->addRow("الوحدة:", unitCombo);
  form->addRow("الماركة (عربي):", brandArEdit);
  form->addRow("الماركة (إنجليزي):", brandEnEdit);
  form->addRow("الحجم:", sizeEdit);
  form->addRow("سعر الشراء:", buyPriceSpin);
  form->addRow("سعر البيع:", sellPriceSpin);
  form->addRow("نسبة الضريبة:", taxSpin);
  form->addRow("الحد الأدنى:", minQtySpin);
  mainLayout->addLayout(form);

  auto *btnLayout = new QHBoxLayout;
  auto *saveBtn = new QPushButton(productId > 0 ? "تحديث" : "حفظ");
  saveBtn->setObjectName("btnSuccess");
  auto *cancelBtn = new QPushButton("إلغاء");
  cancelBtn->setObjectName("btnDanger");
  btnLayout->addWidget(saveBtn);
  btnLayout->addWidget(cancelBtn);
  mainLayout->addLayout(btnLayout);

  connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
  connect(saveBtn, &QPushButton::clicked, [&]() {
    if (nameEdit->text().trimmed().isEmpty()) {
      QMessageBox::warning(&dlg, "تنبيه", "يرجى إدخال اسم المنتج");
      return;
    }
    if (productId > 0) {
      db.updateProduct(
          productId, nameEdit->text(), barcodeEdit->text(),
          catCombo->currentData().toInt(), unitCombo->currentData().toInt(),
          buyPriceSpin->value(), sellPriceSpin->value(), taxSpin->value(),
          minQtySpin->value(), nameEnEdit->text(), brandArEdit->text(),
          brandEnEdit->text(), sizeEdit->text(), currentImagePath);
    } else {
      db.addProduct(
          nameEdit->text(), barcodeEdit->text(),
          catCombo->currentData().toInt(), unitCombo->currentData().toInt(),
          buyPriceSpin->value(), sellPriceSpin->value(), taxSpin->value(),
          minQtySpin->value(), nameEnEdit->text(), brandArEdit->text(),
          brandEnEdit->text(), sizeEdit->text(), "");
    }
    dlg.accept();
  });

  if (dlg.exec() == QDialog::Accepted)
    loadProducts();
}

void ProductsWidget::onAdd() { showProductDialog(); }
void ProductsWidget::onEdit() {
  if (m_table->currentRow() < 0)
    return;
  int id = m_table->item(m_table->currentRow(), 0)->text().toInt();
  showProductDialog(id);
}
void ProductsWidget::onDelete() {
  if (m_table->currentRow() < 0)
    return;
  int id = m_table->item(m_table->currentRow(), 0)->text().toInt();
  if (QMessageBox::question(this, "تأكيد", "هل أنت متأكد من حذف هذا المنتج؟") ==
      QMessageBox::Yes) {
    DatabaseManager::instance().deleteProduct(id);
    loadProducts();
  }
}
