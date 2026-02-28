#pragma once
#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>


class SidebarWidget : public QWidget {
  Q_OBJECT
public:
  explicit SidebarWidget(QWidget *parent = nullptr);
  void setUserInfo(const QString &name, const QString &role);

signals:
  void menuClicked(int index);

private:
  QButtonGroup *m_btnGroup;
  QLabel *m_userNameLabel;
  QLabel *m_userRoleLabel;
  void addMenuButton(QVBoxLayout *layout, const QString &icon,
                     const QString &text, int index);
};
