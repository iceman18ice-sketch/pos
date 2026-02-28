#include "StockWidget.h"
#include "database/DatabaseManager.h"
#include <QHeaderView>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>


StockWidget::StockWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("🏭  إدارة المخزون");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  auto *tabs = new QTabWidget;

  // Stock Report Tab
  auto *stockTab = new QWidget;
  auto *stockLayout = new QVBoxLayout(stockTab);
  m_stockTable = new QTableWidget;
  m_stockTable->setColumnCount(7);
  m_stockTable->setHorizontalHeaderLabels({"المنتج", "الباركود", "التصنيف",
                                           "المتوفر", "الحد الأدنى",
                                           "سعر الشراء", "قيمة المخزون"});
  m_stockTable->horizontalHeader()->setSectionResizeMode(0,
                                                         QHeaderView::Stretch);
  m_stockTable->setAlternatingRowColors(true);
  m_stockTable->verticalHeader()->hide();
  m_stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_stockTable->setLayoutDirection(Qt::RightToLeft);
  stockLayout->addWidget(m_stockTable);
  tabs->addTab(stockTab, "📦 تقرير المخزون");

  // Movements Tab
  auto *movTab = new QWidget;
  auto *movLayout = new QVBoxLayout(movTab);
  m_movementsTable = new QTableWidget;
  m_movementsTable->setColumnCount(6);
  m_movementsTable->setHorizontalHeaderLabels(
      {"التاريخ", "المنتج", "النوع", "الكمية", "المرجع", "ملاحظات"});
  m_movementsTable->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::Stretch);
  m_movementsTable->setAlternatingRowColors(true);
  m_movementsTable->verticalHeader()->hide();
  m_movementsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_movementsTable->setLayoutDirection(Qt::RightToLeft);
  movLayout->addWidget(m_movementsTable);
  tabs->addTab(movTab, "📋 حركات المخزون");

  layout->addWidget(tabs);
  refresh();
}

void StockWidget::refresh() {
  loadStockReport();
  loadMovements();
}

void StockWidget::loadStockReport() {
  m_stockTable->setRowCount(0);
  auto q = DatabaseManager::instance().getStockReport();
  while (q.next()) {
    int r = m_stockTable->rowCount();
    m_stockTable->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_stockTable->setItem(r, 0, mi(q.value("name").toString()));
    m_stockTable->setItem(r, 1, mi(q.value("barcode").toString()));
    m_stockTable->setItem(r, 2, mi(q.value("category_name").toString()));
    m_stockTable->setItem(
        r, 3, mi(QString::number(q.value("current_stock").toDouble(), 'f', 0)));
    m_stockTable->setItem(
        r, 4, mi(QString::number(q.value("min_quantity").toDouble(), 'f', 0)));
    m_stockTable->setItem(
        r, 5, mi(QString::number(q.value("buy_price").toDouble(), 'f', 2)));
    m_stockTable->setItem(
        r, 6, mi(QString::number(q.value("stock_value").toDouble(), 'f', 2)));
    double s = q.value("current_stock").toDouble(),
           mn = q.value("min_quantity").toDouble();
    if (mn > 0 && s <= mn) {
      m_stockTable->item(r, 3)->setForeground(QColor("#EF4444"));
      m_stockTable->item(r, 4)->setForeground(QColor("#EF4444"));
    }
  }
}

void StockWidget::loadMovements() {
  m_movementsTable->setRowCount(0);
  auto q = DatabaseManager::instance().getStockMovements();
  int count = 0;
  while (q.next() && count < 100) {
    int r = m_movementsTable->rowCount();
    m_movementsTable->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_movementsTable->setItem(
        r, 0, mi(q.value("date").toDateTime().toString("yyyy-MM-dd HH:mm")));
    m_movementsTable->setItem(r, 1, mi(q.value("product_name").toString()));
    QString type = q.value("type").toString();
    QString typeAr = type == "in"         ? "وارد"
                     : type == "out"      ? "صادر"
                     : type == "transfer" ? "تحويل"
                                          : "تعديل";
    auto *typeItem = mi(typeAr);
    typeItem->setForeground(type == "in" ? QColor("#10B981")
                                         : QColor("#EF4444"));
    m_movementsTable->setItem(r, 2, typeItem);
    m_movementsTable->setItem(
        r, 3, mi(QString::number(q.value("quantity").toDouble(), 'f', 2)));
    m_movementsTable->setItem(r, 4, mi(q.value("reference_type").toString()));
    m_movementsTable->setItem(r, 5, mi(q.value("notes").toString()));
    count++;
  }
}
