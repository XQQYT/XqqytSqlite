#include "SqlHighlighter.h"

#include <QColor>
#include <QFont>

namespace remote_sqlite_qt {

SqlHighlighter::SqlHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    initFormats();
    initRules();
}

void SqlHighlighter::initFormats() {
    // 关键字——紫色加粗
    keywordFormat_.setForeground(QColor("#c792ea"));
    keywordFormat_.setFontWeight(QFont::Bold);

    // 数字——橙色
    numberFormat_.setForeground(QColor("#f78c6c"));

    // 字符串——绿色
    stringFormat_.setForeground(QColor("#c3e88d"));

    // 注释——灰色斜体
    commentFormat_.setForeground(QColor("#546e7a"));
    commentFormat_.setFontItalic(true);

    // 函数——蓝色
    functionFormat_.setForeground(QColor("#82aaff"));
    functionFormat_.setFontWeight(QFont::Bold);

    // 运算符——青色
    operatorFormat_.setForeground(QColor("#89ddff"));
}

void SqlHighlighter::initRules() {
    // 单行注释: -- ...
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression("--[^\n]*");
        rule.format = commentFormat_;
        rules_.append(rule);
    }

    // 块注释: /* ... */
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression("/\\*[^*]*\\*+(?:[^/*][^*]*\\*+)*/");
        rule.format = commentFormat_;
        rules_.append(rule);
    }

    // 字符串: '...'
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression("'[^']*'");
        rule.format = stringFormat_;
        rules_.append(rule);
    }

    // 字符串: "..."
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression("\"[^\"]*\"");
        rule.format = stringFormat_;
        rules_.append(rule);
    }

    // 数字: 整数和浮点
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
        rule.format = numberFormat_;
        rules_.append(rule);
    }

    // 函数名: 标识符后跟 (
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression("\\b[A-Za-z_]\\w*(?=\\s*\\()");
        rule.format = functionFormat_;
        rules_.append(rule);
    }

    // 运算符
    {
        HighlightRule rule;
        rule.pattern = QRegularExpression("[=<>!]+|\\|\\||&&|\\+|-|\\*|/|%");
        rule.format = operatorFormat_;
        rules_.append(rule);
    }

    // SQL 关键字（大小写不敏感）
    const QStringList keywords = {
        "SELECT", "FROM", "WHERE", "INSERT", "INTO", "VALUES", "UPDATE", "SET",
        "DELETE", "CREATE", "TABLE", "ALTER", "DROP", "INDEX", "VIEW", "TRIGGER",
        "PRIMARY", "KEY", "FOREIGN", "REFERENCES", "NOT", "NULL", "DEFAULT",
        "CHECK", "UNIQUE", "CONSTRAINT", "JOIN", "INNER", "LEFT", "RIGHT", "OUTER",
        "CROSS", "ON", "AS", "AND", "OR", "IN", "BETWEEN", "LIKE", "IS",
        "ORDER", "BY", "ASC", "DESC", "GROUP", "HAVING", "LIMIT", "OFFSET",
        "UNION", "ALL", "DISTINCT", "CASE", "WHEN", "THEN", "ELSE", "END",
        "BEGIN", "COMMIT", "ROLLBACK", "TRANSACTION", "PRAGMA", "EXPLAIN",
        "EXISTS", "IF", "REPLACE", "INTO", "VACUUM", "ATTACH", "DETACH",
        "REINDEX", "ANALYZE", "WITH", "RECURSIVE", "TEMP", "TEMPORARY",
        "INTEGER", "TEXT", "REAL", "BLOB", "NUMERIC", "BOOLEAN", "DATE",
        "DATETIME", "TRUE", "FALSE", "AUTOINCREMENT", "ROWID",
    };

    for (const auto& kw : keywords) {
        HighlightRule rule;
        rule.pattern = QRegularExpression(
            "\\b" + kw + "\\b",
            QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat_;
        rules_.append(rule);
    }
}

void SqlHighlighter::highlightBlock(const QString& text) {
    for (const auto& rule : rules_) {
        auto it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

}  // namespace remote_sqlite_qt
