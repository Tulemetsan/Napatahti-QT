#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QScreen>
#include <QDir>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include "mask.h"
#include "shared.h"
#include "Kernel/Person.h"
#include "Kernel/RefBook.h"
#include "Kernel/AspPage.h"
#include "Kernel/AspTable.h"
#include "Kernel/PrimeTest.h"
#include "MainWindow.h"
#include "Canvas.h"
#include "PersonDialog.h"
#include "AtlasDialog.h"
#include "CityDialog.h"
#include "DataBaseDialog.h"
#include "AspPageDialog.h"
#include "AspTableDialog.h"
#include "AspPatchDialog.h"
#include "ConfigDialog.h"
#include "ui_ConfigDialog.h"

namespace napatahti {

ConfigDialog::ConfigDialog(AtlasDialog* atlasDialog, QWidget* root)
    : QDialog (root)
    , ui (new Ui::ConfigDialog)
    , colorDialog_ (new QColorDialog(this))
    , atlasDialog_ (atlasDialog)
{
    ui->setupUi(this);
    setLocale(locale_);

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

    ui->grid->addItem(ui->grid->takeAt(ui->grid->indexOf(ui->sideList)), 0, 0, 10);

    auto type (0);
    for (auto& i : {tr("General"), tr("Interface"), tr("Triggers")})
    {
        new QListWidgetItem(i, ui->sideList, type);
        ++type;
    }
    ui->sideList->setCurrentRow(0);

    auto buttonHL (new QHBoxLayout);
    auto saveButton (new QPushButton(tr("Save"), this));
    auto redoButton (new QPushButton(tr("Reset"), this));
    auto factButton (new QPushButton(tr("Default"), this));
    auto exitButton (new QPushButton(tr("Close"), this));

    saveButton->setFocusPolicy(Qt::NoFocus);
    saveButton->setToolTip(tr("Save settings and close dialog"));
    redoButton->setFocusPolicy(Qt::NoFocus);
    redoButton->setToolTip(tr("Reset settings to state when dialog was running"));
    factButton->setFocusPolicy(Qt::NoFocus);
    factButton->setToolTip(tr("Restore default settings"));
    exitButton->setFocusPolicy(Qt::NoFocus);
    exitButton->setToolTip(tr("Reject settings and close dialog"));

    buttonHL->addWidget(saveButton);
    buttonHL->addWidget(redoButton);
    buttonHL->addWidget(factButton);
    buttonHL->addWidget(exitButton);
    ui->grid->addLayout(buttonHL, 9, 1, 1, 4);

    connect(ui->sideList, &QListWidget::itemClicked, this, &ConfigDialog::onClickList);
    connect(colorDialog_, &QColorDialog::colorSelected, this, &ConfigDialog::onColorSelect);

    connect(saveButton, &QPushButton::clicked, this, &ConfigDialog::onClickSave);
    connect(redoButton, &QPushButton::clicked, this, &ConfigDialog::onClickRedo);
    connect(factButton, &QPushButton::clicked, this, &ConfigDialog::onClickFact);
    connect(exitButton, &QPushButton::clicked, this, &ConfigDialog::reject);

    connect(this, &QDialog::rejected, this, [this](){ onReject(false); });
    connect(qApp, &QApplication::destroyed, this, [this](){ onReject(true); });
}


ConfigDialog::~ConfigDialog()
{
    delete ui;
}


void ConfigDialog::onLoadProfile(const QAction* action)
{
    if (action == nullptr)
        profileName_ = profileNameR_;
    else
    {
        if (profileName_ == action->text())
            return;

        profileName_ = action->text();
    }

    loadProFile();

    if (isVisible())
    {
        isSaveA_ = false;
        isSaveB_ = false;

        copyReset();
        onClickList(ui->sideList->currentItem());
        setDialogTitle();
    }

    emit cacheNeedUpdate(CanvasMask::CacheFull);
    emit settingsUpdated(0, 0);
    emit canvasBkgUpdated();
    emit aspPageNeedSync();
    emit aspTableNeedSync();
}


void ConfigDialog::onClickList(const QListWidgetItem* item)
{
    if (item == nullptr)
        return;

    switch (item->type()) {
    case 0 :
        showGeneralSide();
        break;
    case 1 :
        showGuiSide();
        break;
    case 2 :
    {
        clearSide();

        auto infoLabel (new QLabel(tr("For reset triggers to factory state click \"Default\"."), this));
        ui->grid->addWidget(infoLabel ,0, 1, 1, 4);

        contList_ = {infoLabel};
    }
        break;
    }
}


void ConfigDialog::onClickSave()
{
    if (profileName_ != profileNameR_)
        dumpProFile(profileName_);

    accept();
}


void ConfigDialog::onClickRedo()
{
    auto item (ui->sideList->currentItem());

    if (item == nullptr)
        return;

    switch (item->type()) {
    case 0 :
        Person::name_ = nameR_;
        Person::location_ = locationR_;
        Person::utc_ = utcR_;
        Person::lat_ = latR_;
        Person::lon_ = lonR_;
        Person::hsys_ = hsysR_;
        AspTable::mvAcc_ = mvAccR_;
        PrimeTest::almuHard_ = almuHardR_;

        isSaveA_ = false;
        showGeneralSide();

        emit cacheNeedUpdate(CanvasMask::CacheFull);
        emit settingsUpdated(0, 0);
        break;
    case 1 :
        Canvas::fontSrc_ = fontSrcR_;
        Canvas::colorSrc_ = colorSrcR_;
        Canvas::penSrc_ = penSrcR_;
        AspPageDialog::brushED_ = apgBrushR_;
        LineDelegate::markPen_ = apgMarkStyleR_;
        AspTableDialog::brush_ = atbBrushR_;
        Canvas::headerSizeSrc_ = headerSizeSrcR_;
        Canvas::recepSizeSrc_ = recepSizeSrcR_;
        Canvas::esseStatSizeSrc_ = esseStatSizeSrcR_;
        Canvas::circleSizeSrc_ = circleSizeSrcR_;
        Canvas::crdTableSizeSrc_ = crdTableSizeSrcR_;
        Canvas::lineBarSizeSrc_ = lineBarSizeSrcR_;
        Canvas::aspCfgSizeSrc_ = aspCfgSizeSrcR_;
        Canvas::aspStatSizeSrc_ = aspStatSizeSrcR_;
        Canvas::coreStatSizeSrc_ = coreStatSizeSrcR_;
        SharedGui::margins_ = mainMargR_;

        isSaveB_ = false;
        onSelectFontVar(fontVarCombo()->currentIndex());
        onSelectColVar(colorVarCombo()->currentIndex());
        onSelectSizeVar(sizeVarCombo()->currentIndex());

        emit cacheNeedUpdate(CanvasMask::CacheFull);
        emit settingsUpdated(-1, 0);
        emit canvasBkgUpdated();
        emit aspPageNeedSync();
        emit aspTableNeedSync();
        break;
    }
}


void ConfigDialog::onClickFact()
{
    auto item (ui->sideList->currentItem());

    if (item == nullptr)
        return;

    switch (item->type()) {
    case 0 :
        restore(RestoreMode::SideA);
        isSaveA_ = true;
        showGeneralSide();

        emit cacheNeedUpdate(CanvasMask::CacheFull);
        emit settingsUpdated(0, 0);
        break;
    case 1 :
        restore(RestoreMode::SideB);
        isSaveB_ = true;
        onSelectFontVar(fontVarCombo()->currentIndex());
        onSelectColVar(colorVarCombo()->currentIndex());
        onSelectSizeVar(sizeVarCombo()->currentIndex());

        emit cacheNeedUpdate(CanvasMask::CacheFull);
        emit settingsUpdated(-1, 0);
        emit canvasBkgUpdated();
        emit aspPageNeedSync();
        emit aspTableNeedSync();
        break;
    case 2 :
        emit triggersReseted();
        break;
    }
}


void ConfigDialog::onSelectFontVar(int index)
{
    if (index == -1)
        return;

    const auto& font (Canvas::fontSrc_[fontVarCombo()->itemData(index).toInt()]);

    mutex_ = false;
    fontSelCombo()->setCurrentFont(font);
    fontSizeSpin()->setValue(font.pointSize());
    mutex_ = true;
}


void ConfigDialog::onSelectColVar(int index)
{
    if (index == -1)
        return;

    auto data (colorVarCombo()->itemData(index).toList());

    QColor color;
    switch (data[0].toInt()) {
    case 0 :
        color = Canvas::colorSrc_.textFg.color();
        break;
    case 1 :
        color = (&Canvas::colorSrc_.textFg + data[1].toInt())->color();
        break;
    case 2 :
        color = Canvas::colorSrc_.signBkg[data[1].toInt()].color();
        break;
    case 3 :
        color = Canvas::colorSrc_.spec[data[1].toInt()].color();
        break;
    case 4 :
        color = *(&Canvas::colorSrc_.cuspLineFg + data[1].toInt());
        break;
    case 5 :
        color = Canvas::colorSrc_.force[data[1].toInt()];
        break;
    case 6 :
        color = (&Canvas::penSrc_.textFg + data[1].toInt())->color();
        break;
    case 7 :
        color = AspPageDialog::brushED_[data[1].toInt()].color();
        break;
    case 8 :
        color = LineDelegate::markPen_.color();
        break;
    case 9 :
        color = AspTableDialog::brush_[data[1].toInt()].color();
        break;
    }

    colorDialog_->setCurrentColor(color);
    colorShowLabel()->setStyleSheet(QString("background : %1").arg(color.name()));
}


void ConfigDialog::onSelectSizeVar(int index)
{
    if (index == -1)
        return;

    mutex_ = false;

    auto data (sizeVarCombo()->itemData(index).toList());
    auto spin (sizeVarSpin());

    spin->setMinimum(data[2].toInt());
    spin->setMaximum(data[3].toInt());

    switch (data[0].toInt()) {
    case 0 :
        spin->setValue(Canvas::headerSizeSrc_.value(data[1].toString()));
        break;
    case 1 :
        spin->setValue(Canvas::recepSizeSrc_.value(data[1].toString()));
        break;
    case 2 :
        spin->setValue(Canvas::esseStatSizeSrc_.value(data[1].toString()));
        break;
    case 3 :
        spin->setValue(Canvas::circleSizeSrc_.value(data[1].toString()));
        break;
    case 4 :
        spin->setValue(Canvas::crdTableSizeSrc_.value(data[1].toString()));
        break;
    case 5 :
        spin->setValue(Canvas::lineBarSizeSrc_.value(data[1].toString()));
        break;
    case 6 :
        spin->setValue(Canvas::aspCfgSizeSrc_.value(data[1].toString()));
        break;
    case 7 :
        spin->setValue(Canvas::aspStatSizeSrc_.value(data[1].toString()));
        break;
    case 8 :
        spin->setValue(Canvas::coreStatSizeSrc_.value(data[1].toString()));
        break;
    case 9 :
        switch (data[1].toInt()) {
        case 0 :
            spin->setValue(SharedGui::margins_.left());
            break;
        case 1 :
            spin->setValue(SharedGui::margins_.top());
            break;
        case 2 :
            spin->setValue(SharedGui::margins_.right());
            break;
        case 3 :
            spin->setValue(SharedGui::margins_.bottom());
            break;
        }
        break;
    }

    mutex_ = true;
}


void ConfigDialog::onFontChanged(const QFont& font)
{
    if (!mutex_)
        return;

    isSaveB_ = true;

    Canvas::fontSrc_[fontVarCombo()->currentData().toInt()].setFamily(font.family());

    emit cacheNeedUpdate(CanvasMask::CacheFull);
    emit settingsUpdated(-1, 0);
}


void ConfigDialog::onFontSizeChanged(int size)
{
    if (!mutex_)
        return;

    isSaveB_ = true;
    Canvas::fontSrc_[fontVarCombo()->currentData().toInt()].setPointSize(size);

    emit cacheNeedUpdate(CanvasMask::CacheFull);
    emit settingsUpdated(-1, 0);
}


void ConfigDialog::onColorSelect(const QColor& color)
{
    colorShowLabel()->setStyleSheet(QString("background : %1").arg(color.name()));

    auto data (colorVarCombo()->currentData().toList());
    auto key (data[1].toInt());

    switch (data[0].toInt()) {
    case 0 :
        Canvas::colorSrc_.textFg.setColor(color);
        Canvas::colorSrc_.spec[4].setColor(color);
        Canvas::colorSrc_.force[0] = color;
        Canvas::penSrc_.textFg.setColor(color);

        emit cacheNeedUpdate(CanvasMask::PenCache);
        emit settingsUpdated(-1, 0);
        break;
    case 1 :
        (&Canvas::colorSrc_.textFg + key)->setColor(color);
        key == 1 ? emit canvasBkgUpdated() : emit settingsUpdated(-1, 0);
        break;
    case 2 :
        Canvas::colorSrc_.signBkg[key].setColor(color);
        emit settingsUpdated(-1, CanvasMask::Circle);
        break;
    case 3 :
        Canvas::colorSrc_.spec[key].setColor(color);
        emit settingsUpdated(-1, CanvasMask::ColorSpecUpd);
        break;
    case 4 :
        *(&Canvas::colorSrc_.cuspLineFg + key) = color;
        emit settingsUpdated(-1, CanvasMask::Circle);
        break;
    case 5 :
        Canvas::colorSrc_.force[key] = color;
        emit settingsUpdated(-1, CanvasMask::ColorForceUpd);
        break;
    case 6 :
        (&Canvas::penSrc_.textFg + key)->setColor(color);
        emit cacheNeedUpdate(CanvasMask::PenCache);
        emit settingsUpdated(-1, CanvasMask::PenUpd);
        break;
    case 7 :
        AspPageDialog::brushED_[key].setColor(color);
        emit aspPageNeedSync();
        break;
    case 8 :
        LineDelegate::markPen_.setColor(color);
        emit aspPageNeedSync();
        break;
    case 9 :
        AspTableDialog::brush_[key].setColor(color);
        emit aspTableNeedSync();
        break;
    }

    isSaveB_ = true;
}


void ConfigDialog::onSizeChanged(int value)
{
    if (!mutex_)
        return;

    auto data (sizeVarCombo()->currentData().toList());

    switch (data[0].toInt()) {
    case 0 :
        Canvas::headerSizeSrc_[data[1].toString()] = value;
        emit cacheNeedUpdate(CanvasMask::Header);
        emit settingsUpdated(-1, CanvasMask::Header);
        break;
    case 1 :
        Canvas::recepSizeSrc_[data[1].toString()] = value;
        emit cacheNeedUpdate(CanvasMask::RecepTree);
        emit settingsUpdated(-1, CanvasMask::RecepTree);
        break;
    case 2 :
        Canvas::esseStatSizeSrc_[data[1].toString()] = value;
        emit cacheNeedUpdate(CanvasMask::EsseStat);
        emit settingsUpdated(-1, CanvasMask::EsseStat);
        break;
    case 3 :
    {
        auto key (data[1].toString());
        Canvas::circleSizeSrc_[key] = value;
        emit cacheNeedUpdate(CanvasMask::Circle);
        emit settingsUpdated(key == "space" ? KernelMask::Coupling : -1, CanvasMask::Circle);
    }
        break;
    case 4 :
        Canvas::crdTableSizeSrc_[data[1].toString()] = value;
        emit cacheNeedUpdate(CanvasMask::CrdTableShort);
        emit settingsUpdated(-1, CanvasMask::CrdTableLong);
        break;
    case 5 :
        Canvas::lineBarSizeSrc_[data[1].toString()] = value;
        emit cacheNeedUpdate(CanvasMask::LineBar);
        emit settingsUpdated(-1, CanvasMask::LineBar);
        break;
    case 6 :
        Canvas::aspCfgSizeSrc_[data[1].toString()] = value;
        emit cacheNeedUpdate(CanvasMask::AspCfg);
        emit settingsUpdated(-1, CanvasMask::AspCfg);
        break;
    case 7 :
        Canvas::aspStatSizeSrc_[data[1].toString()] = value;
        emit cacheNeedUpdate(CanvasMask::AspStat);
        emit settingsUpdated(-1, CanvasMask::AspStat);
        break;
    case 8 :
        Canvas::coreStatSizeSrc_[data[1].toString()] = value;
        emit cacheNeedUpdate(CanvasMask::CoreStat);
        emit settingsUpdated(-1, CanvasMask::CoreStat);
        break;
    case 9 :
        switch (data[1].toInt()) {
        case 0 :
            SharedGui::margins_.setLeft(value);
            break;
        case 1 :
            SharedGui::margins_.setTop(value);
            break;
        case 2 :
            SharedGui::margins_.setRight(value);
            break;
        case 3 :
            SharedGui::margins_.setBottom(value);
            break;
        }
        break;
    }

    isSaveB_ = true;
}


void ConfigDialog::onClickAtlas()
{
    auto beg (contList_[2]->geometry().bottomRight());
    auto end (screen()->geometry().bottomRight());

    beg.ry() += topHeight_;
    end.rx() -= margins_.right() + 1;
    end.ry() -= margins_.bottom() + 1;

    QRect geo (mapToGlobal(beg), PersonDialog::atlasSize_);

    if (geo.right() > end.x())
        geo.moveLeft(end.x() - geo.width());
    if (geo.bottom() > end.y())
        geo.moveTop(end.y() - geo.height());

    PersonDialog::atlasRoot_ = this;
    atlasDialog_->setGeometry(geo);
    atlasDialog_->exec();
}


void ConfigDialog::onClickSaveCity()
{
    auto utc (static_cast<QLineEdit*>(contList_[7])->text());

    atlasDialog_->onUpdateBase({
        {"title", static_cast<QLineEdit*>(contList_[1])->text()},
        {"utc", (utc == "+00:00" || utc == "-00:00") ? "UTC" : utc},
        {"lat", static_cast<QLineEdit*>(contList_[5])->text()},
        {"lon", static_cast<QLineEdit*>(contList_[6])->text()},
        {"mode", false}
    });
}


void ConfigDialog::onSelectCity(const QVariantMap& city)
{
    if (PersonDialog::atlasRoot_ == this)
    {
        static_cast<QLineEdit*>(contList_[1])->setText(city["title"].toString());
        static_cast<QLineEdit*>(contList_[7])->setText(city["utc"].toString());
        static_cast<QLineEdit*>(contList_[5])->setText(city["lat"].toString());
        static_cast<QLineEdit*>(contList_[6])->setText(city["lon"].toString());
    }
}


void ConfigDialog::onMapViewAccChange(int value)
{
    isSaveA_ = true;
    AspTable::mvAcc_ = value;

    emit settingsUpdated(KernelMask::MapViewAcc, CanvasMask::LineBar);
}


void ConfigDialog::onAlmuHardChange(bool value)
{
    isSaveA_ = true;
    PrimeTest::almuHard_ = value;

    emit settingsUpdated(KernelMask::PrimeTest, CanvasMask::AlmutenHard);
}


auto ConfigDialog::showEvent(QShowEvent* event) -> void
{
    if (topHeight_ < 0)
        topHeight_ = frameGeometry().height() - geometry().height();

    isSaveA_ = false;
    isSaveB_ = false;

    copyReset();
    onClickList(ui->sideList->currentItem());
    setDialogTitle();
    QDialog::showEvent(event);
}


auto ConfigDialog::setDialogTitle() -> void
{
    setWindowTitle(tr("Settings") + " - " + profileName_);
}


auto ConfigDialog::copyReset() -> void
{
    nameR_ = Person::name_;
    locationR_ = Person::location_;
    utcR_ = Person::utc_;
    latR_ = Person::lat_;
    lonR_ = Person::lon_;
    hsysR_ = Person::hsys_;
    mvAccR_ = AspTable::mvAcc_;
    almuHardR_ = PrimeTest::almuHard_;

    fontSrcR_ = Canvas::fontSrc_;
    colorSrcR_ = Canvas::colorSrc_;
    penSrcR_ = Canvas::penSrc_;
    apgBrushR_ = AspPageDialog::brushED_;
    apgMarkStyleR_ = LineDelegate::markPen_;
    atbBrushR_ = AspTableDialog::brush_;
    headerSizeSrcR_ = Canvas::headerSizeSrc_;
    recepSizeSrcR_ = Canvas::recepSizeSrc_;
    esseStatSizeSrcR_ = Canvas::esseStatSizeSrc_;
    circleSizeSrcR_ = Canvas::circleSizeSrc_;
    crdTableSizeSrcR_ = Canvas::crdTableSizeSrc_;
    lineBarSizeSrcR_ = Canvas::lineBarSizeSrc_;
    aspCfgSizeSrcR_ = Canvas::aspCfgSizeSrc_;
    aspStatSizeSrcR_ = Canvas::aspStatSizeSrc_;
    coreStatSizeSrcR_ = Canvas::coreStatSizeSrc_;
    mainMargR_ = SharedGui::margins_;
}


auto ConfigDialog::onReject(bool quit) -> void
{
    if (isSaveA_)
    {
        Person::name_ = nameR_;
        Person::location_ = locationR_;
        Person::utc_ = utcR_;
        Person::lat_ = latR_;
        Person::lon_ = lonR_;
        Person::hsys_ = hsysR_;
        AspTable::mvAcc_ = mvAccR_;
        PrimeTest::almuHard_ = almuHardR_;
    }

    if (isSaveB_)
    {
        Canvas::fontSrc_ = fontSrcR_;
        Canvas::colorSrc_ = colorSrcR_;
        Canvas::penSrc_ = penSrcR_;
        AspPageDialog::brushED_ = apgBrushR_;
        LineDelegate::markPen_ = apgMarkStyleR_;
        AspTableDialog::brush_ = atbBrushR_;
        Canvas::headerSizeSrc_ = headerSizeSrcR_;
        Canvas::recepSizeSrc_ = recepSizeSrcR_;
        Canvas::esseStatSizeSrc_ = esseStatSizeSrcR_;
        Canvas::circleSizeSrc_ = circleSizeSrcR_;
        Canvas::crdTableSizeSrc_ = crdTableSizeSrcR_;
        Canvas::lineBarSizeSrc_ = lineBarSizeSrcR_;
        Canvas::aspCfgSizeSrc_ = aspCfgSizeSrcR_;
        Canvas::aspStatSizeSrc_ = aspStatSizeSrcR_;
        Canvas::coreStatSizeSrc_ = coreStatSizeSrcR_;
        SharedGui::margins_ = mainMargR_;
    }

    if (!quit && (isSaveA_ || isSaveB_))
    {
        emit cacheNeedUpdate(CanvasMask::CacheFull);
        emit settingsUpdated(0, 0);
        emit canvasBkgUpdated();
        emit aspPageNeedSync();
        emit aspTableNeedSync();
    }

    if (quit)
        dumpBase();
}


auto ConfigDialog::personDefChanged(const QWidget* wdg) -> void
{
    isSaveA_ = true;

    if (wdg == contList_[0])
        Person::name_ = static_cast<QLineEdit*>(contList_[0])->text();
    else if (wdg == contList_[1])
        Person::location_ = static_cast<QLineEdit*>(contList_[1])->text();
    else if (wdg == contList_[4])
        Person::hsys_ = static_cast<QComboBox*>(contList_[4])->currentData().toInt();
    else if (wdg == contList_[5])
        Person::lat_ = toIntCrd(static_cast<QLineEdit*>(contList_[5])->text());
    else if (wdg == contList_[6])
        Person::lon_ = toIntCrd(static_cast<QLineEdit*>(contList_[6])->text());
    else if (wdg == contList_[7])
        Person::utc_ = toIntUtc(static_cast<QLineEdit*>(contList_[7])->text());
}


auto ConfigDialog::clearSide() -> void
{
    for (auto i : contList_)
        delete i;

    for (auto i : boneList_)
        delete i;

    contList_.clear();
    boneList_.clear();
}


auto ConfigDialog::showGeneralSide() -> void
{
    clearSide();

    auto cityHL (new QHBoxLayout);
    auto dataHL (new QHBoxLayout);

    auto nameEdit (new QLineEdit(Person::name_, this));
    auto locEdit (new QLineEdit(Person::location_, this));
    auto atlasButton (new QPushButton(QIcon(":/24x24/earth.png"), "", this));
    auto saveButton (new QPushButton(QIcon(":/24x24/save.png"), "", this));
    auto hsysBox (new QComboBox(this));
    auto latEdit (new QLineEdit(toStrCrd(Person::lat_, true), this));
    auto lonEdit (new QLineEdit(toStrCrd(Person::lon_, false), this));
    auto utcEdit (new QLineEdit(toStrUtc(Person::utc_, true), this));

    auto mvAccSpin (new QSpinBox(this));
    auto almuCheck (new QCheckBox(tr("Almuten by middle point") + " ", this));

    auto localeBox (new QComboBox(this));
    auto infoLabel (new QLabel(tr("Application restart required to apply language."), this));

    atlasButton->setIconSize({18, 18});
    atlasButton->setFocusPolicy(Qt::NoFocus);
    atlasButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    atlasButton->setToolTip(tr("Atlas..."));

    saveButton->setIconSize({18, 18});
    saveButton->setFocusPolicy(Qt::NoFocus);
    saveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    saveButton->setToolTip(tr("Save city"));

    for (auto i : PersonDialog::hsysOrder)
        hsysBox->addItem(PersonDialog::hsysTitle_.at(i), i);

    hsysBox->setCurrentIndex(hsysBox->findData(Person::hsys_));

    latEdit->setMaximumWidth(140);
    latEdit->setAlignment(Qt::AlignCenter);
    latEdit->setInputMask("99º 99' 99\" A");
    latEdit->setValidator(new QRegularExpressionValidator(CityDialog::latReg, this));

    lonEdit->setMaximumWidth(140);
    lonEdit->setAlignment(Qt::AlignCenter);
    lonEdit->setInputMask("999º 99' 99\" A");
    lonEdit->setValidator(new QRegularExpressionValidator(CityDialog::lonReg, this));

    utcEdit->setMaximumWidth(66);
    utcEdit->setAlignment(Qt::AlignCenter);
    utcEdit->setInputMask("#99:99");
    utcEdit->setValidator(new QRegularExpressionValidator(CityDialog::utcReg, this));

    mvAccSpin->setMinimum(10);
    mvAccSpin->setMaximum(30);
    mvAccSpin->setPrefix(tr("Map view compute accuracy") + "  :  ");
    mvAccSpin->setValue(AspTable::mvAcc_);
    mvAccSpin->setSuffix("º ");

    almuCheck->setChecked(PrimeTest::almuHard_);
    almuCheck->setLayoutDirection(Qt::RightToLeft);

    localeBox->addItem(QLocale::languageToString(QLocale::English), "en_US");

    QDir locDir ("locale/");
    for (const auto& i : locDir.entryInfoList(QDir::Files))
        if (i.completeSuffix() == "qm")
        {
            auto name (i.baseName().split("Napatahti_"));
            if (name.size() == 2)
                localeBox->addItem(QLocale::languageToString(QLocale(name[1]).language()), name[1]);
        }

    localeBox->setCurrentIndex(localeBox->findData(locale_.name()));
    infoLabel->setVisible(locale_ != locale());

    cityHL->addWidget(locEdit);
    cityHL->addWidget(atlasButton);
    cityHL->addWidget(saveButton);

    dataHL->addWidget(hsysBox);
    dataHL->addWidget(latEdit);
    dataHL->addWidget(lonEdit);
    dataHL->addWidget(utcEdit);

    ui->grid->addWidget(nameEdit, 0, 1, 1, 4);
    ui->grid->addLayout(cityHL, 1, 1, 1, 4);
    ui->grid->addLayout(dataHL, 2, 1, 1, 4);
    ui->grid->addWidget(mvAccSpin, 4, 1, 1, 2);
    ui->grid->addWidget(almuCheck, 4, 3, 1, 2);
    ui->grid->addWidget(localeBox, 6, 1, 1, 2);
    ui->grid->addWidget(infoLabel, 7, 1, 1, 4);

    contList_ = {
        nameEdit, locEdit, atlasButton, saveButton, hsysBox,
        latEdit, lonEdit, utcEdit, mvAccSpin, almuCheck, localeBox, infoLabel};
    boneList_ = {cityHL, dataHL};

    connect(nameEdit, &QLineEdit::textChanged, this, [this, nameEdit](){ personDefChanged(nameEdit); });
    connect(locEdit, &QLineEdit::textChanged, this, [this, locEdit](){ personDefChanged(locEdit); });
    connect(hsysBox, &QComboBox::currentIndexChanged, this, [this, hsysBox](){ personDefChanged(hsysBox); });
    connect(latEdit, &QLineEdit::textChanged, this, [this, latEdit](){ personDefChanged(latEdit); });
    connect(lonEdit, &QLineEdit::textChanged, this, [this, lonEdit](){ personDefChanged(lonEdit); });
    connect(utcEdit, &QLineEdit::textChanged, this, [this, utcEdit](){ personDefChanged(utcEdit); });

    connect(atlasButton, &QPushButton::clicked, this, &ConfigDialog::onClickAtlas);
    connect(saveButton, &QPushButton::clicked, this, &ConfigDialog::onClickSaveCity);
    connect(atlasDialog_, &AtlasDialog::citySelected, this, &ConfigDialog::onSelectCity);

    connect(mvAccSpin, &QSpinBox::valueChanged, this, &ConfigDialog::onMapViewAccChange);
    connect(almuCheck, &QCheckBox::clicked, this, &ConfigDialog::onAlmuHardChange);

    connect(localeBox, &QComboBox::currentIndexChanged, this, [this, localeBox, infoLabel](){
        SharedGui::setAppLocale(localeBox->currentData().toString());
        infoLabel->setVisible(locale_ != locale());
    });
}


auto ConfigDialog::showGuiSide() -> void
{
    clearSide();

    auto fontHL (new QHBoxLayout);
    auto colHL (new QHBoxLayout);
    auto sizeHL (new QHBoxLayout);

    auto fontVarBox (new QComboBox(this));
    auto fontSelBox (new QFontComboBox(this));
    auto fontSpin (new QSpinBox(this));

    auto colVarBox (new QComboBox(this));
    auto colViewLabel (new QLabel(this));
    auto colSelButton (new QPushButton(QIcon(":/24x24/diagram.png"), "", this));

    auto sizeVarBox (new QComboBox(this));
    auto sizeSpin (new QSpinBox(this));

    fontVarBox->setMaximumWidth(248);
    fontVarBox->setMinimumWidth(248);
    fontVarBox->addItem(tr("Base font (Text)"), 2);
    fontVarBox->addItem(tr("Base font (Symbol)"), 3);
    fontVarBox->addItem(tr("Circle - zodiac sign"), 6);
    fontVarBox->addItem(tr("Circle - cuspid sign"), 7);
    fontVarBox->addItem(tr("Circle - cuspid degree sign"), 8);
    fontVarBox->addItem(tr("Circle - planet sign"), 9);
    fontVarBox->addItem(tr("Circle - planet degree sign"), 10);
    fontVarBox->addItem(tr("Circle - planet speed sign"), 11);
    fontVarBox->addItem(tr("Circle - aspect sign (Text)"), 0);
    fontVarBox->addItem(tr("Circle - aspect sign (Symbol)"), 1);
    fontVarBox->addItem(tr("Coordinates - fixed stars"), 12);
    fontVarBox->addItem(tr("Planet statistic - markers"), 13);
    fontVarBox->addItem(tr("Aspect statistic - square (Symbol)"), 4);
    fontVarBox->addItem(tr("Aspect statistic - square (Text)"), 5);
    fontVarBox->setCurrentIndex(0);

    fontSelBox->setMaximumWidth(200);
    fontSelBox->setMinimumWidth(200);

    fontSpin->setMinimum(1);
    fontSpin->setMaximum(20);
    fontSpin->setMaximumWidth(45);
    fontSpin->setMinimumWidth(45);
    fontSpin->setAlignment(Qt::AlignCenter);

    colVarBox->setMaximumWidth(402);
    colVarBox->setMinimumWidth(402);
    colVarBox->addItem(tr("Base text fg"), QVariantList{0, 0});
    colVarBox->addItem(tr("Base text bkg"), QVariantList{1, 1});
    colVarBox->addItem(tr("Circle - fire bkg"), QVariantList{2, 0});
    colVarBox->addItem(tr("Circle - land bkg"), QVariantList{2, 1});
    colVarBox->addItem(tr("Circle - air bkg"), QVariantList{2, 2});
    colVarBox->addItem(tr("Circle - water bkg"), QVariantList{2, 3});
    colVarBox->addItem(tr("Circle - zodiac sign fg"), QVariantList{1, 4});
    colVarBox->addItem(tr("Circle - cuspid sign fg"), QVariantList{1, 5});
    colVarBox->addItem(tr("Circle - planet sign fg"), QVariantList{1, 6});
    colVarBox->addItem(tr("Circle - base bkg"), QVariantList{1, 2});
    colVarBox->addItem(tr("Circle - aspect field bkg"), QVariantList{1, 3});
    colVarBox->addItem(tr("Circle - border fg"), QVariantList{6, 1});
    colVarBox->addItem(tr("Circle - planet line fg"), QVariantList{6, 2});
    colVarBox->addItem(tr("Circle - cuspid line fg"), QVariantList{4, 0});
    colVarBox->addItem(tr("Circle - aspect config hightlight fg"), QVariantList{4, 1});
    colVarBox->addItem(tr("Planet sign special - ruler"), QVariantList{3, 0});
    colVarBox->addItem(tr("Planet sign special - detriment"), QVariantList{3, 1});
    colVarBox->addItem(tr("Planet sign special - exaltation"), QVariantList{3, 2});
    colVarBox->addItem(tr("Planet sign special - fall"), QVariantList{3, 3});
    colVarBox->addItem(tr("Coordinates - king degree fg"), QVariantList{1, 7});
    colVarBox->addItem(tr("Coordinates - destruct degree fg"), QVariantList{1, 8});
    colVarBox->addItem(tr("Planet statistic - markers"), QVariantList{6, 3});
    colVarBox->addItem(tr("Planet statistic - positive"), QVariantList{6, 4});
    colVarBox->addItem(tr("Planet statistic - negative"), QVariantList{6, 5});
    colVarBox->addItem(tr("Planet statistic - power"), QVariantList{6, 6});
    colVarBox->addItem(tr("Planet statistic - creative"), QVariantList{6, 7});
    colVarBox->addItem(tr("Force fg - dominant"), QVariantList{5, 1});
    colVarBox->addItem(tr("Force fg - weak"), QVariantList{5, 2});
    colVarBox->addItem(tr("Aspect page - enable marker fg"), QVariantList{7, 1});
    colVarBox->addItem(tr("Aspect page - disable marker fg"), QVariantList{7, 0});
    colVarBox->addItem(tr("Aspect page - style cell selection bkg"), QVariantList{8, 0});
    colVarBox->addItem(tr("Aspect table - no aspect fg"), QVariantList{9, 0});
    colVarBox->addItem(tr("Aspect table - neutral cell bkg"), QVariantList{9, 1});
    colVarBox->addItem(tr("Aspect table - patch cell bkg"), QVariantList{9, 2});
    colVarBox->setCurrentIndex(0);

    colViewLabel->setMaximumWidth(45);
    colViewLabel->setMinimumWidth(45);
    colViewLabel->setFrameStyle(QFrame::Box);
    colViewLabel->setFrameShadow(QFrame::Sunken);

    colSelButton->setIconSize({18, 18});
    colSelButton->setMaximumWidth(45);
    colSelButton->setMinimumWidth(45);
    colSelButton->setFocusPolicy(Qt::NoFocus);
    colSelButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    colSelButton->setToolTip(tr("Color..."));

    sizeVarBox->setMaximumWidth(402);
    sizeVarBox->setMinimumWidth(402);
    sizeVarBox->addItem(tr("Header - left margin"), QVariantList{0, "padX", 0, 2000});
    sizeVarBox->addItem(tr("Header - top margin"), QVariantList{0, "padY", 0, 2000});
    sizeVarBox->addItem(tr("Reception tree - left margin"), QVariantList{1, "padX", 0, 2000});
    sizeVarBox->addItem(tr("Reception tree - top margin (from middle)"), QVariantList{1, "padY", -1000, 1000});
    sizeVarBox->addItem(tr("Reception tree - max length"), QVariantList{1, "length", 1, 15});
    sizeVarBox->addItem(tr("Essentials - left margin"), QVariantList{2, "padX", 0, 2000});
    sizeVarBox->addItem(tr("Essentials - bottom margin"), QVariantList{2, "padY", 0, 2000});
    sizeVarBox->addItem(tr("Essentials - planet Y correction"), QVariantList{2, "crutch", -5, 5});
    sizeVarBox->addItem(tr("Circle - vertical margin"), QVariantList{3, "margin", 0, 200});
    sizeVarBox->addItem(tr("Circle - horizontal shift from middle"), QVariantList{3, "offset", -1000, 1000});
    sizeVarBox->addItem(tr("Circle - out sign margin"), QVariantList{3, "outSignAdd", 0, 300});
    sizeVarBox->addItem(tr("Circle - sign width"), QVariantList{3, "insSignAdd", 0, 300});
    sizeVarBox->addItem(tr("Circle - inside sign margin"), QVariantList{3, "insRectAdd", 0, 300});
    sizeVarBox->addItem(tr("Circle - out cuspid delta radius"), QVariantList{3, "outCuspAdd", 0, 300});
    sizeVarBox->addItem(tr("Circle - cuspid arrow length"), QVariantList{3, "tipLen", 10, 50});
    sizeVarBox->addItem(tr("Circle - cuspid arrow angle"), QVariantList{3, "tipAng", 1, 15});
    sizeVarBox->addItem(tr("Circle - planet delta radius"), QVariantList{3, "insPlanAdd", 0, 300});
    sizeVarBox->addItem(tr("Circle - minimum space between planets"), QVariantList{3, "space", 1, 10});
    sizeVarBox->addItem(tr("Coordinates - right margin"), QVariantList{4, "padX", -500, 2000});
    sizeVarBox->addItem(tr("Coordinates - top margin"), QVariantList{4, "padY", -100, 2000});
    sizeVarBox->addItem(tr("Planet statistic - right margin"), QVariantList{5, "padX", 0, 2000});
    sizeVarBox->addItem(tr("Planet statistic - prime scale"), QVariantList{5, "primeScale", 1, 150});
    sizeVarBox->addItem(tr("Planet statistic - asha scale"), QVariantList{5, "ashaScale", 1, 150});
    sizeVarBox->addItem(tr("Planet statistic - cosmic scale"), QVariantList{5, "cosmicScale", 1, 150});
    sizeVarBox->addItem(tr("Aspect configs - top margin"), QVariantList{6, "topMargin", -20, 20});
    sizeVarBox->addItem(tr("Aspect configs - horizontal shift"), QVariantList{6, "shiftLen", -30, 30});
    sizeVarBox->addItem(tr("Aspect configs - title Y correction"), QVariantList{6, "titleDy", -5, 5});
    sizeVarBox->addItem(tr("Aspect statistic - right margin"), QVariantList{7, "padX", 0, 2000});
    sizeVarBox->addItem(tr("Aspect statistic - bottom margin"), QVariantList{7, "padY", 0, 2000});
    sizeVarBox->addItem(tr("Core statistic - right margin"), QVariantList{8, "padX", 0, 2000});
    sizeVarBox->addItem(tr("Core statistic - bottom margin"), QVariantList{8, "padY", 0, 2000});
    sizeVarBox->addItem(tr("Main window - left margin (need reload)"), QVariantList{9, 0, 0, 300});
    sizeVarBox->addItem(tr("Main window - top margin (need reload)"), QVariantList{9, 1, 0, 300});
    sizeVarBox->addItem(tr("Main window - right margin (need reload)"), QVariantList{9, 2, 0, 300});
    sizeVarBox->addItem(tr("Main window - bottom margin (need reload)"), QVariantList{9, 3, 0, 300});
    sizeVarBox->setCurrentIndex(0);

    sizeSpin->setMaximumWidth(96);
    sizeSpin->setMinimumWidth(96);
    sizeSpin->setAlignment(Qt::AlignCenter);

    fontHL->addWidget(fontVarBox);
    fontHL->addWidget(fontSelBox);
    fontHL->addWidget(fontSpin);

    colHL->addWidget(colVarBox);
    colHL->addWidget(colViewLabel);
    colHL->addWidget(colSelButton);

    sizeHL->addWidget(sizeVarBox);
    sizeHL->addWidget(sizeSpin);

    ui->grid->addLayout(fontHL, 0, 1, 1, 4);
    ui->grid->addLayout(colHL, 2, 1, 1, 4);
    ui->grid->addLayout(sizeHL, 4, 1, 1, 4);

    contList_ = {
        fontVarBox, fontSelBox, fontSpin, colVarBox,
        colViewLabel, colSelButton, sizeVarBox, sizeSpin};
    boneList_ = {fontHL, colHL, sizeHL};

    onSelectFontVar(0);
    onSelectColVar(0);
    onSelectSizeVar(0);

    connect(fontVarBox, &QComboBox::currentIndexChanged, this, &ConfigDialog::onSelectFontVar);
    connect(colVarBox, &QComboBox::currentIndexChanged, this, &ConfigDialog::onSelectColVar);
    connect(colSelButton, &QPushButton::clicked, this, [this](){ colorDialog_->exec(); });
    connect(sizeVarBox, &QComboBox::currentIndexChanged, this, &ConfigDialog::onSelectSizeVar);
    connect(fontSelBox, &QFontComboBox::currentFontChanged, this, &ConfigDialog::onFontChanged);
    connect(fontSpin, &QSpinBox::valueChanged, this, &ConfigDialog::onFontSizeChanged);
    connect(sizeSpin, &QSpinBox::valueChanged, this, &ConfigDialog::onSizeChanged);
}


auto ConfigDialog::loadBase() -> bool
{
    if (!QDir(path).exists())
        QDir().mkdir(path);

    QFile base (path + "config.ini");

    if (base.exists() && base.open(QIODevice::ReadOnly))
    {
        QJsonParseError parseError;
        QJsonDocument   json (QJsonDocument::fromJson(base.readAll(), &parseError));

        base.close();

        if (json.isNull())
        {
            errorLog(base.fileName() + " parsing error: " + parseError.errorString());
            return false;
        }

        auto part (0);
        for (const auto& i : json.array())
        {
            const auto& dec (i.toArray());

            switch (part) {
            case 0 :
                profileName_ = dec[0].toString();
                locale_ = QLocale(dec[1].toString());
                DataBaseDialog::setTableName(dec[2].toString());
                AspPage::setTitle(dec[3].toString());
                break;
            case 1 :
            {
                auto trig (MainWindow::trigger_.begin());
                for (const auto& j : dec)
                {
                    *trig = j.toBool();
                    ++trig;
                }
            }
                break;
            }

            ++part;
        }
    }
    else
    {
        errorLog(base.fileName() + " not found");
        return false;
    }

    return true;
}


auto ConfigDialog::dumpBase() -> void
{
    QJsonArray data {
        profileName_, SharedGui::locale_.name(),
        DataBaseDialog::getTableName(), AspPage::getTitle()
    };

    QJsonArray trig;
    for (auto i : MainWindow::trigger_)
        trig.push_back(i);

    QFile cfg (path + "config.ini");
    if (cfg.open(QIODevice::WriteOnly))
    {
        cfg.write(QByteArray(QJsonDocument({data, trig}).toJson()));
        cfg.close();
    }
    else
        errorLog("Can not save file " + cfg.fileName());
}


auto ConfigDialog::loadProFile() -> void
{
    if (profileNameR_.isEmpty())
        profileNameR_ = tr("Builtin");

    if (profileName_ == profileNameR_)
    {
        profileName_ = profileNameR_;
        restore(RestoreMode::SideAB);
        return;
    }

    QFile cfg (path + profileName_ + ".json");
    if (cfg.exists() && cfg.open(QIODevice::ReadOnly))
    {
        QJsonParseError parseError;
        QJsonDocument   json (QJsonDocument::fromJson(cfg.readAll(), &parseError));

        cfg.close();
        if (json.isNull())
        {
            errorLog(cfg.fileName() + " parsing error: " + parseError.errorString());
            profileName_ = profileNameR_;
            restore(RestoreMode::SideAB);
            return;
        }

        auto part (0);
        for (const auto& i : json.array())
        {
            const auto& dec (i.toArray());

            switch (part) {
            case 0 :
                Person::name_        = dec[0].toString();
                Person::location_    = dec[1].toString();
                Person::utc_         = dec[2].toInt();
                Person::lat_         = dec[3].toInt();
                Person::lon_         = dec[4].toInt();
                Person::hsys_        = dec[5].toInt();
                AspTable::mvAcc_     = dec[6].toInt();
                PrimeTest::almuHard_ = dec[7].toBool();
                break;
            case 1 :
            {
                auto ind (0);
                for (auto& j : Canvas::fontSrc_)
                {
                    j.setFamily(dec[ind].toString());
                    j.setPointSize(dec[ind+1].toInt());
                    ind += 2;
                }
            }
                break;
            case 2 :
                for (auto j (0); j < dec.size(); ++j)
                {
                    QColor color (dec[j].toString());

                    if (j <= 8)
                        (&Canvas::colorSrc_.textFg + j)->setColor(color);
                    else if (j >= 9 && j <= 12)
                        Canvas::colorSrc_.signBkg[j-9].setColor(color);
                    else if (j >= 13 && j <= 17)
                        Canvas::colorSrc_.spec[j-13].setColor(color);
                    else if (j == 18)
                        Canvas::colorSrc_.cuspLineFg = color;
                    else if (j == 19)
                        Canvas::colorSrc_.aspCfgMarkFg = color;
                    else if (j >= 20 && j <= 22)
                        Canvas::colorSrc_.force[j-20] = color;
                    else if (j >= 23 && j <= 30)
                        (Canvas::penSrc_.begin() + (j - 23))->setColor(color);
                    else if (j == 31 || j == 32)
                        AspPageDialog::brushED_[j-31].setColor(color);
                    else if (j == 33)
                        LineDelegate::markPen_.setColor(color);
                    else if (j >= 34 && j <= 36)
                        AspTableDialog::brush_[j-34].setColor(color);
                }
                break;
            case 3 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::headerSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 4 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::recepSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 5 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::esseStatSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 6 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::circleSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 7 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::crdTableSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 8 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::lineBarSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 9 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::aspCfgSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 10 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::aspStatSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 11 :
                for (auto j (0); j < dec.size(); j += 2)
                    Canvas::coreStatSizeSrc_[dec[j].toString()] = dec[j+1].toInt();
                break;
            case 12 :
                SharedGui::margins_ = {dec[0].toInt(), dec[1].toInt(), dec[2].toInt(), dec[3].toInt()};
                break;
            }

            ++part;
        }
    }
    else
    {
        profileName_ = profileNameR_;
        restore(RestoreMode::SideAB);
        return;
    }
}


auto ConfigDialog::dumpProFile(const QString& name) -> void
{
    QJsonArray sideA {
        Person::name_, Person::location_, Person::utc_,
        Person::lat_, Person::lon_, Person::hsys_,
        AspTable::mvAcc_, PrimeTest::almuHard_
    };

    QJsonArray fontArr;
    QJsonArray colorArr;

    QJsonArray headerArr;
    QJsonArray recepArr;
    QJsonArray esseStatArr;
    QJsonArray circleArr;
    QJsonArray crdTableArr;
    QJsonArray lineBarArr;
    QJsonArray aspCfgArr;
    QJsonArray aspStatArr;
    QJsonArray coreStatArr;
    QJsonArray appMargArr;

    for (auto& i : Canvas::fontSrc_)
    {
        fontArr.push_back(i.family());
        fontArr.push_back(i.pointSize());
    }

    for (auto i (&Canvas::colorSrc_.textFg); i <= &Canvas::colorSrc_.destDegFg; ++i)
        colorArr.push_back(i->color().name());
    for (auto& i : Canvas::colorSrc_.signBkg)
        colorArr.push_back(i.color().name());
    for (auto& i : Canvas::colorSrc_.spec)
        colorArr.push_back(i.color().name());

    colorArr.push_back(Canvas::colorSrc_.cuspLineFg.name());
    colorArr.push_back(Canvas::colorSrc_.aspCfgMarkFg.name());

    for (auto& i : Canvas::colorSrc_.force)
        colorArr.push_back(i.name());
    for (auto& i : Canvas::penSrc_)
        colorArr.push_back(i.color().name());

    for (auto& i : AspPageDialog::brushED_)
        colorArr.push_back(i.color().name());

    colorArr.push_back(LineDelegate::markPen_.color().name());

    for (auto& i : AspTableDialog::brush_)
        colorArr.push_back(i.color().name());

    for (auto i (Canvas::headerSizeSrc_.begin()); i != Canvas::headerSizeSrc_.end(); ++i)
    {
        headerArr.push_back(i.key());
        headerArr.push_back(i.value());
    }
    for (auto i (Canvas::recepSizeSrc_.begin()); i != Canvas::recepSizeSrc_.end(); ++i)
    {
        recepArr.push_back(i.key());
        recepArr.push_back(i.value());
    }
    for (auto i (Canvas::esseStatSizeSrc_.begin()); i != Canvas::esseStatSizeSrc_.end(); ++i)
    {
        esseStatArr.push_back(i.key());
        esseStatArr.push_back(i.value());
    }
    for (auto i (Canvas::circleSizeSrc_.begin()); i != Canvas::circleSizeSrc_.end(); ++i)
    {
        circleArr.push_back(i.key());
        circleArr.push_back(i.value());
    }
    for (auto i (Canvas::crdTableSizeSrc_.begin()); i != Canvas::crdTableSizeSrc_.end(); ++i)
    {
        crdTableArr.push_back(i.key());
        crdTableArr.push_back(i.value());
    }
    for (auto i (Canvas::lineBarSizeSrc_.begin()); i != Canvas::lineBarSizeSrc_.end(); ++i)
    {
        lineBarArr.push_back(i.key());
        lineBarArr.push_back(i.value());
    }
    for (auto i (Canvas::aspCfgSizeSrc_.begin()); i != Canvas::aspCfgSizeSrc_.end(); ++i)
    {
        aspCfgArr.push_back(i.key());
        aspCfgArr.push_back(i.value());
    }
    for (auto i (Canvas::aspStatSizeSrc_.begin()); i != Canvas::aspStatSizeSrc_.end(); ++i)
    {
        aspStatArr.push_back(i.key());
        aspStatArr.push_back(i.value());
    }
    for (auto i (Canvas::coreStatSizeSrc_.begin()); i != Canvas::coreStatSizeSrc_.end(); ++i)
    {
        coreStatArr.push_back(i.key());
        coreStatArr.push_back(i.value());
    }

    appMargArr.push_back(SharedGui::margins_.left());
    appMargArr.push_back(SharedGui::margins_.top());
    appMargArr.push_back(SharedGui::margins_.right());
    appMargArr.push_back(SharedGui::margins_.bottom());

    QFile cfg (path + name + ".json");
    if (cfg.open(QIODevice::WriteOnly))
    {
        cfg.write(QByteArray(QJsonDocument({
                sideA, fontArr, colorArr, headerArr, recepArr,
                esseStatArr, circleArr, crdTableArr, lineBarArr,
                aspCfgArr, aspStatArr, coreStatArr, appMargArr
            }).toJson()));
        cfg.close();
    }
    else
        errorLog("can not create " + cfg.fileName());
}


auto ConfigDialog::profileNameList() -> QStringList
{
    QStringList cfgList {profileNameR_};

    for (const auto& i : QDir(path).entryInfoList(QDir::Files))
        if (i.completeSuffix() == "json")
            cfgList.push_back(i.baseName());

    return cfgList;
}


auto ConfigDialog::restore(int mode) -> void
{
    if ((mode & RestoreMode::SideA) != 0)
    {
        Person::name_ = tr("Today");
        Person::location_ = tr("Krasnodar, Krasnodar Territory, Russia");
        Person::utc_ = 10800;
        Person::lat_ = 162120;
        Person::lon_ = 140400;
        Person::hsys_ = 'P';
        AspTable::mvAcc_ = 25;
        PrimeTest::almuHard_ = true;
    }
    if ((mode & RestoreMode::SideB) != 0)
    {
        Canvas::fontSrc_ = {};
        Canvas::colorSrc_ = {};
        Canvas::penSrc_ = {};
        AspPageDialog::brushED_ = {QBrush("#d0d0d0"), QBrush("#0000ff")};
        LineDelegate::markPen_ = QPen("#d0d0d0");
        AspTableDialog::brush_ = {QBrush("#a0a0a0"), QBrush("#e2edfa"), QBrush("#e0e0e0")};
        Canvas::headerSizeSrc_ = {{"padX", 10}, {"padY", 10}};
        Canvas::recepSizeSrc_ = {{"padX", 10}, {"padY", 3}, {"length", 4}};
        Canvas::esseStatSizeSrc_ = {{"padX", 10}, {"padY", 10}, {"crutch", 2}};
        Canvas::circleSizeSrc_ = {
            {"margin", 60}, {"offset", -180}, {"outSignAdd", 40}, {"insSignAdd", 30},
            {"insRectAdd", 80}, {"outCuspAdd", 35}, {"insPlanAdd", 45}, {"space", 6},
            {"tipLen", 30}, {"tipAng", 7}};
        Canvas::crdTableSizeSrc_ = {{"padX", 10}, {"padY", 10}};
        Canvas::lineBarSizeSrc_ = {
            {"padX", 15}, {"primeScale", 100}, {"ashaScale", 27}, {"cosmicScale", 70}};
        Canvas::aspCfgSizeSrc_ = {{"topMargin", 2}, {"shiftLen", 0}, {"titleDy", 3}};
        Canvas::aspStatSizeSrc_ = {{"padX", 501}, {"padY", 270}};
        Canvas::coreStatSizeSrc_ = {{"padX", 50}, {"padY", 10}};
        SharedGui::margins_ = {0, 30, 0, 40};
    }
    if ((mode & RestoreMode::Base) != 0)
    {
        if (profileNameR_.isEmpty())
            profileNameR_ = tr("Builtin");

        profileName_ = profileNameR_;
        SharedGui::locale_ = QLocale::English;
        DataBaseDialog::setTableName("Builtin");
        AspPage::setTitle("");
        MainWindow::trigger_ = {};
    }
}


auto ConfigDialog::loadExt() -> void
{
    AspPage::setTitleR(tr("Builtin", "about aspect page"));

    AspTable::classic3Point_ = {
        {{0}, {AspGroup::Union, {}, 0, 1, tr("Core")}},
        {{0}, {AspGroup::Union, {}, 0, 0, tr("Stellium")}},
        {{90, 90, 180}, {AspGroup::Black, {}, 90, 0, tr("Tau-quadrat")}},
        {{135, 135, 90}, {AspGroup::Black, {}, 135, 1, tr("Pole-axe")}},
        {{45, 45, 90}, {AspGroup::Black, {}, 45, 1, tr("Dart")}},

        {{120, 120, 120}, {AspGroup::Red, {}, 120, 0, tr("Big trine")}},
        {{60, 60, 120}, {AspGroup::Red, {}, 60, 0, tr("Bisextile")}},
        {{150, 150, 60}, {AspGroup::Red, {}, 150, 1, tr("Forks")}},
        {{30, 30, 60}, {AspGroup::Red, {}, 30, 1, tr("Roof")}},

        {{72, 72, 144}, {AspGroup::Green, {}, 72, 0, tr("Prism")}},
        {{144, 144, 72}, {AspGroup::Green, {}, 144, 0, tr("Palm")}},
        {{108, 108, 144}, {AspGroup::Green, {}, 108, 1, tr("Ship")}},
        {{36, 36, 72}, {AspGroup::Green, {}, 36, 1, tr("Boat")}},

        {{40, 40, 80}, {AspGroup::Blue, {}, 40, 0, tr("Arcan")}},
        {{80, 80, 160}, {AspGroup::Blue, {}, 80, 0, tr("Tunnel")}},
        {{160, 160, 40}, {AspGroup::Blue, {}, 160, 1, tr("Wedge")}},
        {{20, 20, 40}, {AspGroup::Blue, {}, 20, 1, tr("Lock")}},
        {{100, 100, 160}, {AspGroup::Blue, {}, 100, NONE, tr("Leash")}},
        {{140, 140, 80}, {AspGroup::Blue, {}, 140, NONE, tr("Case")}}
    };
    AspTable::classic4Point_ = {
        {{90, 90, 90, 90, 180, 180}, {AspGroup::Black, {2}, 180, 0, tr("Cross")}},
        {{45, 45, 135, 135, 90, 180}, {AspGroup::Black, {3, 4}, NONE, 1, tr("Sword")}},
        {{45, 45, 45, 135, 90, 90}, {AspGroup::Black, {4}, NONE, 1, tr("Springboard")}},

        {{60, 60, 120, 120, 120, 180}, {AspGroup::BlackRed, {5, 6}, NONE, 0, tr("Staff")}},
        {{60, 60, 60, 180, 120, 120}, {AspGroup::BlackRed, {6}, NONE, 0, tr("Bell")}},
        {{60, 120, 60, 120, 180, 180}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Carriage")}},
        {{30, 30, 150, 150, 60, 180}, {AspGroup::BlackRed, {7, 8}, NONE, 1, tr("Flute")}},
        {{30, 30, 30, 90, 60, 60}, {AspGroup::BlackRed, {8}, NONE, 1, tr("Sphinx")}},

        {{36, 36, 36, 108, 72, 72}, {AspGroup::Green, {12}, NONE, 1, tr("Veil")}},
        {{36, 36, 144, 144, 72, 180}, {AspGroup::BlackGreen, {10, 12}, NONE, 0, tr("Liana")}},
        {{72, 72, 108, 108, 144, 180}, {AspGroup::BlackGreen, {9, 11}, NONE, 0, tr("Torch")}},
        {{72, 36, 72, 180, 108, 108}, {AspGroup::BlackGreen, {}, NONE, NONE, tr("Geyser")}},
        {{72, 108, 72, 108, 180, 180}, {AspGroup::BlackGreen, {}, NONE, NONE, tr("Envelope")}},

        {{40, 40, 140, 140, 80, 180}, {AspGroup::BlackBlue, {13, 18}, NONE, 0, tr("Shield")}},
        {{80, 80, 100, 100, 160, 180}, {AspGroup::BlackBlue, {14, 17}, NONE, 0, tr("Crossbow")}},
        {{20, 20, 160, 160, 40, 180}, {AspGroup::BlackBlue, {15, 16}, NONE, 1, tr("Tyn")}},

        {{40, 40, 40, 120, 80, 80}, {AspGroup::RedBlue, {13}, NONE, 0, tr("Arch")}},
        {{80, 80, 80, 120, 160, 160}, {AspGroup::RedBlue, {14}, NONE, 0, tr("Loom")}},
        {{20, 20, 20, 60, 40, 40}, {AspGroup::RedBlue, {16}, NONE, 1, tr("Nectar")}},
        {{40, 20, 40, 100, 60, 60}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Chest")}}
    };
    AspTable::ext3PointR_ = {
        {{ASP_S, ASP_S, ASP_BS}, {AspGroup::Violet, {}, NONE, NONE, tr("Laser")}},
        {{ASP_BS, ASP_BS, ASP_TS}, {AspGroup::Violet, {}, NONE, NONE, tr("Biseptile")}},
        {{ASP_TS, ASP_TS, ASP_S}, {AspGroup::Violet, {}, NONE, NONE, tr("Tree")}},
        {{ASP_PS, ASP_PS, ASP_S}, {AspGroup::Violet, {}, NONE, NONE, tr("Shuttle")}},
    };
    AspTable::ext4PointR_ = {
        {{45, 90, 45, 180, 135, 135}, {AspGroup::Black, {}, NONE, NONE, tr("Anvil")}},
        {{90, 45, 90, 135, 135, 135}, {AspGroup::Black, {}, NONE, NONE, tr("Iron")}},
        {{45, 135, 45, 135, 180, 180}, {AspGroup::Black, {}, NONE, NONE, tr("Stretching")}},

        {{30, 60, 30, 120, 90, 90}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Source")}},
        {{60, 30, 60, 150, 90, 90}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Airship")}},
        {{30, 90, 30, 150, 120, 120}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Sprout")}},
        {{30, 120, 30, 180, 150, 150}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Bridge")}},
        {{90, 30, 90, 150, 120, 120}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Parrot")}},
        {{120, 30, 120, 90, 150, 150}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Statue")}},
        {{30, 150, 30, 150, 180, 180}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Lifts")}},
        {{60, 90, 60, 150, 150, 150}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Boomerang")}},
        {{90, 60, 90, 120, 150, 150}, {AspGroup::BlackRed, {}, NONE, NONE, tr("Hawk")}},

        {{36, 72, 36, 144, 108, 108}, {AspGroup::Green, {}, NONE, NONE, tr("Trail")}},
        {{108, 36, 108, 108, 144, 144}, {AspGroup::Green, {}, NONE, NONE, tr("Diary")}},
        {{36, 144, 36, 144, 180, 180}, {AspGroup::BlackGreen, {}, NONE, NONE, tr("Pump")}},
        {{36, 108, 36, 180, 144, 144}, {AspGroup::BlackGreen, {}, NONE, NONE, tr("Mill")}},

        {{40, 140, 40, 140, 180, 180}, {AspGroup::BlackBlue, {}, NONE, NONE, tr("Spring")}},
        {{80, 100, 80, 100, 180, 180}, {AspGroup::BlackBlue, {}, NONE, NONE, tr("Fortress")}},
        {{40, 100, 40, 180, 140, 140}, {AspGroup::BlackBlue, {}, NONE, NONE, tr("Insulator")}},
        {{80, 20, 80, 180, 100, 100}, {AspGroup::BlackBlue, {}, NONE, NONE, tr("Hole")}},
        {{20, 160, 20, 160, 180, 180}, {AspGroup::BlackBlue, {}, NONE, NONE, tr("Gyroscope")}},
        {{20, 140, 20, 180, 160, 160}, {AspGroup::BlackBlue, {}, NONE, NONE, tr("Quagmire")}},

        {{80, 40, 80, 160, 120, 120}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Tower")}},
        {{40, 80, 40, 160, 120, 120}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Collar")}},
        {{120, 40, 120, 80, 160, 160}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Oasis")}},
        {{40, 120, 40, 160, 160, 160}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Stigma")}},
        {{60, 40, 60, 160, 100, 100}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Trap")}},
        {{40, 60, 40, 140, 100, 100}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Template")}},
        {{60, 80, 60, 160, 140, 140}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Vase")}},
        {{80, 60, 80, 140, 140, 140}, {AspGroup::RedBlue, {}, NONE, NONE, tr("Bonds")}}
    };

    PersonDialog::sexTitle_ = {
        {'E', tr("Event")}, {'M', tr("Male")}, {'F', tr("Female")}};
    PersonDialog::hsysTitle_ = {
        {'P', tr("Placidus")}, {'K', tr("Koch")}, {'D', tr("Equal MC")},
        {'A', tr("Equal AC")}, {'S', tr("Sripati")}, {'O', tr("Porphyrius")},
        {'C', tr("Campanus")}, {'B', tr("Alcabitus")}
    };
    PersonDialog::atlasSize_ = {840, 430};
    PersonDialog::leftMgn_ = 80;
    PersonDialog::topMgn_ = 160;

    AtlasDialog::newCity_ = {
        {"title", tr("New city")},
        {"utc", "+00:00"},
        {"lat", "00º 00' 00\" N"},
        {"lon", "000º 00' 00\" E"},
        {"mode", false}
    };

    AspPatchDialog::accDesc_ = {tr("Normal"), tr("Maximum"), tr("Minimum")};
}

} // namespace napatahti
