#pragma once
#include <QString>

struct Product {
  int id = 0;
  QString name;
  QString barcode;
  int categoryId = 1;
  int unitId = 1;
  double buyPrice = 0;
  double sellPrice = 0;
  double taxRate = 15;
  double minQuantity = 0;
  double currentStock = 0;
  QString description;
  bool active = true;
  QString categoryName;
  QString unitName;
  QString nameEn;
  QString brandAr;
  QString brandEn;
  QString sizeInfo;
  QString imagePath;
};
