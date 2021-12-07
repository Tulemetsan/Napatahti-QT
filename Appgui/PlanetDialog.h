#ifndef PLANETDIALOG_H
#define PLANETDIALOG_H

#include <QDialog>

namespace Ui {
class PlanetDialog;
}

namespace napatahti {

class PlanetDialog : public QDialog
{
    Q_OBJECT

public :
    explicit PlanetDialog(QStringList&& data, QWidget* root=nullptr);
    ~PlanetDialog();

signals :
    void created(const QStringList& data);
    void updated(const QStringList& data);
    void deleted(int ind);

private slots :
    void onSave();
    void onDelete();
    void onRedo();

private :
    Ui::PlanetDialog* ui;

    QStringList data_;

private :
    auto keyPressEvent(QKeyEvent* event) -> void;
};

} // namespace napatahti

#endif // PLANETDIALOG_H
