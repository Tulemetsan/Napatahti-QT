#ifndef LINEEDITDIALOG_H
#define LINEEDITDIALOG_H

#include <QDialog>
#include <QValidator>

namespace Ui {
class LineEditDialog;
}

namespace napatahti {

class LineEditDialog : public QDialog
{
    Q_OBJECT

public :
    explicit LineEditDialog(QWidget* root=nullptr);
    ~LineEditDialog();

    auto setText(const QString& text) -> void;
    auto setValidator(const QValidator* validator) -> void;

signals :
    void accepted(const QString& text);

private :
    Ui::LineEditDialog* ui;

private slots :
    void onAccept();

private :
    auto keyPressEvent(QKeyEvent* event) -> void;
};

} // namespace napatahti

#endif // LINEEDITDIALOG_H
