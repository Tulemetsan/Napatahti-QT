#include <QMessageBox>
#include <QMenu>
#include <QKeyEvent>
#include "shared.h"
#include "mask.h"
#include "Kernel/Person.h"
#include "Kernel/AstroBase.h"
#include "Kernel/AspPage.h"
#include "Kernel/AspTable.h"
#include "SharedGui.h"
#include "AspPatchDialog.h"
#include "AspTableDialog.h"
#include "ui_AspTableDialog.h"

namespace napatahti {

AspTableDialog::AspTableDialog(
    Person&          person,
    const AstroBase& astroBase,
    const AspPage&   aspPage,
    const AspTable&  aspTable,
    QWidget*         root
    )
    : QDialog(root)
    , ui(new Ui::AspTableDialog)
    , patchDialog_ (new AspPatchDialog(person, astroBase, aspPage, aspTable, this))
    , person_ (person)
    , astroBase_ (astroBase)
    , aspPage_ (aspPage)
    , aspTable_ (aspTable)
{
    ui->setupUi(this);
    setLocale(SharedGui::getAppLocale());

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    ui->table->horizontalHeader()->setVisible(true);
    ui->table->verticalHeader()->setVisible(true);

    auto font (ui->table->font());
    baseFont_ = {
        QFont(font.family(), 0.73 * font.pointSize()),
        QFont("HamburgSymbols", font.pointSize())
    };

    reloadTable();

    connect(ui->editButton, &QPushButton::clicked, this, &AspTableDialog::onClickEdit);
    connect(ui->deleteButton, &QPushButton::clicked, this, &AspTableDialog::onClickDelete);
    connect(ui->clearButton, &QPushButton::clicked, this, &AspTableDialog::onClickClear);

    connect(ui->actEdit, &QAction::triggered, this, &AspTableDialog::onClickEdit);
    connect(ui->actDelete, &QAction::triggered, this, &AspTableDialog::onClickDelete);
    connect(ui->actClear, &QAction::triggered, this, &AspTableDialog::onClickClear);

    connect(ui->table, &QTableWidget::cellDoubleClicked, this, &AspTableDialog::onCellDoubleClick);
    connect(ui->table, &QTableWidget::customContextMenuRequested, this, &AspTableDialog::onTableContext);
}


AspTableDialog::~AspTableDialog()
{
    delete ui;
}


void AspTableDialog::reloadTable()
{
    auto& planetOrder (astroBase_.getPlanetOrder());
    auto& planetCatalog (astroBase_.getPlanetCatalog());
    auto& planetX2Table (aspTable_.getPlanetX2Table());
    auto& aspCatalog (aspPage_.getAspCatalog());
    auto& aspPen (aspPage_.getAspPen());
    auto& aspType (aspPage_.getAspType());

    auto row (0);
    auto size (planetOrder.size());
    auto patch (aspTable_.makePatch(person_.patch));

    ui->table->clear();
    ui->table->setRowCount(size);
    ui->table->setColumnCount(size);

    for (auto i : planetOrder)
    {
        auto view (true);
        auto column (0);

        auto newItem (new QTableWidgetItem(planetCatalog.at(i)));
        newItem->setFont(baseFont_[1]);
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setHorizontalHeaderItem(row, newItem);

        newItem = new QTableWidgetItem(*newItem);
        newItem->setText("    " + newItem->text() + "    ");
        ui->table->setVerticalHeaderItem(row, newItem);

        for (auto j : planetOrder)
        {
            newItem = new QTableWidgetItem;
            newItem->setTextAlignment(Qt::AlignCenter);

            if (row == column)
            {
                view = false;
                newItem->setBackground(brush_[1]);
                ui->table->setItem(row, column, newItem);

                ++column;
                continue;
            }

            auto& data (planetX2Table.at({i,  j}));

            if (data.asp != -1)
            {
                newItem->setText(view ? aspCatalog.at(data.asp) : roundTo(data.ang, 2));
                newItem->setForeground(aspPen.at(data.asp).brush());

                if (contains({i, j}, patch))
                    newItem->setBackground(brush_[2]);

                if (view)
                    newItem->setFont(baseFont_[aspType.at(data.asp)]);
            }
            else if (!view)
            {
                newItem->setText(roundTo(data.ang, 2));
                newItem->setForeground(brush_[0]);
            }

            ui->table->setItem(row, column, newItem);
            ++column;
        }

        ++row;
    }
}


void AspTableDialog::onClickEdit()
{
    onCellDoubleClick(ui->table->currentRow(), ui->table->currentColumn());
}


void AspTableDialog::onClickDelete()
{
    if (person_.patch.isEmpty())
        return;

    auto row (ui->table->currentRow());
    auto column (ui->table->currentColumn());

    if (row == column)
        return;

    auto& order (astroBase_.getPlanetOrder());
    auto  patch (AspTable::makePatch(person_.patch));
    auto  item (patch.find({order[row], order[column]}));

    if (item != patch.end())
    {
        patch.erase(item);
        person_.patch = AspTable::makePatch(patch);

        emit patchChanged(KernelMask::AspTableFull, CanvasMask::AspTableFull);
        emit personNeedSync();

        reloadTable();
        ui->table->setCurrentCell(row, column);
    }
}


void AspTableDialog::onClickClear()
{
    if (person_.patch.isEmpty())
        return;

    auto msgBox (new QMessageBox(
        QMessageBox::Question, tr("Clear"),
        tr("Do you want to clear patch map?"),
        QMessageBox::Yes | QMessageBox::No, this));

    msgBox->setAttribute(Qt::WA_WindowPropagation, true);
    msgBox->setWindowIcon(QIcon(":/24x24/trash.png"));

    if (msgBox->exec() == QMessageBox::Yes)
    {
        person_.patch.clear();

        emit patchChanged(KernelMask::AspTableFull, CanvasMask::AspTableFull);
        emit personNeedSync();

        auto row (ui->table->currentRow());
        auto column (ui->table->currentColumn());

        reloadTable();
        ui->table->setCurrentCell(row, column);
    }
}


void AspTableDialog::onCellDoubleClick(int row, int column)
{
    if (row == column)
        return;

    if (patchDialog_->execm(row, column) == QDialog::Accepted)
    {
        emit patchChanged(KernelMask::AspTableFull, CanvasMask::AspTableFull);
        emit personNeedSync();

        reloadTable();
        ui->table->setCurrentCell(row, column);
    }
}


void AspTableDialog::onTableContext(QPoint pos)
{
    auto& order (astroBase_.getPlanetOrder());
    auto  patch (AspTable::makePatch(person_.patch));

    auto row (ui->table->rowAt(pos.y()));
    auto column (ui->table->columnAt(pos.x()));
    auto cell (aspTable_.getPlanetX2Table().find({order[row], order[column]}));

    auto menu (new QMenu(this));

    if (row != column && row != -1 && column != -1)
    {
        menu->addAction(ui->actEdit);

        if (contains(cell->first, patch))
            menu->addAction(ui->actDelete);
    }

    if (!patch.empty())
    {
        menu->addSeparator();
        menu->addAction(ui->actClear);
    }

    if (!menu->isEmpty())
    {
        pos.rx() += ui->table->verticalHeader()->width();
        pos.ry() += ui->table->horizontalHeader()->height();
        menu->exec(ui->table->mapToGlobal(pos));
    }
}


auto AspTableDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_E :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
            onClickEdit();
        break;
    case Qt::Key_Delete :
        onClickDelete();
        break;
    case Qt::Key_R :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
            onClickClear();
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}

} // namespace napatahti
