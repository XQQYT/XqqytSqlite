#include "database_tree_model.h"

namespace remote_sqlite_qt {

DatabaseTreeModel::DatabaseTreeModel(QObject* parent)
    : QAbstractItemModel(parent) {
    root_ = std::make_unique<DbTreeNode>();
    root_->name = "root";
    root_->type = "root";
}

QModelIndex DatabaseTreeModel::index(int row, int col, const QModelIndex& parent) const {
    if (!hasIndex(row, col, parent)) return {};
    auto* p = parent.isValid() ? static_cast<DbTreeNode*>(parent.internalPointer()) : root_.get();
    return createIndex(row, col, p->children[row].get());
}

QModelIndex DatabaseTreeModel::parent(const QModelIndex& idx) const {
    if (!idx.isValid()) return {};
    auto* node = static_cast<DbTreeNode*>(idx.internalPointer());
    if (!node->parent || node->parent == root_.get()) return {};
    auto* gp = node->parent->parent;
    if (!gp) return {};
    for (size_t i = 0; i < gp->children.size(); ++i)
        if (gp->children[i].get() == node->parent)
            return createIndex(static_cast<int>(i), 0, node->parent);
    return {};
}

int DatabaseTreeModel::rowCount(const QModelIndex& parent) const {
    auto* node = parent.isValid() ? static_cast<DbTreeNode*>(parent.internalPointer()) : root_.get();
    return static_cast<int>(node->children.size());
}

int DatabaseTreeModel::columnCount(const QModelIndex&) const { return 1; }

QVariant DatabaseTreeModel::data(const QModelIndex& idx, int role) const {
    if (!idx.isValid()) return {};
    auto* n = static_cast<DbTreeNode*>(idx.internalPointer());
    switch (role) {
    case NameRole: case Qt::DisplayRole: return QString::fromStdString(n->name);
    case TypeRole:     return QString::fromStdString(n->type);
    case ConnectionIdRole: return QString::fromStdString(n->connectionId);
    case DbPathRole:   return QString::fromStdString(n->dbPath);
    case SqlRole:      return QString::fromStdString(n->sql);
    case IconRole:     return QString::fromStdString(iconForType(n->type));
    case HostRole:     return QString::fromStdString(n->host);
    case UsernameRole: return QString::fromStdString(n->username);
    }
    return {};
}

QHash<int, QByteArray> DatabaseTreeModel::roleNames() const {
    return {
        {NameRole, "nodeName"}, {TypeRole, "nodeType"},
        {ConnectionIdRole, "connectionId"}, {DbPathRole, "dbPath"},
        {SqlRole, "sql"}, {IconRole, "icon"},
        {HostRole, "host"}, {UsernameRole, "username"},
    };
}

// ===== 设备管理 =====

void DatabaseTreeModel::addConnection(const QString& connId, const QString& host, const QString& user) {
    if (findDevice(connId.toStdString())) return;  // 已存在
    int pos = static_cast<int>(root_->children.size());
    beginInsertRows(QModelIndex(), pos, pos);
    auto node = std::make_unique<DbTreeNode>();
    node->name = host.toStdString() + " (" + user.toStdString() + ")";
    node->type = "device";
    node->connectionId = connId.toStdString();
    node->host = host.toStdString();
    node->username = user.toStdString();
    node->parent = root_.get();
    root_->children.push_back(std::move(node));
    endInsertRows();
}

void DatabaseTreeModel::removeConnection(const QString& connId) {
    for (size_t i = 0; i < root_->children.size(); ++i) {
        if (root_->children[i]->connectionId == connId.toStdString()) {
            beginRemoveRows(QModelIndex(), static_cast<int>(i), static_cast<int>(i));
            root_->children.erase(root_->children.begin() + i);
            endRemoveRows();
            return;
        }
    }
}

// ===== 数据库管理 =====

void DatabaseTreeModel::addDatabase(const QString& connId, const QString& dbPath) {
    auto* dev = findDevice(connId.toStdString());
    if (!dev) return;
    if (findDatabase(dev, dbPath.toStdString())) return;

    int pos = static_cast<int>(dev->children.size());
    auto devIdx = createIndex(0, 0, dev);  // need proper index
    // Find dev's position in root
    for (size_t i = 0; i < root_->children.size(); ++i) {
        if (root_->children[i].get() == dev) {
            devIdx = createIndex(static_cast<int>(i), 0, dev);
            break;
        }
    }

    beginInsertRows(devIdx, pos, pos);
    auto node = std::make_unique<DbTreeNode>();
    auto slash = dbPath.toStdString().find_last_of("/\\");
    node->name = (slash != std::string::npos) ? dbPath.toStdString().substr(slash + 1) : dbPath.toStdString();
    node->type = "database";
    node->connectionId = connId.toStdString();
    node->dbPath = dbPath.toStdString();
    node->parent = dev;
    dev->children.push_back(std::move(node));
    endInsertRows();
}

void DatabaseTreeModel::removeDatabase(const QString& connId, const QString& dbPath) {
    auto* dev = findDevice(connId.toStdString());
    if (!dev) return;
    for (size_t i = 0; i < dev->children.size(); ++i) {
        if (dev->children[i]->dbPath == dbPath.toStdString()) {
            QModelIndex devIdx;
            for (size_t j = 0; j < root_->children.size(); ++j)
                if (root_->children[j].get() == dev) { devIdx = createIndex(static_cast<int>(j), 0, dev); break; }
            beginRemoveRows(devIdx, static_cast<int>(i), static_cast<int>(i));
            dev->children.erase(dev->children.begin() + i);
            endRemoveRows();
            return;
        }
    }
}

// ===== 表加载 =====

void DatabaseTreeModel::loadTables(const QString& connId, const QString& dbPath,
                                    const std::vector<TableInfo>& tables) {
    auto* dev = findDevice(connId.toStdString());
    if (!dev) return;
    auto* dbNode = findDatabase(dev, dbPath.toStdString());
    if (!dbNode) return;

    // 清空旧的表数据
    QModelIndex dbIdx;
    for (size_t i = 0; i < dev->children.size(); ++i)
        if (dev->children[i].get() == dbNode) {
            for (size_t j = 0; j < root_->children.size(); ++j)
                if (root_->children[j].get() == dev)
                    dbIdx = createIndex(static_cast<int>(j), 0, dbNode);
            break;
        }

    if (dbNode->children.size() > 0) {
        beginRemoveRows(dbIdx, 0, static_cast<int>(dbNode->children.size()) - 1);
        dbNode->children.clear();
        endRemoveRows();
    }

    if (tables.empty()) return;

    // 分类
    std::vector<TableInfo> tbls, views, idxs, trigs;
    for (auto& t : tables) {
        if (t.type == "table") tbls.push_back(t);
        else if (t.type == "view") views.push_back(t);
        else if (t.type == "index") idxs.push_back(t);
        else if (t.type == "trigger") trigs.push_back(t);
    }

    struct { const char* name; std::vector<TableInfo> items; } folders[] = {
        {"Tables", tbls}, {"Views", views}, {"Indexes", idxs}, {"Triggers", trigs}
    };

    int total = 0;
    for (auto& f : folders) if (!f.items.empty()) ++total;

    beginInsertRows(dbIdx, 0, total - 1);
    for (auto& f : folders) {
        if (f.items.empty()) continue;
        auto folder = std::make_unique<DbTreeNode>();
        folder->name = f.name;
        folder->type = "folder";
        folder->parent = dbNode;
        for (auto& item : f.items) {
            auto it = std::make_unique<DbTreeNode>();
            it->name = item.name;
            it->type = item.type;
            it->dbPath = dbPath.toStdString();
            it->sql = item.sql;
            it->parent = folder.get();
            folder->children.push_back(std::move(it));
        }
        dbNode->children.push_back(std::move(folder));
    }
    endInsertRows();
}

void DatabaseTreeModel::clear() {
    beginResetModel();
    root_.reset(new DbTreeNode{});
    root_->name = "root";
    root_->type = "root";
    endResetModel();
}

// ===== 私有方法 =====

DbTreeNode* DatabaseTreeModel::nodeFromIndex(const QModelIndex& idx) const {
    if (!idx.isValid()) return root_.get();
    return static_cast<DbTreeNode*>(idx.internalPointer());
}

DbTreeNode* DatabaseTreeModel::findDevice(const std::string& connId) const {
    for (auto& c : root_->children)
        if (c->connectionId == connId) return c.get();
    return nullptr;
}

DbTreeNode* DatabaseTreeModel::findDatabase(DbTreeNode* device, const std::string& dbPath) const {
    for (auto& c : device->children)
        if (c->dbPath == dbPath) return c.get();
    return nullptr;
}

std::string DatabaseTreeModel::iconForType(const std::string& type) {
    if (type == "device") return "📡";
    if (type == "database") return "🗄️";
    if (type == "folder") return "📁";
    if (type == "table") return "📋";
    if (type == "view") return "👁️";
    if (type == "index") return "🔍";
    if (type == "trigger") return "⚡";
    return "📄";
}

}  // namespace remote_sqlite_qt
