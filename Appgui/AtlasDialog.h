#ifndef ATLASDIALOG_H
#define ATLASDIALOG_H

#include <set>
#include <QDialog>
#include <QSqlDatabase>
#include "SharedGui.h"

namespace Ui {
class AtlasDialog;
}

namespace napatahti {

class ConfigDialog;


class AtlasDialog : public QDialog, protected SharedGui
{
    Q_OBJECT

public :
    explicit AtlasDialog(QWidget* root=nullptr);
    ~AtlasDialog();

signals :
    void citySelected(const QVariantMap& city);

public slots :
    void onUpdateBase(const QVariantMap& data);

private :
    Ui::AtlasDialog* ui;

    QSqlDatabase dbase_;

    static QString dbaseName_;
    static QVariantMap newCity_;

private slots :
    void onTextChange(const QString& text);
    void onContextMenu(QPoint pos);
    void onSelectCity();
    void onDeleteCity();

private :
    auto showEvent(QShowEvent*) -> void;
    auto keyPressEvent(QKeyEvent* event) -> void;

    auto execCityDialog(bool mode) -> void;
    auto getCityData(int mode=2) -> QVariantMap;
    auto selectedRows() const -> std::set<int>;

    friend ConfigDialog;
    friend auto staticInit(QApplication& app) -> void;
};

} // namespace napatahti

#endif // ATLASDIALOG_H
