#ifndef ASPPATCHDIALOG_H
#define ASPPATCHDIALOG_H

#include <QDialog>

namespace Ui {
class AspPatchDialog;
}

namespace napatahti {

class Person;
class AstroBase;
class AspPage;
class AspTable;
class ConfigDialog;

class AspPatchDialog : public QDialog
{
    Q_OBJECT

public :
    explicit AspPatchDialog(
        Person&          person,
        const AstroBase& astroBase,
        const AspPage&   aspPage,
        const AspTable&  aspTable,
        QWidget*         root=nullptr);

    ~AspPatchDialog();

    auto execm(int row, int column) -> int;

private slots :
    void onAspSelect(int index);
    void onClickAdd();
    void onClickDelete();

private :
    Ui::AspPatchDialog* ui;

    Person& person_;

    const AstroBase& astroBase_;
    const AspPage&   aspPage_;
    const AspTable&  aspTable_;

    std::array<int, 2> key_;

    static QStringList accDesc_;

private :
    auto keyPressEvent(QKeyEvent* event) -> void;

    friend ConfigDialog;
};

} // namespace napatahti

#endif // ASPPATCHDIALOG_H
