#pragma once
#include <QString>

struct Customer {
  int id = 0;
  QString name;
  QString phone;
  QString address;
  int type = 0; // 0=customer, 1=supplier, 2=both
  double balance = 0;
  double creditLimit = 0;
  QString taxNumber;
  QString notes;
  bool active = true;
};
