#include "sql_snippet_model.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

namespace remote_sqlite_qt {

SqlSnippetModel::SqlSnippetModel(QObject* parent)
    : QAbstractListModel(parent) {
    // 预置一些常用片段
    addSnippet("SELECT *", "SELECT * FROM table_name LIMIT 10;",
               "Basic select with limit", "Query");
    addSnippet("CREATE TABLE", "CREATE TABLE table_name (\n"
                "    id INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                "    name TEXT NOT NULL,\n"
                "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP\n"
                ");", "Create table template", "DDL");
    addSnippet("INSERT", "INSERT INTO table_name (col1, col2) VALUES ('val1', 'val2');",
               "Insert single row", "DML");
    addSnippet("COUNT", "SELECT COUNT(*) FROM table_name WHERE condition;",
               "Count rows with condition", "Query");
    addSnippet("PRAGMA table_info", "PRAGMA table_info('table_name');",
               "Show table column info", "Meta");
    addSnippet("PRAGMA index_list", "PRAGMA index_list('table_name');",
               "Show table indexes", "Meta");
}

int SqlSnippetModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(snippets_.size());
}

QVariant SqlSnippetModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(snippets_.size()))
        return {};

    const auto& s = snippets_[static_cast<size_t>(index.row())];
    switch (role) {
    case NameRole:
        return QString::fromStdString(s.name);
    case SqlRole:
        return QString::fromStdString(s.sql);
    case DescriptionRole:
        return QString::fromStdString(s.description);
    case CategoryRole:
        return QString::fromStdString(s.category);
    case Qt::DisplayRole:
        return QString::fromStdString(s.name);
    }
    return {};
}

QHash<int, QByteArray> SqlSnippetModel::roleNames() const {
    return {
        {NameRole, "snippetName"},
        {SqlRole, "sql"},
        {DescriptionRole, "description"},
        {CategoryRole, "category"},
    };
}

void SqlSnippetModel::addSnippet(const QString& name, const QString& sql,
                                  const QString& description,
                                  const QString& category) {
    if (static_cast<int>(snippets_.size()) >= kMaxSnippets) return;

    beginInsertRows(QModelIndex(), 0, 0);
    snippets_.insert(snippets_.begin(), {
        name.toStdString(),
        sql.toStdString(),
        description.toStdString(),
        category.toStdString()
    });
    endInsertRows();
    emit snippetsChanged();
}

void SqlSnippetModel::removeSnippet(int index) {
    if (index < 0 || index >= static_cast<int>(snippets_.size())) return;
    beginRemoveRows(QModelIndex(), index, index);
    snippets_.erase(snippets_.begin() + index);
    endRemoveRows();
    emit snippetsChanged();
}

void SqlSnippetModel::updateSnippet(int index, const QString& name,
                                     const QString& sql,
                                     const QString& description,
                                     const QString& category) {
    if (index < 0 || index >= static_cast<int>(snippets_.size())) return;
    auto& s = snippets_[static_cast<size_t>(index)];
    s.name = name.toStdString();
    s.sql = sql.toStdString();
    s.description = description.toStdString();
    s.category = category.toStdString();
    emit dataChanged(createIndex(index, 0), createIndex(index, 0));
    emit snippetsChanged();
}

QString SqlSnippetModel::getSql(int index) const {
    if (index < 0 || index >= static_cast<int>(snippets_.size())) return {};
    return QString::fromStdString(snippets_[static_cast<size_t>(index)].sql);
}

bool SqlSnippetModel::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;

    auto doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) return false;

    beginResetModel();
    snippets_.clear();

    for (const auto& item : doc.array()) {
        auto obj = item.toObject();
        Snippet s;
        s.name = obj["name"].toString().toStdString();
        s.sql = obj["sql"].toString().toStdString();
        s.description = obj["description"].toString().toStdString();
        s.category = obj["category"].toString().toStdString();
        snippets_.push_back(std::move(s));
    }
    endResetModel();
    emit snippetsChanged();
    return true;
}

bool SqlSnippetModel::saveToFile(const QString& path) {
    QJsonArray arr;
    for (const auto& s : snippets_) {
        QJsonObject obj;
        obj["name"] = QString::fromStdString(s.name);
        obj["sql"] = QString::fromStdString(s.sql);
        obj["description"] = QString::fromStdString(s.description);
        obj["category"] = QString::fromStdString(s.category);
        arr.append(obj);
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(arr).toJson());
    return true;
}

QList<int> SqlSnippetModel::search(const QString& keyword) const {
    QList<int> results;
    auto kw = keyword.toLower();
    for (size_t i = 0; i < snippets_.size(); ++i) {
        const auto& s = snippets_[i];
        auto name = QString::fromStdString(s.name).toLower();
        auto desc = QString::fromStdString(s.description).toLower();
        auto cat = QString::fromStdString(s.category).toLower();
        if (name.contains(kw) || desc.contains(kw) || cat.contains(kw))
            results.append(static_cast<int>(i));
    }
    return results;
}

}  // namespace remote_sqlite_qt
