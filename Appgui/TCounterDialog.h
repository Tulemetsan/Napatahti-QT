#ifndef TCOUNTERDIALOG_H
#define TCOUNTERDIALOG_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class TCounterDialog;
}

namespace napatahti {

class Person;


class TCounterDialog : public QDialog
{
    Q_OBJECT

public :
    explicit TCounterDialog(Person* person, QWidget* root=nullptr);
    ~TCounterDialog();

    auto modeShow() -> void;

signals :
    void personChanged(int kMask, int cMask);
    void personNeedSync();

public slots :
    void onDialogSync();

private :
    Ui::TCounterDialog* ui;

    QButtonGroup* bgStepForw_;
    QButtonGroup* bgStepBack_;

    Person* personAct_;
    Person* personRes_;

    const std::array<QDateTime, 2>& epheRange_;

private slots :
    void onClickStep(const QAbstractButton* button);
    void onClickMX(const QAbstractButton* button);
    void onClickSpec(const QAbstractButton* button);
};

} // namespace napatahti

#endif // TCOUNTERDIALOG_H
