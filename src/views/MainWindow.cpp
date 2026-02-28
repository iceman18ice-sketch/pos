#include "MainWindow.h"
#include "database/DatabaseManager.h"
#include <QApplication>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("نظام المحاسبة - POS System v3.0");
  setMinimumSize(1200, 750);
  showMaximized();
  setLayoutDirection(Qt::RightToLeft);

  createMenuBar();

  auto *centralWidget = new QWidget;
  auto *mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  m_sidebar = new SidebarWidget;
  m_sidebar->setUserInfo(DatabaseManager::instance().currentUserName(),
                         DatabaseManager::instance().currentUserRole());
  mainLayout->addWidget(m_sidebar);

  auto *rightWidget = new QWidget;
  auto *rightLayout = new QVBoxLayout(rightWidget);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(0);

  auto *headerWidget = new QWidget;
  headerWidget->setObjectName("headerBar");
  auto *headerLayout = new QHBoxLayout(headerWidget);
  m_headerTitle = new QLabel("🏠 لوحة التحكم");
  m_headerTitle->setObjectName("headerTitle");
  headerLayout->addWidget(m_headerTitle);
  headerLayout->addStretch();
  auto *dateLabel = new QLabel(
      QDateTime::currentDateTime().toString("yyyy/MM/dd - hh:mm AP"));
  dateLabel->setObjectName("headerDate");
  headerLayout->addWidget(dateLabel);
  rightLayout->addWidget(headerWidget);

  auto *contentWrapper = new QWidget;
  contentWrapper->setObjectName("contentArea");
  auto *contentLayout = new QVBoxLayout(contentWrapper);
  contentLayout->setContentsMargins(16, 16, 16, 16);

  m_stack = new QStackedWidget;
  m_dashboard = new DashboardWidget;
  m_stack->addWidget(m_dashboard);               // 0  - لوحة التحكم
  m_stack->addWidget(new SalesWidget);           // 1  - المبيعات
  m_stack->addWidget(new PurchasesWidget);       // 2  - المشتريات
  m_stack->addWidget(new ProductsWidget);        // 3  - المنتجات
  m_stack->addWidget(new StockWidget);           // 4  - المخزون
  m_stack->addWidget(new CustomersWidget);       // 5  - العملاء
  m_stack->addWidget(new ExpensesWidget);        // 6  - المصروفات
  m_stack->addWidget(new ReportsWidget);         // 7  - التقارير
  m_stack->addWidget(new SalesReturnsWidget);    // 8  - مرتجع مبيعات
  m_stack->addWidget(new PurchaseReturnsWidget); // 9  - مرتجع مشتريات
  m_stack->addWidget(new TreasuryWidget);        // 10 - الخزينة
  m_stack->addWidget(new EmployeesWidget);       // 11 - الموظفين
  m_stack->addWidget(new AttendanceWidget);      // 12 - حضور وانصراف
  m_stack->addWidget(new StockTransferWidget);   // 13 - تحويل أصناف
  m_stack->addWidget(new BookingWidget);         // 14 - فواتير الحجز
  m_stack->addWidget(new SalaryWidget);          // 15 - الرواتب
  m_stack->addWidget(new VacationWidget);        // 16 - الإجازات
  m_stack->addWidget(new MaintenanceWidget);     // 17 - الصيانة
  m_stack->addWidget(new SettingsWidget);        // 18 - الإعدادات

  contentLayout->addWidget(m_stack);
  rightLayout->addWidget(contentWrapper, 1);
  mainLayout->addWidget(rightWidget, 1);
  setCentralWidget(centralWidget);

  // Status bar matching Sales Manager's bottom bar
  auto *sb = statusBar();
  sb->setStyleSheet("QStatusBar { background: #1E1E2E; color: #CDD6F4; "
                    "border-top: 1px solid #313244; padding: 4px; }"
                    "QStatusBar::item { border: none; }");
  sb->addPermanentWidget(new QLabel("🏢 الفرع: المقر الرئيسي   "));
  sb->addPermanentWidget(new QLabel(
      "👤 المستخدم: " + DatabaseManager::instance().currentUserName() + "   "));
  sb->addPermanentWidget(new QLabel(
      "📅 التاريخ: " + QDateTime::currentDateTime().toString("yyyy/MM/dd") +
      "   "));
  sb->addPermanentWidget(new QLabel("💻 نظام المحاسبة v3.0.0"));

  connect(m_sidebar, &SidebarWidget::menuClicked, this,
          &MainWindow::onMenuClicked);
}

void MainWindow::createMenuBar() {
  auto *bar = menuBar();
  bar->setLayoutDirection(Qt::RightToLeft);

  // ===== 1. ملف (File) =====
  auto *fileMenu = bar->addMenu("📁 ملف");
  fileMenu->addAction("⚙️ الإعدادات", [this]() { onMenuClicked(18); });
  fileMenu->addAction("👤 المستخدمين", [this]() {
    QMessageBox::information(
        this, "المستخدمين",
        "إدارة المستخدمين وصلاحياتهم.\n"
        "يمكنك إضافة مستخدمين جدد وتعيين أدوارهم من الإعدادات.");
  });
  fileMenu->addAction("🗄️ قاعدة البيانات", [this]() {
    QMessageBox::information(this, "قاعدة البيانات",
                             "📂 مسار قاعدة البيانات:\ndata/pos_database.db\n\n"
                             "يمكنك أخذ نسخة احتياطية من المجلد data.");
  });
  fileMenu->addAction("🏢 الفروع", [this]() {
    QMessageBox::information(this, "الفروع",
                             "إدارة فروع الشركة.\nالفرع الحالي: المقر الرئيسي");
  });
  fileMenu->addAction("🔧 الأدوات", [this]() {
    QMessageBox::information(this, "الأدوات",
                             "🔧 أدوات النظام:\n\n"
                             "• حاسبة مدمجة\n"
                             "• تصدير البيانات\n"
                             "• استيراد البيانات");
  });
  fileMenu->addSeparator();

  // Hardware devices sub-menu
  auto *devicesMenu = fileMenu->addMenu("🖨️ الأجهزة");
  devicesMenu->addAction("👆 جهاز البصمة", [this]() {
    QMessageBox::information(this, "جهاز البصمة",
                             "👆 إعدادات جهاز البصمة\n"
                             "══════════════════════\n\n"
                             "الحالة: ❌ غير متصل\n\n"
                             "الأجهزة المدعومة:\n"
                             "• ZKTeco K40/K50\n"
                             "• ZKTeco iClock\n"
                             "• FingerTec R2/R3\n\n"
                             "💡 تأكد من توصيل الجهاز عبر USB أو الشبكة\n"
                             "ثم اضغط 'بحث عن الأجهزة'");
  });
  devicesMenu->addAction("🖨️ طابعة POS", [this]() {
    QMessageBox::information(this, "طابعة POS",
                             "🖨️ إعدادات طابعة الإيصالات\n"
                             "══════════════════════\n\n"
                             "الحالة: 🔍 جاري البحث...\n\n"
                             "الطابعات المكتشفة:\n"
                             "• طابعة Windows الافتراضية\n\n"
                             "الطابعات المدعومة:\n"
                             "• Epson TM-T88/T20\n"
                             "• Star TSP100/TSP650\n"
                             "• Xprinter XP-58/XP-80\n"
                             "• Bixolon SRP-350\n\n"
                             "عرض الورقة: 80mm / 58mm\n"
                             "💡 تأكد من تعريف الطابعة على الويندوز");
  });
  devicesMenu->addAction("🗃️ درج الكاش", [this]() {
    QMessageBox::information(this, "درج الكاش",
                             "🗃️ إعدادات درج الكاش\n"
                             "══════════════════════\n\n"
                             "الحالة: ❌ غير متصل\n\n"
                             "طريقة الفتح:\n"
                             "• تلقائي عند حفظ الفاتورة\n"
                             "• يدوي بالضغط على زر\n\n"
                             "الاتصال: عبر طابعة POS (RJ11)\n"
                             "💡 معظم أدراج الكاش تتصل عبر الطابعة");
  });
  devicesMenu->addAction("📷 قارئ الباركود", [this]() {
    QMessageBox::information(this, "قارئ الباركود",
                             "📷 إعدادات قارئ الباركود\n"
                             "══════════════════════\n\n"
                             "الحالة: ✅ جاهز (وضع لوحة المفاتيح)\n\n"
                             "💡 قارئ الباركود يعمل تلقائياً\n"
                             "كوضع إدخال لوحة مفاتيح USB HID\n"
                             "لا يحتاج إعداد إضافي");
  });

  fileMenu->addSeparator();
  fileMenu->addAction("🔄 تحديث البيانات", [this]() {
    if (m_dashboard)
      m_dashboard->refresh();
    QMessageBox::information(this, "تم", "تم تحديث البيانات بنجاح!");
  });
  fileMenu->addSeparator();
  fileMenu->addAction("🚪 خروج", [this]() {
    if (QMessageBox::question(this, "خروج", "هل تريد الخروج من البرنامج؟") ==
        QMessageBox::Yes)
      qApp->exit(0);
  });

  // ===== 2. العملاء والموردين =====
  auto *custMenu = bar->addMenu("👥 العملاء و الموردين");
  custMenu->addAction("👥 إدارة العملاء والموردين",
                      [this]() { onMenuClicked(5); });
  custMenu->addSeparator();
  custMenu->addAction("📊 كشف حساب عميل", [this]() { onMenuClicked(7); });
  custMenu->addAction("📊 كشف حساب مورد", [this]() { onMenuClicked(7); });
  custMenu->addSeparator();
  custMenu->addAction("📋 تقرير أرصدة العملاء", [this]() { onMenuClicked(7); });
  custMenu->addAction("📋 تقرير أرصدة الموردين",
                      [this]() { onMenuClicked(7); });

  // ===== 3. المخازن =====
  auto *stockMenu = bar->addMenu("🏭 المخازن");
  stockMenu->addAction("📦 الأصناف (المنتجات)", [this]() { onMenuClicked(3); });
  stockMenu->addAction("🏭 المخزون الحالي", [this]() { onMenuClicked(4); });
  stockMenu->addSeparator();
  stockMenu->addAction("🔀 تحويل أصناف بين المخازن",
                       [this]() { onMenuClicked(13); });
  stockMenu->addAction("📋 جرد المخزون", [this]() { onMenuClicked(4); });
  stockMenu->addAction("📅 بداية مدة (رصيد افتتاحي)",
                       [this]() { onMenuClicked(4); });
  stockMenu->addSeparator();
  stockMenu->addAction("📊 تقرير حركة الأصناف", [this]() { onMenuClicked(7); });
  stockMenu->addAction("⚠️ تقرير الأصناف تحت الحد الأدنى",
                       [this]() { onMenuClicked(7); });

  // ===== 4. المشتريات =====
  auto *purchMenu = bar->addMenu("🛒 المشتريات");
  purchMenu->addAction("📝 فاتورة مشتريات", [this]() { onMenuClicked(2); });
  purchMenu->addAction("🔄 مرتجع مشتريات", [this]() { onMenuClicked(9); });
  purchMenu->addSeparator();
  purchMenu->addAction("📋 عروض أسعار الموردين",
                       [this]() { onMenuClicked(2); });
  purchMenu->addAction("📊 تقرير المشتريات", [this]() { onMenuClicked(7); });

  // ===== 5. المبيعات =====
  auto *salesMenu = bar->addMenu("💰 المبيعات");
  salesMenu->addAction("📝 فاتورة مبيعات", [this]() { onMenuClicked(1); });
  salesMenu->addAction("🔄 مرتجع مبيعات", [this]() { onMenuClicked(8); });
  salesMenu->addAction("📋 فاتورة حجز", [this]() { onMenuClicked(14); });
  salesMenu->addSeparator();
  salesMenu->addAction("💲 عروض الأسعار", [this]() { onMenuClicked(1); });
  salesMenu->addAction("📊 تقرير المبيعات", [this]() { onMenuClicked(7); });
  salesMenu->addAction("📊 تقرير الأرباح", [this]() { onMenuClicked(7); });

  // ===== 6. الحسابات =====
  auto *accMenu = bar->addMenu("💰 الحسابات");
  accMenu->addAction("💰 الخزينة", [this]() { onMenuClicked(10); });
  accMenu->addAction("💸 المصروفات", [this]() { onMenuClicked(6); });
  accMenu->addAction("💵 الرواتب", [this]() { onMenuClicked(15); });
  accMenu->addSeparator();
  accMenu->addAction("📊 كشف حساب", [this]() { onMenuClicked(7); });
  accMenu->addAction("📋 ميزان المراجعة", [this]() { onMenuClicked(7); });
  accMenu->addAction("📈 الأرباح والخسائر", [this]() { onMenuClicked(7); });

  // ===== 7. الموظفين =====
  auto *empMenu = bar->addMenu("👨‍💼 الموظفين");
  empMenu->addAction("👨‍💼 إدارة الموظفين",
                     [this]() { onMenuClicked(11); });
  empMenu->addAction("🕐 حضور وانصراف", [this]() { onMenuClicked(12); });
  empMenu->addAction("🏖️ الإجازات", [this]() { onMenuClicked(16); });
  empMenu->addSeparator();
  empMenu->addAction("💵 السلف والقروض", [this]() { onMenuClicked(15); });
  empMenu->addAction("📊 تقرير الرواتب", [this]() { onMenuClicked(7); });

  // ===== 8. الصيانة =====
  auto *maintMenu = bar->addMenu("🔧 الصيانة");
  maintMenu->addAction("🔧 طلبات الصيانة", [this]() { onMenuClicked(17); });
  maintMenu->addAction("📋 متابعة الصيانة", [this]() { onMenuClicked(17); });
  maintMenu->addSeparator();
  maintMenu->addAction("📊 تقرير الصيانة", [this]() { onMenuClicked(7); });

  // ===== 9. التقارير =====
  auto *repMenu = bar->addMenu("📊 التقارير");
  repMenu->addAction("📊 لوحة التحكم", [this]() { onMenuClicked(0); });
  repMenu->addSeparator();
  repMenu->addAction("💰 تقرير المبيعات", [this]() { onMenuClicked(7); });
  repMenu->addAction("🛒 تقرير المشتريات", [this]() { onMenuClicked(7); });
  repMenu->addAction("📈 تقرير الأرباح", [this]() { onMenuClicked(7); });
  repMenu->addSeparator();
  repMenu->addAction("📦 تقرير المخزون", [this]() { onMenuClicked(7); });
  repMenu->addAction("💸 تقرير المصروفات", [this]() { onMenuClicked(7); });
  repMenu->addAction("👥 تقرير العملاء", [this]() { onMenuClicked(7); });
  repMenu->addSeparator();
  repMenu->addAction("📋 جميع التقارير", [this]() { onMenuClicked(7); });

  // ===== 10. اتصل بنا =====
  bar->addAction("📞 اتصل بنا")->setData(-2);
  connect(bar->actions().last(), &QAction::triggered, [this]() {
    QMessageBox::information(this, "اتصل بنا",
                             "📞 للدعم الفني والاستفسارات:\n\n"
                             "📧 البريد الإلكتروني: support@pos-system.com\n"
                             "📱 الجوال: +966 50 000 0000\n"
                             "🌐 الموقع: www.pos-system.com\n\n"
                             "نسعد بخدمتكم! 🙏");
  });
}

void MainWindow::onMenuClicked(int index) {
  if (index == -1) {
    if (QMessageBox::question(this, "تسجيل الخروج", "هل تريد تسجيل الخروج؟") ==
        QMessageBox::Yes)
      qApp->exit(1000);
    return;
  }
  if (index >= 0 && index < m_stack->count()) {
    m_stack->setCurrentIndex(index);
    if (index < m_titles.size())
      m_headerTitle->setText(m_titles[index]);
    if (index == 0)
      m_dashboard->refresh();
  }
}
