#include <QKeyEvent>
#include "shared.h"
#include "Kernel/Person.h"
#include "Kernel/AstroBase.h"
#include "Kernel/AspPage.h"
#include "Kernel/AspTable.h"
#include "AspPatchDialog.h"
#include "ui_AspPatchDialog.h"

namespace napatahti {

AspPatchDialog::AspPatchDialog(
    Person&          person,
    const AstroBase& astroBase,
    const AspPage&   aspPage,
    const AspTable&  aspTable,
    QWidget*         root
    )
    : QDialog(root)
    , ui(new Ui::AspPatchDialog)
    , person_ (person)
    , astroBase_ (astroBase)
    , aspPage_ (aspPage)
    , aspTable_ (aspTable)
{
    ui->setupUi(this);

    if (root != nullptr)
        setLocale(root->locale());

    connect(ui->aspBox, &QComboBox::currentIndexChanged, this, &AspPatchDialog::onAspSelect);
    connect(ui->addButton, &QPushButton::clicked, this, &AspPatchDialog::onClickAdd);
    connect(ui->deleteButton, &QPushButton::clicked, this, &AspPatchDialog::onClickDelete);
}

AspPatchDialog::~AspPatchDialog()
{
    delete ui;
}


void AspPatchDialog::onAspSelect(int index)
{
    auto asp (ui->aspBox->itemData(index).toDouble());
    auto ang (aspTable_.getPlanetX2Table().at(key_).ang);

    if (asp != -1)
    {
        auto orbR (std::abs(ang - asp));
        auto orbE (aspPage_.getOrbTable().get(key_[0], key_[1], asp));

        ui->orbEdit->setText(roundTo(orbR, 4) + "°");
        ui->orbRelEdit->setText(roundTo(100 * orbR / orbE, 2) + "%");
        ui->accEdit->setText(accDesc_[2]);
        ui->addButton->setEnabled(true);
    }
    else
    {
        ui->orbEdit->setText("");
        ui->orbRelEdit->setText("");
        ui->accEdit->setText("");
        ui->addButton->setEnabled(false);
    }
}


void AspPatchDialog::onClickAdd()
{
    auto asp (ui->aspBox->currentData().toDouble());
    auto ang (aspTable_.getPlanetX2Table().at(key_).ang);
    auto patch (AspTable::makePatch(person_.patch));

    patch[key_] = {ang, asp, std::abs(ang - asp), 2};
    person_.patch = AspTable::makePatch(patch);

    accept();
}


void AspPatchDialog::onClickDelete()
{
    auto patch (AspTable::makePatch(person_.patch));

    patch.erase(key_);
    person_.patch = AspTable::makePatch(patch);

    accept();
}


auto AspPatchDialog::execm(int row, int column) -> int
{
    auto& order (astroBase_.getPlanetOrder());
    auto& table (aspTable_.getPlanetX2Table());

    key_ = {order[row], order[column]};

    auto& [key, data] (*table.find(key_));
    auto  patch (AspTable::makePatch(person_.patch));
    auto  isSet (contains(key, patch));

    setWindowTitle(
        AstroBase::getPlanetName(key.less) + " : " +
        AstroBase::getPlanetName(key.more) +
        (isSet ? " " + tr("(patch)") : ""));

    ui->angEdit->setText(roundTo(data.ang, 4));
    ui->aspBox->clear();

    if (data.asp != -1)
    {
        auto orb (aspPage_.getOrbTable().get(key_[0], key_[1], data.asp));

        ui->orbEdit->setText(roundTo(data.orb, 4) + "°");
        ui->orbRelEdit->setText(roundTo(100 * data.orb / orb, 2) + "%");
        ui->aspBox->addItem(roundTo(data.asp, 2), data.asp);
        ui->accEdit->setText(accDesc_[data.acc]);
        ui->addButton->setEnabled(false);
        ui->deleteButton->setEnabled(isSet);

        setWindowIcon(QIcon(isSet ? ":/24x24/edit.png" : ":/24x24/view.png"));
    }
    else
    {
        auto& catalog (aspPage_.getAspCatalog());

        ui->aspBox->addItem("", -1);
        if (!catalog.empty())
        {
            auto beg (catalog.begin());
            auto end (--catalog.end());

            if (data.ang < beg->first)
                ui->aspBox->addItem(roundTo((beg->first), 2), beg->first);
            else if (data.ang > end->first)
                ui->aspBox->addItem(roundTo(end->first, 2), end->first);
            else
                for (auto i (beg), j (++catalog.begin()); i != end; ++i, ++j)
                    if (data.ang > i->first && data.ang < j->first)
                    {
                        ui->aspBox->addItem(roundTo(i->first, 2), i->first);
                        ui->aspBox->addItem(roundTo(j->first, 2), j->first);
                    }
        }

        ui->orbEdit->setText("");
        ui->orbRelEdit->setText("");
        ui->accEdit->setText("");
        ui->addButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);

        setWindowIcon(QIcon(":/24x24/edit.png"));
    }

    return exec();
}


auto AspPatchDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_Return :
    case Qt::Key_Enter :
        if (ui->addButton->isEnabled())
            onClickAdd();
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}

} // namespace napatahti
