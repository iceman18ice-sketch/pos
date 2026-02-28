#include "InvoiceTable.h"
#include <QHeaderView>

InvoiceTable::InvoiceTable(QWidget *parent) : QTableWidget(parent) {
  setColumnCount(8);
  setHorizontalHeaderLabels({"#", "المنتج", "الكمية", "السعر", "الخصم",
                             "الضريبة", "الإجمالي", "معرف"});
  horizontalHeader()->setStretchLastSection(false);
  horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  setColumnHidden(7, true); // Hide product ID column
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setAlternatingRowColors(true);
  setLayoutDirection(Qt::RightToLeft);
  verticalHeader()->hide();

  setColumnWidth(0, 40);
  setColumnWidth(2, 80);
  setColumnWidth(3, 100);
  setColumnWidth(4, 80);
  setColumnWidth(5, 80);
  setColumnWidth(6, 110);
}

void InvoiceTable::addItem(int productId, const QString &name, double qty,
                           double price, double taxRate) {
  // Check if product already exists
  for (int r = 0; r < rowCount(); r++) {
    if (item(r, 7) && item(r, 7)->text().toInt() == productId) {
      double curQty = item(r, 2)->text().toDouble();
      curQty += qty;
      item(r, 2)->setText(QString::number(curQty, 'f', 2));
      double lineTotal = curQty * price;
      double taxVal = lineTotal * taxRate / 100.0;
      item(r, 5)->setText(QString::number(taxVal, 'f', 2));
      item(r, 6)->setText(QString::number(lineTotal + taxVal, 'f', 2));
      return;
    }
  }

  int row = rowCount();
  insertRow(row);

  double lineTotal = qty * price;
  double taxVal = lineTotal * taxRate / 100.0;
  double total = lineTotal + taxVal;

  auto makeItem = [](const QString &text) {
    auto *item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    return item;
  };

  setItem(row, 0, makeItem(QString::number(row + 1)));
  setItem(row, 1, makeItem(name));
  setItem(row, 2, makeItem(QString::number(qty, 'f', 2)));
  setItem(row, 3, makeItem(QString::number(price, 'f', 2)));
  setItem(row, 4, makeItem("0.00"));
  setItem(row, 5, makeItem(QString::number(taxVal, 'f', 2)));
  setItem(row, 6, makeItem(QString::number(total, 'f', 2)));
  setItem(row, 7, makeItem(QString::number(productId)));
}

void InvoiceTable::removeSelectedItem() {
  auto rows = selectionModel()->selectedRows();
  if (!rows.isEmpty()) {
    removeRow(rows.first().row());
    // Re-number
    for (int r = 0; r < rowCount(); r++) {
      item(r, 0)->setText(QString::number(r + 1));
    }
  }
}

double InvoiceTable::getSubtotal() const {
  double total = 0;
  for (int r = 0; r < rowCount(); r++) {
    double qty = item(r, 2)->text().toDouble();
    double price = item(r, 3)->text().toDouble();
    total += qty * price;
  }
  return total;
}

double InvoiceTable::getTaxTotal() const {
  double total = 0;
  for (int r = 0; r < rowCount(); r++) {
    total += item(r, 5)->text().toDouble();
  }
  return total;
}

double InvoiceTable::getGrandTotal() const {
  double total = 0;
  for (int r = 0; r < rowCount(); r++) {
    total += item(r, 6)->text().toDouble();
  }
  return total;
}

QList<InvoiceTable::ItemData> InvoiceTable::getAllItems() const {
  QList<ItemData> items;
  for (int r = 0; r < rowCount(); r++) {
    ItemData d;
    d.productId = item(r, 7)->text().toInt();
    d.name = item(r, 1)->text();
    d.qty = item(r, 2)->text().toDouble();
    d.price = item(r, 3)->text().toDouble();
    d.discount = item(r, 4)->text().toDouble();
    d.taxRate = 15; // from settings
    d.taxValue = item(r, 5)->text().toDouble();
    d.total = item(r, 6)->text().toDouble();
    items.append(d);
  }
  return items;
}

void InvoiceTable::clearAll() { setRowCount(0); }

bool InvoiceTable::removeItemByProductId(int productId) {
  for (int r = 0; r < rowCount(); r++) {
    if (item(r, 7) && item(r, 7)->text().toInt() == productId) {
      double curQty = item(r, 2)->text().toDouble();
      double price = item(r, 3)->text().toDouble();
      double taxRate = 15.0;
      if (curQty <= 1.0) {
        // Remove the entire row
        removeRow(r);
        // Re-number
        for (int i = 0; i < rowCount(); i++) {
          item(i, 0)->setText(QString::number(i + 1));
        }
      } else {
        // Decrease quantity by 1
        curQty -= 1.0;
        item(r, 2)->setText(QString::number(curQty, 'f', 2));
        double lineTotal = curQty * price;
        double taxVal = lineTotal * taxRate / 100.0;
        item(r, 5)->setText(QString::number(taxVal, 'f', 2));
        item(r, 6)->setText(QString::number(lineTotal + taxVal, 'f', 2));
      }
      return true;
    }
  }
  return false;
}
