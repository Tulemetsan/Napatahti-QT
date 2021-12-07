#include <QActionGroup>
#include <QFileDialog>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QMessageBox>
#include "mask.h"
#include "shared.h"
#include "Kernel/Kernel.h"
#include "SharedGui.h"
#include "Canvas.h"
#include "LineEditDialog.h"
#include "AtlasDialog.h"
#include "PersonDialog.h"
#include "DataBaseDialog.h"
#include "AspPageDialog.h"
#include "CatalogDialog.h"
#include "AspTableDialog.h"
#include "TCounterDialog.h"
#include "ConfigDialog.h"
#include "AboutDialog.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

namespace napatahti {

MainWindow::MainWindow(QWidget* root)
    : QMainWindow (root)
    , ui (new Ui::MainWindow)
    , kernel_ (new Kernel)
    , canvas_ (new Canvas(*kernel_, this))
    , atlasDialog_ (new AtlasDialog(this))
    , personDialog_ (new PersonDialog(atlasDialog_, this))
    , dbaseDialog_ (new DataBaseDialog(kernel_->person(), atlasDialog_, this))
    , aspPageDialog_ (new AspPageDialog(kernel_->aspPage(), this))
    , catalogDialog_ (new CatalogDialog(kernel_->astroBase(), this))
    , aspTableDialog_ (new AspTableDialog(
        kernel_->person(), kernel_->astroBase(), kernel_->aspPage(), kernel_->aspTable(), this))
    , tCounterDialog_ (new TCounterDialog(&kernel_->person(), this))
    , configDialog_ (new ConfigDialog(atlasDialog_, this))
{
    setupBase();

    // PersonDialog -------------------------------------------------------- //
    connect(personDialog_, &PersonDialog::personChanged, dbaseDialog_, &DataBaseDialog::onPersonChange);
    connect(personDialog_, &PersonDialog::personInserted, dbaseDialog_, &DataBaseDialog::onPersonInsert);
    connect(personDialog_, &PersonDialog::personUpdated, dbaseDialog_, &DataBaseDialog::onPersonUpdate);
    connect(personDialog_, &PersonDialog::personDeleted, dbaseDialog_, &DataBaseDialog::onPersonDelete);

    // DataBaseDialog setup ------------------------------------------------ //
    connect(dbaseDialog_, &DataBaseDialog::personChanged, this, &MainWindow::onUpdate);
    connect(dbaseDialog_, &DataBaseDialog::patchNeedSync, aspTableDialog_, &AspTableDialog::reloadTable);
    connect(dbaseDialog_, &DataBaseDialog::personNeedSync, personDialog_, &PersonDialog::onDialogSync);
    connect(dbaseDialog_, &DataBaseDialog::personNeedSync, tCounterDialog_, &TCounterDialog::onDialogSync);

    // AspPageDialog setup ------------------------------------------------- //
    connect(aspPageDialog_, &AspPageDialog::aspPageChanged, this, &MainWindow::onUpdate);
    connect(aspPageDialog_, &AspPageDialog::aspPageNeedSync, aspTableDialog_, &AspTableDialog::reloadTable);

    // CatalogDialog setup ------------------------------------------------- //
    connect(catalogDialog_, &CatalogDialog::catalogUpdated, this, &MainWindow::onUpdate);
    connect(catalogDialog_, &CatalogDialog::catalogNeedSync, aspTableDialog_, &AspTableDialog::reloadTable);

    // AspTableDialog setup ------------------------------------------------ //
    connect(aspTableDialog_, &AspTableDialog::patchChanged, this, &MainWindow::onUpdate);
    connect(aspTableDialog_, &AspTableDialog::personNeedSync, personDialog_, &PersonDialog::onDialogSync);

    // TCounterDialog setup ------------------------------------------------ //
    connect(tCounterDialog_, &TCounterDialog::personChanged, this, &MainWindow::onUpdate);
    connect(tCounterDialog_, &TCounterDialog::personNeedSync, personDialog_, &PersonDialog::onDialogSync);
    connect(tCounterDialog_, &TCounterDialog::personNeedSync, aspTableDialog_, &AspTableDialog::reloadTable);

    // ConfigDialog setup -------------------------------------------------- //
    connect(configDialog_, &ConfigDialog::cacheNeedUpdate, canvas_, &Canvas::updateCache);
    connect(configDialog_, &ConfigDialog::settingsUpdated, this, &MainWindow::onUpdate);
    connect(configDialog_, &ConfigDialog::canvasBkgUpdated, canvas_, &Canvas::onChangeBkg);
    connect(configDialog_, &ConfigDialog::aspPageNeedSync, aspPageDialog_, &AspPageDialog::reloadPage);
    connect(configDialog_, &ConfigDialog::aspTableNeedSync, aspTableDialog_, &AspTableDialog::reloadTable);
    connect(configDialog_, &ConfigDialog::triggersReseted, this, &MainWindow::onTriggerReset);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onUpdate(int kMask, int cMask)
{
    if (kMask >= 0)
    {
        if (kMask == 0 || (kMask & KernelMask::AstroBase) != 0)
        {
            kernel_->setHeaderStr();
            kernel_->AstroBase::update(Canvas::getPlanetSpace(), SharedGui::getAppLocale());
        }
        if ((kMask & KernelMask::Coupling) != 0)
            kernel_->computeCoupling(Canvas::getPlanetSpace());
        if (kMask == 0 || (kMask & KernelMask::AspTable) != 0)
            kernel_->AspTable::update(true, kernel_->getAccKey(), trigger_.aspCfgClassOnly);
        if ((kMask & KernelMask::AspCfg) != 0)
            kernel_->AspTable::update(false, kernel_->getAccKey(), trigger_.aspCfgClassOnly);
        if ((kMask & KernelMask::AspTableStar) != 0)
            kernel_->computeStarTable();
        if (kMask == 0 || (kMask & KernelMask::PrimeTest) != 0)
            kernel_->PrimeTest::update();
        if (kMask == 0 || (kMask & KernelMask::AshaTest) != 0)
            kernel_->AshaTest::update();
        if (kMask == 0 || (kMask & KernelMask::CosmicTest) != 0)
            kernel_->CosmicTest::update();
    }

    if (cMask >= 0)
    {
        if (trigger_.viewHeader && (cMask == 0 || (cMask & CanvasMask::Header) != 0))
            canvas_->renderHeader();
        if (trigger_.viewRecepTree && (cMask == 0 || (cMask & CanvasMask::RecepTree) != 0))
            canvas_->renderRecepTree();
        if (trigger_.viewEsseStat && (cMask == 0 || (cMask & CanvasMask::EsseStat) != 0))
            canvas_->renderEsseStat();
        if (trigger_.viewCircle && (cMask == 0 || (cMask & CanvasMask::Circle) != 0))
          canvas_->renderCircle();
        if (trigger_.viewCircle && (cMask & CanvasMask::Aspects) != 0)
            canvas_->renderAspects();
        if (trigger_.viewCrdTable && (cMask == 0 || (cMask & CanvasMask::CrdTable) != 0))
            canvas_->renderCrdTable();
        if (trigger_.viewLineBar && (cMask == 0 || (cMask & CanvasMask::LineBar) != 0))
            canvas_->renderLineBar();
        if (trigger_.viewAspCfg && (cMask == 0 || (cMask & CanvasMask::AspCfg) != 0))
            canvas_->renderAspCfg();
        if (trigger_.viewAspStat && (cMask == 0 || (cMask & CanvasMask::AspStat) != 0))
            canvas_->renderAspStat();
        if (trigger_.viewCoreStat && (cMask == 0 || (cMask & CanvasMask::CoreStat) != 0))
            canvas_->renderCoreStat();
    }
}


void MainWindow::onDataSaveAs(const QAction* action)
{
    QString name (kernel_->person().name);
    QString data;
    QString format (tr("Text files") + " (*.txt)");

    if (action == ui->actSaveAsMap)
        format = tr("Image") + " (*.png)";
    else if (action == ui->actSaveAsCalendar)
    {
        name += " - " + tr("Moon calendar");
        data = kernel_->toStrMoonCalendar();
    }
    else if (action == ui->actSaveAsPrime)
    {
        name += " - " + tr("Prime test");
        data = kernel_->toStrPrimeStat();
    }
    else if (action == ui->actSaveAsAsha)
    {
        name += " - " + tr("Asha test");
        data = kernel_->toStrAshaStat();
    }
    else if (action == ui->actSaveAsCosmic)
    {
        name += " - " + tr("Cosmic test");
        data = kernel_->toStrCosmicStat();
    }
    else if (action == ui->actSaveAsCoreStat)
    {
        name += " - " + tr("Core statistic");
        data = kernel_->toStrCoreStat();
    }
    else
        return;

    auto fileName (QFileDialog::getSaveFileName(this, tr("Save as"), name, format));
    if (!fileName.isEmpty())
    {
        if (action == ui->actSaveAsMap)
        {
            if (!canvas_->grab().save(fileName))
                errorLog("can not save " + name);
        }
        else
        {
            QFile file (fileName);
            if (file.open(QIODevice::WriteOnly))
            {
                QTextStream out (&file);
                out << data;
                file.close();
            }
            else
                errorLog("can not save " + name);
        }
    }
}


void MainWindow::onPrintMap()
{
    QPrinter printer;
    QPrintDialog printDialog(&printer, this);

    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setPrintRange(QPrinter::PrintRange::PageRange);
    printer.setFromTo(1, 1);

    if (printDialog.exec() == QDialog::Accepted)
    {
        QPixmap imgGrab (canvas_->grab());
        QPainter painter (&printer);
        QRectF imgRect (imgGrab.rect());
        QRectF rect (painter.viewport());
        double k (imgRect.height() * rect.width() / rect.height());

        painter.setWindow(0, (imgRect.height() - k) / 2, imgRect.width(), k);
        painter.drawPixmap(0, 0, imgGrab);
    }
}


void MainWindow::onViewSet(const QAction* action)
{
    trigger_.set(action);

    auto view (action->isChecked());
    if (action == ui->trigViewToolBar)
        view ? ui->toolbar->show() : ui->toolbar->hide();
    else if (action == ui->trigViewHeader)
        canvas_->renderHeader(view);
    else if (action == ui->trigViewRecepTree)
        canvas_->renderRecepTree(view);
    else if (action == ui->trigViewEsseStat)
        canvas_->renderEsseStat(view);
    else if (action == ui->trigViewCircle)
        canvas_->renderCircle(view);
    else if (action == ui->trigViewCrdTable)
        canvas_->renderCrdTable(view);
    else if (action == ui->trigViewLineBar)
        canvas_->renderLineBar(view);
    else if (action == ui->trigViewAspCfg)
        canvas_->renderAspCfg(view);
    else if (action == ui->trigViewAspStat)
        canvas_->renderAspStat(view);
    else if (action == ui->trigViewCoreStat)
        canvas_->renderCoreStat(view);
    else if (action == ui->trigViewStatusBar)
        view ? ui->statusbar->show() : ui->statusbar->hide();
}


void MainWindow::onHeaderSet(const QAction* action)
{
    trigger_.set(action);
    canvas_->renderHeader();
}


void MainWindow::onEsseStatSet(const QAction* action)
{
    trigger_.set(action);
    canvas_->renderEsseStat();
}


void MainWindow::onCircleSet(const QAction* action)
{
    if (trigger_.set(action))
    {
        if (agAccKey_ == action->actionGroup())
        {
            kernel_->AspTable::update(false, kernel_->getAccKey(), trigger_.aspCfgClassOnly);
            canvas_->renderAspects();
            canvas_->renderAspCfg();
        }
        else
            canvas_->renderCircle();
    }
}


void MainWindow::onCrdTableSet(bool checked)
{
    trigger_.fixedStars = checked;
    canvas_->renderCrdTable();
}


void MainWindow::onLineBarSet(const QAction* action)
{
    if (trigger_.set(action))
        canvas_->renderLineBar();
}


void MainWindow::onAspCfgSet(const QAction* action)
{
    trigger_.set(action);

    if (action == ui->trigAspCfgClassicOnly)
        kernel_->AspTable::update(false, kernel_->getAccKey(), trigger_.aspCfgClassOnly);

    canvas_->renderAspCfg();
}


void MainWindow::onCoreStatSet(const QAction* action)
{
    if (trigger_.set(action))
    {
        if (agCoreMode_ == action->actionGroup() && trigger_.modePrime)
            canvas_->renderLineBar();

        canvas_->renderCoreStat();
    }
}


void MainWindow::onDialogShow(const QAction* action)
{
    if (action == ui->actNew)
        personDialog_->modeShow(false, &kernel_->person());
    else if (action == ui->actEdit)
        personDialog_->modeShow(true, &kernel_->person());
    else if (action == ui->actDataBase)
    {
        dbaseDialog_->showNormal();
        dbaseDialog_->activateWindow();
    }
    else if (action == ui->actAspPage)
    {
        aspPageDialog_->showNormal();
        aspPageDialog_->activateWindow();
    }
    else if (action == ui->actPlanets)
    {
        catalogDialog_->showNormal();
        catalogDialog_->activateWindow();
    }
    else if (action == ui->actAspTable)
    {
        aspTableDialog_->showNormal();
        aspTableDialog_->activateWindow();
    }
    else if (action == ui->actTCounter)
        tCounterDialog_->modeShow();
    else if (action == ui->actSettings)
    {
        configDialog_->showNormal();
        configDialog_->activateWindow();
    }
}


void MainWindow::onCanvasMenu(const QPoint& pos)
{
    auto& context (canvas_->getContextMap());

    if (trigger_.viewHeader && context.header.contains(pos))
    {
        auto menu (new QMenu(this));
        menu->addAction(ui->trigModeJyotisa);
        menu->addAction(ui->trigModeWestern);
        menu->addSeparator();
        menu->addAction(ui->actSaveAsCalendar);
        menu->exec(canvas_->mapToGlobal(pos));
    }
    else if (trigger_.viewEsseStat && context.esseStat.contains(pos))
        ui->menuEsseStat->exec(canvas_->mapToGlobal(pos));
    else if (trigger_.viewCircle && context.circle.contains(pos))
        ui->menuCircle->exec(canvas_->mapToGlobal(pos));
    else if (trigger_.viewCrdTable && context.crdTable.contains(pos))
        ui->menuCrdTable->exec(canvas_->mapToGlobal(pos));
    else if (trigger_.viewLineBar && context.lineBar.contains(pos))
    {
        auto menu (new QMenu(this));
        menu->addMenu(ui->menuLineBarMode);
        menu->addSeparator();

        if (trigger_.modeAsha)
        {
            menu->addAction(ui->trigAshaPowKind);
            menu->addAction(ui->trigAshaPowCreat);
            menu->addAction(ui->trigAshaKindDet);
            menu->addSeparator();
            menu->addAction(ui->actSaveAsAsha);
        }
        else if (trigger_.modePrime)
        {
            menu->addAction(ui->trigPrimeCosDet);
            menu->addAction(ui->trigPrimeHorDet);
            menu->addAction(ui->trigPrimeNormal);
            menu->addSeparator();
            menu->addAction(ui->actSaveAsPrime);
        }
        else
        {
            menu->addAction(ui->trigCosmicSum);
            menu->addAction(ui->trigCosmicDet);
            menu->addSeparator();
            menu->addAction(ui->actSaveAsCosmic);
        }

        menu->exec(canvas_->mapToGlobal(pos));
    }
    else if (trigger_.viewAspCfg && context.aspCfg.contains(pos))
        ui->menuAspCfg->exec(canvas_->mapToGlobal(pos));
    else if (trigger_.viewCoreStat && context.coreStat.contains(pos))
    {
        auto menu (new QMenu(this));
        menu->addAction(ui->trigModeGloba);
        menu->addAction(ui->trigModeSchit);
        menu->addSeparator();
        menu->addAction(ui->trigCoreStatCosEnb);
        menu->addAction(ui->trigCoreStatHorEnb);
        menu->addAction(ui->trigCoreStatInvert);
        menu->addSeparator();
        menu->addAction(ui->actSaveAsCoreStat);
        menu->exec(canvas_->mapToGlobal(pos));
    }
}


void MainWindow::onTriggerReset()
{
    trigger_ = {};
    applyTrigger();
    onUpdate(KernelMask::AspCfg, 0);
}


void MainWindow::onClickSaveAsProfile()
{
    auto dialog (new LineEditDialog(this));

    dialog->setWindowTitle(tr("Save as"));
    dialog->setWindowIcon(QIcon(":/24x24/save.png"));
    dialog->setText(ConfigDialog::profileName());
    dialog->setValidator(new QRegularExpressionValidator(QRegularExpression{"^[^/\\\\:*?<>|\\.]+$"}, this));

    connect(dialog, &LineEditDialog::accepted, this, &MainWindow::onSaveAsProfile);

    dialog->exec();
}


void MainWindow::onClickDeleteProfile()
{
    auto profile (ConfigDialog::profileName());

    if (profile == ConfigDialog::profileNameR())
        return;

    auto msgBox (new QMessageBox(
        QMessageBox::Question, tr("Delete"),
        tr("Do you want to delete profile") + ' ' + profile + '?',
        QMessageBox::Yes | QMessageBox::No, this));

    msgBox->setAttribute(Qt::WA_WindowPropagation, true);
    msgBox->setWindowIcon(QIcon(":/24x24/erase.png"));

    if (msgBox->exec() == QMessageBox::Yes)
    {
        QFile::remove(ConfigDialog::path + profile + ".json");
        configDialog_->onLoadProfile(nullptr);
        setProfileMenu();
    }
}


void MainWindow::onSaveAsProfile(const QString& text)
{
    if (text.isEmpty() || text == ConfigDialog::profileNameR())
        return;

    ConfigDialog::dumpProFile(text);
    setProfileMenu();
}


auto MainWindow::setupBase() -> void
{
    ui->setupUi(this);

    setLocale(SharedGui::getAppLocale());

    ui->tableLabel->setText(DataBaseDialog::getTableName());
    ui->statusbar->addWidget(ui->sizeLabel);
    ui->statusbar->addWidget(ui->tableLabel);
    ui->vbox->addWidget(canvas_);

    // Toolbar & Context menu ---------------------------------------------- //
    auto agDialog (new QActionGroup(this));
    agDialog->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    agDialog->addAction(ui->actNew);
    agDialog->addAction(ui->actEdit);
    agDialog->addAction(ui->actDataBase);
    agDialog->addAction(ui->actAspPage);
    agDialog->addAction(ui->actPlanets);
    agDialog->addAction(ui->actAspTable);
    agDialog->addAction(ui->actTCounter);
    agDialog->addAction(ui->actSettings);

    connect(agDialog, &QActionGroup::triggered, this, &MainWindow::onDialogShow);
    connect(ui->actAppQuit, &QAction::triggered, qApp, &QApplication::quit);

    connect(canvas_, &Canvas::customContextMenuRequested, this, &MainWindow::onCanvasMenu);

    // Menu File ----------------------------------------------------------- //
    auto agSaveAs (new QActionGroup(this));
    agSaveAs->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    agSaveAs->addAction(ui->actSaveAsMap);
    agSaveAs->addAction(ui->actSaveAsCalendar);
    agSaveAs->addAction(ui->actSaveAsPrime);
    agSaveAs->addAction(ui->actSaveAsCoreStat);
    agSaveAs->addAction(ui->actSaveAsAsha);
    agSaveAs->addAction(ui->actSaveAsCosmic);

    connect(agSaveAs, &QActionGroup::triggered, this, &MainWindow::onDataSaveAs);
    connect(ui->actPrintMap, &QAction::triggered, this, &MainWindow::onPrintMap);

    applyTrigger();

    // Menu View ----------------------------------------------------------- //
    auto agView (new QActionGroup(this));
    agView->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    agView->addAction(ui->trigViewToolBar);
    agView->addAction(ui->trigViewHeader);
    agView->addAction(ui->trigViewRecepTree);
    agView->addAction(ui->trigViewEsseStat);
    agView->addAction(ui->trigViewCircle);
    agView->addAction(ui->trigViewCrdTable);
    agView->addAction(ui->trigViewLineBar);
    agView->addAction(ui->trigViewAspCfg);
    agView->addAction(ui->trigViewAspStat);
    agView->addAction(ui->trigViewCoreStat);
    agView->addAction(ui->trigViewStatusBar);

    connect(agView, &QActionGroup::triggered, this, &MainWindow::onViewSet);

    // Menu options / Moon day --------------------------------------------- //
    auto agMoonDay (new QActionGroup(this));
    agMoonDay->addAction(ui->trigModeJyotisa);
    agMoonDay->addAction(ui->trigModeWestern);

    connect(agMoonDay, &QActionGroup::triggered, this, &MainWindow::onHeaderSet);

    // Menu options / Essentials ------------------------------------------- //
    auto agEsseStat (new QActionGroup(this));
    agEsseStat->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    agEsseStat->addAction(ui->trigBySunSpec);
    agEsseStat->addAction(ui->trigByEsseSpec);
    agEsseStat->addAction(ui->trigByAspSpec);
    agEsseStat->addAction(ui->trigDorAuriga);
    agEsseStat->addAction(ui->trigByAshaSpec);

    connect(agEsseStat, &QActionGroup::triggered, this, &MainWindow::onEsseStatSet);

    // Menu options / Circle / Accuracy ------------------------------------ //
    agAccKey_ = new QActionGroup(this);
    agAccKey_->addAction(ui->trigAccMid);
    agAccKey_->addAction(ui->trigAccMax);
    agAccKey_->addAction(ui->trigAccMin);
    agAccKey_->addAction(ui->trigAccAny);

    connect(agAccKey_, &QActionGroup::triggered, this, &MainWindow::onCircleSet);

    // Menu options / Circle / Zero angle ---------------------------------- //
    auto agZeroAng (new QActionGroup(this));
    agZeroAng->addAction(ui->trigZeroAngAsc);
    agZeroAng->addAction(ui->trigZeroAngAri);

    connect(agZeroAng, &QActionGroup::triggered, this, &MainWindow::onCircleSet);

    // Menu options / Circle ----------------------------------------------- //
    auto agCircle (new QActionGroup(this));
    agCircle->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    agCircle->addAction(ui->trigCircleDegrees);
    agCircle->addAction(ui->trigCircleSpeeds);
    agCircle->addAction(ui->trigCircleCuspids);
    agCircle->addAction(ui->trigCircleColors);

    connect(agCircle, &QActionGroup::triggered, this, &MainWindow::onCircleSet);

    // Menu options / Coordinates ------------------------------------------ //
    connect(ui->trigFixedStars, &QAction::triggered, this, &MainWindow::onCrdTableSet);

    // Menu options / Planet statistic / Method ---------------------------- //
    auto agLineBar (new QActionGroup(this));
    agLineBar->addAction(ui->trigModePrime);
    agLineBar->addAction(ui->trigModeAsha);
    agLineBar->addAction(ui->trigModeCosmic);

    connect(agLineBar, &QActionGroup::triggered, this, &MainWindow::onLineBarSet);

    // Menu options / Planet statistic / Prime ----------------------------- //
    auto agPrimeView (new QActionGroup(this));
    agPrimeView->addAction(ui->trigPrimeCosDet);
    agPrimeView->addAction(ui->trigPrimeHorDet);
    agPrimeView->addAction(ui->trigPrimeNormal);

    connect(agPrimeView, &QActionGroup::triggered, this, &MainWindow::onLineBarSet);

    // Menu options / Planet statistic / Asha ------------------------------ //
    auto agAshaView (new QActionGroup(this));
    agAshaView->addAction(ui->trigAshaPowKind);
    agAshaView->addAction(ui->trigAshaPowCreat);
    agAshaView->addAction(ui->trigAshaKindDet);

    connect(agAshaView, &QActionGroup::triggered, this, &MainWindow::onLineBarSet);

    // Menu options / Planet statistic / Cosmic ---------------------------- //
    auto agCosmicView (new QActionGroup(this));
    agCosmicView->addAction(ui->trigCosmicSum);
    agCosmicView->addAction(ui->trigCosmicDet);

    connect(agCosmicView, &QActionGroup::triggered, this, &MainWindow::onLineBarSet);

    // Menu options / Aspect configs --------------------------------------- //
    auto agAspCfg (new QActionGroup(this));
    agAspCfg->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    agAspCfg->addAction(ui->trigAspCfgClassicOnly);
    agAspCfg->addAction(ui->trigAspCfgBonesOnly);

    connect(agAspCfg, &QActionGroup::triggered, this, &MainWindow::onAspCfgSet);

    // Menu options / Core statistic / Method ------------------------------ //
    agCoreMode_ = new QActionGroup(this);
    agCoreMode_->addAction(ui->trigModeGloba);
    agCoreMode_->addAction(ui->trigModeSchit);

    connect(agCoreMode_, &QActionGroup::triggered, this, &MainWindow::onCoreStatSet);

    // Menu options / Core statistic --------------------------------------- //
    auto agCoreStat (new QActionGroup(this));
    agCoreStat->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
    agCoreStat->addAction(ui->trigCoreStatCosEnb);
    agCoreStat->addAction(ui->trigCoreStatHorEnb);
    agCoreStat->addAction(ui->trigCoreStatInvert);

    connect(agCoreStat, &QActionGroup::triggered, this, &MainWindow::onCoreStatSet);

    // Menu configs -------------------------------------------------------- //
    setProfileMenu();
    connect(ui->actSaveAsProfile, &QAction::triggered, this, &MainWindow::onClickSaveAsProfile);
    connect(ui->actDeleteProfile, &QAction::triggered, this, &MainWindow::onClickDeleteProfile);

    // Additional signals -------------------------------------------------- //
    connect(canvas_, &Canvas::sizeChanged, this,
            [this](const QString& size){ ui->sizeLabel->setText(size); });
    connect(dbaseDialog_, &DataBaseDialog::tableChanged, this,
            [this](const QString& table){ ui->tableLabel->setText(table); });

    // About --------------------------------------------------------------- //
    auto actAbout (new QAction("?", this));
    ui->menubar->addAction(actAbout);

    connect(actAbout, &QAction::triggered, this, [this](){ (new AboutDialog(this))->show(); });
}


auto MainWindow::setProfileMenu() -> void
{
    ui->menuConfigs->clear();

    auto agProfile = new QActionGroup(this);
    auto profile (ConfigDialog::profileName());

    for (auto& i : ConfigDialog::profileNameList())
    {
        auto newAct (new QAction(i, this));
        newAct->setCheckable(true);
        newAct->setChecked(i == profile);

        agProfile->addAction(newAct);
        ui->menuConfigs->addAction(newAct);
    }

    ui->menuConfigs->addSeparator();
    ui->menuConfigs->addAction(ui->actSaveAsProfile);
    ui->menuConfigs->addAction(ui->actDeleteProfile);

    connect(agProfile, &QActionGroup::triggered, configDialog_, &ConfigDialog::onLoadProfile);
}


auto MainWindow::applyTrigger() -> void
{
    QList list {
        ui->trigViewToolBar, ui->trigViewHeader, ui->trigViewRecepTree, ui->trigViewEsseStat,
        ui->trigViewCircle, ui->trigViewCrdTable, ui->trigViewLineBar, ui->trigViewAspCfg,
        ui->trigViewAspStat, ui->trigViewCoreStat, ui->trigViewStatusBar,
        ui->trigBySunSpec, ui->trigByEsseSpec, ui->trigByAspSpec, ui->trigDorAuriga, ui->trigByAshaSpec,
        ui->trigCircleDegrees, ui->trigCircleSpeeds, ui->trigCircleCuspids, ui->trigCircleColors,
        ui->trigFixedStars,
        ui->trigAspCfgClassicOnly, ui->trigAspCfgBonesOnly,
        ui->trigCoreStatCosEnb, ui->trigCoreStatHorEnb, ui->trigCoreStatInvert,
        ui->trigModeJyotisa, ui->trigModeWestern,
        ui->trigAccMid, ui->trigAccMax, ui->trigAccMin, ui->trigAccAny,
        ui->trigZeroAngAsc, ui->trigZeroAngAri,
        ui->trigModePrime, ui->trigModeAsha, ui->trigModeCosmic,
        ui->trigPrimeCosDet, ui->trigPrimeHorDet, ui->trigPrimeNormal,
        ui->trigAshaPowKind, ui->trigAshaPowCreat, ui->trigAshaKindDet,
        ui->trigCosmicSum, ui->trigCosmicDet,
        ui->trigModeGloba, ui->trigModeSchit
    };

    auto checked (trigger_.begin());
    auto index (0);

    for (auto i : list)
    {
        i->setChecked(*checked);
        i->setData(index);
        ++checked;
        ++index;
    }

    trigger_.viewToolBar ? ui->toolbar->show() : ui->toolbar->hide();
    trigger_.viewStatusBar ? ui->statusbar->show() : ui->statusbar->hide();
}

} // namespace napatahti
