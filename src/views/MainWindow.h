#pragma once
#include "views/AttendanceWidget.h"
#include "views/BookingWidget.h"
#include "views/CustomersWidget.h"
#include "views/DashboardWidget.h"
#include "views/EmployeesWidget.h"
#include "views/ExpensesWidget.h"
#include "views/MaintenanceWidget.h"
#include "views/ProductsWidget.h"
#include "views/PurchaseReturnsWidget.h"
#include "views/PurchasesWidget.h"
#include "views/ReportsWidget.h"
#include "views/SalaryWidget.h"
#include "views/SalesReturnsWidget.h"
#include "views/SalesWidget.h"
#include "views/SettingsWidget.h"
#include "views/StockTransferWidget.h"
#include "views/StockWidget.h"
#include "views/TreasuryWidget.h"
#include "views/VacationWidget.h"
#include "widgets/SidebarWidget.h"
#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QStackedWidget>

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void onMenuClicked(int index);

private:
  SidebarWidget *m_sidebar;
  QStackedWidget *m_stack;
  QLabel *m_headerTitle;
  DashboardWidget *m_dashboard;

  QStringList m_titles = {
      "🏠 لوحة التحكم",   "💰 المبيعات",    "🛒 المشتريات",
      "📦 المنتجات",      "🏭 المخزون",     "👥 العملاء والموردين",
      "💸 المصروفات",     "📊 التقارير",    "🔄 مرتجع مبيعات",
      "🔄 مرتجع مشتريات", "💰 الخزينة",     "👨‍💼 الموظفين",
      "🕐 حضور وانصراف",  "🔀 تحويل أصناف", "📋 فواتير الحجز",
      "💵 الرواتب",       "🏖️ الإجازات",    "🔧 الصيانة",
      "⚙️ الإعدادات"};
  void createMenuBar();
};
