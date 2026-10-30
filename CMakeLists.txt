cmake_minimum_required(VERSION 3.16)

project(RetailNext
    VERSION 1.0.0
    DESCRIPTION "Configurable Enterprise Desktop POS and Inventory Management Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    PrintSupport
)

set(PROJECT_SOURCES
    # Application startup & shell
    src/app/main.cpp
    src/app/MainWindow.h
    src/app/MainWindow.cpp
    src/app/MainWindow.ui

    # Core
    src/core/Constants.h
    src/core/BusinessType.h

    # Database
    src/database/DatabaseManager.h
    src/database/DatabaseManager.cpp

    # Feature Modules
    src/modules/dashboard/DashboardPage.h
    src/modules/dashboard/DashboardPage.cpp
    src/modules/dashboard/DashboardPage.ui

    src/modules/pos/POSBillingPage.h
    src/modules/pos/POSBillingPage.cpp

    src/modules/products/ProductDialog.h
    src/modules/products/ProductDialog.cpp
    src/modules/products/ProductsPage.h
    src/modules/products/ProductsPage.cpp

    src/modules/inventory/InventoryPage.h
    src/modules/inventory/InventoryPage.cpp

    src/modules/purchases/PurchasesPage.h
    src/modules/purchases/PurchasesPage.cpp

    src/modules/sales/SalesPage.h
    src/modules/sales/SalesPage.cpp

    src/modules/customers/CustomersPage.h
    src/modules/customers/CustomersPage.cpp

    src/modules/finance/FinancePage.h
    src/modules/finance/FinancePage.cpp

    src/modules/reports/ReportsPage.h
    src/modules/reports/ReportsPage.cpp

    src/modules/settings/SettingsPage.h
    src/modules/settings/SettingsPage.cpp

    src/modules/access/AccessPage.h
    src/modules/access/AccessPage.cpp

    # Shared UI Dialogs & Utilities
    src/ui/ReceiptDialog.h
    src/ui/ReceiptDialog.cpp
    src/ui/BarcodeLabelDialog.h
    src/ui/BarcodeLabelDialog.cpp
    src/ui/QuickCustomerDialog.h
    src/ui/QuickCustomerDialog.cpp
    src/ui/GlobalSearchDialog.h
    src/ui/GlobalSearchDialog.cpp
    src/ui/PlaceholderPage.h
    src/ui/PlaceholderPage.cpp
)

add_executable(RetailNext
    WIN32
    ${PROJECT_SOURCES}
)

target_include_directories(RetailNext PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(RetailNext PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(RetailNext PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Automated Business Logic Test Suite
add_executable(BusinessLogicTest
    tests/BusinessLogicTest.cpp
    src/core/Constants.h
    src/core/BusinessType.h
    src/database/DatabaseManager.h
    src/database/DatabaseManager.cpp
)

target_include_directories(BusinessLogicTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(BusinessLogicTest PRIVATE
    Qt6::Core
    Qt6::Sql
)
