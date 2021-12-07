#include <QKeyEvent>
#include "LineEditDialog.h"
#include "ui_LineEditDialog.h"

namespace napatahti {

LineEditDialog::LineEditDialog(QWidget* root)
    : QDialog(root)
    , ui(new Ui::LineEditDialog)
{
    ui->setupUi(this);

    if (root != nullptr)
        setLocale(root->locale());

    ui->lineEdit->setFocus();

    connect(ui->okButton, &QPushButton::clicked, this, &LineEditDialog::onAccept);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}


LineEditDialog::~LineEditDialog()
{
    delete ui;
}


void LineEditDialog::onAccept()
{
    emit accepted(ui->lineEdit->text());
    accept();
}


auto LineEditDialog::setText(const QString& text) -> void
{
    ui->lineEdit->setText(text);
}


auto LineEditDialog::setValidator(const QValidator* validator) -> void
{
    ui->lineEdit->setValidator(validator);
}


auto LineEditDialog::keyPressEvent(QKeyEvent* event) -> void
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
