#ifndef ASPTABLEDIALOG_H
#define ASPTABLEDIALOG_H

#include <QDialog>

namespace Ui {
class AspTableDialog;
}

namespace napatahti {

class Person;
class AstroBase;
class AspPage;
class AspTable;
class AspPatchDialog;
class ConfigDialog;

class AspTableDialog : public QDialog
{
    Q_OBJECT

public :
    explicit AspTableDialog(
        Person&          person,
        const AstroBase& astroBase,
        const AspPage&   aspPage,
        const AspTable&  aspTable,
        QWidget*         root=nullptr);

    ~AspTableDialog();

signals :
    void patchChanged(int kMask, int cMask);
    void personNeedSync();

public slots :
    void reloadTable();

private slots :
    void onClickEdit();
    void onClickDelete();
    void onClickClear();
    void onCellDoubleClick(int row, int column);
    void onTableContext(QPoint pos);

private :
    Ui::AspTableDialog* ui;
    AspPatchDialog* patchDialog_;

    Person&   person_;

    const AstroBase& astroBase_;
    const AspPage&   aspPage_;
    const AspTable&  aspTable_;

    QList<QFont> baseFont_;

    static std::array<QBrush, 3> brush_;

private :
    auto keyPressEvent(QKeyEvent* event) -> void;

    friend ConfigDialog;
};

} // namespace napatahti

#endif // ASPTABLEDIALOG_H
