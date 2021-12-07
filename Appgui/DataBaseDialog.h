#ifndef DATABASEDIALOG_H
#define DATABASEDIALOG_H

#include <set>
#include <QSqlDatabase>
#include <QDialog>
#include <QListWidgetItem>
#include "SharedGui.h"

namespace Ui {
class DataBaseDialog;
}

namespace napatahti {

class Person;
class AtlasDialog;
class PersonDialog;


class DataBaseDialog : public QDialog, protected SharedGui
{
    Q_OBJECT

public :
    explicit DataBaseDialog(Person& person, AtlasDialog* atlas, QWidget* root=nullptr);
    ~DataBaseDialog();

    static const auto& getTableName() { return table_; }
    static auto setTableName(const QString& name) { table_ = name; }

signals :
    void personChanged(int kMask, int cMask);
    void personNeedSync();
    void patchNeedSync();
    void tableChanged(const QString& table);

public slots :
    void onPersonChange(const Person& person);
    void onPersonInsert(const Person& person);
    void onPersonUpdate(const QVariantMap& key, const Person& person);
    void onPersonDelete(const QVariantMap& data);

public :
    struct SqlMask {
        QString tables {"SELECT name FROM sqlite_master WHERE type='table';"};
        QString create {"CREATE TABLE %1 ("
                             "name BLOB, dateTime BLOB, sex BLOB, "
                             "location BLOB, lat BLOB, lon BLOB, "
                             "hsys BLOB, patch BLOB, "
                             "PRIMARY KEY (name, dateTime, sex));"};
        QString rename {"ALTER TABLE %1 RENAME TO %2;"};
        QString drop {"DROP TABLE %1;"};
        QString select {"SELECT * FROM %1 ORDER BY name ASC;"};
        QString insert {"INSERT INTO %1 (name, dateTime, sex, location, lat, lon, hsys, patch) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);"};
        QString update {"UPDATE %1 SET name=?, dateTime=?, sex=?, location=?, lat=?, lon=?, hsys=?, patch=? "
                        "WHERE name=? AND dateTime=? AND sex=?;"};
        QString erase {"DELETE FROM %1 WHERE name=? AND dateTime=? AND sex=?"};
    };

private :
    Ui::DataBaseDialog* ui;
    PersonDialog* personDialog_;

    bool dialogMode_ {true};
    bool pasteMode_;

    Person& person_;
    QSqlDatabase dbase_;
    std::set<Person*> clipboard_;

    static QString table_;

    static const SqlMask mask_;
    static const QString dbaseName_;
    static const QString tableBack_;
    static const QRegularExpression tableReg_;

private slots :
    void onCreateTable(const QString& text);
    void onRenameTable(const QString& text);
    void onDeleteTable();
    void onChangeTable(QListWidgetItem* item);
    void onTextChange(const QString& text);
    void onApply();
    void onDelete();
    void onCopy();
    void onCut();
    void onPaste();
    void onContextMenuTableList(const QPoint& pos);
    void onContextMenuTable(QPoint pos);

private :
    auto showEvent(QShowEvent*) -> void;
    auto keyPressEvent(QKeyEvent* event) -> void;

    auto reloadTable(bool mode=false) -> void;
    auto execTableDialog(bool mode) -> void;

    auto addToTable(const Person& person) -> int;
    auto selectedRows() const -> std::set<int>;
    auto execPersonDialog(bool mode) -> void;
    auto getPerson(int row=-1) const -> Person*;
    auto getPersonData(int row, bool table) -> QVariantMap;
};

} // namespace napatahti

#endif // DATABASEDIALOG_H
