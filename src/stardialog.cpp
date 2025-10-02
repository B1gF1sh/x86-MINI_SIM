#include "stardialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

StarDialog::StarDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Support the Project ⭐");
    setModal(true);
    resize(400, 200);

    auto *layout = new QVBoxLayout(this);

    auto *label = new QLabel(
        "If you find this project useful,<br>"
        "please consider supporting it by giving us a ⭐ on GitHub.<br><br>"
        "Your support means a lot — thank you!"
        );
    label->setAlignment(Qt::AlignCenter);

    auto *btnStar = new QPushButton("⭐ Go to GitHub", this);
    auto *btnClose = new QPushButton("Later", this);

    connect(btnStar, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/B1gF1sh/x86-MINI_SIM"));
    });
    connect(btnClose, &QPushButton::clicked, this, &StarDialog::reject);

    layout->addWidget(label);
    layout->addWidget(btnStar);
    layout->addWidget(btnClose);
}
