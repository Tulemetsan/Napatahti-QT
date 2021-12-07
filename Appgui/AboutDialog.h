#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

namespace napatahti {

class AboutDialog : public QDialog
{
    Q_OBJECT

public :
    explicit AboutDialog(QWidget* root=nullptr);
    ~AboutDialog();

private :
    Ui::AboutDialog* ui;
};

} // namespace napatahti

#endif // ABOUTDIALOG_H
