#include <QSqlError>
#include <QMenu>
#include <QKeyEvent>
#include <QMessageBox>
#include "shared.h"
#include "Kernel/Person.h"
#include "LineEditDialog.h"
#include "PersonDialog.h"
#include "DataBaseDialog.h"
#include "ui_DataBaseDialog.h"

namespace napatahti {

DataBaseDialog::DataBaseDialog(Person& person, AtlasDialog* atlas, QWidget* root)
    : QDialog (root)
    , ui (new Ui::DataBaseDialog)
    , personDialog_ (new PersonDialog(atlas, this))
    , person_ (person)
    , dbase_ (QSqlDatabase::addDatabase("QSQLITE", "DataBaseDialog"))
{
    ui->setupUi(this);
    setLocale(locale_);

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    ui->table->horizontalHeader()->setVisible(true);
    ui->table->setColumnWidth(0, 300);
    ui->table->setColumnWidth(1, 100);
    ui->table->setColumnWidth(2, 100);
    ui->table->setColumnWidth(3, 105);
    ui->table->setColumnWidth(4, 50);
    ui->table->setColumnWidth(5, 150);
    ui->table->setColumnWidth(6, 150);
    ui->table->setColumnWidth(7, 390);
    ui->table->setColumnWidth(8, 70);
    ui->table->setColumnWidth(9, 50);

    dbase_.setDatabaseName(dbaseName_);
    reloadTable(true);

    connect(personDialog_, &PersonDialog::personChanged, this, &DataBaseDialog::onPersonChange);
    connect(personDialog_, &PersonDialog::personInserted, this, &DataBaseDialog::onPersonInsert);
    connect(personDialog_, &PersonDialog::personUpdated, this, &DataBaseDialog::onPersonUpdate);
    connect(personDialog_, &PersonDialog::personDeleted, this, &DataBaseDialog::onPersonDelete);

    connect(ui->tableList, &QListWidget::itemClicked, this, &DataBaseDialog::onChangeTable);
    connect(ui->tableList, &QListWidget::customContextMenuRequested,
                                this, &DataBaseDialog::onContextMenuTableList);

    connect(ui->selectLine, &QLineEdit::textChanged, this, &DataBaseDialog::onTextChange);

    connect(ui->table, &QTableWidget::cellDoubleClicked, this, &DataBaseDialog::onApply);
    connect(ui->table, &QTableWidget::customContextMenuRequested,
                                    this, &DataBaseDialog::onContextMenuTable);

    connect(ui->applyButton, &QPushButton::clicked, this, &DataBaseDialog::onApply);
    connect(ui->newButton, &QPushButton::clicked, this, [this](){ execPersonDialog(false); });
    connect(ui->editButton, &QPushButton::clicked, this, [this](){ execPersonDialog(true); });
    connect(ui->deleteButton, &QPushButton::clicked, this, &DataBaseDialog::onDelete);
    connect(ui->cutButton, &QPushButton::clicked, this, &DataBaseDialog::onCut);
    connect(ui->copyButton, &QPushButton::clicked, this, &DataBaseDialog::onCopy);
    connect(ui->pasteButton, &QPushButton::clicked, this, &DataBaseDialog::onPaste);

    connect(ui->actionApply, &QAction::triggered, this, &DataBaseDialog::onApply);
    connect(ui->actionNew, &QAction::triggered, this, [this](){ execPersonDialog(false); });
    connect(ui->actionEdit, &QAction::triggered, this, [this](){ execPersonDialog(true); });
    connect(ui->actionDelete, &QAction::triggered, this, &DataBaseDialog::onDelete);
    connect(ui->actionCut, &QAction::triggered, this, &DataBaseDialog::onCut);
    connect(ui->actionCopy, &QAction::triggered, this, &DataBaseDialog::onCopy);
    connect(ui->actionPaste, &QAction::triggered, this, &DataBaseDialog::onPaste);
    connect(ui->currentButton, &QPushButton::clicked,
                    this, [this](){ personDialog_->modeExec(true, &person_); });

    connect(ui->newTableButton, &QPushButton::clicked, this, [this](){ execTableDialog(false); });
    connect(ui->editTableButton, &QPushButton::clicked, this, [this](){ execTableDialog(true); });
    connect(ui->deleteTableButton, &QPushButton::clicked, this, &DataBaseDialog::onDeleteTable);

    connect(ui->actionNewTable, &QAction::triggered, this, [this](){ execTableDialog(false); });
    connect(ui->actionRenameTable, &QAction::triggered, this, [this](){ execTableDialog(true); });
    connect(ui->actionDeleteTable, &QAction::triggered, this, &DataBaseDialog::onDeleteTable);
}


DataBaseDialog::~DataBaseDialog()
{
    delete ui;
}


void DataBaseDialog::onPersonChange(const Person& person)
{
    if (person != person_)
    {
        person_ = person;

        if (isVisible())
            ui->table->clearSelection();

        emit personChanged(0, 0);
        emit personNeedSync();
        emit patchNeedSync();
    }
}


void DataBaseDialog::onPersonInsert(const Person& person)
{
    auto resp (false);

    if (dbase_.open())
    {
        auto query (dbase_.exec());

        query.prepare(mask_.insert.arg(table_));
        query.addBindValue(person.name);
        query.addBindValue(person.dateTime);
        query.addBindValue(person.sex);
        query.addBindValue(person.location);
        query.addBindValue(person.lat);
        query.addBindValue(person.lon);
        query.addBindValue(person.hsys);
        query.addBindValue(person.patch);

        resp = query.exec();

        if (resp)
        {
            ui->table->setSortingEnabled(false);

            auto row (addToTable(person));

            if (dialogMode_)
            {
                ui->table->setCurrentItem(ui->table->item(row, 0));

                for (auto i (0); i < 10; ++i)
                    ui->table->item(row, i)->setSelected(true);
            }

            ui->table->setSortingEnabled(true);
        }

        dbase_.close();
    }
    else
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());

    if (dialogMode_)
    {
        person_ = person;
        if (resp)
            person_.table = table_;

        emit personChanged(0, 0);
        emit personNeedSync();
        emit patchNeedSync();
    }
}


void DataBaseDialog::onPersonUpdate(const QVariantMap& key, const Person& person)
{
    if (dbase_.open())
    {
        auto query (dbase_.exec());

        query.prepare(mask_.update.arg(person.table));
        query.addBindValue(person.name);
        query.addBindValue(person.dateTime);
        query.addBindValue(person.sex);
        query.addBindValue(person.location);
        query.addBindValue(person.lat);
        query.addBindValue(person.lon);
        query.addBindValue(person.hsys);
        query.addBindValue(person.patch);
        query.addBindValue(key["name"]);
        query.addBindValue(key["dateTime"]);
        query.addBindValue(key["sex"]);

        if (query.exec() && person.table == table_)
        {
            auto match (ui->table->findItems(key["name"].toString(), Qt::MatchExactly));
            for (auto i : match)
            {
                auto row (i->row());
                if (key == getPersonData(row, false))
                {
                    ui->table->setSortingEnabled(false);

                    ui->table->item(row, 0)->setText(person.name);
                    ui->table->item(row, 1)->setText(person.dateTime.date().toString("dd.MM.yyyy"));
                    ui->table->item(row, 1)->setData(Qt::UserRole, person.dateTime);
                    ui->table->item(row, 2)->setText(person.dateTime.time().toString("hh:mm"));
                    ui->table->item(row, 3)->setText(toStrUtc(person.dateTime.offsetFromUtc(), false));
                    ui->table->item(row, 4)->setText(static_cast<QChar>(person.sex));
                    ui->table->item(row, 4)->setData(Qt::UserRole, person.sex);
                    ui->table->item(row, 5)->setText(toStrCrd(person.lat, true));
                    ui->table->item(row, 5)->setData(Qt::UserRole, person.lat);
                    ui->table->item(row, 6)->setText(toStrCrd(person.lon, false));
                    ui->table->item(row, 6)->setData(Qt::UserRole, person.lon);
                    ui->table->item(row, 7)->setText(person.location);
                    ui->table->item(row, 8)->setText(static_cast<QChar>(person.hsys));
                    ui->table->item(row, 8)->setData(Qt::UserRole, person.hsys);
                    ui->table->item(row, 9)->setText(person.patch.isEmpty() ? "-" : "+");
                    ui->table->item(row, 9)->setData(Qt::UserRole, person.patch);

                    ui->table->setSortingEnabled(true);
                    break;
                }
            }
        }

        dbase_.close();
    }
    else
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());

    if (dialogMode_ || person_.isCurrent(key, person.table))
    {
        person_ = person;
        emit personChanged(0, 0);
        emit personNeedSync();
        emit patchNeedSync();
    }
}


void DataBaseDialog::onPersonDelete(const QVariantMap& data)
{
    auto table (data["table"].toString());

    if (table.isEmpty())
        return;

    if (dbase_.open())
    {
        auto query(dbase_.exec());

        query.prepare(mask_.erase.arg(table));
        query.addBindValue(data["name"]);
        query.addBindValue(data["dateTime"]);
        query.addBindValue(data["sex"]);

        if (query.exec() && table == table_)
        {
            auto match (ui->table->findItems(data["name"].toString(), Qt::MatchExactly));
            for (auto i : match)
            {
                auto row (i->row());
                if (data == getPersonData(row, true))
                {
                    ui->table->removeRow(row);
                    break;
                }
            }
        }

        dbase_.close();
    }
    else
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());

    if (person_.isCurrent(data))
    {
        person_.table.clear();
        emit personNeedSync();
    }
}


void DataBaseDialog::onCreateTable(const QString& text)
{
    if (text.isEmpty())
        return;

    if (dbase_.open())
    {
        auto query (dbase_.exec());
        if (query.exec(mask_.create.arg(text)))
            ui->tableList->addItem(text);

        dbase_.close();
    }
    else
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());
}


void DataBaseDialog::onRenameTable(const QString& text)
{
    if (text.isEmpty())
        return;

    auto selected (ui->tableList->selectedItems());

    if (selected.isEmpty())
        return;

    auto table (selected[0]->text());

    if (dbase_.open())
    {
        auto query (dbase_.exec());
        if (query.exec(mask_.rename.arg(table).arg(text)))
        {
            selected[0]->setText(text);
            if (table_ == table)
            {
                table_ = text;
                emit tableChanged(table_);
            }
            if (person_.table == table)
            {
                person_.table = text;
                emit personNeedSync();
            }
        }
    }
    else
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());
}


void DataBaseDialog::onDeleteTable()
{
    auto selected (ui->tableList->selectedItems());

    if (selected.isEmpty())
        return;

    auto table (selected[0]->text());
    auto msgBox (new QMessageBox(
        QMessageBox::Question, tr("Delete"),
        tr("Do you want to delete") + ' ' + table + '?',
        QMessageBox::Yes | QMessageBox::No, this));

    msgBox->setAttribute(Qt::WA_WindowPropagation, true);
    msgBox->setWindowIcon(QIcon(":/24x24/erase.png"));

    if (msgBox->exec() == QMessageBox::Yes)
    {
        auto resp (false);
        if (dbase_.open())
        {
            auto query (dbase_.exec());

            resp = query.exec(mask_.drop.arg(table));
            dbase_.close();
        }
        else
            errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());

        if (resp)
        {
            if (person_.table == table)
            {
                person_.table.clear();
                emit personNeedSync();
            }
            ui->tableList->removeItemWidget(ui->tableList->takeItem(ui->tableList->row(selected[0])));
        }

        if (ui->tableList->count() == 0)
            reloadTable(true);
    }
}


void DataBaseDialog::onChangeTable(QListWidgetItem* item)
{
    if (item == nullptr)
        return;
    else
    {
        auto table (item->text());

        if (table != table_)
        {
            table_ = table;
            reloadTable();
            emit tableChanged(table_);
        }
    }
}


void DataBaseDialog::onTextChange(const QString& text)
{
    ui->table->clearSelection();
    ui->table->setCurrentCell(-1, -1);

    if (!text.isEmpty())
    {
        auto match (ui->table->findItems(text, Qt::MatchStartsWith));
        if (!match.isEmpty())
        {
            auto row (match[0]->row());
            auto item (ui->table->item(row, 0));

            ui->table->scrollToItem(item);
            ui->table->setCurrentItem(item);
            item->setSelected(true);

            for (auto i (1); i < 10; ++i)
                ui->table->item(row, i)->setSelected(true);
        }
    }
}


void DataBaseDialog::onApply()
{
    auto rows (selectedRows());

    if (rows.size() != 1)
        return;

    auto row (*rows.begin());

    Person person {
        ui->table->item(row, 0)->text(),
        ui->table->item(row, 1)->data(Qt::UserRole).toDateTime(),
        ui->table->item(row, 4)->data(Qt::UserRole).toInt(),
        ui->table->item(row, 7)->text(),
        ui->table->item(row, 5)->data(Qt::UserRole).toInt(),
        ui->table->item(row, 6)->data(Qt::UserRole).toInt(),
        ui->table->item(row, 8)->data(Qt::UserRole).toInt(),
        ui->table->item(row, 9)->data(Qt::UserRole).toString(),
        table_
    };

    if (person_ == person)
    {
        close();
        return;
    }

    person_ = std::move(person);
    emit personChanged(0, 0);
    emit personNeedSync();
    emit patchNeedSync();
    close();
}


void DataBaseDialog::onDelete()
{
    auto rows (selectedRows());

    if (rows.empty())
        return;

    auto part (rows.size() == 1
        ? ui->table->item(*rows.begin(), 0)->text() + ", " +
          ui->table->item(*rows.begin(), 1)->data(Qt::UserRole).toDateTime().toString("dd.MM.yyyy h:mm:ss t")
        : tr("selected rows"));

    auto msgBox (new QMessageBox(
        QMessageBox::Question, tr("Delete"),
        tr("Do you want to delete") + '\n' + part + '?',
        QMessageBox::Yes | QMessageBox::No, this));

    msgBox->setAttribute(Qt::WA_WindowPropagation, true);
    msgBox->setWindowIcon(QIcon(":/24x24/erase.png"));

    if (msgBox->exec() == QMessageBox::Yes)
    {
        auto isCurrentDeleted (false);
        auto match (ui->table->findItems(person_.name, Qt::MatchExactly));

        for (auto i : match)
            if (person_.isCurrent(getPersonData(i->row(), true)))
            {
                isCurrentDeleted = contains(i->row(), rows);
                break;
            }

        if (!dbase_.open())
        {
            errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());
            return;
        }

        auto query (dbase_.exec());

        for (auto i (rows.rbegin()); i != rows.rend(); ++i)
        {
            query.prepare(mask_.erase.arg(table_));
            query.addBindValue(ui->table->item(*i, 0)->text());
            query.addBindValue(ui->table->item(*i, 1)->data(Qt::UserRole));
            query.addBindValue(ui->table->item(*i, 4)->data(Qt::UserRole));

            if (query.exec())
                ui->table->removeRow(*i);
        }

        dbase_.close();

        if (isCurrentDeleted)
        {
            person_.table.clear();
            emit personNeedSync();
        }
    }
}


void DataBaseDialog::onCopy()
{
    auto rows (selectedRows());

    if (rows.empty())
        return;

    clipboard_.clear();
    pasteMode_ = false;

    for (auto i : rows)
        clipboard_.insert(getPerson(i));
}


void DataBaseDialog::onCut()
{
    auto rows (selectedRows());

    if (rows.empty())
        return;

    clipboard_.clear();
    pasteMode_ = true;

    for (auto i (rows.rbegin()); i != rows.rend(); ++i)
    {
        clipboard_.insert(getPerson(*i));
        ui->table->removeRow(*i);
    }
}


void DataBaseDialog::onPaste()
{
    if (clipboard_.empty())
        return;

    if (dbase_.open())
    {
        ui->table->setSortingEnabled(false);

        auto query (dbase_.exec());

        for (auto i : clipboard_)
        {
            if (pasteMode_)
            {
                if (person_.isCurrent(*i))
                {
                    person_.table = table_;
                    emit personNeedSync();
                }

                query.prepare(mask_.erase.arg(i->table));
                query.addBindValue(i->name);
                query.addBindValue(i->dateTime);
                query.addBindValue(i->sex);
                query.exec();
            }

            query.prepare(mask_.insert.arg(table_));
            query.addBindValue(i->name);
            query.addBindValue(i->dateTime);
            query.addBindValue(i->sex);
            query.addBindValue(i->location);
            query.addBindValue(i->lat);
            query.addBindValue(i->lon);
            query.addBindValue(i->hsys);
            query.addBindValue(i->patch);

            if (query.exec())
                addToTable(*i);
        }

        pasteMode_ = false;

        ui->table->setSortingEnabled(true);
        dbase_.close();
    }
    else
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());
}


void DataBaseDialog::onContextMenuTableList(const QPoint& pos)
{
    auto menu (new QMenu(this));

    menu->addAction(ui->actionNewTable);

    if (ui->tableList->itemAt(pos) != nullptr)
    {
        menu->addAction(ui->actionRenameTable);
        menu->addAction(ui->actionDeleteTable);
    }

    menu->exec(ui->tableList->mapToGlobal(pos));
}


void DataBaseDialog::onContextMenuTable(QPoint pos)
{
    auto menu (new QMenu(this));

    if (ui->table->itemAt(pos) != nullptr)
    {
        if (selectedRows().size() > 1)
        {
            menu->addAction(ui->actionNew);
            menu->addAction(ui->actionDelete);
        }
        else
        {
            menu->addAction(ui->actionApply);
            menu->addAction(ui->actionNew);
            menu->addAction(ui->actionEdit);
            menu->addAction(ui->actionDelete);
        }
        menu->addAction(ui->actionCut);
        menu->addAction(ui->actionCopy);
        if (!clipboard_.empty())
            menu->addAction(ui->actionPaste);
    }
    else
    {
        menu->addAction(ui->actionNew);
        if (!clipboard_.empty())
            menu->addAction(ui->actionPaste);
    }

    pos.ry() += ui->table->horizontalHeader()->height();
    menu->exec(ui->table->mapToGlobal(pos));
}


auto DataBaseDialog::showEvent(QShowEvent*) -> void
{
    ui->table->scrollToTop();

    if (!ui->selectLine->text().isEmpty())
        ui->selectLine->clear();
    else
    {
        ui->table->clearSelection();
        ui->table->setCurrentCell(-1, -1);
    }

    ui->selectLine->setFocus();

    auto match (ui->table->findItems(person_.name, Qt::MatchExactly));
    for (auto i : match)
    {
        auto row (i->row());
        if (person_.isCurrent(getPersonData(row, true)))
        {
            ui->table->setCurrentItem(i);
            ui->table->scrollToItem(i);

            for (auto i (1); i < 10; ++i)
                ui->table->item(row, i)->setSelected(true);

            break;
        }
    }

    auto geo (geometry());
    auto end (screen()->geometry().bottomRight());

    end.rx() -= margins_.right() + 1;
    end.ry() -= margins_.bottom() + 1;

    if (geo.right() > end.x())
        geo.moveLeft(end.x() - geo.width());
    if (geo.bottom() > end.y())
        geo.moveTop(end.y() - geo.height());

    setGeometry(geo);
}


auto DataBaseDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_Return :
    case Qt::Key_Enter :
    {
        auto wdg (focusWidget());
        if (ui->tableList == wdg)
            onChangeTable(ui->tableList->currentItem());
        else
            onApply();
    }
        break;
    case Qt::Key_Delete :
    {
        auto wdg (focusWidget());
        if (ui->table == wdg)
            onDelete();
        else if (ui->tableList == wdg)
            onDeleteTable();
    }
        break;
    case Qt::Key_C :
        if (ui->table == focusWidget())
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onCopy();
        break;
    case Qt::Key_X :
        if (ui->table == focusWidget())
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onCut();
        break;
    case Qt::Key_V :
        if (ui->table == focusWidget())
            if ((event->modifiers() & Qt::ControlModifier) != 0)
                onPaste();
        break;
    case Qt::Key_N :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
        {
            if (focusWidget() == ui->tableList)
                execTableDialog(false);
            else
                execPersonDialog(false);
        }
        break;
    case Qt::Key_E :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
        {
            if (focusWidget() == ui->tableList)
                execTableDialog(true);
            else
                execPersonDialog(true);
        }
        break;
    case Qt::Key_B :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
            personDialog_->modeExec(true, &person_);
        break;
    case Qt::Key_Q :
        if ((event->modifiers() & Qt::ControlModifier) != 0 && ui->table->hasFocus())
            ui->tableList->setFocus();
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}


auto DataBaseDialog::reloadTable(bool mode) -> void
{
    if (dbase_.open())
    {
        auto query (dbase_.exec());

        if (mode)
        {
            query.exec(mask_.tables);

            auto count (0);
            while (query.next())
            {
                auto newItem (new QListWidgetItem(query.value(0).toString()));
                ui->tableList->addItem(newItem);

                if (newItem->text() == table_)
                    ui->tableList->setCurrentItem(newItem);

                ++count;
            }

            if (count == 0 || ui->tableList->currentRow() < 0)
            {
                if (count == 0 || !tableReg_.match(table_).hasMatch())
                    table_ = tableBack_;

                dbase_.exec(mask_.create.arg(table_));

                auto newItem (new QListWidgetItem(table_));
                ui->tableList->addItem(newItem);
                ui->tableList->setCurrentItem(newItem);

                emit tableChanged(table_);
            }
        }

        if (ui->table->rowCount() > 0)
        {
            ui->table->clearContents();
            ui->table->setRowCount(0);
        }

        query.exec(mask_.select.arg(table_));
        ui->table->setSortingEnabled(false);

        while (query.next())
            addToTable(Person(query));

        ui->table->setSortingEnabled(true);
        dbase_.close();
    }
    else
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());
}


auto DataBaseDialog::addToTable(const Person& person) -> int
{
    auto row (ui->table->rowCount());
    ui->table->setRowCount(row + 1);

    auto newItem (new QTableWidgetItem(person.name));
    ui->table->setItem(row, 0, newItem);

    newItem = new QTableWidgetItem(person.dateTime.date().toString("dd.MM.yyyy"));
    newItem->setData(Qt::UserRole, person.dateTime);
    newItem->setTextAlignment(Qt::AlignCenter);
    ui->table->setItem(row, 1, newItem);

    newItem = new QTableWidgetItem(person.dateTime.time().toString("hh:mm"));
    newItem->setTextAlignment(Qt::AlignCenter);
    ui->table->setItem(row, 2, newItem);

    newItem = new QTableWidgetItem(toStrUtc(person.dateTime.offsetFromUtc(), false));
    newItem->setTextAlignment(Qt::AlignCenter);
    ui->table->setItem(row, 3, newItem);

    newItem = new QTableWidgetItem(static_cast<QChar>(person.sex));
    newItem->setData(Qt::UserRole, person.sex);
    newItem->setTextAlignment(Qt::AlignCenter);
    ui->table->setItem(row, 4, newItem);

    newItem = new QTableWidgetItem(toStrCrd(person.lat, true));
    newItem->setData(Qt::UserRole, person.lat);
    newItem->setTextAlignment(Qt::AlignCenter);
    ui->table->setItem(row, 5, newItem);

    newItem = new QTableWidgetItem(toStrCrd(person.lon, false));
    newItem->setData(Qt::UserRole, person.lon);
    newItem->setTextAlignment(Qt::AlignCenter);
    ui->table->setItem(row, 6, newItem);

    newItem = new QTableWidgetItem(person.location);
    ui->table->setItem(row, 7, newItem);

    newItem = new QTableWidgetItem(static_cast<QChar>(person.hsys));
    newItem->setData(Qt::UserRole, person.hsys);
    newItem->setTextAlignment(Qt::AlignCenter);
    ui->table->setItem(row, 8, newItem);

    newItem = new QTableWidgetItem(person.patch.isEmpty() ? "-" : "+");
    newItem->setData(Qt::UserRole, person.patch);
    newItem->setTextAlignment(Qt::AlignCenter);
    ui->table->setItem(row, 9, newItem);

    return row;
}


auto DataBaseDialog::selectedRows() const -> std::set<int>
{
    std::set<int> res;

    for (auto& i : ui->table->selectedRanges())
        for (auto j (i.topRow()); j <= i.bottomRow(); ++j)
            res.insert(j);

    return res;
}


auto DataBaseDialog::execTableDialog(bool mode) -> void
{
    auto dialog (new LineEditDialog(this));

    dialog->setValidator(new QRegularExpressionValidator(tableReg_, dialog));

    if (mode)
    {
        dialog->setWindowIcon(QIcon(":/24x24/edit.png"));
        dialog->setWindowTitle(tr("Rename table"));
        dialog->setText(ui->tableList->currentItem()->text());

        connect(dialog, &LineEditDialog::accepted, this, &DataBaseDialog::onRenameTable);
    }
    else
    {
        dialog->setWindowIcon(QIcon(":/24x24/new.png"));
        dialog->setWindowTitle(tr("New table"));
        dialog->setText("NewTitle");

        connect(dialog, &LineEditDialog::accepted, this, &DataBaseDialog::onCreateTable);
    }

    dialog->exec();
}


auto DataBaseDialog::execPersonDialog(bool mode) -> void
{
    dialogMode_ = false;

    if (mode)
    {
        if (selectedRows().size() == 1)
            personDialog_->modeExec(true, getPerson());
    }
    else
        personDialog_->modeExec();

    dialogMode_ = true;
}


auto DataBaseDialog::getPerson(int row) const -> Person*
{
    if (row < 0)
        row = ui->table->currentRow();

    return new Person {
        ui->table->item(row, 0)->text(),
        ui->table->item(row, 1)->data(Qt::UserRole).toDateTime(),
        ui->table->item(row, 4)->data(Qt::UserRole).toInt(),
        ui->table->item(row, 7)->text(),
        ui->table->item(row, 5)->data(Qt::UserRole).toInt(),
        ui->table->item(row, 6)->data(Qt::UserRole).toInt(),
        ui->table->item(row, 8)->data(Qt::UserRole).toInt(),
        ui->table->item(row, 9)->data(Qt::UserRole).toString(),
        table_
    };
}


auto DataBaseDialog::getPersonData(int row, bool table) -> QVariantMap
{
    QVariantMap data {
        {"name", ui->table->item(row, 0)->text()},
        {"dateTime", ui->table->item(row, 1)->data(Qt::UserRole)},
        {"sex", ui->table->item(row, 4)->data(Qt::UserRole)}
    };

    if (table)
        data["table"] = table_;

    return data;
}

} // namespace napatahti
