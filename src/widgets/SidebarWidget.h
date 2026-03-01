#pragma once
#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
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
  void addSectionLabel(QVBoxLayout *layout, const QString &text);
  void addMenuButton(QVBoxLayout *layout, const QString &icon,
                     const QString &text, int index);
  QButtonGroup *m_btnGroup;
  QLabel *m_userNameLabel;
  QLabel *m_userRoleLabel;
};
