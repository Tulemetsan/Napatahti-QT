#ifndef CANVAS_H
#define CANVAS_H

#include <QGraphicsView>
#include "CanvasBase.h"

namespace napatahti {

class Kernel;
class AppTrigger;
class ConfigDialog;


class Canvas : public QGraphicsView
{
    Q_OBJECT

public :
    explicit Canvas(const Kernel& kernel, QWidget* root=nullptr);

    auto renderHeader(bool mode=true) -> void;
    auto renderRecepTree(bool mode=true) -> void;
    auto renderEsseStat(bool mode=true) -> void;
    auto renderCircle(bool mode=true) -> void;
    auto renderAspects(QGraphicsItem* item=nullptr) -> void;
    auto renderCrdTable(bool mode=true) -> void;
    auto renderLineBar(bool mode=true) -> void;
    auto renderAspCfg(bool mode=true) -> void;
    auto renderAspStat(bool mode=true) -> void;
    auto renderCoreStat(bool mode=true) -> void;

    auto& getContextMap() const { return cmap_; }

    static auto getPlanetSpace() { return circleSizeSrc_.value("space"); }

public slots :
    void updateCache(int cMask=0);
    void onChangeBkg();

signals :
    void sizeChanged(const QString& size);

private :
    const Kernel&     kernel_;
    const AppTrigger& trigger_;

    QGraphicsScene*  scene_;
    ItemEventFilter* itemEventFilter_;
    QTimer*          timer_;

    SceneGroup group_;
    ContextMap cmap_;

    QRect   geo_;
    QPointF zeroPos_;
    double  zeroAng_;
    double  scale_;

    CanvasFont font_;
    CanvasPen  pen_;

    HeaderSizeCache   headerSize_;
    RecepSizeCache    recepSize_;
    EsseStatSizeCache esseStatSize_;
    MapBaseSizeCache  mapBaseSize_;
    CuspidSizeCache   cuspidSize_;
    PlanetSizeCache   planetSize_;
    AspectSizeCache   aspectSize_;
    CrdTableSizeCache crdTableSize_;
    LineBarSizeCache  lineBarSize_;
    AspCfgSizeCache   aspCfgSize_;
    AspStatSizeCache  aspStatSize_;
    CoreStatSizeCache coreStatSize_;

    static CanvasFont  fontSrc_;
    static CanvasColor colorSrc_;
    static CanvasPen   penSrc_;

    static QMap<QString, int> headerSizeSrc_;
    static QMap<QString, int> recepSizeSrc_;
    static QMap<QString, int> esseStatSizeSrc_;
    static QMap<QString, int> circleSizeSrc_;
    static QMap<QString, int> crdTableSizeSrc_;
    static QMap<QString, int> lineBarSizeSrc_;
    static QMap<QString, int> aspCfgSizeSrc_;
    static QMap<QString, int> aspStatSizeSrc_;
    static QMap<QString, int> coreStatSizeSrc_;

private slots :
    void onRenderScene();

private :
    auto resizeEvent(QResizeEvent* event) -> void;

    auto cleanGroup(QGraphicsItemGroup* group) -> void;
    auto renderMapBase() -> void;
    auto renderCuspids() -> void;
    auto renderPlanets() -> void;

    auto symbol(int ind, const QPen& fg, const QRectF& base) -> QGraphicsItemGroup*;

    auto getKadData(int crd) const -> std::pair<QString, const QBrush*>;

    friend ItemEventFilter;
    friend ConfigDialog;
};

} // namespace napatahti

#endif // CANVAS_H
