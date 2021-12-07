#ifndef CITYDIALOG_H
#define CITYDIALOG_H

#include <QDialog>

namespace Ui {
class CityDialog;
}

namespace napatahti {

class CityDialog : public QDialog
{
    Q_OBJECT

public :
    explicit CityDialog(const QVariantMap& data, QWidget* root=nullptr);
    ~CityDialog();

signals :
    void accepted(const QVariantMap& data);

public :
    const static QRegularExpression utcReg;
    const static QRegularExpression latReg;
    const static QRegularExpression lonReg;

private :
    bool mode_;
    Ui::CityDialog* ui;

private slots :
    void onAccept();

private :
    auto keyPressEvent(QKeyEvent* event) -> void;
};

} // namespace napatahti

#endif // CITYDIALOG_H
