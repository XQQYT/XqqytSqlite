#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <string>

namespace remote_sqlite_qt {

// 列定义
struct ColumnDef {
    std::string name;
    std::string type{"TEXT"};    // TEXT, INTEGER, REAL, BLOB, NUMERIC
    bool primaryKey{false};
    bool autoIncrement{false};
    bool notNull{false};
    bool unique{false};
    std::string defaultValue;
    std::string checkExpr;       // CHECK(expr)
    int order{0};
};

// 索引定义
struct IndexDef {
    std::string name;
    std::vector<std::string> columns;
    bool unique{false};
    bool ifNotExists{true};
};

// 外键定义
struct ForeignKeyDef {
    std::string column;
    std::string refTable;
    std::string refColumn;
    std::string onDelete;       // NO ACTION, CASCADE, SET NULL, SET DEFAULT, RESTRICT
    std::string onUpdate;       // 同上
};

// 可视化建表模型
class TableDesignerModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(QString tableName READ tableName WRITE setTableName NOTIFY tableChanged)
    Q_PROPERTY(bool withoutRowid READ withoutRowid WRITE setWithoutRowid NOTIFY tableChanged)

    // 索引列表属性
    Q_PROPERTY(int indexCount READ indexCount NOTIFY tableChanged)
    Q_PROPERTY(int foreignKeyCount READ foreignKeyCount NOTIFY tableChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        TypeRole,
        PrimaryKeyRole,
        AutoIncrementRole,
        NotNullRole,
        UniqueRole,
        DefaultValueRole,
        CheckExprRole,
        OrderRole,
    };

    explicit TableDesignerModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // 表属性
    QString tableName() const;
    void setTableName(const QString& name);
    bool withoutRowid() const { return withoutRowid_; }
    void setWithoutRowid(bool v);

    int indexCount() const { return static_cast<int>(indexes_.size()); }
    int foreignKeyCount() const { return static_cast<int>(foreignKeys_.size()); }

    // 列操作
    Q_INVOKABLE void addColumn();
    Q_INVOKABLE void removeColumn(int row);
    Q_INVOKABLE void moveColumn(int fromRow, int toRow);

    // 索引操作
    Q_INVOKABLE void addIndex(const QString& name, const QStringList& columns,
                               bool unique);
    Q_INVOKABLE void removeIndex(int row);
    Q_INVOKABLE QVariantMap getIndex(int row) const;

    // 外键操作
    Q_INVOKABLE void addForeignKey(const QString& column, const QString& refTable,
                                    const QString& refColumn,
                                    const QString& onDelete, const QString& onUpdate);
    Q_INVOKABLE void removeForeignKey(int row);
    Q_INVOKABLE QVariantMap getForeignKey(int row) const;

    // 生成 DDL
    Q_INVOKABLE QString generateDDL() const;

    // 清空
    Q_INVOKABLE void clear();

    // 从现有表解析（编辑模式）
    Q_INVOKABLE void loadFromExisting(const QString& createTableSql);

    // 获取数据（供 Controller 使用）
    std::vector<ColumnDef> columns() const;
    std::vector<IndexDef> indexes() const { return indexes_; }
    std::vector<ForeignKeyDef> foreignKeys() const { return foreignKeys_; }

signals:
    void tableChanged();

private:
    std::vector<ColumnDef> columns_;
    std::vector<IndexDef> indexes_;
    std::vector<ForeignKeyDef> foreignKeys_;
    std::string tableName_{"new_table"};
    bool withoutRowid_{false};

    // DDL 生成辅助
    std::string generateColumnDDL(const ColumnDef& col) const;
    std::string generatePKConstraint() const;
};

}  // namespace remote_sqlite_qt
