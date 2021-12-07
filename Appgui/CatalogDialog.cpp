#include <QMenu>
#include <QMessageBox>
#include <QKeyEvent>
#include "shared.h"
#include "mask.h"
#include "Kernel/AstroBase.h"
#include "SharedGui.h"
#include "PlanetDialog.h"
#include "CatalogDialog.h"
#include "ui_CatalogDialog.h"

namespace napatahti {

CatalogDialog::CatalogDialog(AstroBase& astroBase, QWidget* root)
    : QDialog(root)
    , ui(new Ui::CatalogDialog)
    , astroBase_ (astroBase)
{
    ui->setupUi(this);
    setLocale(SharedGui::getAppLocale());

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

    ui->table->horizontalHeader()->setVisible(true);
    ui->table->setColumnWidth(0, 40);
    ui->table->setColumnWidth(1, 80);
    ui->table->setColumnWidth(2, 40);
    ui->table->setColumnWidth(3, 40);
    ui->table->setColumnWidth(4, 130);

    updateTable(false);

    connect(ui->actSave, &QAction::triggered, this, &CatalogDialog::onDumpCatalog);
    connect(ui->actNew, &QAction::triggered, this, [this](){ editDialogExec({}); });
    connect(ui->actEdit, &QAction::triggered, this, [this](){ onClickEdit(false); });
    connect(ui->actDelete, &QAction::triggered, this, [this](){ onClickDelete(false); });
    connect(ui->actCrd, &QAction::triggered, this, [this](){ onDoubleClick(ui->table->currentRow(), 2); });
    connect(ui->actAsp, &QAction::triggered, this, [this](){ onDoubleClick(ui->table->currentRow(), 3); });

    connect(ui->saveButton, &QPushButton::clicked, this, &CatalogDialog::onDumpCatalog);
    connect(ui->newButton, &QPushButton::clicked, this, [this](){ editDialogExec({}); });
    connect(ui->editButton, &QPushButton::clicked, this, [this](){ onClickEdit(true); });
    connect(ui->deleteButton, &QPushButton::clicked, this, [this](){ onClickDelete(true); });

    connect(ui->table, &QTableWidget::cellDoubleClicked, this, &CatalogDialog::onDoubleClick);
    connect(ui->table, &QTableWidget::customContextMenuRequested, this, &CatalogDialog::onContextMenuTable);
    connect(qApp, &QApplication::destroyed, this, &CatalogDialog::onDumpCatalog);
}


CatalogDialog::~CatalogDialog()
{
    delete ui;
}


void CatalogDialog::onCreate(const QStringList& data)
{
    if (astroBase_.createPlanet(data))
    {
        isDump_ = true;
        updateTable(false, data[1].toInt());

        emit catalogUpdated(KernelMask::AspTable, -1);
    }
}


void CatalogDialog::onUpdate(const QStringList& data)
{
    if (astroBase_.updatePlanet(data))
    {
        isDump_ = true;
        ui->table->item(ui->table->currentRow(), 1)->setText(data[0]);

        emit catalogUpdated(-1, CanvasMask::PlanetCatFull);
        emit catalogNeedSync();
    }
}


void CatalogDialog::onDelete(int ind)
{
    if (astroBase_.deletePlanet(ind))
    {
        isDump_ = true;
        ui->table->removeRow(ui->table->currentRow());

        emit catalogUpdated(KernelMask::AspTable, CanvasMask::PlanetCatFull);
        emit catalogNeedSync();
    }
}


void CatalogDialog::onDoubleClick(int row, int column)
{
    switch (column) {
    case 2 :
    case 3 :
    {
        auto isCrd (column == 2);
        auto item (ui->table->item(row, column));

        isDump_ = true;
        item->setText(item->text() == "-" ? "|" : "-");

        astroBase_.switchEnb(ui->table->item(row, 0)->text().toInt(), isCrd);

        if (isCrd)
        {
            emit catalogUpdated(KernelMask::PlanetCatFull, CanvasMask::PlanetCatFull);
            emit catalogNeedSync();
        }
        else
            emit catalogUpdated(KernelMask::AspCfg, CanvasMask::PlanetCatAsp);
    }
        break;
    default :
        onClickEdit(false);
    }
}


void CatalogDialog::onContextMenuTable(QPoint pos)
{
    auto menu (new QMenu(this));
    auto rows (selectedRows());

    menu->addAction(ui->actNew);

    if (rows.size() == 1)
    {
        auto  ind (ui->table->item(*rows.begin(), 0)->text().toInt());
        auto& enb (astroBase_.getPlanetEnb().at(ind));

        ui->actCrd->setChecked(enb.crd);
        ui->actAsp->setChecked(enb.asp);

        menu->addAction(ui->actEdit);

        if (!contains(ind, AstroBase::planetOrder))
            menu->addAction(ui->actDelete);

        menu->addSeparator();
        menu->addAction(ui->actCrd);
        menu->addAction(ui->actAsp);
    }

    menu->addSeparator();
    menu->addAction(ui->actSave);

    pos.ry() += ui->table->horizontalHeader()->height();
    menu->exec(ui->table->mapToGlobal(pos));
}


void CatalogDialog::onDumpCatalog()
{
    if (isDump_)
    {
        astroBase_.dumpCatalog();
        isDump_ = false;
    }
}


auto CatalogDialog::onClickEdit(bool mode) -> void
{
    auto row (ui->table->currentRow());

    if (mode && selectedRows().size() != 1)
        return;

    editDialogExec({ui->table->item(row, 1)->text(), ui->table->item(row, 0)->text()});
}


auto CatalogDialog::onClickDelete(bool mode) -> void
{
    auto ind (ui->table->item(ui->table->currentRow(), 0)->text().toInt());

    if (mode)
    {
        if (selectedRows().size() != 1 || contains(ind, AstroBase::planetOrder))
            return;
    }

    auto msgBox (new QMessageBox(
        QMessageBox::Question, tr("Delete"),
        tr("Do you want to delete") + ' ' + AstroBase::getPlanetName(ind) + '?',
        QMessageBox::Yes | QMessageBox::No, this));

    msgBox->setAttribute(Qt::WA_WindowPropagation, true);
    msgBox->setWindowIcon(QIcon(":/24x24/erase.png"));

    if (msgBox->exec() == QMessageBox::Yes)
        onDelete(ind);
}


auto CatalogDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_S :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
            onDumpCatalog();
        break;
    case Qt::Key_N :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
            editDialogExec({});
        break;
    case Qt::Key_E :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
            onClickEdit(true);
        break;
    case Qt::Key_Delete :
        onClickDelete(true);
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}


auto CatalogDialog::updateTable(bool clean, int ind) -> void
{
    auto& planetCatalog (astroBase_.getPlanetCatalog());
    auto& planetEnb (astroBase_.getPlanetEnb());
    auto& planetCrdStr (astroBase_.getPlanetCrdStr());

    QFont   symFont ("HamburgSymbols", font().pointSize());
    QString enbMask ("-|");

    if (clean)
        ui->table->clearContents();

    auto beg (planetCatalog.begin());
    auto end (planetCatalog.end());

    if (ind >= 0)
    {
         auto res (planetCatalog.find(ind));
         beg = res;
         end = ++res;
    }

    for (auto i (beg); i != end; ++i)
    {
        auto row (ui->table->rowCount());
        ui->table->setRowCount(row + 1);

        auto newItem (new QTableWidgetItem(QString("%1").arg(i->first)));
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 0, newItem);

        newItem = new QTableWidgetItem(i->second);
        newItem->setFont(symFont);
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 1, newItem);

        auto& enb (planetEnb.at(i->first));

        newItem = new QTableWidgetItem(enbMask[enb.crd]);
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 2, newItem);

        newItem = new QTableWidgetItem(enbMask[enb.asp]);
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 3, newItem);

        newItem = new QTableWidgetItem(planetCrdStr.at(i->first)[0]);
        newItem->setFont(symFont);
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 4, newItem);
    }
}


auto CatalogDialog::editDialogExec(QStringList&& data) -> void
{
    auto dialog (new PlanetDialog(std::move(data), this));

    connect(dialog, &PlanetDialog::created, this, &CatalogDialog::onCreate);
    connect(dialog, &PlanetDialog::updated, this, &CatalogDialog::onUpdate);
    connect(dialog, &PlanetDialog::deleted, this, &CatalogDialog::onDelete);

    dialog->exec();
}


auto CatalogDialog::selectedRows() const -> std::set<int>
{
    std::set<int> res;

    for (auto& i : ui->table->selectedRanges())
        for (auto j (i.topRow()); j <= i.bottomRow(); ++j)
            res.insert(j);

    return res;
}

} // namespace napatahti
