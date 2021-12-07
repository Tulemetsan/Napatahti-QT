#ifndef STARORBDIALOG_H
#define STARORBDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class StarOrbDialog;
}

namespace napatahti {

class AspPage;


class StarOrbDialog : public QDialog
{
    Q_OBJECT

public :
    explicit StarOrbDialog(AspPage& aspPage, QWidget* root=nullptr);
    ~StarOrbDialog();

private slots :
    void onItemChanged(QTableWidgetItem* item);
    void onClickSave();
    void onClickReset();

private :
    Ui::StarOrbDialog* ui;

    AspPage& aspPage_;

    std::array<double, 5> orb_;

private :
    auto keyPressEvent(QKeyEvent* event) -> void;
};

} // namespace napatahti

#endif // STARORBDIALOG_H
