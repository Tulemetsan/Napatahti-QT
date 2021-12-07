#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QLabel>
#include <QSpinBox>
#include <QFontComboBox>
#include <QHBoxLayout>
#include <QColorDialog>
#include "SharedGui.h"
#include "CanvasBase.h"

namespace Ui {
class ConfigDialog;
}

namespace napatahti {

class AtlasDialog;


enum RestoreMode {
    SideA  = 1,
    SideB  = 2,
    Base   = 4,
    SideAB = SideA | SideB,
    Full   = SideA | SideB | Base
};


class ConfigDialog : public QDialog, protected SharedGui
{
    Q_OBJECT

public :
    explicit ConfigDialog(AtlasDialog* atlasDialog, QWidget* root=nullptr);
    ~ConfigDialog();

    static auto loadBase() -> bool;
    static auto dumpBase() -> void;
    static auto loadProFile() -> void;
    static auto dumpProFile(const QString& name) -> void;
    static auto restore(int mode) -> void;
    static auto loadExt() -> void;

    static auto profileName() -> const QString& { return profileName_; }
    static auto profileNameR() -> const QString& { return profileNameR_; }
    static auto profileNameList() -> QStringList;

signals :
    void cacheNeedUpdate(int cMask);
    void settingsUpdated(int kMask, int cMask);
    void canvasBkgUpdated();
    void aspPageNeedSync();
    void aspTableNeedSync();
    void triggersReseted();

public slots :
    void onLoadProfile(const QAction* action);

public :
    static const QString path;

private :
    Ui::ConfigDialog* ui;
    QColorDialog*     colorDialog_;
    AtlasDialog*      atlasDialog_;

    QList<QWidget*>     contList_;
    QList<QHBoxLayout*> boneList_;

private slots :
    void onClickList(const QListWidgetItem* item);
    void onClickSave();
    void onClickRedo();
    void onClickFact();

    void onSelectFontVar(int index);
    void onSelectColVar(int index);
    void onSelectSizeVar(int index);

    void onFontChanged(const QFont& font);
    void onFontSizeChanged(int size);
    void onColorSelect(const QColor& color);
    void onSizeChanged(int value);

    void onClickAtlas();
    void onClickSaveCity();
    void onSelectCity(const QVariantMap& city);

    void onMapViewAccChange(int value);
    void onAlmuHardChange(bool value);

private :
    QString nameR_;
    QString locationR_;
    int     utcR_;
    int     latR_;
    int     lonR_;
    int     hsysR_;
    int     mvAccR_;
    bool    almuHardR_;

    CanvasFont  fontSrcR_;
    CanvasColor colorSrcR_;
    CanvasPen   penSrcR_;
    QPen        apgMarkStyleR_;

    std::array<QBrush, 2> apgBrushR_;
    std::array<QBrush, 3> atbBrushR_;

    QMap<QString, int> headerSizeSrcR_;
    QMap<QString, int> recepSizeSrcR_;
    QMap<QString, int> esseStatSizeSrcR_;
    QMap<QString, int> circleSizeSrcR_;
    QMap<QString, int> crdTableSizeSrcR_;
    QMap<QString, int> lineBarSizeSrcR_;
    QMap<QString, int> aspCfgSizeSrcR_;
    QMap<QString, int> aspStatSizeSrcR_;
    QMap<QString, int> coreStatSizeSrcR_;
    QMargins           mainMargR_;

    bool isSaveA_;
    bool isSaveB_;
    bool mutex_ {true};
    int  topHeight_ {-1};

    static QString profileName_;
    static QString profileNameR_;

private :
    auto showEvent(QShowEvent* event) -> void;
    auto setDialogTitle() -> void;
    auto copyReset() -> void;
    auto onReject(bool quit) -> void;

    auto fontVarCombo() { return static_cast<QComboBox*>(contList_[0]); }
    auto fontSelCombo() { return static_cast<QFontComboBox*>(contList_[1]); }
    auto fontSizeSpin()  { return static_cast<QSpinBox*>(contList_[2]); }

    auto colorVarCombo() { return static_cast<QComboBox*>(contList_[3]); }
    auto colorShowLabel() { return static_cast<QLabel*>(contList_[4]); }

    auto sizeVarCombo() { return static_cast<QComboBox*>(contList_[6]); }
    auto sizeVarSpin() { return static_cast<QSpinBox*>(contList_[7]); }

    auto personDefChanged(const QWidget* wdg) -> void;

    auto clearSide() -> void;
    auto showGeneralSide() -> void;
    auto showGuiSide() -> void;

    friend auto staticInit(QApplication& app) -> void;
};

} // namespace napatahti

#endif // CONFIGDIALOG_H
