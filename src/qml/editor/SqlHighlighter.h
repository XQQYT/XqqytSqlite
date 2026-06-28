#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>

namespace remote_sqlite_qt {

// SQL 语法高亮器——基于 QSyntaxHighlighter，支持 SQL 关键字、字符串、数字、注释
class SqlHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit SqlHighlighter(QTextDocument* parent = nullptr);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> rules_;

    // 格式化定义
    QTextCharFormat keywordFormat_;
    QTextCharFormat numberFormat_;
    QTextCharFormat stringFormat_;
    QTextCharFormat commentFormat_;
    QTextCharFormat functionFormat_;
    QTextCharFormat operatorFormat_;

    void initFormats();
    void initRules();
};

}  // namespace remote_sqlite_qt
