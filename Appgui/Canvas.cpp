#include <QTimer>
#include "mask.h"
#include "shared.h"
#include "Kernel/Kernel.h"
#include "Kernel/RefBook.h"
#include "MainWindow.h"
#include "Canvas.h"

namespace napatahti {

Canvas::Canvas(const Kernel& kernel, QWidget* root)
    : QGraphicsView (root)
    , kernel_ (kernel)
    , trigger_ (MainWindow::getTrigger())
    , scene_ (new QGraphicsScene)
{
    setFrameShape(QFrame::NoFrame);
    setStyleSheet(colorSrc_.baseBkg());
    setRenderHint(QPainter::Antialiasing);
    setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    for (auto& i : group_)
    {
        i = new QGraphicsItemGroup;
        scene_->addItem(i);
    }

    setScene(scene_);

    itemEventFilter_ = new ItemEventFilter(this);
    scene_->addItem(itemEventFilter_);

    timer_ = new QTimer(this);
    timer_->setSingleShot(true);

    connect(timer_, &QTimer::timeout, this, &Canvas::onRenderScene);
}


void Canvas::onRenderScene()
{
    updateCache();

    if (trigger_.viewHeader)
        renderHeader();
    if (trigger_.viewRecepTree)
       renderRecepTree();
    if (trigger_.viewEsseStat)
        renderEsseStat();
    if (trigger_.viewCircle)
        renderCircle();
    if (trigger_.viewCrdTable)
        renderCrdTable();
    if (trigger_.viewLineBar)
        renderLineBar();
    if (trigger_.viewAspCfg)
        renderAspCfg();
    if (trigger_.viewAspStat)
        renderAspStat();
    if (trigger_.viewCoreStat)
        renderCoreStat();
}


void Canvas::updateCache(int cMask)
{
    auto geo (geometry());
    auto isGeo (geo_ != geo);

    if (isGeo)
    {
        geo_ = geo;
        scene_->setSceneRect(geo);
    }

    if (!isGeo && cMask <= 0)
        return;

    auto width (geo.width());
    auto height (geo.height());

    scale_ = height / 929.0;
    zeroPos_ = {0.5 * width + (scale_ * circleSizeSrc_.value("offset")), 0.5 * height};

    //-----------------------------------------------------------------------//

    if (isGeo || (cMask & CanvasMask::FontCache) != 0)
        for (auto i (font_.begin()), j (fontSrc_.begin()); i != font_.end(); ++i, ++j)
        {
            *i = *j;
            i->setPointSizeF(scale_ * i->pointSizeF());
        }

    //-----------------------------------------------------------------------//

    auto item (new QGraphicsSimpleTextItem("Q"));
    item->setFont(font_.canvasSym);

    auto sh (0.23 * item->boundingRect().height());

    if (isGeo || (cMask & CanvasMask::PenCache) != 0)
    {
        for (auto i (pen_.begin()), j (penSrc_.begin()); i != pen_.end(); ++i, ++j)
        {
            *i = *j;
            i->setWidthF(i < pen_.posIt() ? scale_ : sh);
        }
        pen_.markLine.setDashPattern({5 * scale_, 2.5 * scale_});
    }

    if (isGeo || (cMask & CanvasMask::RecepTree) != 0)
        recepSize_ = {height, scale_, 4.35 * sh, recepSizeSrc_};

    if (isGeo || (cMask & CanvasMask::AspCfg) != 0)
        aspCfgSize_ = {scale_, item->boundingRect().width(), 5 * sh, aspCfgSizeSrc_};

    //-----------------------------------------------------------------------//

    item->setFont(font_.canvasTxt);
    sh = item->boundingRect().height();

    if (isGeo || (cMask & CanvasMask::Header) != 0)
        headerSize_ = {scale_, sh, headerSizeSrc_};

    if (isGeo || (cMask & CanvasMask::EsseStat) != 0)
        esseStatSize_ = {height, scale_, sh, esseStatSizeSrc_};

    //-----------------------------------------------------------------------//

    item->setText("Q*   10; 50' 24\" j");
    item->setFont(font_.canvasSym);

    if (isGeo || (cMask & CanvasMask::CrdTable) != 0)
        crdTableSize_ = {width, scale_, item->boundingRect(), crdTableSizeSrc_};

    if (isGeo || (cMask & CanvasMask::LineBar) != 0)
        lineBarSize_ = {scale_, crdTableSize_, lineBarSizeSrc_};

    //-----------------------------------------------------------------------//

    sh = 1.4 * font_.canvasTxt.pointSizeF();

    if (isGeo || (cMask & CanvasMask::AspStat) != 0)
        aspStatSize_ = {width, height, scale_, sh, aspStatSizeSrc_};

    if (isGeo || (cMask & CanvasMask::CoreStat) != 0)
        coreStatSize_ = {width, height, scale_, sh, coreStatSizeSrc_};

    //-----------------------------------------------------------------------//

    if (isGeo || (cMask & CanvasMask::Circle) != 0)
    {
        mapBaseSize_ = {width, height, scale_, circleSizeSrc_};
        cuspidSize_ = {height, scale_, circleSizeSrc_};
        planetSize_ = {0.5 * mapBaseSize_.insRect.width(), scale_, circleSizeSrc_};
        aspectSize_ = {planetSize_.insRad, scale_};
    }
}


void Canvas::onChangeBkg()
{
    setStyleSheet(colorSrc_.baseBkg());
}


auto Canvas::resizeEvent(QResizeEvent* event) -> void
{
    timer_->start(50);
    QGraphicsView::resizeEvent(event);
    emit sizeChanged(QString("%1x%2").arg(width()).arg(height()));
}


auto Canvas::renderHeader(bool mode) -> void
{
    cleanGroup(group_.header);

    if (!mode)
        return;

    auto& moonDay (kernel_.getMoonDay());

    auto dataItem (new QGraphicsSimpleTextItem(kernel_.getHeaderStr()));
    dataItem->setFont(font_.canvasTxt);
    dataItem->setBrush(colorSrc_.textFg);
    dataItem->setPos(headerSize_.begPos);
    group_.header->addToGroup(dataItem);

    auto extItem (new QGraphicsSimpleTextItem(moonDay[0]));
    extItem->setFont(font_.canvasTxt);
    extItem->setBrush(colorSrc_.textFg);
    extItem->setPos(headerSize_.extPos);
    extItem->setToolTip(moonDay[1]);
    group_.header->addToGroup(extItem);

    cmap_.header = {headerSize_.begPos, dataItem->boundingRect().size()};
}


auto Canvas::renderRecepTree(bool mode) -> void
{
    cleanGroup(group_.recepTree);

    if (!mode)
        return;

    auto& recepTree (kernel_.getRecepTree());
    auto& planetCatalog (kernel_.getPlanetCatalog());

    auto strPos (recepSize_.begPos);
    auto strNum (recepTree.total / recepSize_.maxLen);

    if (recepTree.total % recepSize_.maxLen > 0)
        ++strNum;

    strPos.ry() -= 0.5 * recepSize_.offset * strNum;

    auto index (0);
    auto count (0);

    QString buffer;
    QString symbol ("=÷+-×");

    for (auto& branch : recepTree)
    {
        for (auto& i : branch)
        {
            if (count == recepSize_.maxLen)
            {
                buffer.remove(buffer.size() - 2, 2);
                auto recepItem (new QGraphicsSimpleTextItem(buffer));
                recepItem->setFont(font_.canvasSym);
                recepItem->setBrush(colorSrc_.textFg);
                recepItem->setPos(strPos);
                group_.recepTree->addToGroup(recepItem);

                count = 0;
                strPos.ry() += recepSize_.offset;
                buffer.clear();
            }

            buffer += QString("%1%2%3, ")
                .arg(planetCatalog.at(i.less))
                .arg(symbol[index])
                .arg(planetCatalog.at(i.more));
            ++count;
        }
        ++index;
    }

    if (!buffer.isEmpty())
    {
        buffer.remove(buffer.size() - 2, 2);
        auto recepItem (new QGraphicsSimpleTextItem(buffer));
        recepItem->setFont(font_.canvasSym);
        recepItem->setBrush(colorSrc_.textFg);
        recepItem->setPos(strPos);
        group_.recepTree->addToGroup(recepItem);
    }
}


auto Canvas::renderEsseStat(bool mode) -> void
{
    cleanGroup(group_.esseStat);

    if (!mode)
        return;

    auto& planetCatalog (kernel_.getPlanetCatalog());
    auto& aspTabSpec (kernel_.getAspTabSpec());
    auto& ashaSpec (kernel_.getAshaSpec());
    auto& cosmicSpec (kernel_.getCosmicSpec());

    std::array<QString, 15> desc {
        tr("In heart"), tr("In burning"), tr("Sun zone"), tr("Free zone"),
        tr("Antaeus"), tr("Icarus"), tr("Atlas"), tr("Sisyphus"),
        tr("Rex aspectarius"), tr("In pit"),
        tr("Doriphoros"), tr("Auriga"),
        tr("Anareta"), tr("Alcocoden"), tr("Dominus Genitura")
    };
    std::array<const std::set<int>*, 15> data {
        trigger_.bySunSpec && !cosmicSpec.heart.empty() ? &cosmicSpec.heart : nullptr,
        trigger_.bySunSpec && !cosmicSpec.burning.empty() ? &cosmicSpec.burning : nullptr,
        trigger_.bySunSpec && !cosmicSpec.sunZone.empty() ? &cosmicSpec.sunZone : nullptr,
        trigger_.bySunSpec && !cosmicSpec.freeZone.empty() ? &cosmicSpec.freeZone : nullptr,
        trigger_.byEsseSpec && !cosmicSpec.antaeus.empty() ? &cosmicSpec.antaeus : nullptr,
        trigger_.byEsseSpec && !cosmicSpec.icarus.empty() ? &cosmicSpec.icarus : nullptr,
        trigger_.byEsseSpec && !cosmicSpec.atlas.empty() ? &cosmicSpec.atlas : nullptr,
        trigger_.byEsseSpec && !cosmicSpec.sisyphus.empty() ? &cosmicSpec.sisyphus : nullptr,
        trigger_.byAspSpec && !aspTabSpec.rexAsp.empty() ? &aspTabSpec.rexAsp : nullptr,
        trigger_.byAspSpec && !aspTabSpec.inPit.empty() ? &aspTabSpec.inPit : nullptr,
        trigger_.dorAuriga && !cosmicSpec.doriph.empty() ? &cosmicSpec.doriph : nullptr,
        trigger_.dorAuriga && !cosmicSpec.auriga.empty() ? &cosmicSpec.auriga : nullptr,
        trigger_.byAshaSpec && !ashaSpec.anareta.empty() ? &ashaSpec.anareta : nullptr,
        trigger_.byAshaSpec && !ashaSpec.alcocoden.empty() ? &ashaSpec.alcocoden : nullptr,
        trigger_.byAshaSpec && !ashaSpec.dGenitura.empty() ? &ashaSpec.dGenitura : nullptr
    };

    auto space (false);
    for (auto i (0); i < 4; ++i)
        if (data[i] != nullptr)
        {
            space = true;
            break;
        }
    if (space)
    {
        space = false;
        for (auto i (4); i < 15; ++i)
            if (data[i] != nullptr)
            {
                space = true;
                break;
            }
    }

    auto descPos (esseStatSize_.descPos);
    auto botRight (esseStatSize_.descPos);

    for (auto i (14); i >= 0; --i)
    {
        descPos.ry() -= esseStatSize_.offset;

        if (i == 3 && space)
            descPos.ry() -= esseStatSize_.offset;

        if (data[i] == nullptr)
        {
            descPos.ry() += esseStatSize_.offset;
            continue;
        }

        auto descItem (new QGraphicsSimpleTextItem(desc[i] + " "));
        descItem->setFont(font_.canvasTxt);
        descItem->setBrush(colorSrc_.textFg);
        descItem->setPos(descPos);
        group_.esseStat->addToGroup(descItem);

        QString buffer;
        for (auto j : *data[i])
            buffer += planetCatalog.at(j);

        auto dataPos (descPos + descItem->boundingRect().topRight());
        dataPos.ry() += esseStatSize_.crutch;

        auto dataItem (new QGraphicsSimpleTextItem(buffer));
        dataItem->setFont(font_.canvasSym);
        dataItem->setBrush(colorSrc_.textFg);
        dataItem->setPos(dataPos);
        group_.esseStat->addToGroup(dataItem);

        auto right (dataPos.x() + dataItem->boundingRect().width());
        if (right > botRight.x())
            botRight.rx() = right;
    }

    cmap_.esseStat = {descPos, botRight};
}


auto Canvas::renderCircle(bool mode) -> void
{
    cleanGroup(group_.circle);

    if (!mode)
    {
        cleanGroup(group_.aspects);
        return;
    }

    zeroAng_ = trigger_.zeroAngAsc ? kernel_.getCuspidCrd()[0] : 0;
    cmap_.circle = mapBaseSize_.outRect;

    renderMapBase();

    if (trigger_.circleCuspids)
        renderCuspids();

    renderPlanets();
    renderAspects();
}


auto Canvas::renderMapBase() -> void
{
    const auto& [outRect, signRect, capRect, insRect, symRad] (mapBaseSize_);

    auto circleItem (new QGraphicsEllipseItem(outRect));
    circleItem->setPen(pen_.borderFg);
    circleItem->setBrush(colorSrc_.circleBkg);
    group_.circle->addToGroup(circleItem);

    for (auto i (0), j (0); i < 12; ++i, ++j)
    {
        if (j == 4)
            j -= 4;

        auto ang (zeroAng_ - 30 * i);
        auto pos (zeroPos_ + symRad * QPointF(cosDeg(ang - 15), -sinDeg(ang - 15)));

        auto signItem (new QGraphicsEllipseItem(signRect));
        signItem->setPen(pen_.borderFg);
        signItem->setBrush(colorSrc_.signBkg[j]);
        signItem->setStartAngle(16 * ang);
        signItem->setSpanAngle(-480);
        group_.circle->addToGroup(signItem);

        auto symItem (new QGraphicsSimpleTextItem(RefBook::signSym[i]));
        symItem->setFont(font_.signSym);
        symItem->setBrush(colorSrc_.signSymFg);
        symItem->setPos(pos - symItem->boundingRect().center());
        group_.circle->addToGroup(symItem);
    }

    circleItem = new QGraphicsEllipseItem(capRect);
    circleItem->setPen(pen_.borderFg);
    circleItem->setBrush(colorSrc_.circleBkg);
    group_.circle->addToGroup(circleItem);

    circleItem = new QGraphicsEllipseItem(insRect);
    circleItem->setPen(pen_.borderFg);
    circleItem->setBrush(colorSrc_.aspFieldBkg);
    group_.circle->addToGroup(circleItem);
}


auto Canvas::renderCuspids() -> void
{
    auto& cuspidCrd (kernel_.getCuspidCrd());
    auto& cuspidDegree (kernel_.getCuspidDegree());
    auto& cuspidCrdStr (kernel_.getCuspidCrdStr());

    auto cuspidLineFg (QPen(colorSrc_.cuspLineFg));
    auto [endR2, endR1, begR2, begR1, tipAng, arrAdd, arrLen, bedLen] (cuspidSize_);

    for (auto i (0); i < 12; ++i)
    {
        auto ang (360 + zeroAng_ - cuspidCrd[i]);
        if (ang >= 360)
            ang -= 360;

        auto shift (QPointF(cosDeg(ang), -sinDeg(ang)));
        auto endP2 (zeroPos_ + endR2 * shift);

        auto begItem (new QGraphicsLineItem({zeroPos_ + begR1 * shift, zeroPos_ + begR2 * shift}));
        auto endItem (new QGraphicsLineItem({zeroPos_ + endR1 * shift, endP2}));

        switch (i) {
        case 0 :
        case 9 :
        {
            cuspidLineFg.setWidthF(2 * scale_);

            auto endP3 (endP2 + arrAdd * shift);
            auto angL (ang - 180 - tipAng);
            auto angR (ang + 180 + tipAng);

            auto arrItem (new QGraphicsPolygonItem({
                endP3, endP3 + arrLen * QPointF(cosDeg(angL), -sinDeg(angL)),
                endP2, endP3 + arrLen * QPointF(cosDeg(angR), -sinDeg(angR))}));
            arrItem->setPen(cuspidLineFg);
            arrItem->setBrush(cuspidLineFg.brush());
            group_.circle->addToGroup(arrItem);
        }
            break;
        case 3 :
        case 6 :
        {
            cuspidLineFg.setWidthF(2 * scale_);

            auto angL (ang + 60 + tipAng);
            auto angR (ang - 60 - tipAng);

            auto bedItem (new QGraphicsLineItem(
                {endP2, endP2 + bedLen * QPointF(cosDeg(angL), -sinDeg(angL))}));
            bedItem->setPen(cuspidLineFg);
            group_.circle->addToGroup(bedItem);

            bedItem = new QGraphicsLineItem(
                {endP2, endP2 + bedLen * QPointF(cosDeg(angR), -sinDeg(angR))});
            bedItem->setPen(cuspidLineFg);
            group_.circle->addToGroup(bedItem);
        }
            break;
        default :
            cuspidLineFg.setWidthF(scale_);
        }

        begItem->setPen(cuspidLineFg);
        endItem->setPen(cuspidLineFg);
        group_.circle->addToGroup(begItem);
        group_.circle->addToGroup(endItem);

        auto symItem (new QGraphicsSimpleTextItem(RefBook::cuspidSym[i]));
        symItem->setFont(font_.cuspidSym);
        symItem->setBrush(colorSrc_.cuspSymFg);
        symItem->setToolTip(cuspidCrdStr[i][1]);

        auto symRect (symItem->boundingRect());
        auto symAng (ang + 180 + (ang > 179.99 && ang < 359.99 ? -90 : 90));
        auto symPos (endP2 + symRect.height() * QPointF(cosDeg(symAng), -sinDeg(symAng)) - symRect.center());

        symItem->setPos(symPos);
        group_.circle->addToGroup(symItem);

        if (trigger_.circleDegrees)
        {
            auto supItem (new QGraphicsSimpleTextItem(QString("%1").arg(cuspidDegree[i])));
            supItem->setFont(font_.cuspidSymSup);
            supItem->setBrush(colorSrc_.cuspSymFg);

            switch (i) { // font crutch
            case 3 :
                symPos.rx() -= scale_;
                break;
            case 10 :
                symPos.rx() -= 2 * scale_;
                break;
            default :
                symPos.rx() += scale_;
            }

            supItem->setPos(symPos + symRect.topRight());
            group_.circle->addToGroup(supItem);
        }
    }
}


auto Canvas::renderPlanets() -> void
{
    auto& planetCatalog (kernel_.getPlanetCatalog());
    auto& planetCrd (kernel_.getPlanetCrd());
    auto& planetSignNo (kernel_.getPlanetSignNo());
    auto& planetDegree (kernel_.getPlanetDegree());
    auto& normalizeCrd (kernel_.getNormalizeCrd());
    auto& planetCrdStr (kernel_.getPlanetCrdStr());
    auto& planetSpeedSym (kernel_.getPlanetSpeedSym());

    auto brush (colorSrc_.planetSymFg);
    auto [insRad, symRad, markRect] (planetSize_);

    for (auto i : kernel_.getPlanetOrder())
    {
        auto insAng (360 + zeroAng_ - planetCrd.at(i));
        auto symAng (360 + zeroAng_ - normalizeCrd.at(i));

        if (insAng >= 360)
            insAng -= 360;
        if (symAng >= 360)
            symAng -= 360;

        auto insPos (zeroPos_ + insRad * QPointF(cosDeg(insAng), -sinDeg(insAng)));
        auto symPos (zeroPos_ + symRad * QPointF(cosDeg(symAng), -sinDeg(symAng)));

        QLineF markLine (insPos, symPos);
        markLine.setLength(0.5 * markLine.length());
        markRect.moveCenter(insPos);

        auto mark1Item (new QGraphicsLineItem(markLine));
        mark1Item->setPen(pen_.planetLineFg);
        group_.circle->addToGroup(mark1Item);

        auto mark2Item (new QGraphicsEllipseItem(markRect));
        mark2Item->setPen(pen_.borderFg);
        mark2Item->setBrush(colorSrc_.aspFieldBkg);
        group_.circle->addToGroup(mark2Item);

        if (trigger_.circleColors)
            brush = colorSrc_.getSpec({i, planetSignNo.at(i)});

        auto symItem (new QGraphicsSimpleTextItem(planetCatalog.at(i)));
        symItem->setFont(font_.planetSym);
        symItem->setBrush(brush);
        symItem->setToolTip(planetCrdStr.at(i)[1]);
        auto symRect (symItem->boundingRect());
        symItem->setPos(symPos -= symRect.center());
        group_.circle->addToGroup(symItem);

        symPos += symRect.topRight();

        QPointF crutch;

        switch (i) {
        case 1 :
        case 2 :
        case 3 :
        case 5 :
        case 7 :
        case 12 :
        case 15 :
        case 56 :
            crutch.rx() -= 2 * scale_;
        }

        if (trigger_.circleDegrees)
        {
            auto degree (planetDegree.at(i));

            auto supItem (new QGraphicsSimpleTextItem(QString("%1").arg(degree)));
            supItem->setFont(font_.planetSymSup);
            supItem->setBrush(brush);
            supItem->setPos(symPos + crutch);
            group_.circle->addToGroup(supItem);

            if (degree > 9)
                crutch.rx() += 0.25 * supItem->boundingRect().width();
        }
        else if (i == 5)
            crutch.rx() += scale_;

        if (trigger_.circleSpeeds)
        {
            auto subItem (new QGraphicsSimpleTextItem(planetSpeedSym.at(i)));
            subItem->setFont(font_.planetSymSub);
            subItem->setBrush(brush);
            symPos.ry() += symRect.height() - 1.15 * subItem->boundingRect().height();
            subItem->setPos(symPos + crutch);
            group_.circle->addToGroup(subItem);
        }
    }
}


auto Canvas::renderAspects(QGraphicsItem* item) -> void
{
    if (!trigger_.viewCircle)
        return;

    auto isNormal (item == nullptr);
    auto group (isNormal ? group_.aspects : group_.markers);

    cleanGroup(group);

    auto& planetCrd (kernel_.getPlanetCrd());
    auto& aspCatalog (kernel_.getAspCatalog());
    auto& aspPen (kernel_.getAspPen());
    auto& aspType (kernel_.getAspType());
    auto& aspGroup (isNormal ? AspTable::aspGroup.normal : std::vector<int>{0});
    auto& aspSample (kernel_.getAspSample(trigger_.aspCfgBonesOnly, item));

    auto pen (isNormal ? QPen() : colorSrc_.aspCfgMarkFg);
    auto widthA (isNormal ? scale_ : 3 * scale_);
    auto [insRad, widthB, unionRect] (aspectSize_);

    for (auto key : aspGroup)
        for (auto& i : aspSample[key])
        {
            auto asp (i.second->asp);

            if (isNormal)
            {
                pen = aspPen.at(asp);

                auto pattern (pen.dashPattern());
                if (!pattern.empty())
                {
                    for (auto& j : pattern)
                        j *= scale_;

                    pen.setDashPattern(pattern);
                }
            }
            pen.setWidthF(asp == 0 ? widthB : widthA);

            auto angA (360 + zeroAng_ - planetCrd.at(i.first.less));
            auto angB (360 + zeroAng_ - planetCrd.at(i.first.more));

            if (angA >= 360)
                angA -= 360;
            if (angB >= 360)
                angB -= 360;
            if (angA < angB)
                std::swap(angA, angB);

            QLineF line (
                zeroPos_ + insRad * QPointF(cosDeg(angA), -sinDeg(angA)),
                zeroPos_ + insRad * QPointF(cosDeg(angB), -sinDeg(angB)));

            if (line.length() <= unionRect.width())
            {
                unionRect.moveCenter(line.pointAt(0.5));

                auto aspItem (new QGraphicsEllipseItem(unionRect));
                aspItem->setPen(pen);
                aspItem->setBrush(pen.brush());
                group->addToGroup(aspItem);
            }
            else
            {
                auto aspItem (new QGraphicsLineItem(line));
                aspItem->setPen(pen);
                group->addToGroup(aspItem);
            }

            if (asp != 0 && asp != 180)
            {
                auto center (line.pointAt(0.5));
                auto fontId (aspType.at(asp));
                auto aspStr (aspCatalog.at(asp));

                auto symItem (new QGraphicsSimpleTextItem(aspStr));
                symItem->setFont(font_[fontId]);
                symItem->setBrush(pen.brush());

                auto rect (symItem->boundingRect());

                if (fontId == 0)
                    rect.setSize(1.1 * rect.size());
                else
                {
                    rect.setSize(0.9 * rect.size());
                    if (aspStr == "r" || aspStr == "t")
                        rect.setY(rect.y() - scale_);
                }

                symItem->setPos(center - rect.center());
                rect.moveCenter(center);

                auto cleanItem (new QGraphicsEllipseItem(rect));
                cleanItem->setPen(Qt::NoPen);
                cleanItem->setBrush(colorSrc_.aspFieldBkg);

                group->addToGroup(cleanItem);
                group->addToGroup(symItem);
            }
        }
}


auto Canvas::renderCrdTable(bool mode) -> void
{
    cleanGroup(group_.crdTable);

    if (!mode)
        return;

    auto& planetCatalog (kernel_.getPlanetCatalog());
    auto& planetCrd (kernel_.getPlanetCrd());
    auto& planetSignNo (kernel_.getPlanetSignNo());
    auto& planetSpeedSym (kernel_.getPlanetSpeedSym());
    auto& planetCrdStr (kernel_.getPlanetCrdStr());
    auto& cuspidCrd (kernel_.getCuspidCrd());
    auto& cuspidCrdStr (kernel_.getCuspidCrdStr());
    auto& planetStarTable (kernel_.getPlanetStarTable());
    auto& cuspidStarTable (kernel_.getCuspidStarTable());

    const auto& baseFg (colorSrc_.spec[SpecState::Neutral]);
    auto [step, symX, crdX, kadX, starX, posY, maxX] (crdTableSize_);

    for (auto i : kernel_.getPlanetOrder())
    {
        auto& strFg (colorSrc_.getSpec({i, planetSignNo.at(i)}));

        auto symItem (new QGraphicsSimpleTextItem(planetCatalog.at(i) + planetSpeedSym.at(i)));
        symItem->setFont(font_.canvasSym);
        symItem->setBrush(strFg);
        symItem->setPos(symX, posY);
        group_.crdTable->addToGroup(symItem);

        auto crdItem (new QGraphicsSimpleTextItem(planetCrdStr.at(i)[0]));
        crdItem->setFont(font_.canvasSym);
        crdItem->setBrush(strFg);
        crdItem->setPos(crdX, posY);
        group_.crdTable->addToGroup(crdItem);

        auto kad (getKadData(planetCrd.at(i)));
        if (kad.second != nullptr)
        {
            auto kadItem (new QGraphicsSimpleTextItem(kad.first));
            kadItem->setFont(font_.canvasSym);
            kadItem->setBrush(*kad.second);
            kadItem->setPos(kadX, posY);
            group_.crdTable->addToGroup(kadItem);
        }

        if (trigger_.fixedStars)
        {
            auto& cell (planetStarTable.at(i));
            if (!cell[0].isEmpty())
            {
                auto starItem (new QGraphicsSimpleTextItem(cell[0]));
                starItem->setFont(font_.fixedStar);
                starItem->setBrush(baseFg);
                starItem->setToolTip(cell[1]);
                starItem->setPos(starX, posY);
                group_.crdTable->addToGroup(starItem);
            }
        }

        posY += step;
    }

    posY += 0.5 * step;

    for (auto i (0); i < 12; ++i)
    {
        auto symItem (new QGraphicsSimpleTextItem(RefBook::cuspidSym[i]));
        symItem->setFont(font_.canvasSym);
        symItem->setBrush(baseFg);

        auto crutch (0.0);
        switch (i) {
        case 0 :
        case 3 :
        case 6 :
        case 9 :
            break;
        case 10 :
            crutch = -0.14 * symItem->boundingRect().width();
            break;
        case 11 :
            crutch = -0.15 * symItem->boundingRect().width();
            break;
        default :
            crutch = 0.23 * symItem->boundingRect().width();
        }

        symItem->setPos(symX + crutch, posY);
        group_.crdTable->addToGroup(symItem);

        auto crdItem (new QGraphicsSimpleTextItem(cuspidCrdStr[i][0]));
        crdItem->setFont(font_.canvasSym);
        crdItem->setBrush(baseFg);
        crdItem->setPos(crdX, posY);
        group_.crdTable->addToGroup(crdItem);

        auto kad (getKadData(cuspidCrd[i]));
        if (kad.second != nullptr)
        {
            auto kadItem (new QGraphicsSimpleTextItem(kad.first));
            kadItem->setFont(font_.canvasSym);
            kadItem->setBrush(*kad.second);
            kadItem->setPos(kadX, posY);
            group_.crdTable->addToGroup(kadItem);
        }

        if (trigger_.fixedStars)
        {
            auto& cell (cuspidStarTable[i]);
            if (!cell[0].isEmpty())
            {
                auto starItem (new QGraphicsSimpleTextItem(cell[0]));
                starItem->setFont(font_.fixedStar);
                starItem->setBrush(baseFg);
                starItem->setToolTip(cell[1]);
                starItem->setPos(starX, posY);
                group_.crdTable->addToGroup(starItem);
            }
        }

        posY += step;
    }

    cmap_.crdTable = {
        QPointF(crdTableSize_.symX, crdTableSize_.posY),
        QPointF(trigger_.fixedStars ? maxX : starX, posY)};
}


auto Canvas::renderLineBar(bool mode) -> void
{
    cleanGroup(group_.lineBar);

    if (!mode)
        return;

    auto& planetStat (kernel_.getPlanetStat());

    auto pen (pen_.baseSet());
    auto scale (lineBarSize_.ashaScale);
    auto uLine (lineBarSize_.uLine);
    auto dLine (lineBarSize_.dLine);

    if (trigger_.modeAsha)
    {
        if (trigger_.ashaPowKind)
        {
            pen[0] = &pen_.ashaPower;
            pen[1] = &pen_.ashaPower;
        }
        else if (trigger_.ashaPowCreat)
            pen = pen_.ashaSet();
    }
    else if (trigger_.modePrime)
        scale = lineBarSize_.primeScale;
    else
        scale = lineBarSize_.cosmicScale;

    auto dItem (new QGraphicsLineItem);
    for (auto i : kernel_.getPlanetOrder())
    {
        auto stat (planetStat.find(i));
        if (stat == planetStat.end())
        {
            uLine.translate(lineBarSize_.offset);
            dLine.translate(lineBarSize_.offset);
            continue;
        }

        uLine.setLength(scale * std::abs(stat->second[0]) + 1e-3);
        dLine.setLength(scale * std::abs(stat->second[1]) + 1e-3);

        auto uItem (new QGraphicsLineItem(uLine));
        dItem = new QGraphicsLineItem(dLine);

        uItem->setPen(stat->second[0] > 0 ? *pen[0] : *pen[1]);
        dItem->setPen(stat->second[1] > 0 ? *pen[2] : *pen[3]);

        if (stat->second[0] != 0)
            group_.lineBar->addToGroup(uItem);
        if (stat->second[1] != 0)
            group_.lineBar->addToGroup(dItem);

        auto isPoint (false);

        switch (i) {
        case 11 :
            isPoint = (ForceFlag::Dominant == checkRelation(stat->second, planetStat.at(24)));
            break;
        case 12 :
            isPoint = (ForceFlag::Dominant == checkRelation(stat->second, planetStat.at(56)));
            break;
        case 24 :
            isPoint = (ForceFlag::Dominant == checkRelation(stat->second, planetStat.at(11)));
            break;
        case 56 :
            isPoint = (ForceFlag::Dominant == checkRelation(stat->second, planetStat.at(12)));
            break;
        }

        if (isPoint)
        {
            auto point (lineBarSize_.point);
            point.moveCenter(
                (stat->second[0] != 0 ? uLine.p2() : dLine.p2()) - 1.5 * point.topRight());

            auto pointItem (new QGraphicsEllipseItem(point));
            pointItem->setPen({Qt::NoPen});
            pointItem->setBrush((stat->second[0] != 0 ? uItem : dItem)->pen().brush());
            group_.lineBar->addToGroup(pointItem);
        }

        uLine.translate(lineBarSize_.offset);
        dLine.translate(lineBarSize_.offset);
    }

    cmap_.lineBar = {lineBarSize_.topLeft, dItem->line().p1() + 0.49 * lineBarSize_.offset};

    if (trigger_.modePrime || trigger_.modeCosmic)
    {
        auto kv (trigger_.modePrime ? 7 : 10);
        auto p1 (dItem->line().p1());
        auto p2 (lineBarSize_.uLine.p1());
        auto dy (0.5 * pen[0]->widthF());

        p1.rx() -= kv * scale * scale_;
        p2.rx()  = p1.x();
        p1.ry() += dy;
        p2.ry() -= dy;

        auto markLine (new QGraphicsLineItem({p1, p2}));
        markLine->setPen(pen_.markLine);
        group_.lineBar->addToGroup(markLine);

        auto markText (new QGraphicsSimpleTextItem(QString("%1").arg(kv)));
        markText->setFont(font_.markLine);
        markText->setBrush(pen_.markLine.brush());

        auto rect (markText->boundingRect());
        markText->setPos(p2 - rect.center() - 0.5 * rect.bottomLeft());
        group_.lineBar->addToGroup(markText);

        if (trigger_.modeCosmic)
        {
            auto sum (std::round(kernel_.getCosmicSum()));
            auto mask (tr("Total") + " %1%2");

            auto sumItem (new QGraphicsSimpleTextItem(mask.arg(sum > 0 ? "+" : "").arg(sum)));
            sumItem->setFont(font_.markLine);
            sumItem->setBrush(pen_.markLine.brush());

            auto rect (sumItem->boundingRect());
            sumItem->setPos(p1 - rect.center() + rect.bottomLeft());
            group_.lineBar->addToGroup(sumItem);
        }
    }
}


auto Canvas::renderAspCfg(bool mode) -> void
{
    cleanGroup(group_.aspCfg);

    if (!mode)
        return;

    auto& planetCatalog (kernel_.getPlanetCatalog());
    auto& aspCfgBone (kernel_.getAspCfgBone());
    auto& maxLen (kernel_.getAspCfgOffset());

    auto isBone (trigger_.aspCfgBonesOnly);
    auto [topMargin, symWidth, shiftLen, baseX, offset, crutch] (aspCfgSize_);

    if (topMargin < 12)
        baseX = symWidth * (maxLen[isBone] > 6 ? shiftLen - maxLen[isBone] - 3 : shiftLen - 9);

    QPointF basePos (
        crdTableSize_.symX + baseX,
        crdTableSize_.posY + offset.y() * (topMargin + kernel_.getPlanetOrder().size()));
    QPointF beg (1e7, basePos.y());
    QPointF end (0, 0);

    for (auto& i : kernel_.getAspCfgTable())
        for (auto& cfg : i.second.first)
        {
            QString str ("   ");
            if (isBone)
                for (auto j : aspCfgBone.at(cfg).first)
                    str += planetCatalog.at(j);
            else
                for (auto j : cfg)
                    str += planetCatalog.at(j);

            QVariantList list {cfg.begin(), cfg.end()};

            auto descItem (new QGraphicsSimpleTextItem(i.second.second->title));
            descItem->setData(0, i.first);
            descItem->setData(1, list);
            descItem->setFont(font_.canvasTxt);
            descItem->setBrush(colorSrc_.textFg);
            descItem->setCursor(Qt::PointingHandCursor);
            descItem->setAcceptHoverEvents(true);
            descItem->setPos(basePos - descItem->boundingRect().topRight() - crutch);
            group_.aspCfg->addToGroup(descItem);

            auto leftX (descItem->pos().x());
            if (leftX < beg.x())
                beg.setX(leftX);

            auto dataItem (new QGraphicsSimpleTextItem(str));
            dataItem->setData(0, i.first);
            dataItem->setData(1, list);
            dataItem->setFont(font_.canvasSym);
            dataItem->setBrush(colorSrc_.textFg);
            dataItem->setCursor(Qt::PointingHandCursor);
            dataItem->setAcceptHoverEvents(true);
            dataItem->setPos(basePos);
            group_.aspCfg->addToGroup(dataItem);

            auto rightX (basePos.x() + dataItem->boundingRect().width());
            if (rightX > end.x())
                end.setX(rightX);

            descItem->installSceneEventFilter(itemEventFilter_);
            dataItem->installSceneEventFilter(itemEventFilter_);

            basePos += offset;
        }

    basePos.setX(0);
    cmap_.aspCfg = {beg, end + basePos - 0.13 * offset};
}


auto Canvas::renderAspStat(bool mode) -> void
{
    cleanGroup(group_.aspStat);

    if (!mode)
        return;

    auto& aspField (kernel_.getAspField());

    auto fg (pen_.textFg);
    auto color (fg.color());
    auto base (aspStatSize_.base);

    for (auto i (0), j (21); i < 6; ++i, ++j)
    {
        if (i == 0 || i == 5)
            fg.setColor(color);
        else
            fg.setColor(colorSrc_.force[aspField[i].second]);

        group_.aspStat->addToGroup(symbol(j, fg, base));

        auto num (i == 5 ? aspField[0].second : std::round(aspField[i].first));
        auto numItem (new QGraphicsSimpleTextItem(QString("%1").arg(num, 2)));
        numItem->setFont(font_.canvasTxt);
        numItem->setBrush(fg.brush());
        numItem->setPos(base.bottomLeft() + aspStatSize_.offset);
        group_.aspStat->addToGroup(numItem);

        base.moveLeft(base.left() + aspStatSize_.step);
    }
}


auto Canvas::renderCoreStat(bool mode) -> void
{
    cleanGroup(group_.coreStat);

    if (!mode)
        return;

    auto  fg (pen_.textFg);
    auto  inv (trigger_.coreStatInvert);
    auto& base (coreStatSize_.base);

    for (auto i (0); i < 2; ++i)
    {
        if (!*(&trigger_.coreStatCosEnb + i))
            continue;

        auto& coreForce (kernel_.getCoreForce(i));
        auto& mapType (kernel_.getMapType(i));

        auto view (i == 0);
        auto eScale (coreForce.elemScale());
        auto bScale (coreForce.baseScale());

        coreStatSize_.resetBase(view, inv);

        auto title (new QGraphicsSimpleTextItem(view ? tr("Cosmogram") : tr("Horoscope")));
        title->setFont(font_.canvasTxt);
        title->setBrush(colorSrc_.textFg);
        title->setPos(base.topLeft());
        group_.coreStat->addToGroup(title);

        base.moveTop(base.top() + coreStatSize_.offset);

        const std::pair<double, int>* src;
        for (auto j (0); j < 21; ++j)
        {
            switch (j) {
            case 4 :
            case 11 :
            case 18 :
                base.moveLeft(base.left() + 0.5 * coreStatSize_.offset);
                break;
            case 7 :
            case 14 :
                coreStatSize_.moveBase(1.9, view, inv);
            }

            switch (j) {
            case 0 :
            case 1 :
            case 2 :
            case 3 :
                src = &coreForce.elem[j];
                break;
            case 4 :
            case 5 :
            case 6 :
                src = &coreForce.cros[j-4];
                break;
            case 7 :
            case 8 :
            case 9 :
            case 10 :
                src = &coreForce.quad[j-7];
                break;
            case 11 :
            case 12 :
            case 13 :
                src = &coreForce.zone[j-11];
                break;
            case 14 :
            case 15 :
                src = &coreForce.hsNS[j-14];
                break;
            case 16 :
            case 17 :
                src = &coreForce.hsEW[j-16];
                break;
            case 18 :
            case 19 :
            case 20 :
                src = &coreForce.base[j-18];
            }

            fg.setColor(colorSrc_.force[src->second]);
            group_.coreStat->addToGroup(symbol(j, fg, base));

            auto numItem (new QGraphicsSimpleTextItem(
                QString("%1").arg(std::round((j > 17 ? bScale : eScale) * src->first), 2)));
            numItem->setFont(font_.canvasTxt);
            numItem->setBrush(fg.brush());
            numItem->setPos(base.bottomLeft() + coreStatSize_.numDp);
            group_.coreStat->addToGroup(numItem);

            base.moveLeft(base.left() + coreStatSize_.offset);
        }

        coreStatSize_.moveBase(1.7, view, inv);

        auto coreStr (tr("Core") + " :  " + coreForce.type + " ");
        auto sum (std::round(coreForce.total));

        coreStr += QString("%2%3").arg(sum > 0 ? "+" : "").arg(sum);
        if (mapType.second != NONE)
            coreStr += ", " + mapType.first;

        auto coreItem (new QGraphicsSimpleTextItem(coreStr));
        coreItem->setFont(font_.canvasTxt);
        coreItem->setBrush(colorSrc_.textFg);
        coreItem->setPos(base.topLeft());
        group_.coreStat->addToGroup(coreItem);
    }

    if (trigger_.coreStatCosEnb || trigger_.coreStatHorEnb)
    {
        cmap_.coreStat = coreStatSize_.rect;

        if (trigger_.coreStatCosEnb && trigger_.coreStatHorEnb)
            return;
        else
        {
            auto& rect (cmap_.coreStat);
            auto  cutX (9.5 * coreStatSize_.offset);

            if (trigger_.coreStatCosEnb)
                inv ? rect.setLeft(rect.left() + cutX) : rect.setRight(rect.right() - cutX);
            else
                inv ? rect.setRight(rect.right() - cutX) : rect.setLeft(rect.left() + cutX);
        }
    }
    else
        cmap_.coreStat = {};
}


auto Canvas::cleanGroup(QGraphicsItemGroup* group) -> void
{
    for(auto item : scene_->items(group->boundingRect()))
        if(item->group() == group)
            delete item;
}


auto Canvas::symbol(int ind, const QPen& fg, const QRectF& base) -> QGraphicsItemGroup*
{
    auto group (new QGraphicsItemGroup);

    switch (ind) {
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    {// Elements
        QPointF p1, p2, p3;

        if (ind == 0 || ind == 2)
        {
            p1 = base.topLeft() + QPointF(0.5 * base.width(), 0);
            p2 = base.bottomLeft();
            p3 = base.bottomRight();
        }
        else
        {
            p1 = base.bottomLeft() + QPointF(0.5 * base.width(), 0);
            p2 = base.topLeft();
            p3 = base.topRight();
        }

        auto symItem (new QGraphicsPolygonItem({p1, p2, p3}));
        symItem->setPen(fg);
        symItem->setBrush(Qt::NoBrush);
        group->addToGroup(symItem);

        if (ind == 2 || ind == 3)
        {
            auto insItem (new QGraphicsLineItem({QLineF(p1, p2).pointAt(0.7), QLineF(p1, p3).pointAt(0.7)}));
            insItem->setPen(fg);
            group->addToGroup(insItem);
        }

        return group;
    }
    case 4 :
    case 5 :
    case 6 :
    case 7 :
    case 8 :
    case 9 :
    case 10 :
    case 21 :
    case 22 :
    case 23 :
    case 24 :
    case 25 :
    case 26 :
    {// Square symbols
        auto symItem (new QGraphicsRectItem(base));
        symItem->setPen(fg);
        symItem->setBrush(Qt::NoBrush);
        group->addToGroup(symItem);

        break;
    }
    case 11 :
    case 12 :
    case 13 :
    case 14 :
    case 15 :
    case 16 :
    case 17 :
    case 18 :
    case 19 :
    case 20 :
        // Round symbols
        auto symItem (new QGraphicsEllipseItem(base));
        symItem->setPen(fg);
        symItem->setBrush(Qt::NoBrush);
        group->addToGroup(symItem);
    }

    switch (ind) {
    case 4 :
    case 18 :
    {// Point
        QRectF point (0, 0, 2 * scale_, 2 * scale_);
        point.moveCenter(base.center());
        auto insItem (new QGraphicsEllipseItem(point));
        insItem->setPen(fg);
        insItem->setBrush(fg.brush());
        group->addToGroup(insItem);

        break;
    }
    case 5 :
    case 19 :
    case 6 :
    case 20 :
    {
        auto rect = base.adjusted(1.5 * scale_, 1.5 * scale_, -1.5 * scale_, -1.5 * scale_);
        QPointF offset (0.5 * rect.width(), 0);
        QPointF p1 (rect.topLeft() + offset);
        QPointF p2 (rect.center() + offset);
        QPointF p3 (rect.bottomLeft() + offset);
        QPointF p4 (rect.center() - offset);

        if (ind == 5 || ind == 19)
        {// Cross
            auto insItem (new QGraphicsLineItem({p1, p3}));
            insItem->setPen(fg);
            group->addToGroup(insItem);
            insItem = new QGraphicsLineItem({p2, p4});
            insItem->setPen(fg);
            group->addToGroup(insItem);
        }
        else
        {// Romb
            auto insItem (new QGraphicsPolygonItem({p1, p2, p3, p4}));
            insItem->setPen(fg);
            insItem->setBrush(Qt::NoBrush);
            group->addToGroup(insItem);
        }

        break;
    }
    case 7 :
    case 8 :
    case 9 :
    case 10 :
    {// Quadrants
        auto rect (base.adjusted(0, 0, -0.5 * base.width(), -0.5 * base.height()));

        switch (ind) {
        case 7 :
            rect.moveBottomRight(base.bottomRight());
            break;
        case 8 :
            rect.moveBottomLeft(base.bottomLeft());
            break;
        case 9 :
            rect.moveTopLeft(base.topLeft());
            break;
        case 10 :
            rect.moveTopRight(base.topRight());
        }

        auto insItem (new QGraphicsRectItem(rect));
        insItem->setPen(fg);
        insItem->setBrush(fg.brush());
        group->addToGroup(insItem);

        break;
    }
    case 11 :
    case 12 :
    case 13 :
    case 14 :
    case 15 :
    case 16 :
    case 17 :
    {// Zones and hemispheres
        auto start (0);
        auto span (0);

        switch (ind) {
        case 11 :
            start = 0;
            span = -1920;
            break;
        case 12 :
            start = -1920;
            span = -1920;
            break;
        case 13 :
            start = -3840;
            span = -1920;
            break;
        case 14 :
            start = -2880;
            span = -2880;
            break;
        case 15 :
            start = 0;
            span = -2880;
            break;
        case 16 :
            start = 1440;
            span = -2880;
            break;
        case 17 :
            start = -1440;
            span = -2880;
        }

        auto insItem (new QGraphicsEllipseItem(base));
        insItem->setPen(fg);
        insItem->setBrush(fg.brush());
        insItem->setStartAngle(start);
        insItem->setSpanAngle(span);
        group->addToGroup(insItem);

        break;
    }
    case 21 :
    case 22 :
    case 23 :
    case 24 :
    case 25 :
    case 26 :
        // Aspect symbols
        auto insItem (new QGraphicsSimpleTextItem(QString("qweQN+")[ind-21]));
        insItem->setBrush(fg.brush());

        QPointF crutch (0, 0);

        if (ind == 24 || ind == 25)
            insItem->setFont(font_.miniSymB);
        else
        {
            insItem->setFont(font_.miniSymA);
            crutch.ry() += scale_;
        }

        insItem->setPos(base.center() - insItem->boundingRect().center() + crutch);
        group->addToGroup(insItem);
    }

    return group;
}


auto Canvas::getKadData(int crd) const -> std::pair<QString, const QBrush*>
{
    auto kad (RefBook::degreeKad.find(crd));

    if (kad != RefBook::degreeKad.end())
    {
        if (kad->second)
            return {"+", &colorSrc_.kingDegFg};
        else
            return {"×", &colorSrc_.destDegFg};
    }

    return {"", nullptr};
}

} // namespace napatahti
