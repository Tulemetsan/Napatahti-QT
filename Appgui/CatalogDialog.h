#ifndef CATALOGDIALOG_H
#define CATALOGDIALOG_H

#include <set>
#include <QDialog>

namespace Ui {
class CatalogDialog;
}

namespace napatahti {

class AstroBase;


class CatalogDialog : public QDialog
{
    Q_OBJECT

public :
    explicit CatalogDialog(AstroBase& astroBase, QWidget* root=nullptr);
    ~CatalogDialog();

signals :
    void catalogUpdated(int kMask, int cMask);
    void catalogNeedSync();

private :
    Ui::CatalogDialog* ui;

    AstroBase& astroBase_;

    bool isDump_ {false};

private slots :
    void onCreate(const QStringList& data);
    void onUpdate(const QStringList& data);
    void onDelete(int ind);
    void onDoubleClick(int row, int column);

    void onContextMenuTable(QPoint pos);
    void onDumpCatalog();

private :
    auto onClickEdit(bool mode) -> void;
    auto onClickDelete(bool mode) -> void;

    auto keyPressEvent(QKeyEvent* event) -> void;

    auto updateTable(bool clean, int ind=-1) -> void;
    auto editDialogExec(QStringList&& data) -> void;
    auto selectedRows() const -> std::set<int>;
};

} // namespace napatahti

#endif // CATALOGDIALOG_H
