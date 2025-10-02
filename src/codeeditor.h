#pragma once

#include <QPlainTextEdit>
#include <QWidget>
#include <QPainter>
#include <QTextBlock>
#include <QTextFormat>
#include <QColor>

// === Main CodeEditor class ===
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int  lineNumberAreaWidth();

public slots:
    void highlightErrorLine(int lineNumber);  // hata satırını boya
    void clearHighlight();                    // highlight temizle

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int);
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
};

// === Line number area helper ===
class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }
protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }
private:
    CodeEditor *codeEditor;
};
