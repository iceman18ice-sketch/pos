cmake_minimum_required(VERSION 3.16)
project(POSSystem VERSION 3.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# UTF-8 encoding for Arabic text in source files
add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

# Fix GCC 15 compatibility with Qt 6
add_compile_options(-Wno-template-body)
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Charts Svg PrintSupport Network)

set(SOURCES
    src/main.cpp
    src/database/DatabaseManager.cpp
    src/models/Product.cpp
    src/models/Invoice.cpp
    src/models/Customer.cpp
    src/models/User.cpp
    src/views/LoginWindow.cpp
    src/views/MainWindow.cpp
    src/views/DashboardWidget.cpp
    src/views/ProductsWidget.cpp
    src/views/SalesWidget.cpp
    src/views/PurchasesWidget.cpp
    src/views/StockWidget.cpp
    src/views/CustomersWidget.cpp
    src/views/ReportsWidget.cpp
    src/views/ExpensesWidget.cpp
    src/views/SalesReturnsWidget.cpp
    src/views/PurchaseReturnsWidget.cpp
    src/views/TreasuryWidget.cpp
    src/views/EmployeesWidget.cpp
    src/views/AttendanceWidget.cpp
    src/views/StockTransferWidget.cpp
    src/views/BookingWidget.cpp
    src/views/SalaryWidget.cpp
    src/views/VacationWidget.cpp
    src/views/MaintenanceWidget.cpp
    src/views/SettingsWidget.cpp
    src/widgets/SidebarWidget.cpp
    src/widgets/StatCard.cpp
    src/widgets/InvoiceTable.cpp
    src/widgets/SearchBar.cpp
    src/utils/ZatcaQR.cpp
    src/utils/qrcodegen.cpp
    src/devices/ZKDevice.cpp
    src/devices/CashDrawer.cpp
    src/views/DeviceSettingsWidget.cpp
)

set(HEADERS
    src/database/DatabaseManager.h
    src/models/Product.h
    src/models/Invoice.h
    src/models/Customer.h
    src/models/User.h
    src/views/LoginWindow.h
    src/views/MainWindow.h
    src/views/DashboardWidget.h
    src/views/ProductsWidget.h
    src/views/SalesWidget.h
    src/views/PurchasesWidget.h
    src/views/StockWidget.h
    src/views/CustomersWidget.h
    src/views/ReportsWidget.h
    src/views/ExpensesWidget.h
    src/views/SalesReturnsWidget.h
    src/views/PurchaseReturnsWidget.h
    src/views/TreasuryWidget.h
    src/views/EmployeesWidget.h
    src/views/AttendanceWidget.h
    src/views/StockTransferWidget.h
    src/views/BookingWidget.h
    src/views/SalaryWidget.h
    src/views/VacationWidget.h
    src/views/MaintenanceWidget.h
    src/views/SettingsWidget.h
    src/widgets/SidebarWidget.h
    src/widgets/StatCard.h
    src/widgets/InvoiceTable.h
    src/widgets/SearchBar.h
    src/utils/ZatcaQR.h
    src/utils/qrcodegen.hpp
    src/devices/ZKDevice.h
    src/devices/CashDrawer.h
    src/views/DeviceSettingsWidget.h
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::Svg
    Qt6::PrintSupport
    Qt6::Network
    winspool
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
