#include <QApplication>
#include "MainWindow.h"
#include "../ui/AppTheme.h"
#include "../database/DatabaseManager.h"
#include "../core/Constants.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName(RetailNext::APP_NAME);
    app.setApplicationDisplayName(RetailNext::APP_TITLE);
    app.setApplicationVersion(RetailNext::APP_VERSION);
    app.setOrganizationName(RetailNext::APP_ORG);
    app.setOrganizationDomain(RetailNext::APP_DOMAIN);

    // Apply RetailNext modern enterprise design system
    RetailNext::applyModernTheme(app);

    // Initialize local SQLite database persistence
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.initialize()) {
        qWarning("Database initialization reported warnings or errors: %s",
                 qPrintable(db.lastError()));
    }

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
