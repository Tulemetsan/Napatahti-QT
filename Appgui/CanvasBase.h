#ifndef CANVASBASE_H
#define CANVASBASE_H

#include <QFont>
#include <QPen>
#include <QGraphicsSimpleTextItem>

namespace napatahti {

class Canvas;


class ItemEventFilter : public QGraphicsSimpleTextItem
{
public :
    explicit ItemEventFilter(Canvas* canvas) : canvas_ (canvas) {}

private :
    Canvas* canvas_;

    auto sceneEventFilter(QGraphicsItem* item, QEvent* event) -> bool;
};


struct SceneGroup {
    QGraphicsItemGroup* header;
    QGraphicsItemGroup* recepTree;
    QGraphicsItemGroup* esseStat;
    QGraphicsItemGroup* crdTable;
    QGraphicsItemGroup* lineBar;
    QGraphicsItemGroup* aspCfg;
    QGraphicsItemGroup* aspStat;
    QGraphicsItemGroup* coreStat;
    QGraphicsItemGroup* circle;
    QGraphicsItemGroup* aspects;
    QGraphicsItemGroup* markers;

    auto begin() { return &header; }
    auto end() { return &markers + 1; }
};


struct CanvasFont {
    QFont aspMarkTxt {"Arial", 8};
    QFont aspMarkSym {"HamburgSymbols", 12};
    QFont canvasTxt {"Arial", 12};
    QFont canvasSym {"HamburgSymbols", 12};
    QFont miniSymA {"HamburgSymbols", 10};
    QFont miniSymB {"Arial", 9};
    QFont signSym {"HamburgSymbols", 14};
    QFont cuspidSym {"HamburgSymbols", 14};
    QFont cuspidSymSup {"HamburgSymbols", 8};
    QFont planetSym {"HamburgSymbols", 16};
    QFont planetSymSup {"HamburgSymbols", 8};
    QFont planetSymSub {"HamburgSymbols", 11};
    QFont fixedStar {"Arial", 11};
    QFont markLine {"Arial", 9};

    auto begin() { return &aspMarkTxt; }
    auto end() { return &markLine + 1; }
    auto& operator[](int key) { return *(&aspMarkTxt + key); }
    auto& operator[](int key) const { return *(&aspMarkTxt + key); }
};


struct CanvasColor {
    QBrush textFg {"#000000"};
    QBrush textBkg {"#ffffff"};
    QBrush circleBkg {"#e2edfa"};
    QBrush aspFieldBkg {"#ffffff"};
    QBrush signSymFg {"#000000"};
    QBrush cuspSymFg {"#000000"};
    QBrush planetSymFg {"#000000"};
    QBrush kingDegFg {"#ee0000"};
    QBrush destDegFg {"#000000"};

    std::array<QBrush, 4> signBkg {QBrush{"#e66159"}, {"#008000"}, {"#ffff00"}, {"#0080ff"}};
    std::array<QBrush, 5> spec {QBrush{"#228b22"}, {"#dc143c"}, {"#0000cd"}, {"#9932cc"}, {"#000000"}};

    QColor cuspLineFg {"#808080"};
    QColor aspCfgMarkFg {"#a000ff"};
    std::array<QColor, 3> force {"#000000", "#00bb00", "#d0d0d0"};

    auto baseBkg() const { return QString("background: %1").arg(textBkg.color().name()); }
    auto getSpec(const std::array<int, 2>& key) const -> const QBrush&;
};


struct CanvasPen {
    QPen textFg {"#000000"};
    QPen borderFg {"#808080"};
    QPen planetLineFg {"#2f97bb"};
    QPen markLine {"#606060"};
    QPen positive {"#00a800"};
    QPen negative {"#ff0000"};
    QPen ashaPower {"#d9d9d9"};
    QPen ashaCreat {"#0080ff"};

    auto begin() { return &textFg; }
    auto end() { return &ashaCreat + 1; }
    auto posIt() { return &positive; }
    auto baseSet() const { return std::array<const QPen*, 4>{&positive, &negative, &positive, &negative}; }
    auto ashaSet() const { return std::array<const QPen*, 4>{&ashaPower, &ashaPower, &ashaCreat, &ashaCreat}; }
};


struct HeaderSizeCache {
    HeaderSizeCache() {}
    HeaderSizeCache(double scale, double sh, const QMap<QString, int>& size);

    QPointF begPos;
    QPointF extPos;
};


struct RecepSizeCache {
    RecepSizeCache() {}
    RecepSizeCache(int h, double scale, double sh, const QMap<QString, int>& size);

    QPointF begPos;
    double  offset;
    int     maxLen;
};


struct EsseStatSizeCache {
    EsseStatSizeCache() {}
    EsseStatSizeCache(int h, double scale, double sh, const QMap<QString, int>& size);

    double  offset;
    double  crutch;
    QPointF descPos;
};


struct MapBaseSizeCache {
    MapBaseSizeCache() {}
    MapBaseSizeCache(int w, int h, double scale, const QMap<QString, int>& size);

    QRectF  outRect;
    QRectF  signRect;
    QRectF  capRect;
    QRectF  insRect;
    double  symRad;
};


struct CuspidSizeCache {
    CuspidSizeCache() {}
    CuspidSizeCache(int h, double scale, const QMap<QString, int>& size);

    double endR2;
    double endR1;
    double begR2;
    double begR1;
    double tipAng;
    double arrAdd;
    double arrLen;
    double bedLen;
};


struct PlanetSizeCache {
    PlanetSizeCache() {}
    PlanetSizeCache(double rad, double scale, const QMap<QString, int>& size);

    double insRad;
    double symRad;
    QRectF markRect;
};


struct AspectSizeCache {
    AspectSizeCache() {}
    AspectSizeCache(double rad, double scale);

    double insRad;
    double widthB;
    QRectF unionRect;
};


struct CrdTableSizeCache {
    CrdTableSizeCache() {}
    CrdTableSizeCache(int w, double scale, const QRectF& rect, const QMap<QString, int>& size);

    double step;
    double symX;
    double crdX;
    double kadX;
    double starX;
    double posY;
    double maxX;
};


struct LineBarSizeCache {
    LineBarSizeCache() {}
    LineBarSizeCache(double scale, const CrdTableSizeCache& cache, const QMap<QString, int>& size);

    QLineF  uLine;
    QLineF  dLine;
    QRectF  point;
    QPointF offset;
    QPointF topLeft;
    double  primeScale;
    double  ashaScale;
    double  cosmicScale;
};


struct AspCfgSizeCache {
    AspCfgSizeCache() {}
    AspCfgSizeCache(double scale, double sw, double step, const QMap<QString, int>& size);

    double  topMargin;
    double  symWidth;
    double  shiftLen;
    double  baseX;
    QPointF offset;
    QPointF crutch;
};


struct AspStatSizeCache {
    AspStatSizeCache() {}
    AspStatSizeCache(int w, int h, double scale, double sh, const QMap<QString, int>& size);

    QRectF  base;
    QPointF offset;
    double  step;
};


struct CoreStatSizeCache {
    CoreStatSizeCache() {}
    CoreStatSizeCache(int w, int h, double scale, double sh, const QMap<QString, int>& size);

    QPointF begL;
    QPointF begR;
    QPointF numDp;
    QRectF  base;
    QRectF  rect;
    double  offset;

    auto moveBase(double mh, bool view, bool inv) -> void;
    auto resetBase(bool view, bool inv) -> void;
};


struct ContextMap {
    QRectF header;
    QRectF esseStat;
    QRectF circle;
    QRectF crdTable;
    QRectF lineBar;
    QRectF aspCfg;
    QRectF coreStat;
};

} // namespace napatahti

#endif // CANVASBASE_H
