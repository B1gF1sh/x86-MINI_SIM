#pragma once
#include <QDialog>

class QTabWidget;
class QTextBrowser;

class UserGuideDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserGuideDialog(QWidget *parent = nullptr);

private:
    QTabWidget *tabs;

    void addTab(const QString &title, const QString &htmlContent);
};
