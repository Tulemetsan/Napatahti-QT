#include "AboutDialog.h"
#include "ui_AboutDialog.h"

namespace napatahti {

AboutDialog::AboutDialog(QWidget* root)
    : QDialog(root)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    if (root != nullptr)
        setLocale(root->locale());

    auto qtPixmap (QPixmap("://qt.png").scaled(QSize(80, 50), Qt::KeepAspectRatio));
    ui->imageLabel->setPixmap(qtPixmap);
}


AboutDialog::~AboutDialog()
{
    delete ui;
}

} // namespace napatahti
