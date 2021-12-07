#include <QRegularExpressionValidator>
#include <QKeyEvent>
#include "CityDialog.h"
#include "ui_CityDialog.h"

namespace napatahti {

CityDialog::CityDialog(const QVariantMap& data, QWidget* root)
    : QDialog (root)
    , mode_ (data["mode"].toBool())
    , ui (new Ui::CityDialog)
{
    ui->setupUi(this);

    if (root != nullptr)
        setLocale(root->locale());

    setWindowTitle(mode_ ? tr("Edit") : tr("New"));

    ui->utcEdit->setValidator(new QRegularExpressionValidator(utcReg, this));
    ui->latEdit->setValidator(new QRegularExpressionValidator(latReg, this));
    ui->lonEdit->setValidator(new QRegularExpressionValidator(lonReg, this));

    ui->titleEdit->setFocus();
    ui->titleEdit->setText(data["title"].toString());
    ui->utcEdit->setText(data["utc"].toString());
    ui->latEdit->setText(data["lat"].toString());
    ui->lonEdit->setText(data["lon"].toString());

    connect(ui->saveButton, &QPushButton::clicked, this, &CityDialog::onAccept);
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);
}


CityDialog::~CityDialog()
{
    delete ui;
}


void CityDialog::onAccept()
{
    auto utc (ui->utcEdit->text());

    emit accepted({
        {"title", ui->titleEdit->text()},
        {"utc", (utc == "+00:00" || utc == "-00:00") ? "UTC" : utc},
        {"lat", ui->latEdit->text()},
        {"lon", ui->lonEdit->text()},
        {"mode", mode_}
    });

    accept();
}


auto CityDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_Enter :
    case Qt::Key_Return :
        onAccept();
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}

} // namespace napatahti
