#pragma once

#include <QObject>
#include <QTranslator>
#include <memory>

namespace remote_sqlite_qt {

// 国际化管理器
class I18nManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY languageChanged)

public:
    static I18nManager& instance();

    I18nManager(const I18nManager&) = delete;
    I18nManager& operator=(const I18nManager&) = delete;

    QString currentLanguage() const { return currentLang_; }

    Q_INVOKABLE void switchToLanguage(const QString& langCode);
    Q_INVOKABLE QStringList availableLanguages() const;

signals:
    void languageChanged();

private:
    I18nManager() = default;
    ~I18nManager() = default;

    std::unique_ptr<QTranslator> translator_;
    std::unique_ptr<QTranslator> qtTranslator_;
    QString currentLang_{"zh"};
};

}  // namespace remote_sqlite_qt
