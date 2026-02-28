#pragma once
#include <QString>

struct User {
  int id = 0;
  QString username;
  QString fullName;
  QString role;
  QString phone;
  bool active = true;
};
