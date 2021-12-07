#include <QKeyEvent>
#include "Kernel/AspPage.h"
#include "AspPageDialog.h"
#include "StarOrbDialog.h"
#include "ui_StarOrbDialog.h"

namespace napatahti {

StarOrbDialog::StarOrbDialog(AspPage& aspPage, QWidget* root)
    : QDialog(root)
    , ui(new Ui::StarOrbDialog)
    , aspPage_ (aspPage)
{
    ui->setupUi(this);

    if (root != nullptr)
        setLocale(root->locale());

    ui->table->horizontalHeader()->setVisible(true);
    onClickReset();

    connect(ui->table, &QTableWidget::itemChanged, this, &StarOrbDialog::onItemChanged);
    connect(ui->saveButton, &QPushButton::clicked, this, &StarOrbDialog::onClickSave);
    connect(ui->resetButton, &QPushButton::clicked, this, &StarOrbDialog::onClickReset);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

StarOrbDialog::~StarOrbDialog()
{
    delete ui;
}


void StarOrbDialog::onItemChanged(QTableWidgetItem* item)
{
    auto value (AspPageDialog::validateText(item->text()));

    if (value != -1)
        orb_[item->column()] = value;
    else
        item->setText(QString("%1").arg(orb_[item->column()]));

}


void StarOrbDialog::onClickSave()
{
    aspPage_.setStarOrb(orb_);
    accept();
}


void StarOrbDialog::onClickReset()
{
    orb_ = aspPage_.getOrbTable().star;
    ui->table->clearContents();

    auto column (0);
    for (auto i : orb_)
    {
        auto item (new QTableWidgetItem(QString("%1").arg(i)));
        item->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(0, column, item);
        ++column;
    }

    ui->table->setCurrentCell(0, 0);
}


auto StarOrbDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_Return :
    case Qt::Key_Enter :
        onClickSave();
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}

}
