#include <QRegularExpressionValidator>
#include <QKeyEvent>
#include "shared.h"
#include "Kernel/AstroBase.h"
#include "PlanetDialog.h"
#include "ui_PlanetDialog.h"

namespace napatahti {

PlanetDialog::PlanetDialog(QStringList&& data, QWidget* root)
    : QDialog(root)
    , ui(new Ui::PlanetDialog)
{
    ui->setupUi(this);

    if (root != nullptr)
        setLocale(root->locale());

    ui->symEdit->setValidator(
        new QRegularExpressionValidator(QRegularExpression("^\\S$"), this));
    ui->indEdit->setValidator(
        new QRegularExpressionValidator(QRegularExpression("^\\d{1,7}$"), this));

    if (data.empty())
    {
        data_ = {ui->symEdit->text(), ""};
        ui->deleteButton->setEnabled(false);
    }
    else
    {
        ui->symEdit->setText(data[0]);
        ui->indEdit->setText(data[1]);
        ui->indEdit->setEnabled(false);

        if (contains(data[1].toInt(), AstroBase::planetOrder))
            ui->deleteButton->setEnabled(false);

        setWindowIcon(QIcon(":/24x24/edit.png"));
        setWindowTitle(AstroBase::getPlanetName(data[1].toInt()));

        data_ = std::move(data);
    }

    connect(ui->redoButton, &QPushButton::clicked, this, &PlanetDialog::onRedo);
    connect(ui->saveButton, &QPushButton::clicked, this, &PlanetDialog::onSave);
    connect(ui->deleteButton, &QPushButton::clicked, this, &PlanetDialog::onDelete);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}


PlanetDialog::~PlanetDialog()
{
    delete ui;
}


void PlanetDialog::onSave()
{
    auto sym (ui->symEdit->text());
    auto ind (ui->indEdit->text());

    if (sym.isEmpty() || ind.isEmpty())
    {
        reject();
        return;
    }

    if (data_[1].isEmpty())
        emit created({sym, ind});
    else
        emit updated({sym, ind});

    accept();
}


void PlanetDialog::onDelete()
{
    emit deleted(ui->indEdit->text().toInt());
    accept();
}


void PlanetDialog::onRedo()
{
    ui->symEdit->setText(data_[0]);
    ui->indEdit->setText(data_[1]);
}


auto PlanetDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_Return :
    case Qt::Key_Enter :
        onSave();
        accept();
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}

} // namespace napatahti
