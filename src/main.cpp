#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QTranslator>
#include <QLocale>

#include "eventbus/event_bus_manager.h"
#include "model/app_bridge.h"
#include "model/theme_model.h"

using namespace remote_sqlite_qt;

// 简易语言切换器
class I18nSwitcher : public QObject {
    Q_OBJECT
public:
    I18nSwitcher(QQmlApplicationEngine* e) : engine(e) {
        translator = new QTranslator(this);
        qtTranslator = new QTranslator(this);
    }
    Q_INVOKABLE void switchLanguage(const QString& lang) {
        qApp->removeTranslator(translator);
        qApp->removeTranslator(qtTranslator);
        if (lang == "zh") {
            if (translator->load(":/i18n/strings_zh.qm"))
                qApp->installTranslator(translator);
        }
        engine->retranslate();
    }
private:
    QQmlApplicationEngine* engine;
    QTranslator* translator;
    QTranslator* qtTranslator;
};

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("XqqytSqlite");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("XqqytSqlite");

    auto& busMgr = EventBusManager::instance();
    busMgr.init();

    AppBridge appBridge;
    appBridge.initialize();

    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:/qml");
    engine.rootContext()->setContextProperty("AppBridge", &appBridge);
    engine.rootContext()->setContextProperty("ThemeModel", &ThemeModel::instance());

    I18nSwitcher i18n(&engine);
    engine.rootContext()->setContextProperty("I18n", &i18n);

    Q_INIT_RESOURCE(qml);
    engine.load(QStringLiteral("qrc:/qml/main.qml"));

    QTimer::singleShot(800, [&]() {
        if (engine.rootObjects().isEmpty())
            engine.load(QStringLiteral("qrc:/qml/minimal.qml"));
    });

    int result = app.exec();
    busMgr.shutdown();
    return result;
}

#include "main.moc"
