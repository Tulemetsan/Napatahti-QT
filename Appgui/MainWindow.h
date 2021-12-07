#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "AppTrigger.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace napatahti {

class Kernel;
class Canvas;
class AtlasDialog;
class PersonDialog;
class DataBaseDialog;
class AspPageDialog;
class CatalogDialog;
class AspTableDialog;
class TCounterDialog;
class ConfigDialog;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public :
    explicit MainWindow(QWidget* root=nullptr);
    ~MainWindow();

    static const auto& getTrigger() { return trigger_; }

private slots :
    void onUpdate(int kMask, int cMask);

    void onDataSaveAs(const QAction* action);
    void onPrintMap();

    void onViewSet(const QAction* action);
    void onHeaderSet(const QAction* action);
    void onEsseStatSet(const QAction* action);
    void onCircleSet(const QAction* action);
    void onCrdTableSet(bool checked);
    void onLineBarSet(const QAction* action);
    void onAspCfgSet(const QAction* action);
    void onCoreStatSet(const QAction* action);

    void onDialogShow(const QAction* action);
    void onCanvasMenu(const QPoint& pos);
    void onTriggerReset();
    void onClickSaveAsProfile();
    void onClickDeleteProfile();
    void onSaveAsProfile(const QString& text);

private :
    Ui::MainWindow* ui;
    Kernel* kernel_;
    Canvas* canvas_;

    AtlasDialog*    atlasDialog_;
    PersonDialog*   personDialog_;
    DataBaseDialog* dbaseDialog_;
    AspPageDialog*  aspPageDialog_;
    CatalogDialog*  catalogDialog_;
    AspTableDialog* aspTableDialog_;
    TCounterDialog* tCounterDialog_;
    ConfigDialog*   configDialog_;

    QActionGroup* agAccKey_;
    QActionGroup* agCoreMode_;

    static AppTrigger trigger_;

private :
    auto setupBase() -> void;
    auto setProfileMenu() -> void;
    auto applyTrigger() -> void;

    friend ConfigDialog;
};

} // namespace napatahti

#endif // MAINWINDOW_H
