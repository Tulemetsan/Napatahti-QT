#ifndef ASPDIALOG_H
#define ASPDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QColorDialog>

namespace Ui {
class AspDialog;
}

namespace napatahti {

class AspPage;

class AspDialog : public QDialog
{
    Q_OBJECT

public :
    explicit AspDialog(AspPage& aspPage, QWidget* root=nullptr);
    ~AspDialog();

    auto execm(double asp) -> int;

private slots :
    void onClickReset();
    void onClickFont();
    void onClickColor();
    void onSelectColor(const QColor& color);
    void onSelectDash(int index);

    void onClickSave();
    void onClickDelete();

private :
    Ui::AspDialog*  ui;
    QColorDialog*   colorDialog_;
    QGraphicsScene* scene_;

    AspPage& aspPage_;

    double asp_;
    QList<QFont> aspFont_;

    QPen   aspPen_;
    QLineF aspLine_;

    static const std::map<int, QColor> baseColor_;

private :
    auto keyPressEvent(QKeyEvent* event) -> void;

    auto validateAsp(const QString& text) -> double;
    auto updateLine() -> void;
};

} // namespace napatahti

#endif // ASPDIALOG_H
