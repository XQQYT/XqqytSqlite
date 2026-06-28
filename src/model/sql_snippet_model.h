#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <string>

namespace remote_sqlite_qt {

constexpr int kMaxSnippets = 200;

// SQL 片段 Model——保存/加载/插入常用 SQL 片段
class SqlSnippetModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        SqlRole,
        DescriptionRole,
        CategoryRole,
    };

    explicit SqlSnippetModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 片段管理
    Q_INVOKABLE void addSnippet(const QString& name, const QString& sql,
                                 const QString& description, const QString& category);
    Q_INVOKABLE void removeSnippet(int index);
    Q_INVOKABLE void updateSnippet(int index, const QString& name, const QString& sql,
                                    const QString& description, const QString& category);
    Q_INVOKABLE QString getSql(int index) const;

    // 持久化
    Q_INVOKABLE bool loadFromFile(const QString& path);
    Q_INVOKABLE bool saveToFile(const QString& path);

    // 搜索
    Q_INVOKABLE QList<int> search(const QString& keyword) const;

signals:
    void snippetsChanged();

private:
    struct Snippet {
        std::string name;
        std::string sql;
        std::string description;
        std::string category;
    };

    std::vector<Snippet> snippets_;
};

}  // namespace remote_sqlite_qt
