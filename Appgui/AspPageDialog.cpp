#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QMessageBox>
#include <QKeyEvent>
#include "mask.h"
#include "shared.h"
#include "Kernel/AstroBase.h"
#include "Kernel/AspPage.h"
#include "SharedGui.h"
#include "LineEditDialog.h"
#include "AspDialog.h"
#include "StarOrbDialog.h"
#include "AspPageDialog.h"
#include "ui_AspPageDialog.h"

namespace napatahti {

AspPageDialog::AspPageDialog(AspPage& aspPage, QWidget* root)
    : QDialog(root)
    , ui(new Ui::AspPageDialog)
    , aspDialog_ (new AspDialog(aspPage, this))
    , aspPage_ (aspPage)
{
    ui->setupUi(this);
    setLocale(SharedGui::getAppLocale());

    setWinTitle();
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    auto baseFont (font());
    aspFont_ = {
        QFont(baseFont.family(), 0.73 * baseFont.pointSize()),
        QFont("HamburgSymbols", baseFont.pointSize())
    };

    ui->table->horizontalHeader()->setVisible(true);
    ui->table->verticalHeader()->setVisible(true);
    ui->table->setColumnWidth(1, 65);
    ui->table->setColumnWidth(19, 60);

    reloadTree();
    reloadPage();

    auto& title (aspPage_.getTitle());

    for (auto i (0); i < pageItem_->childCount(); ++i)
    {
        auto child (pageItem_->child(i));
        if (title == child->text(0))
        {
            child->setSelected(true);
            ui->tree->setCurrentItem(child);
            break;
        }
    }

    ui->tree->setFocus();

    connect(ui->tree, &QTreeWidget::itemClicked, this, &AspPageDialog::onClickTree);
    connect(ui->tree, &QTreeWidget::customContextMenuRequested, this, &AspPageDialog::onTreeContext);

    connect(ui->actSavePageAs, &QAction::triggered, this, &AspPageDialog::onExecSavePageAs);
    connect(ui->actNewPage, &QAction::triggered, this, &AspPageDialog::onExecNewPage);
    connect(ui->actNewSamp, &QAction::triggered, this, &AspPageDialog::onExecNewSamp);
    connect(ui->actRename4T, &QAction::triggered, this, &AspPageDialog::onExecRename4T);
    connect(ui->actDelete4T, &QAction::triggered, this, &AspPageDialog::onExecDelete4T);

    connect(ui->savePageAsButton, &QPushButton::clicked, this, &AspPageDialog::onExecSavePageAs);
    connect(ui->newPageButton, &QPushButton::clicked, this, &AspPageDialog::onExecNewPage);
    connect(ui->newSampButton, &QPushButton::clicked, this, &AspPageDialog::onExecNewSamp);
    connect(ui->rename4TButton, &QPushButton::clicked, this, &AspPageDialog::onExecRename4T);
    connect(ui->delete4TButton, &QPushButton::clicked, this, &AspPageDialog::onExecDelete4T);

    connect(ui->table, &QTableWidget::cellDoubleClicked, this, &AspPageDialog::onCellDoubleClick);
    connect(ui->table, &QTableWidget::cellChanged, this, &AspPageDialog::onCellChange);
    connect(ui->table, &QTableWidget::customContextMenuRequested, this, &AspPageDialog::onTableContext);

    connect(ui->actApply, &QAction::triggered, this, &AspPageDialog::onClickApply);
    connect(ui->actNew, &QAction::triggered, this, [this](){ execAspDialog(false); });
    connect(ui->actEdit, &QAction::triggered, this, [this](){ execAspDialog(true); });
    connect(ui->actDelete, &QAction::triggered, this, &AspPageDialog::onClickDelete);
    connect(ui->actCut, &QAction::triggered, this, &AspPageDialog::onClickCut);
    connect(ui->actCopy, &QAction::triggered, this, &AspPageDialog::onClickCopy);
    connect(ui->actPaste, &QAction::triggered, this, &AspPageDialog::onClickPaste);

    connect(ui->applyButton, &QPushButton::clicked, this, &AspPageDialog::onClickApply);
    connect(ui->newButton, &QPushButton::clicked, this, [this](){ execAspDialog(false); });
    connect(ui->editButton, &QPushButton::clicked, this, [this](){ execAspDialog(true); });
    connect(ui->deleteButton, &QPushButton::clicked, this, &AspPageDialog::onClickDelete);
    connect(ui->cutButton, &QPushButton::clicked, this, &AspPageDialog::onClickCut);
    connect(ui->copyButton, &QPushButton::clicked, this, &AspPageDialog::onClickCopy);
    connect(ui->pasteButton, &QPushButton::clicked, this, &AspPageDialog::onClickPaste);
    connect(ui->starButton, &QPushButton::clicked, this, &AspPageDialog::onClickStar);

    connect(qApp, &QApplication::destroyed, this, &AspPageDialog::onDestroy);
}


AspPageDialog::~AspPageDialog()
{
    delete ui;
}


void AspPageDialog::onClickTree(const QTreeWidgetItem* item)
{
    if (item != nullptr)
    {
        switch (item->type()) {
        case 1 :
        case 2 :
        {
            auto& title (aspPage_.getTitle());
            auto  text (item->text(0));

            if (text == title)
                return;

            if (isDump_[0])
            {
                aspPage_.dumpPage(title);
                isDump_[0] = false;
            }

            loadPage(text);
        }
            break;
        case 4 :
        case 5 :
        {
            aspPage_.setAspEnb(item->data(0, Qt::UserRole).toList());
            isDump_[0] = true;
            emit aspPageChanged(KernelMask::AspCfg, CanvasMask::AspPageED);

            auto row (0);
            for (auto i : aspPage_.getAspEnb())
            {
                auto itemED (ui->table->item(row, 2));
                itemED->setForeground(brushED_[i.second]);
                itemED->setText(i.second ? "E" : "D");
                ++row;
            }
        }
            break;
        }
    }
}


void AspPageDialog::onTreeContext(const QPoint& pos)
{
    auto item (ui->tree->itemAt(pos));
    auto menu (new QMenu(this));

    if (item == nullptr)
    {
        menu->addAction(ui->actNewPage);
        menu->addAction(ui->actNewSamp);
    }
    else
        switch (item->type()) {
        case 0 :
            menu->addAction(ui->actNewPage);
            break;
        case 1 :
            menu->addAction(ui->actNewPage);
            menu->addAction(ui->actSavePageAs);
            break;
        case 2 :
            menu->addAction(ui->actNewPage);
            menu->addAction(ui->actRename4T);
            menu->addAction(ui->actDelete4T);
            menu->addAction(ui->actSavePageAs);
            break;
        case 3 :
        case 4 :
            menu->addAction(ui->actNewSamp);
            break;
        case 5 :
            menu->addAction(ui->actNewSamp);
            menu->addAction(ui->actRename4T);
            menu->addAction(ui->actDelete4T);
            break;
        }

    menu->exec(ui->tree->mapToGlobal(pos));
}


void AspPageDialog::onCellDoubleClick(int row, int column)
{
    auto item (ui->table->item(row, column));

    switch (column) {
    case 0 :
    case 1 :
    case 2 :
    {
        auto asp (ui->table->item(row, 0)->data(Qt::UserRole).toDouble());

        if (column == 2)
        {
            auto enb (aspPage_.setAspEnb(asp));

            isDump_[0] = true;
            item->setForeground(brushED_[enb]);
            item->setText(enb ? "E" : "D");

            emit aspPageChanged(KernelMask::AspCfg, CanvasMask::AspPageED);
        }
        else
            execAspDialog(true, asp);
    }
        break;
    default :
        ui->table->editItem(item);
    }
}


void AspPageDialog::onCellChange(int row, int column)
{
    auto item (ui->table->item(row, column));
    auto asp (ui->table->item(row, 0)->data(Qt::UserRole).toDouble());

    switch (column) {
    case 0 :
        item->setText(roundTo(asp, 2));
        break;
    case 1 :
        item->setText("");
        break;
    case 2 :
    {
        auto enb (aspPage_.getAspEnb().at(asp));
        item->setForeground(brushED_[enb]);
        item->setText(enb ? "E" : "D");
    }
        break;
    default :
    {
        auto value (validateText(item->text()));

        if (value != -1)
        {
            isDump_[0] = true;

            switch (column) {
            case 19 :
                aspPage_.setAsterOrb(asp, value);
                item->setText(roundTo(value, 4));
                break;
            case 20 :
                aspPage_.setCuspidOrb(asp, value);
                item->setText(roundTo(value, 4));
                break;
            case 21 :
                aspPage_.setAspPoint(asp, value);
                item->setText(roundTo(value, 2));
                break;
            default :
                aspPage_.setPlanetOrb({AstroBase::planetOrder[column-3], asp}, value);
                item->setText(roundTo(value, 4));
            }
        }
        else
            switch (column) {
            case 19 :
                item->setText(roundTo(aspPage_.getOrbTable().asteroid.at(asp), 4));
                break;
            case 20 :
                item->setText(roundTo(aspPage_.getOrbTable().cuspid.at(asp), 4));
                break;
            case 21 :
                item->setText(roundTo(aspPage_.getAspPoint().at(asp), 2));
                break;
            default :
                item->setText(roundTo(
                    aspPage_.getOrbTable().planet.at({AstroBase::planetOrder[column-3], asp}), 4));
            }
    }
        break;
    }
}


void AspPageDialog::onTableContext(QPoint pos)
{
    auto menu (new QMenu(this));

    menu->addAction(ui->actApply);
    menu->addAction(ui->actNew);

    if (ui->table->itemAt(pos) != nullptr)
    {
        if (selectedRows().size() == 1)
            menu->addAction(ui->actEdit);

        menu->addAction(ui->actDelete);
        menu->addAction(ui->actCut);
        menu->addAction(ui->actCopy);
    }

    if (!clipboard_.isEmpty())
        menu->addAction(ui->actPaste);

    pos.rx() += ui->table->verticalHeader()->width();
    pos.ry() += ui->table->horizontalHeader()->height();
    menu->exec(ui->table->mapToGlobal(pos));
}


void AspPageDialog::onExecNewPage()
{
    auto dialog (new LineEditDialog(this));

    dialog->setWindowIcon(QIcon(":/24x24/new.png"));
    dialog->setWindowTitle(tr("New"));
    dialog->setValidator(new QRegularExpressionValidator(pageReg_, this));
    dialog->setText(tr("New page"));

    connect(dialog, &LineEditDialog::accepted, this, &AspPageDialog::onCreatePage);

    dialog->exec();
}


void AspPageDialog::onExecNewSamp()
{
    auto dialog (new LineEditDialog(this));

    dialog->setWindowIcon(QIcon(":/24x24/filter.png"));
    dialog->setWindowTitle(tr("New"));
    dialog->setText(tr("New sample"));

    connect(dialog, &LineEditDialog::accepted, this, &AspPageDialog::onCreateSamp);

    dialog->exec();
}


void AspPageDialog::onExecRename4T()
{
    auto selected (ui->tree->selectedItems());

    if (selected.isEmpty())
        return;

    auto type (selected[0]->type());

    switch (type) {
    case 2 :
    case 5 :
    {
        auto dialog (new LineEditDialog(this));

        dialog->setWindowIcon(QIcon(":/24x24/edit.png"));
        dialog->setText(selected[0]->text(0));

        if (type == 2)
        {
            dialog->setWindowTitle(tr("Rename page"));
            dialog->setValidator(new QRegularExpressionValidator(pageReg_, this));
            connect(dialog, &LineEditDialog::accepted, this, &AspPageDialog::onRenamePage);
        }
        else
        {
            dialog->setWindowTitle(tr("Rename sample"));
            connect(dialog, &LineEditDialog::accepted, this, &AspPageDialog::onRenameSamp);
        }

        dialog->exec();
    }
        break;
    default :
        return;
    }
}


void AspPageDialog::onExecDelete4T()
{
    auto selected (ui->tree->selectedItems());

    if (selected.isEmpty())
        return;

    auto type (selected[0]->type());

    switch (type) {
    case 2 :
    case 5 :
    {
        auto text (selected[0]->text(0));
        auto msgBox (new QMessageBox(
            QMessageBox::Question, tr("Delete"),
            tr("Do you want to delete") + ' ' + text + '?',
            QMessageBox::Yes | QMessageBox::No, this));

        msgBox->setAttribute(Qt::WA_WindowPropagation, true);
        msgBox->setWindowIcon(QIcon(":/24x24/erase.png"));

        if (msgBox->exec() == QMessageBox::Yes)
        {
            if (type == 2)
            {
                if (QFile::remove(AspPage::path + text + ".json"))
                {
                    pageItem_->removeChild(selected[0]);

                    if (text == aspPage_.getTitle())
                        loadPage(ui->tree->selectedItems()[0]->text(0));
                }
            }
            else
            {
                sampItem_->removeChild(selected[0]);
                isDump_[1] = true;
            }
        }
    }
        break;
    default :
        return;
    }
}


void AspPageDialog::onExecSavePageAs()
{
    auto selected (ui->tree->selectedItems());

    if (selected.isEmpty())
        return;

    switch (selected[0]->type()) {
    case 1 :
    case 2 :
        auto dialog (new LineEditDialog(this));

        dialog->setWindowIcon(QIcon(":/24x24/save.png"));
        dialog->setWindowTitle(tr("Save as"));
        dialog->setValidator(new QRegularExpressionValidator(pageReg_, this));
        dialog->setText(selected[0]->text(0));

        connect(dialog, &LineEditDialog::accepted, this, &AspPageDialog::onSavePageAs);

        dialog->exec();
    }
}


void AspPageDialog::onSavePageAs(const QString& text)
{
    if (!validateText(text, pageItem_))
        return;

    auto& title (savePage());
    auto source (ui->tree->selectedItems()[0]->text(0));
    auto result (false);

    if (source == AspPage::titleR())
    {
        if (title == source)
            result = aspPage_.dumpPage(text);
        else
        {
            aspPage_.loadPage();
            result = aspPage_.dumpPage(text);
            aspPage_.loadPage(title);
        }
    }
    else
        result = QFile::copy(AspPage::path + source + ".json", AspPage::path + text + ".json");

    if (result)
    {
        auto childItem (new QTreeWidgetItem(pageItem_, QStringList(), 2));
        childItem->setText(0, text);
    }
    else
        errorLog("Can not save aspect page " + source + " as " + text);
}


void AspPageDialog::onCreatePage(const QString& text)
{
    if (!validateText(text, pageItem_))
        return;

    if (aspPage_.createPage(text))
    {
        auto childItem (new QTreeWidgetItem(pageItem_, QStringList(), 2));
        childItem->setText(0, text);
    }
}


void AspPageDialog::onRenamePage(const QString& text)
{
    if (!validateText(text, pageItem_))
        return;

    auto& title (savePage());
    auto  child (ui->tree->selectedItems()[0]);

    if (QFile::rename(AspPage::path + child->text(0) + ".json", AspPage::path + text + ".json"))
    {
        if (child->text(0) == title)
        {
            aspPage_.setTitle(text);
            setWinTitle(text);
        }
        child->setText(0, text);
    }
}


void AspPageDialog::onCreateSamp(const QString& text)
{
    if (!validateText(text, sampItem_))
        return;

    QVariantList data;

    for (auto& i : aspPage_.getAspEnb())
        if (i.second)
            data.append(i.first);

    auto childItem (new QTreeWidgetItem(sampItem_, QStringList(), 5));
    childItem->setData(0, Qt::UserRole, data);
    childItem->setText(0, text);

    isDump_[1] = true;
}


void AspPageDialog::onRenameSamp(const QString& text)
{
    if (!validateText(text, sampItem_))
        return;

    ui->tree->selectedItems()[0]->setText(0, text);
    isDump_[1] = true;
}


void AspPageDialog::onClickApply()
{
    savePage();
    emit aspPageChanged(KernelMask::AspPageFull, CanvasMask::AspPageFull);
    emit aspPageNeedSync();
}


void AspPageDialog::onClickDelete()
{
    auto rows (selectedRows());

    if (rows.empty())
        return;

    auto text (rows.size() == 1 ? ' ' + ui->table->item(*rows.begin(), 0)->text()
                                : '\n' + tr("selected aspects"));
    auto msgBox (new QMessageBox(
        QMessageBox::Question, tr("Delete"),
        tr("Do you want to delete") + text + '?',
        QMessageBox::Yes | QMessageBox::No, this));

    msgBox->setAttribute(Qt::WA_WindowPropagation, true);
    msgBox->setWindowIcon(QIcon(":/24x24/erase.png"));

    if (msgBox->exec() == QMessageBox::Yes)
    {
        for (auto i : rows)
            aspPage_.deleteAsp(ui->table->item(i, 0)->data(Qt::UserRole).toDouble());

        isDump_[0] = true;
        reloadPage();

        emit aspPageChanged(KernelMask::AspPageFull, CanvasMask::AspPageFull);
        emit aspPageNeedSync();
    }
}


void AspPageDialog::onClickCut()
{
    auto rows (selectedRows());

    if (rows.empty())
        return;

    clipboard_.clear();

    for (auto i : rows)
    {
        auto asp (ui->table->item(i, 0)->data(Qt::UserRole).toDouble());

        clipboard_.push_back(aspPage_.copyAsp(asp));
        aspPage_.deleteAsp(asp);
    }

    isDump_[0] = true;
    reloadPage();

    emit aspPageChanged(KernelMask::AspPageFull, CanvasMask::AspPageFull);
    emit aspPageNeedSync();
}


void AspPageDialog::onClickCopy()
{
    auto rows (selectedRows());

    if (rows.empty())
        return;

    clipboard_.clear();

    for (auto i : rows)
        clipboard_.push_back(aspPage_.copyAsp(
            ui->table->item(i, 0)->data(Qt::UserRole).toDouble()));
}


void AspPageDialog::onClickPaste()
{
    if (clipboard_.isEmpty())
        return;

    for (const auto& i : clipboard_)
        aspPage_.insertAsp(i);

    isDump_[0] = true;
    reloadPage();

    auto  row (0);
    for (auto& i : aspPage_.getAspCatalog())
    {
        for (const auto& j : clipboard_)
            if (j[0] == i.first)
            {
                if (j == *clipboard_.begin())
                    ui->table->setCurrentCell(row, 1);

                ui->table->item(row, 1)->setSelected(true);

                break;
            }

        ++row;
    }

    emit aspPageChanged(KernelMask::AspPageFull, CanvasMask::AspPageFull);
    emit aspPageNeedSync();
}


void AspPageDialog::onClickStar()
{
    if ((new StarOrbDialog(aspPage_, this))->exec() == QDialog::Accepted)
    {
        isDump_[0] = true;
        emit aspPageChanged(KernelMask::AspTableStar, CanvasMask::CrdTable);
    }
}


void AspPageDialog::onDestroy() const
{
    if (isDump_[0])
        aspPage_.dumpPage(aspPage_.getTitle());

    if (isDump_[1])
        dumpSamp();
}


void AspPageDialog::reloadPage()
{
    auto& aspCatalog (aspPage_.getAspCatalog());
    auto& aspType (aspPage_.getAspType());
    auto& aspEnb (aspPage_.getAspEnb());
    auto& aspPoint (aspPage_.getAspPoint());
    auto& orbTable (aspPage_.getOrbTable());

    ui->table->clearContents();
    ui->table->setRowCount(0);

    auto styleItem (new LineDelegate(aspPage_.getAspPen(), ui->table));
    ui->table->setItemDelegateForColumn(1, styleItem);

    auto row (0);
    for (auto& i : aspCatalog)
    {
        ui->table->setRowCount(row + 1);

        auto newItem (new QTableWidgetItem(i.second));
        newItem->setFont(aspFont_[aspType.at(i.first)]);
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setVerticalHeaderItem(row, newItem);

        newItem = new QTableWidgetItem(roundTo(i.first, 2));
        newItem->setData(Qt::UserRole, i.first);
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 0, newItem);

        newItem = new QTableWidgetItem;
        ui->table->setItem(row, 1, newItem);

        auto enb (aspEnb.at(i.first));
        newItem = new QTableWidgetItem(enb ? "E" : "D");
        newItem->setForeground(brushED_[enb]);
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 2, newItem);

        auto column (3);
        for (auto j : AstroBase::planetOrder)
        {
            newItem = new QTableWidgetItem(roundTo(orbTable.planet.at({j, i.first}), 4));
            newItem->setTextAlignment(Qt::AlignCenter);
            ui->table->setItem(row, column, newItem);
            ++column;
        }

        newItem = new QTableWidgetItem(roundTo(orbTable.asteroid.at(i.first), 4));
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 19, newItem);

        newItem = new QTableWidgetItem(roundTo(orbTable.cuspid.at(i.first), 4));
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 20, newItem);

        newItem = new QTableWidgetItem(roundTo(aspPoint.at(i.first), 2));
        newItem->setTextAlignment(Qt::AlignCenter);
        ui->table->setItem(row, 21, newItem);

        ++row;
    }
}


auto AspPageDialog::keyPressEvent(QKeyEvent* event) -> void
{
    if (ui->tree->hasFocus())
    {
        switch (event->key()) {
        case Qt::Key_Return :
        case Qt::Key_Enter :
            onClickTree(ui->tree->currentItem());
            break;
        case Qt::Key_N :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onExecNewPage();
            break;
        case Qt::Key_S :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onExecSavePageAs();
            break;
        case Qt::Key_M :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onExecNewSamp();
            break;
        case Qt::Key_E :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onExecRename4T();
            break;
        case Qt::Key_Delete :
                onExecDelete4T();
            break;
        default :
            QDialog::keyPressEvent(event);
        }
    }
    else
    {
        switch (event->key()) {
        case Qt::Key_Return :
        case Qt::Key_Enter :
        {
            auto range (ui->table->selectedRanges());
            if (range.size() == 1)
            {
                auto beg (range.begin());
                if (beg->rowCount() == 1 && beg->columnCount() == 1)
                    onCellDoubleClick(beg->topRow(), beg->leftColumn());
            }
            break;
        }
        case Qt::Key_Q :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                ui->tree->setFocus();
            break;
        case Qt::Key_S :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onClickApply();
            break;
        case Qt::Key_N :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                execAspDialog(false);
            break;
        case Qt::Key_E :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                execAspDialog(true);
            break;
        case Qt::Key_Delete :
                onClickDelete();
            break;
        case Qt::Key_X :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onClickCut();
            break;
        case Qt::Key_C :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onClickCopy();
            break;
        case Qt::Key_V :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onClickPaste();
            break;
        case Qt::Key_B :
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onClickStar();
            break;
        default :
            QDialog::keyPressEvent(event);
        }

    }
}


auto AspPageDialog::reloadTree() -> void
{
    QDir pageDir (AspPage::path);
    if (!pageDir.exists())
    {
        errorLog("pages directory not found");
        return;
    }

    auto rootItem (new QTreeWidgetItem(ui->tree, QStringList(), 0));
    rootItem->setText(0, tr("Pages"));
    rootItem->setExpanded(true);
    pageItem_ = rootItem;

    auto childItem (new QTreeWidgetItem(rootItem, QStringList(), 1));
    childItem->setText(0, AspPage::titleR());

    for (const auto& i : pageDir.entryInfoList(QDir::Files))
    {
        if (i.completeSuffix() == "json")
        {
            childItem = new QTreeWidgetItem(rootItem, QStringList(), 2);
            childItem->setText(0, i.baseName());
        }
    }

    rootItem = new QTreeWidgetItem(ui->tree, QStringList(), 3);
    rootItem->setText(0, tr("Samples"));
    rootItem->setExpanded(true);
    sampItem_ = rootItem;

    childItem = new QTreeWidgetItem(rootItem, QStringList(), 4);
    childItem->setText(0, tr("All aspects"));
    childItem->setData(0, Qt::UserRole, QList<QVariant>{-1});

    auto samp (loadSamp());
    for (auto i (samp.constKeyValueBegin()); i != samp.constKeyValueEnd(); ++i)
    {
        childItem = new QTreeWidgetItem(rootItem, QStringList(), 5);
        childItem->setText(0, i->first);
        childItem->setData(0, Qt::UserRole, i->second);
    }
}


auto AspPageDialog::setWinTitle(const QString& title) -> void
{
    setWindowTitle(tr("Aspect pages") + " - " + (title.isEmpty() ? aspPage_.getTitle() : title));
}


auto AspPageDialog::selectedRows() const -> std::set<int>
{
    std::set<int> res;

    for (auto& i : ui->table->selectedRanges())
        for (auto j (i.topRow()); j <= i.bottomRow(); ++j)
            res.insert(j);

    return res;
}


auto AspPageDialog::savePage() -> const QString&
{
    auto& title (aspPage_.getTitle());

    if (isDump_[0])
    {
        isDump_[0] = false;
        aspPage_.dumpPage(title);
    }

    return title;
}


auto AspPageDialog::loadPage(const QString& title) -> void
{
    aspPage_.loadPage(title);

    emit aspPageChanged(KernelMask::AspPageFull, CanvasMask::AspPageFull);
    emit aspPageNeedSync();

    setWinTitle(title);
    reloadPage();
}


auto AspPageDialog::dumpSamp() const -> void
{
    QVariantMap src;

    for (auto i (1); i < sampItem_->childCount(); ++i)
    {
        auto child (sampItem_->child(i));
        src[child->text(0)] = child->data(0, Qt::UserRole);
    }

    if (src.isEmpty())
        return;

    QFile file (AspPageDialog::sampPath_ + QString("samples.txt"));

    if (file.open(QIODevice::WriteOnly))
    {
        file.write(QJsonDocument(QJsonObject::fromVariantMap(src)).toJson());
        file.close();
    }
    else
        errorLog("can not create " + file.fileName());
}


auto AspPageDialog::loadSamp() const -> QVariantMap
{
    QFile      file (AspPageDialog::sampPath_ + QString("samples.txt"));
    QByteArray source;

    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        source = file.readAll();
        file.close();
    }
    else
    {
        errorLog(file.fileName() + " not found");
        return {};
    }

    QJsonParseError parseError;
    QJsonDocument json (QJsonDocument::fromJson(source, &parseError));

    if (json.isNull())
    {
        errorLog(file.fileName() + " parsing error: " + parseError.errorString());
        return {};
    }

    return json.object().toVariantMap();
}


auto AspPageDialog::execAspDialog(bool mode, double asp) -> void
{
    if (mode && asp == -1)
    {
        auto rows (selectedRows());
        if (rows.size() != 1)
            return;

        asp = ui->table->item(*rows.begin(), 0)->data(Qt::UserRole).toDouble();
    }

    if (aspDialog_->execm(asp) == QDialog::Accepted)
    {
        auto column (mode ? ui->table->currentColumn() : -1);

        isDump_[0] = true;
        reloadPage();

        if (mode)
        {
            auto row (0);
            for (auto& i : aspPage_.getAspCatalog())
            {
                if (asp == i.first)
                {
                    ui->table->setCurrentCell(row, column);
                    ui->table->item(row, column)->setSelected(true);
                    break;
                }
                ++row;
            }
        }

        emit aspPageChanged(KernelMask::AspPageFull, CanvasMask::AspPageFull);
        emit aspPageNeedSync();
    }
}


auto AspPageDialog::validateText(const QString& text, const QTreeWidgetItem* root) -> bool
{
    if (text.isEmpty())
        return false;

    for (auto i (0); i < root->childCount(); ++i)
        if (text == root->child(i)->text(0))
            return false;

    return true;
}


auto AspPageDialog::validateText(const QString& text, double min, double max) -> double
{
    auto ok (false);
    auto value (text.toDouble(&ok));

    if (!ok || value < min || value > max)
        return -1;

    return value;
}


auto LineDelegate::paint(QPainter* painter, const QStyleOptionViewItem& opt, const QModelIndex& ind) const -> void
{
    auto rect (opt.rect);
    auto half (QPoint(5, 0.5 * rect.height()));
    auto line (QLine(rect.topLeft() + half, rect.bottomRight() - half));

    if (opt.state & QStyle::State_Selected)
    {
        painter->setPen(markPen_);
        painter->setBrush(markPen_.brush());
        painter->drawRect(rect);
    }

    painter->setPen(aspPen_.at(table_->item(ind.row(), 0)->data(Qt::UserRole).toDouble()));
    painter->drawLine(line.translated(0, -1));
    painter->drawLine(line);
    painter->drawLine(line.translated(0, 1));
}

} // namespace napatahti
