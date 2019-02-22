
#include "CutterApplication.h"
#include "MainWindow.h"

/*!
 * \brief Migrate Settings used before Cutter 1.8
 */
static void migrateSettings(QSettings &newSettings)
{
    QSettings oldSettings(QSettings::NativeFormat, QSettings::Scope::UserScope, "Cutter", "Cutter");
    for (const QString &key : oldSettings.allKeys()) {
        newSettings.setValue(key, oldSettings.value(key));
    }
}

int main(int argc, char *argv[])
{
    qRegisterMetaType<QList<StringDescription>>();
    qRegisterMetaType<QList<FunctionDescription>>();

    QCoreApplication::setApplicationName("Cutter");
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QSettings settings;
    if (!settings.value("settings_migrated", false).toBool()) {
        qInfo() << "Settings have not been migrated before, trying to migrate from pre-1.8 if possible.";
        migrateSettings(settings);
        settings.setValue("settings_migrated", true);
    }

    CutterApplication a(argc, argv);

    int ret = a.exec();

    return ret;
}
