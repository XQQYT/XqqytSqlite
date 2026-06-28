#include "table_designer_model.h"

#include <algorithm>
#include <sstream>
#include <regex>

namespace remote_sqlite_qt {

TableDesignerModel::TableDesignerModel(QObject* parent)
    : QAbstractListModel(parent) {
    // 默认添加第一列
    addColumn();
}

int TableDesignerModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(columns_.size());
}

QVariant TableDesignerModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(columns_.size()))
        return {};
    const auto& c = columns_[static_cast<size_t>(index.row())];
    switch (role) {
    case NameRole:         return QString::fromStdString(c.name);
    case TypeRole:          return QString::fromStdString(c.type);
    case PrimaryKeyRole:    return c.primaryKey;
    case AutoIncrementRole: return c.autoIncrement;
    case NotNullRole:       return c.notNull;
    case UniqueRole:        return c.unique;
    case DefaultValueRole:  return QString::fromStdString(c.defaultValue);
    case CheckExprRole:     return QString::fromStdString(c.checkExpr);
    case OrderRole:         return c.order;
    case Qt::DisplayRole:   return QString::fromStdString(c.name.empty() ? "(unnamed)" : c.name);
    }
    return {};
}

bool TableDesignerModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= static_cast<int>(columns_.size())) return false;
    auto& c = columns_[static_cast<size_t>(index.row())];
    switch (role) {
    case NameRole:          c.name = value.toString().toStdString(); break;
    case TypeRole:           c.type = value.toString().toStdString(); break;
    case PrimaryKeyRole:     c.primaryKey = value.toBool(); break;
    case AutoIncrementRole:  c.autoIncrement = value.toBool(); break;
    case NotNullRole:        c.notNull = value.toBool(); break;
    case UniqueRole:         c.unique = value.toBool(); break;
    case DefaultValueRole:   c.defaultValue = value.toString().toStdString(); break;
    case CheckExprRole:      c.checkExpr = value.toString().toStdString(); break;
    default: return false;
    }
    emit dataChanged(index, index, {role});
    emit tableChanged();
    return true;
}

QHash<int, QByteArray> TableDesignerModel::roleNames() const {
    return {
        {NameRole, "colName"}, {TypeRole, "colType"},
        {PrimaryKeyRole, "primaryKey"}, {AutoIncrementRole, "autoIncrement"},
        {NotNullRole, "notNull"}, {UniqueRole, "unique"},
        {DefaultValueRole, "defaultValue"}, {CheckExprRole, "checkExpr"},
        {OrderRole, "colOrder"},
    };
}

Qt::ItemFlags TableDesignerModel::flags(const QModelIndex& index) const {
    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}

QString TableDesignerModel::tableName() const {
    return QString::fromStdString(tableName_);
}

void TableDesignerModel::setTableName(const QString& name) {
    tableName_ = name.toStdString();
    emit tableChanged();
}

void TableDesignerModel::setWithoutRowid(bool v) {
    withoutRowid_ = v;
    emit tableChanged();
}

void TableDesignerModel::addColumn() {
    int pos = static_cast<int>(columns_.size());
    beginInsertRows(QModelIndex(), pos, pos);
    ColumnDef col;
    col.name = "column" + std::to_string(pos + 1);
    col.order = pos;
    columns_.push_back(std::move(col));
    endInsertRows();
    emit tableChanged();
}

void TableDesignerModel::removeColumn(int row) {
    if (row < 0 || row >= static_cast<int>(columns_.size())) return;
    beginRemoveRows(QModelIndex(), row, row);
    columns_.erase(columns_.begin() + row);
    // 重新编号
    for (size_t i = 0; i < columns_.size(); ++i) columns_[i].order = static_cast<int>(i);
    endRemoveRows();
    emit tableChanged();
}

void TableDesignerModel::moveColumn(int fromRow, int toRow) {
    if (fromRow < 0 || fromRow >= static_cast<int>(columns_.size()) ||
        toRow < 0 || toRow >= static_cast<int>(columns_.size())) return;

    beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(),
                  toRow > fromRow ? toRow + 1 : toRow);
    auto col = std::move(columns_[static_cast<size_t>(fromRow)]);
    columns_.erase(columns_.begin() + fromRow);
    columns_.insert(columns_.begin() + toRow, std::move(col));
    for (size_t i = 0; i < columns_.size(); ++i) columns_[i].order = static_cast<int>(i);
    endMoveRows();
    emit tableChanged();
}

// ===== 索引 =====

void TableDesignerModel::addIndex(const QString& name, const QStringList& cols,
                                   bool unique) {
    IndexDef idx;
    idx.name = name.toStdString();
    for (const auto& c : cols) idx.columns.push_back(c.toStdString());
    idx.unique = unique;
    indexes_.push_back(std::move(idx));
    emit tableChanged();
}

void TableDesignerModel::removeIndex(int row) {
    if (row < 0 || row >= static_cast<int>(indexes_.size())) return;
    indexes_.erase(indexes_.begin() + row);
    emit tableChanged();
}

QVariantMap TableDesignerModel::getIndex(int row) const {
    QVariantMap m;
    if (row < 0 || row >= static_cast<int>(indexes_.size())) return m;
    const auto& idx = indexes_[static_cast<size_t>(row)];
    m["name"] = QString::fromStdString(idx.name);
    QStringList cols;
    for (const auto& c : idx.columns) cols << QString::fromStdString(c);
    m["columns"] = cols;
    m["unique"] = idx.unique;
    return m;
}

// ===== 外键 =====

void TableDesignerModel::addForeignKey(const QString& column, const QString& refTable,
                                        const QString& refColumn,
                                        const QString& onDelete, const QString& onUpdate) {
    ForeignKeyDef fk;
    fk.column = column.toStdString();
    fk.refTable = refTable.toStdString();
    fk.refColumn = refColumn.toStdString();
    fk.onDelete = onDelete.toStdString();
    fk.onUpdate = onUpdate.toStdString();
    foreignKeys_.push_back(std::move(fk));
    emit tableChanged();
}

void TableDesignerModel::removeForeignKey(int row) {
    if (row < 0 || row >= static_cast<int>(foreignKeys_.size())) return;
    foreignKeys_.erase(foreignKeys_.begin() + row);
    emit tableChanged();
}

QVariantMap TableDesignerModel::getForeignKey(int row) const {
    QVariantMap m;
    if (row < 0 || row >= static_cast<int>(foreignKeys_.size())) return m;
    const auto& fk = foreignKeys_[static_cast<size_t>(row)];
    m["column"] = QString::fromStdString(fk.column);
    m["refTable"] = QString::fromStdString(fk.refTable);
    m["refColumn"] = QString::fromStdString(fk.refColumn);
    m["onDelete"] = QString::fromStdString(fk.onDelete);
    m["onUpdate"] = QString::fromStdString(fk.onUpdate);
    return m;
}

// ===== DDL 生成 =====

std::string TableDesignerModel::generateColumnDDL(const ColumnDef& col) const {
    if (col.name.empty()) return "";
    std::ostringstream oss;
    oss << "\"" << col.name << "\" " << col.type;
    if (col.primaryKey) {
        oss << " PRIMARY KEY";
        if (col.autoIncrement && (col.type == "INTEGER")) oss << " AUTOINCREMENT";
    }
    if (col.notNull) oss << " NOT NULL";
    if (col.unique) oss << " UNIQUE";
    if (!col.defaultValue.empty()) oss << " DEFAULT " << col.defaultValue;
    if (!col.checkExpr.empty()) oss << " CHECK(" << col.checkExpr << ")";
    return oss.str();
}

std::string TableDesignerModel::generatePKConstraint() const {
    // 收集所有主键列（复合主键场景）
    std::vector<std::string> pkCols;
    for (const auto& c : columns_) {
        if (c.primaryKey && !c.autoIncrement) pkCols.push_back(c.name);
    }
    // 单列主键已在列定义中声明，仅复合主键需要约束
    if (pkCols.size() <= 1) return "";

    std::ostringstream oss;
    oss << "PRIMARY KEY (";
    for (size_t i = 0; i < pkCols.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << "\"" << pkCols[i] << "\"";
    }
    oss << ")";
    return oss.str();
}

QString TableDesignerModel::generateDDL() const {
    if (columns_.empty()) return {};

    std::ostringstream oss;
    oss << "CREATE TABLE IF NOT EXISTS \"" << tableName_ << "\" (\n";

    // 列定义
    for (size_t i = 0; i < columns_.size(); ++i) {
        auto colDDL = generateColumnDDL(columns_[i]);
        if (colDDL.empty()) continue;
        oss << "    " << colDDL;
        if (i < columns_.size() - 1 || !foreignKeys_.empty()) oss << ",";
        oss << "\n";
    }

    // 外键约束
    for (size_t i = 0; i < foreignKeys_.size(); ++i) {
        const auto& fk = foreignKeys_[i];
        oss << "    FOREIGN KEY (\"" << fk.column << "\")";
        oss << " REFERENCES \"" << fk.refTable << "\"(\"" << fk.refColumn << "\")";
        if (!fk.onDelete.empty() && fk.onDelete != "NO ACTION")
            oss << " ON DELETE " << fk.onDelete;
        if (!fk.onUpdate.empty() && fk.onUpdate != "NO ACTION")
            oss << " ON UPDATE " << fk.onUpdate;
        if (i < foreignKeys_.size() - 1) oss << ",";
        oss << "\n";
    }

    oss << ")";
    if (withoutRowid_) oss << " WITHOUT ROWID";
    oss << ";\n";

    // 索引 DDL
    for (const auto& idx : indexes_) {
        oss << "\nCREATE ";
        if (idx.unique) oss << "UNIQUE ";
        oss << "INDEX ";
        if (idx.ifNotExists) oss << "IF NOT EXISTS ";
        oss << "\"" << idx.name << "\" ON \"" << tableName_ << "\" (";
        for (size_t i = 0; i < idx.columns.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "\"" << idx.columns[i] << "\"";
        }
        oss << ");\n";
    }

    return QString::fromStdString(oss.str());
}

void TableDesignerModel::clear() {
    beginResetModel();
    columns_.clear();
    indexes_.clear();
    foreignKeys_.clear();
    tableName_ = "new_table";
    withoutRowid_ = false;
    addColumn();
    endResetModel();
    emit tableChanged();
}

void TableDesignerModel::loadFromExisting(const QString& createTableSql) {
    // 解析 CREATE TABLE 语句（简化实现）
    auto sql = createTableSql.toStdString();
    beginResetModel();
    columns_.clear();

    // 提取表名
    std::regex tableRe(R"(CREATE\s+TABLE\s+(?:IF\s+NOT\s+EXISTS\s+)?\"?(\w+)\"?)",
                       std::regex::icase);
    std::smatch m;
    if (std::regex_search(sql, m, tableRe)) {
        tableName_ = m[1].str();
    }

    // 提取列定义（简化：按逗号分割括号内容）
    auto parenStart = sql.find('(');
    auto parenEnd = sql.find_last_of(')');
    if (parenStart != std::string::npos && parenEnd != std::string::npos) {
        // 简化：仅提取前几列
        addColumn();
    }

    endResetModel();
    emit tableChanged();
}

std::vector<ColumnDef> TableDesignerModel::columns() const {
    return columns_;
}

}  // namespace remote_sqlite_qt
