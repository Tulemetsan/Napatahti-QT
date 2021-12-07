#include <QEvent>
#include "shared.h"
#include "Kernel/RefBook.h"
#include "Canvas.h"

namespace napatahti {

auto ItemEventFilter::sceneEventFilter(QGraphicsItem* item, QEvent *event) -> bool
{
    switch (event->type()) {
    case QEvent::GraphicsSceneHoverEnter :
        canvas_->renderAspects(item);
        return true;
    case QEvent::GraphicsSceneHoverLeave :
        canvas_->cleanGroup(canvas_->group_.markers);
        return true;
    default :
        return false;
    }
}


auto CanvasColor::getSpec(const std::array<int, 2>& key) const -> const QBrush&
{
    auto iter (RefBook::specState.find(key));

    return spec[iter != RefBook::specState.end() ? iter->second : SpecState::Neutral];
}


HeaderSizeCache::HeaderSizeCache(double scale, double sh, const QMap<QString, int>& size)
    : begPos (scale * size.value("padX"), scale * size.value("padY"))
    , extPos (begPos)
{
    extPos.ry() += 6 * sh;
}


RecepSizeCache::RecepSizeCache(int h, double scale, double sh, const QMap<QString, int>& size)
    : begPos (scale * size.value("padX"), 0.5 * h + scale * size.value("padY"))
    , offset (1.15 * sh)
    , maxLen (size.value("length"))
{}


EsseStatSizeCache::EsseStatSizeCache(int h, double scale, double sh, const QMap<QString, int>& size)
    : offset (1.15 * sh)
    , crutch (scale * size.value("crutch"))
    , descPos (scale * size.value("padX"), h - scale * size.value("padY") + 0.17 * offset)
{}


MapBaseSizeCache::MapBaseSizeCache(int w, int h, double scale, const QMap<QString, int>& size)
{
    auto margin (scale * size.value("margin"));
    auto offset (scale * size.value("offset"));
    auto circSize (h - 2 * margin);
    auto outSignAdd (scale * size.value("outSignAdd"));
    auto insSignAdd (scale * size.value("insSignAdd"));
    auto insRectAdd (scale * size.value("insRectAdd"));

    outRect = {0.5 * (w - h) + margin + offset, margin, circSize, circSize};
    signRect = outRect.adjusted(outSignAdd, outSignAdd, -outSignAdd, -outSignAdd);;
    capRect = signRect.adjusted(insSignAdd, insSignAdd, -insSignAdd, -insSignAdd);;
    insRect = capRect.adjusted(insRectAdd, insRectAdd, -insRectAdd, -insRectAdd);;
    symRad = 0.5 * (circSize - insSignAdd) - outSignAdd;
}


CuspidSizeCache::CuspidSizeCache(int h, double scale, const QMap<QString, int>& size)
    : tipAng (size.value("tipAng"))
{
    auto rad (0.5 * h - scale * size.value("margin"));
    auto len (scale * size.value("tipLen"));

    endR2 = rad + scale * size.value("outCuspAdd");
    endR1 = rad - scale * (size.value("outSignAdd") - 1);
    begR2 = endR1 - scale * (size.value("insSignAdd") + 2);
    begR1 = begR2 - scale * (size.value("insRectAdd") - 2);
    arrAdd = 0.6 * len;
    arrLen = len / cosDeg(tipAng);
    bedLen = 0.09 * len / cosDeg(60 + tipAng);
}


PlanetSizeCache::PlanetSizeCache(double rad, double scale, const QMap<QString, int>& size)
    : insRad (rad)
    , symRad (rad + scale * size.value("insPlanAdd"))
    , markRect (0, 0, 6 * scale, 6 * scale)
{}


AspectSizeCache::AspectSizeCache(double rad, double scale)
    : insRad (rad - 3 * scale)
    , widthB (5 * scale)
    , unionRect (0, 0, widthB, widthB)
{}


CrdTableSizeCache::CrdTableSizeCache(int w, double scale, const QRectF& rect, const QMap<QString, int>& size)
    : step (1.15 * rect.height())
    , posY (step + scale * size.value("padY"))
{
    auto sw (rect.width());

    symX = w - 2.2 * sw - scale * size.value("padX");
    crdX = symX + 0.26 * sw;
    kadX = crdX + 0.782 * sw;
    starX = crdX + 0.895 * sw;
    maxX = symX + 2.2 * sw;
}


LineBarSizeCache::LineBarSizeCache(double scale, const CrdTableSizeCache& cache, const QMap<QString, int>& size)
    : point (0, 0, 0.2 * cache.step, 0.2 * cache.step)
    , offset (0, cache.step)
    , primeScale (0.1 * scale * size.value("primeScale"))
    , ashaScale (0.1 * scale * size.value("ashaScale"))
    , cosmicScale (0.1 * scale * size.value("cosmicScale"))
{
    auto posX = cache.symX - scale * size.value("padX");
    auto posY = 0.35 * cache.step + cache.posY;
    auto shiftY = 0.15 * cache.step;

    uLine = {posX, posY - shiftY, posX - 1, posY - shiftY};
    dLine = {posX, posY + shiftY, posX - 1, posY + shiftY};
    topLeft = uLine.p1() - 0.39 * offset - QPointF(130 * scale, 0);
}


AspCfgSizeCache::AspCfgSizeCache(double scale, double sw, double step, const QMap<QString, int>& size)
    : topMargin (0.5 + size.value("topMargin"))
    , symWidth (sw)
    , shiftLen (scale * size.value("shiftLen"))
    , baseX (sw * shiftLen)
    , offset (0, step)
    , crutch (0, scale * size.value("titleDy"))
{}


AspStatSizeCache::AspStatSizeCache(int w, int h, double scale, double sh, const QMap<QString, int>& size)
    : base (w - scale * size.value("padX"), h - scale * size.value("padY"), sh, sh)
    , offset (0, 0.35 * base.height())
    , step (1.6 * sh)
{}


CoreStatSizeCache::CoreStatSizeCache(int w, int h, double scale, double sh, const QMap<QString, int>& size)
    : begL (w - scale * size.value("padX") - 26.88 * sh, h - scale * size.value("padY") - 11.47 * sh)
    , begR (begL + QPointF(15.2 * sh, 0))
    , numDp (0, 0.35 * sh)
    , base (0, 0, sh, sh)
    , rect (begL, begL + QPointF(26.72 * sh, 11.47 * sh))
    , offset (1.6 * sh)
{}


auto CoreStatSizeCache::moveBase(double mh, bool view, bool inv) -> void
{
    base.moveTop(base.top() + mh * offset);
    if (view)
        base.moveTopLeft({inv ? begR.x() : begL.x(), base.y()});
    else
        base.moveTopLeft({inv ? begL.x() : begR.x(), base.y()});
}


auto CoreStatSizeCache::resetBase(bool view, bool inv) -> void
{
    base.moveTopLeft(view ? (inv ? begR : begL) : (inv ? begL : begR));
}

} // namespace napatahti
