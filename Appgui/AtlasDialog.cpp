#include <QMenu>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSqlQuery>
#include <QSqlError>
#include "shared.h"
#include "CityDialog.h"
#include "AtlasDialog.h"
#include "ui_AtlasDialog.h"

namespace napatahti {

AtlasDialog::AtlasDialog(QWidget* root)
    : QDialog (root)
    , ui (new Ui::AtlasDialog)
    , dbase_ (QSqlDatabase::addDatabase("QSQLITE", "AtlasDialog"))
{
    ui->setupUi(this);
    setLocale(locale_);

    ui->table->horizontalHeader()->setVisible(true);
    ui->table->setColumnWidth(0, 400);
    ui->table->setColumnWidth(1, 100);
    ui->table->setColumnWidth(2, 150);
    ui->table->setColumnWidth(3, 150);

    //-------------------------------------------------------------------------//

    dbase_.setDatabaseName(dbaseName_);

    if (dbase_.open())
    {
        dbase_.exec("CREATE TABLE IF NOT EXISTS base ("
                        "title BLOB, utc BLOB, lat BLOB, lon BLOB, "
                        "PRIMARY KEY (title, utc));");

        auto query (dbase_.exec("SELECT * FROM base ORDER BY title ASC"));
        auto row (0);

        ui->table->setSortingEnabled(false);

        while (query.next())
        {
            ui->table->setRowCount(row + 1);

            auto newItem (new QTableWidgetItem(query.value("title").toString()));
            ui->table->setItem(row, 0, newItem);

            newItem = new QTableWidgetItem(toStrUtc(query.value("utc").toInt(), false));
            newItem->setTextAlignment(Qt::AlignCenter);
            ui->table->setItem(row, 1, newItem);

            newItem = new QTableWidgetItem(toStrCrd(query.value("lat").toInt(), true));
            ui->table->setItem(row, 2, newItem);
            newItem->setTextAlignment(Qt::AlignCenter);

            newItem = new QTableWidgetItem(toStrCrd(query.value("lon").toInt(), false));
            ui->table->setItem(row, 3, newItem);
            newItem->setTextAlignment(Qt::AlignCenter);

            ++row;
        }

        ui->table->setSortingEnabled(true);
        dbase_.close();
    }
    else
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());

    //-------------------------------------------------------------------------//

    connect(ui->selectLine, &QLineEdit::textChanged, this, &AtlasDialog::onTextChange);
    connect(ui->table, &QTableWidget::cellDoubleClicked, this, &AtlasDialog::onSelectCity);
    connect(ui->table, &QTableWidget::customContextMenuRequested, this, &AtlasDialog::onContextMenu);

    connect(ui->actionSelect, &QAction::triggered, this, &AtlasDialog::onSelectCity);
    connect(ui->actionNew, &QAction::triggered, this, [this](){ execCityDialog(false); });
    connect(ui->actionEdit, &QAction::triggered, this, [this](){ execCityDialog(true); });
    connect(ui->actionDelete, &QAction::triggered, this, &AtlasDialog::onDeleteCity);
}


AtlasDialog::~AtlasDialog()
{
    delete ui;
}


void AtlasDialog::onTextChange(const QString& text)
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
            ui->table->item(row, 1)->setSelected(true);
            ui->table->item(row, 2)->setSelected(true);
            ui->table->item(row, 3)->setSelected(true);
        }
    }
}


void AtlasDialog::onContextMenu(QPoint pos)
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
            menu->addAction(ui->actionSelect);
            menu->addAction(ui->actionNew);
            menu->addAction(ui->actionEdit);
            menu->addAction(ui->actionDelete);
        }
    }
    else
        menu->addAction(ui->actionNew);

    pos.ry() += ui->table->horizontalHeader()->height();
    menu->exec(ui->table->mapToGlobal(pos));
}


void AtlasDialog::onSelectCity()
{
    emit citySelected(getCityData());
    accept();
}


void AtlasDialog::onUpdateBase(const QVariantMap& data)
{
    if (!dbase_.open())
    {
        errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());
        return;
    }

    auto query (dbase_.exec());

    ui->table->setSortingEnabled(false);

    if (data["mode"].toBool())
    {
        auto row (ui->table->currentRow());

        query.prepare("UPDATE base SET title=?, utc=?, lat=?, lon=? WHERE title=? AND utc=?;");
        query.addBindValue(data["title"]);
        query.addBindValue(toIntUtc(data["utc"].toString()));
        query.addBindValue(toIntCrd(data["lat"].toString()));
        query.addBindValue(toIntCrd(data["lon"].toString()));
        query.addBindValue(ui->table->item(row, 0)->text());
        query.addBindValue(toIntUtc(ui->table->item(row, 1)->text()));

        if (query.exec())
        {
            ui->table->item(row, 0)->setText(data["title"].toString());
            ui->table->item(row, 1)->setText(data["utc"].toString());
            ui->table->item(row, 2)->setText(data["lat"].toString());
            ui->table->item(row, 3)->setText(data["lon"].toString());
        }
    }
    else
    {
        query.prepare("INSERT INTO base (title, utc, lat, lon) VALUES (?, ?, ?, ?);");
        query.addBindValue(data["title"]);
        query.addBindValue(toIntUtc(data["utc"].toString()));
        query.addBindValue(toIntCrd(data["lat"].toString()));
        query.addBindValue(toIntCrd(data["lon"].toString()));

        if (query.exec())
        {
            auto row (ui->table->rowCount());
            ui->table->setRowCount(row + 1);

            auto newItem (new QTableWidgetItem(data["title"].toString()));
            ui->table->setItem(row, 0, newItem);

            newItem = new QTableWidgetItem(data["utc"].toString());
            newItem->setTextAlignment(Qt::AlignCenter);
            ui->table->setItem(row, 1, newItem);

            newItem = new QTableWidgetItem(data["lat"].toString());
            ui->table->setItem(row, 2, newItem);
            newItem->setTextAlignment(Qt::AlignCenter);

            newItem = new QTableWidgetItem(data["lon"].toString());
            ui->table->setItem(row, 3, newItem);
            newItem->setTextAlignment(Qt::AlignCenter);
        }
    }

    ui->table->setSortingEnabled(true);
    dbase_.close();
}


void AtlasDialog::onDeleteCity()
{
    auto rows (selectedRows());

    if (rows.empty())
        return;

    auto part (rows.size() == 1
        ? ui->table->item(*rows.begin(), 0)->text() + ", " +
          ui->table->item(*rows.begin(), 1)->text()
        : tr("selected rows"));

    auto msgBox (new QMessageBox(
        QMessageBox::Question, tr("Delete"),
        tr("Do you want to delete") + '\n' + part + '?',
        QMessageBox::Yes | QMessageBox::No, this));

    msgBox->setAttribute(Qt::WA_WindowPropagation, true);
    msgBox->setWindowIcon(QIcon(":/24x24/compass.png"));

    if (msgBox->exec() == QMessageBox::Yes)
    {
        if (!dbase_.open())
        {
            errorLog("Can not open " + dbaseName_ + " : " + dbase_.lastError().text());
            return;
        }

        auto query (dbase_.exec());

        for (auto i (rows.rbegin()); i != rows.rend(); ++i)
        {
            query.prepare("DELETE FROM base WHERE title=? AND utc=?");
            query.addBindValue(ui->table->item(*i, 0)->text());
            query.addBindValue(toIntUtc(ui->table->item(*i, 1)->text()));

            if (query.exec())
                ui->table->removeRow(*i);
        }

        dbase_.close();
    }
}


auto AtlasDialog::showEvent(QShowEvent*) -> void
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
}


auto AtlasDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_Enter :
    case Qt::Key_Return :
        if (selectedRows().size() == 1)
        {
            emit citySelected(getCityData());
            accept();
        }
        break;
    case Qt::Key_Delete :
        onDeleteCity();
        break;
    case Qt::Key_N :
        if ((event->modifiers() & Qt::ControlModifier) != 0)
            execCityDialog(false);
        break;
    case Qt::Key_E :
        if ((event->modifiers() & Qt::ControlModifier) != 0 && selectedRows().size() == 1)
            execCityDialog(true);
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}


auto AtlasDialog::execCityDialog(bool mode) -> void
{
    auto dialog (new CityDialog(getCityData(mode), this));

    connect(dialog, &CityDialog::accepted, this, &AtlasDialog::onUpdateBase);
    dialog->exec();
}


auto AtlasDialog::getCityData(int mode) -> QVariantMap
{
    if (mode > 0)
    {
        auto row (ui->table->currentRow());
        auto utc (ui->table->item(row, 1)->text());

        QVariantMap data {
            {"title", ui->table->item(row, 0)->text()},
            {"utc", utc == "UTC" ? "+00:00" : utc},
            {"lat", ui->table->item(row, 2)->text()},
            {"lon", ui->table->item(row, 3)->text()}
        };

        if (mode == 1)
            data["mode"] = true;

        return data;
    }

    return newCity_;
}


auto AtlasDialog::selectedRows() const -> std::set<int>
{
    std::set<int> res;

    for (auto& i : ui->table->selectedRanges())
        for (auto j (i.topRow()); j <= i.bottomRow(); ++j)
            res.insert(j);

    return res;
}

} // namespace napatahti
