#include "SidebarWidget.h"

SidebarWidget::SidebarWidget(QWidget *parent) : QWidget(parent) {
  setObjectName("sidebar");
  setFixedWidth(190);

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  // Logo
  auto *logoLabel = new QLabel("  🏪 نظام المحاسبة");
  logoLabel->setObjectName("sidebarLogo");
  layout->addWidget(logoLabel);

  layout->addSpacing(10);

  m_btnGroup = new QButtonGroup(this);
  m_btnGroup->setExclusive(true);

  addMenuButton(layout, "🏠", "  لوحة التحكم", 0);
  addMenuButton(layout, "💰", "  المبيعات", 1);
  addMenuButton(layout, "🛒", "  المشتريات", 2);
  addMenuButton(layout, "📦", "  المنتجات", 3);
  addMenuButton(layout, "🏭", "  المخزون", 4);
  addMenuButton(layout, "👥", "  العملاء والموردين", 5);
  addMenuButton(layout, "💸", "  المصروفات", 6);
  addMenuButton(layout, "📊", "  التقارير", 7);
  addMenuButton(layout, "🔄", "  مرتجع مبيعات", 8);
  addMenuButton(layout, "🔄", "  مرتجع مشتريات", 9);
  addMenuButton(layout, "💰", "  الخزينة", 10);
  addMenuButton(layout, "👨‍💼", "  الموظفين", 11);
  addMenuButton(layout, "🕐", "  حضور وانصراف", 12);
  addMenuButton(layout, "🔀", "  تحويل أصناف", 13);
  addMenuButton(layout, "📋", "  فواتير الحجز", 14);
  addMenuButton(layout, "💵", "  الرواتب", 15);
  addMenuButton(layout, "🏖️", "  الإجازات", 16);
  addMenuButton(layout, "🔧", "  الصيانة", 17);
  addMenuButton(layout, "⚙️", "  الإعدادات", 18);

  layout->addStretch();

  // User section
  auto *userSection = new QWidget;
  userSection->setObjectName("sidebarUserSection");
  auto *userLayout = new QVBoxLayout(userSection);
  userLayout->setContentsMargins(15, 10, 15, 10);

  m_userNameLabel = new QLabel("مدير النظام");
  m_userNameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: "
                                 "#E8EAED; background: transparent;");
  userLayout->addWidget(m_userNameLabel);

  m_userRoleLabel = new QLabel("مدير");
  m_userRoleLabel->setStyleSheet(
      "font-size: 11px; color: #6B7280; background: transparent;");
  userLayout->addWidget(m_userRoleLabel);

  auto *logoutBtn = new QPushButton("🚪  تسجيل الخروج");
  logoutBtn->setObjectName("btnDanger");
  logoutBtn->setCursor(Qt::PointingHandCursor);
  logoutBtn->setStyleSheet("margin-top: 8px; padding: 8px; font-size: 12px;");
  connect(logoutBtn, &QPushButton::clicked, [this]() { emit menuClicked(-1); });
  userLayout->addWidget(logoutBtn);

  layout->addWidget(userSection);

  if (auto *btn = m_btnGroup->button(0)) {
    btn->setChecked(true);
  }

  connect(m_btnGroup, &QButtonGroup::idClicked, this,
          &SidebarWidget::menuClicked);
}

void SidebarWidget::setUserInfo(const QString &name, const QString &role) {
  m_userNameLabel->setText(name);
  QString roleAr = role;
  if (role == "admin")
    roleAr = "مدير";
  else if (role == "cashier")
    roleAr = "كاشير";
  else if (role == "accountant")
    roleAr = "محاسب";
  m_userRoleLabel->setText(roleAr);
}

void SidebarWidget::addMenuButton(QVBoxLayout *layout, const QString &icon,
                                  const QString &text, int index) {
  auto *btn = new QPushButton(icon + text);
  btn->setObjectName("sidebarBtn");
  btn->setCheckable(true);
  btn->setCursor(Qt::PointingHandCursor);
  btn->setLayoutDirection(Qt::RightToLeft);
  m_btnGroup->addButton(btn, index);
  layout->addWidget(btn);
}
