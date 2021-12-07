#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include "swelib/src/swephexp.h"
#include "shared.h"
#include "Person.h"
#include "RefBook.h"
#include "AstroBase.h"
#include "AspPage.h"
#include "AspTable.h"

namespace napatahti {

AspTable::AspTable(const Person& person, const AstroBase& astroBase, const AspPage& aspPage)
    : person_ (person)
    , astroBase_ (astroBase)
    , aspPage_ (aspPage)
{
    loadExtCfg();

    auto ind (0);
    for (auto& i : classic3Point_)
    {
        srcAspConfigTable_[ind] = {&i.first, &i.second};
        ++ind;
    }
    for (auto& i : classic4Point_)
    {
        srcAspConfigTable_[ind] = {&i.first, &i.second};
        ++ind;
    }
    aspConfigEnd_[1] = ind;

    for (const auto& i : ext3Point_)
    {
        srcAspConfigTable_[ind] = {&i.first, &i.second};
        ++ind;
    }
    for (const auto& i : ext4Point_)
    {
        srcAspConfigTable_[ind] = {&i.first, &i.second};
        ++ind;
    }
    aspConfigEnd_[0] = ind;
}


auto AspTable::update(bool mode, int accKey, int viewKey) -> void
{
    if (mode)
    {
        computePlanetX2Table();
        computeStarTable();
        computeAspConfig();

        auto check (aspCfgTable_.find(1));
        if (check != aspCfgTable_.end())
        {
            auto& planetSignNo (astroBase_.getPlanetSignNo());
            for (auto& stellium : check->second.first)
            {
                std::set<int> buf;
                for (auto j : stellium)
                    buf.insert(planetSignNo.at(j));

                aspTabSpec_.stelliumSign.push_back(std::move(buf));
            }
        }
    }

    aspCfgTable_.clear();
    aspCfgBone_.clear();
    aspCfgOffset_ = {};

    makeSample(accKey);
    findZeroCfg();

    auto end (aspConfigEnd_[viewKey]);
    for (auto i (2); i < end; ++i)
        findAspCfg(i, false);
}


auto AspTable::getAspSample(bool bonesOnly, QGraphicsItem* item) const -> const asp_sample_t&
{
    if (item == nullptr)
        return aspSample_;

    auto& cell (srcAspConfigTable_.at(item->data(0).toInt()));
    auto group (cell.second->aspGroup);

    std::map<KeyPair, const AspData*> sample;
    std::vector<int> cfg;

    for (const auto& i : item->data(1).toJsonArray())
        cfg.push_back(i.toInt());

    auto& bone (aspCfgBone_.at(cfg).first);
    for (auto& i : aspSample_[group])
    {
        if (!contains(i.second->asp, *cell.first))
            continue;

        if (!isInclude(i.first, bone))
            continue;

        sample[i.first] = i.second;
    }

    if (!bonesOnly)
        for (auto& i : aspSample_[AspGroup::Union])
        {
            if (!isInclude(i.first, cfg))
                continue;

            sample[i.first] = i.second;
        }

    aspSampleMarkCfg_.clear();
    aspSampleMarkCfg_.push_back(std::move(sample));

    return aspSampleMarkCfg_;
}


auto AspTable::computePlanetX2Table() -> void
{
    planetX2Table_.clear();

    auto& planetCrd (astroBase_.getPlanetCrd());
    auto& aspCatalog (aspPage_.getAspCatalog());
    auto& orbTable (aspPage_.getOrbTable());

    auto aspPatch (makePatch(person_.patch));
    auto aspPatchEnd (aspPatch.end());

    for (auto& i : planetCrd)
        for (auto& j : planetCrd)
        {
            if (i.first >= j.first)
                continue;

            auto patch (aspPatch.find({i.first, j.first}));
            if (patch != aspPatchEnd)
            {
                planetX2Table_[{i.first, j.first}] = patch->second;
                continue;
            }

            auto ang (std::abs(i.second - j.second));
            if (ang > 180)
                ang = 360 - ang;

            auto& cell (planetX2Table_[{i.first, j.first}]);
            cell = {ang, NONE, NONE, NONE};

            for (auto& k : aspCatalog)
            {
                auto orb (orbTable.get(i.first, j.first, k.first));
                auto acc (std::make_pair(0.06375 * orb, 0.1275 * orb));

                if ((k.first - orb) <= ang && ang <= (k.first + orb))
                {
                    cell.asp = k.first;
                    cell.orb = std::abs(ang - k.first);
                    if (ang >= (k.first - acc.first) && ang <= (k.first + acc.first))
                        cell.acc = 1;
                    else if (
                        (ang >= (k.first + orb - acc.second) && ang <= (k.first + orb)) ||
                        (ang >= (k.first - orb) && ang <= (k.first - orb + acc.second)))
                        cell.acc = 2;
                    else
                        cell.acc = 0;
                    break;
                }
            }
        }
}


auto AspTable::computeStarTable() -> void
{
    planetStarTable_.clear();
    cuspidStarTable_ = {};

    auto& planetCrd (astroBase_.getPlanetCrd());
    auto& cuspidCrd (astroBase_.getCuspidCrd());
    auto& orbTable (aspPage_.getOrbTable());

    std::map<int, double> planetStarMag;
    std::vector<double> cuspidStarMag (12, 5);

    for (auto& i : planetCrd)
    {
        planetStarMag[i.first] = 5;
        planetStarTable_[i.first] = {};
    }

    auto ind (0);
    auto julDay (astroBase_.getJulDay());

    double mag;
    double res[6];
    char star[50];
    char serr[AS_MAXCH];

    while (true)
    {
        sprintf(star, "%d", ++ind);
        if (swe_fixstar2_ut(star, julDay, SEFLG_SWIEPH, res, serr) == ERR)
            break;

        swe_fixstar2_mag(star, &mag, serr);
        if (mag >= 5 || star[0] == ',')
            continue;

        auto starList (QString(star).split(","));
        if (starList[1][0].isUpper() || starList[1][1].isUpper())
            continue;

        auto orb (orbTable.star[mag < 0 ? 0 : static_cast<int>(mag)]);

        for (auto& i : planetCrd)
        {
            auto ang (std::abs(i.second - res[0]));
            if (ang > 180)
                ang = 360 - ang;

            if (ang <= orb && mag < planetStarMag.at(i.first))
            {
                auto crd (makeAsTime(res[0]));
                auto signNo (static_cast<int>(res[0] / 30));
                planetStarTable_[i.first] = {
                    starList[0],
                    QString("%1 %2°%3'%4\" %5, %6")
                        .arg(starList[1])
                        .arg(crd[0] - 30 * signNo)
                        .arg(crd[1], 2, 10, QChar('0'))
                        .arg(crd[2], 2, 10, QChar('0'))
                        .arg(RefBook::signSymStr[signNo])
                        .arg(mag)
                };
                planetStarMag[i.first] = mag;
            }
        }

        for (auto i (0); i < 12; ++i)
        {
            auto ang (std::abs(cuspidCrd[i] - res[0]));
            if (ang > 180)
                ang = 360 - ang;

            if (ang <= orb && mag < cuspidStarMag[i])
            {
                auto crd (makeAsTime(res[0]));
                auto signNo (static_cast<int>(res[0] / 30));
                cuspidStarTable_[i] = {
                    starList[0],
                    QString("%1 %2°%3'%4\" %5, %6")
                        .arg(starList[1])
                        .arg(crd[0] - 30 * signNo)
                        .arg(crd[1], 2, 10, QChar('0'))
                        .arg(crd[2], 2, 10, QChar('0'))
                        .arg(RefBook::signSymStr[signNo])
                        .arg(mag)
                };
                cuspidStarMag[i] = mag;
            }
        }
    }
}


auto AspTable::computeAspConfig() -> void
{
    aspCfgTable_.clear();
    aspCfgBone_.clear();
    aspCfgOffset_ = {};
    isMajorCfg_ = false;
    aspTabSpec_ = {};

    makeSample();
    findZeroCfg();

    for (auto i (2); i < aspConfigEnd_[1]; ++i)
    {
        findAspCfg(i, true);
        if (i == 19)
        {
            computeAspField();
            computeRexAspInPit();
            computeAspFictStat();
            computeMapView();
        }
    }
}


auto AspTable::computeAspField() -> void
{
    aspField_ = {};

    auto& aspPoint (aspPage_.getAspPoint());

    std::array<double, 3> multip {1, 1.5, 0.666667};

    for (auto i : aspGroup.compute)
    {
        aspField_[0].second += aspSample_[i].size();
        for (const auto& j : aspSample_[i])
            aspField_[i].first += multip[j.second->acc] * aspPoint.at(j.second->asp);
    }

    for (const auto& i : aspCfgTable_)
    {
        if (i.first == 0)
        {
            aspField_[0].first += 0.75 * aspPoint.at(0) * i.second.first.size();
            continue;
        }
        aspField_[i.second.second->aspGroup].first +=
            aspPoint.at(i.second.second->baseAsp) * i.second.first.size();
    }

    aspField_[1].second = checkRelation(aspField_[1].first, aspField_[2].first);
    aspField_[2].second = checkRelation(aspField_[2].first, aspField_[1].first);
    aspField_[3].second = checkRelation(aspField_[3].first, aspField_[4].first);
    aspField_[4].second = checkRelation(aspField_[4].first, aspField_[3].first);
}


auto AspTable::computeRexAspInPit() -> void
{
    auto& aspPoint (aspPage_.getAspPoint());

    std::map<int, std::array<double, 3>> degreeOfAsp;
    for (auto i : AstroBase::planetOrder)
        degreeOfAsp[i] = {};

    for (auto i : aspGroup.compute)
        for (const auto& j : aspSample_[i])
        {
            auto asp (j.second->asp);
            auto point (aspPoint.at(asp));
            auto isMajor (asp == 0 || contains(asp, majorAsp_.schit));

            for (auto k : j.first)
            {
                degreeOfAsp[k][0] += point;
                ++degreeOfAsp[k][1];
                if (isMajor)
                    degreeOfAsp[k][2] = 1;
            }
        }

    for (const auto& i : aspCfgTable_)
    {
        auto baseAsp (i.second.second->baseAsp);
        auto point (i.first > 0 ? aspPoint.at(baseAsp) : 0.75 * aspPoint.at(baseAsp));

        for (auto& cfg : i.second.first)
            for (auto j : cfg)
            {
                degreeOfAsp[j][0] += point;
                ++degreeOfAsp[j][1];
                degreeOfAsp[j][2] = 1;
            }
    }

    std::vector<std::pair<double, int>> buf;
    for (const auto& i : degreeOfAsp)
    {
        buf.push_back({i.second[0] * i.second[1], i.first});
        if (i.second[2] == 0)
            aspTabSpec_.inPit.insert(i.first);
    }

    std::sort(buf.begin(), buf.end());

    auto back (--buf.end());
    auto rexA (back);
    auto rexB (--back);

    switch (checkRelation(rexA->first, rexB->first)) {
    case ForceFlag::Dominant :
        aspTabSpec_.rexAsp.insert(rexA->second);
        break;
    case ForceFlag::Neutral :
        auto check (--back);
        if (checkRelation(rexA->first, check->first) == ForceFlag::Dominant &&
            checkRelation(rexB->first, check->first) == ForceFlag::Dominant)
        {
            aspTabSpec_.rexAsp.insert(rexA->second);
            aspTabSpec_.rexAsp.insert(rexB->second);
        }
    }
}


auto AspTable::computeAspFictStat() -> void
{
    aspFictStat_ = {};

    auto& planetSignNo (astroBase_.getPlanetSignNo());

    std::map<int, std::set<int>> zero {{11, {}}, {12, {}}, {24, {}}, {56, {}}};

    for (const auto& i : aspSample_[AspGroup::Union])
        for (auto j : i.first)
            if (contains(j, AstroBase::fictPlanet))
            {
                auto other (i.first.other(j));
                auto point (other == 0 || other == 1 ? 4 : 3);
                zero[j].insert(other);
                aspFictStat_.globa[j] += point;
                aspFictStat_.schit[j] += point;
                if (i.second->acc == 1)
                    ++aspFictStat_.schit[j];
            }

    for (auto i : AstroBase::fictPlanet)
    {
        auto sign (planetSignNo.at(i));
        for (auto j : AstroBase::planetOrder)
        {
            if (i == j)
                continue;

            if (sign == planetSignNo.at(j) && !contains(j, zero[i]))
            {
                auto point (j == 0 || j == 1 ? 3 : 2);
                aspFictStat_.globa[i] += point;
                aspFictStat_.schit[i] += point;
            }
        }
    }

    for (auto i : aspGroup.compute)
    {
        if (i == AspGroup::Union)
            continue;

        for (const auto& j : aspSample_[i])
        {
            if (j.first.less == 11 && j.first.more == 24)
                continue;

            for (auto k : j.first)
                if (contains(k, AstroBase::fictPlanet))
                {
                    auto asp (j.second->asp);
                    aspFictStat_.globa[k] += contains(asp, majorAsp_.globa) ? 2 : 1;
                    aspFictStat_.schit[k] += contains(asp, majorAsp_.schit) ? 2 : 1;
                    if (j.second->acc == 1)
                        ++aspFictStat_.schit[k];
                }
        }
    }

    for (const auto& i : aspCfgTable_)
        for (auto& cfg : i.second.first)
            for (auto j : cfg)
                if (contains(j, AstroBase::fictPlanet))
                {
                    auto isMajor (i.second.second->type == 0);
                    auto point (isMajor ? 2 : 1);

                    switch (static_cast<int>(i.second.second->baseAsp)) {
                    case 80 :
                    case 160 :
                        break;
                    case 40 :
                    case 72 :
                    case 144 :
                        aspFictStat_.globa[j] += 1;
                        break;
                    default :
                        aspFictStat_.globa[j] += point;
                    }

                    aspFictStat_.schit[j] += point;
                    break;
                }
}


auto AspTable::computeMapView() ->void
{
    mapView_ = NONE;

    map_view_chain_t chain;
    makePlanetChain(chain);

    switch (chain.size()) {
    case 1 :
    {
        auto key (KeyPair(chain[0][0].second, chain[0][11].second));
        auto asp (planetX2Table_.at(key).asp);
        auto ang (chain[0][11].first);

        if (asp == 90 || 360 - ang < 90)
        {
            mapView_ = isMajorCfg_ ? 1 : 0;
            return;
        }

        if (ang <= 240 + mvAcc_ && ang >= 240 - mvAcc_)
        {
            mapView_ = 5;
            return;
        }

        switch (static_cast<int>(asp)) {
        case 80 :
        case 120 :
        case 144 :
        case 160 :
        case 180 :
            auto checkAsp (asp / 2);
            for (const auto& i : chain[0])
            {
                if (key.contains(i.second))
                    continue;

                if (planetX2Table_.at({i.second, key.less}).asp == checkAsp &&
                    planetX2Table_.at({i.second, key.more}).asp == checkAsp)
                {
                    mapView_ = 57;
                    setMapViewSpec(chain);
                    return;
                }
            }
        }

        if (ang <= 180 + mvAcc_ && ang >= 180 - mvAcc_)
        {
            mapView_ = 8;
            setMapViewSpec(chain);
            return;
        }

        if (ang <= 120 + mvAcc_ && ang >= 120 - mvAcc_)
        {
            mapView_ = 6;
            setMapViewSpec(chain);
            return;
        }
    }
        break;
    case 2 :
    {
        auto isFront (chain[0].size() > chain[1].size());
        auto lessLink (isFront ? &chain[1] : &chain[0]);
        auto moreLink (isFront ? &chain[0] : &chain[1]);
        auto lessFront (lessLink->cbegin());
        auto lessBack (--lessLink->cend());

        auto lessNum (lessLink->size());
        auto lessAng (lessBack->first - lessFront->first);
        auto moreAng ((--moreLink->cend())->first - moreLink->cbegin()->first);
        auto lessAsp (lessNum == 1 ? 0 :
            planetX2Table_.at({lessFront->second, lessBack->second}).asp);

        if (lessNum > 2 && moreAng <= 60 + mvAcc_ && moreAng >= 60 - mvAcc_)
        {
            mapView_ = 2;
            return;
        }

        if (moreAng <= 120 + mvAcc_ && moreAng >= 120 - mvAcc_)
        {
            if (lessAsp == 0 && lessNum < 3)
            {
                mapView_ = 9;
                setMapViewSpec(chain);
                return;
            }
            if (lessAng <= 30 + mvAcc_)
            {
                mapView_ = 15;
                setMapViewSpec(chain);
                return;
            }
        }

        if (moreAng <= 180 + mvAcc_ && moreAng >= 180 - mvAcc_)
        {
            if (lessAsp == 0 && lessNum < 3)
            {
                mapView_ = 4;
                setMapViewSpec(chain);
                return;
            }
            if (lessAng <= 30 + mvAcc_)
            {
                mapView_ = 3;
                setMapViewSpec(chain);
                return;
            }
        }
    }
        break;
    case 3 :
    {
        auto check (true);
        for (const auto& i : chain)
            if (i.size() < 3 || (--i.end())->first - i[0].first > 30 + mvAcc_)
            {
                check = false;
                break;
            }
        if (check)
        {
            mapView_ = 7;
            return;
        }
    }
        break;
    }

    auto endLink (--chain.cend());
    if (360 - (--endLink->end())->first < 90 ||
        planetX2Table_.at({chain[0][0].second, (--endLink->end())->second}).asp == 90)
    {
        for (auto i (chain.cbegin()), j (++chain.cbegin()); i != endLink; ++i, ++j)
        {
            auto thisBack (--i->end());
            auto nextFront (j->begin());

            if (nextFront->first - thisBack->first >= 90 &&
                planetX2Table_.at({thisBack->second, nextFront->second}).asp != 90)
                return;
        }
        mapView_ = isMajorCfg_ ? 1 : 0;
    }
}


auto AspTable::makeSample() -> void
{
    aspSample_ = {{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
    keySeqList_ = {{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};

    for (auto i : AstroBase::planetOrder)
        for (auto j : AstroBase::planetOrder)
        {
            if (i >= j)
                continue;

            addToSample({i, j}, &planetX2Table_.at({i, j}));
        }
}


auto AspTable::makeSample(int accKey) -> void
{
    aspSample_ = {{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
    keySeqList_ = {{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}};

    auto& planetEnb (astroBase_.getPlanetEnb());
    auto& aspEnb (aspPage_.getAspEnb());

    for (const auto& i : planetX2Table_)
    {
        if (!planetEnb.at(i.first.less) || !planetEnb.at(i.first.more))
            continue;

        if (i.second.asp == NONE || !aspEnb.at(i.second.asp))
            continue;

        if (accKey != 3 && accKey != i.second.acc)
            continue;

        addToSample(i.first, &i.second);
    }
}


auto AspTable::addToSample(const KeyPair& key, const AspData* data) -> void
{
    auto group (getAspGroupInd(data->asp));

    if (group != NONE)
    {
        aspSample_[group][key] = data;
        keySeqList_[group].insert(key.less);
        keySeqList_[group].insert(key.more);

        switch (group) {
        case AspGroup::Black :
            for (auto i : {AspGroup::BlackRed, AspGroup::BlackGreen, AspGroup::BlackBlue})
            {
                aspSample_[i][key] = data;
                keySeqList_[i].insert(key.less);
                keySeqList_[i].insert(key.more);
            }
            break;
        case AspGroup::Red :
            for (auto i : {AspGroup::BlackRed, AspGroup::RedBlue})
            {
                aspSample_[i][key] = data;
                keySeqList_[i].insert(key.less);
                keySeqList_[i].insert(key.more);
            }
            break;
        case AspGroup::Green :
            aspSample_[AspGroup::BlackGreen][key] = data;
            keySeqList_[AspGroup::BlackGreen].insert(key.less);
            keySeqList_[AspGroup::BlackGreen].insert(key.more);
            break;
        case AspGroup::Blue :
            for (auto i : {AspGroup::BlackBlue, AspGroup::RedBlue})
            {
                aspSample_[i][key] = data;
                keySeqList_[i].insert(key.less);
                keySeqList_[i].insert(key.more);
            }
            break;
        }
    }
}


auto AspTable::findZeroCfg() -> void
{
    const auto& sample (aspSample_[AspGroup::Union]);
    const auto& keySeq (keySeqList_[AspGroup::Union]);

    std::set<std::set<int>> bones;
    for (auto i : keySeq)
    {
        std::set<int> buf {i};

        for (auto& j : sample)
            if (j.first.contains(i))
                buf.insert(j.first.other(i));

        bones.insert(std::move(buf));
    }

    std::vector<std::set<int>> mass (bones.begin(), bones.end());
    for (auto i (mass.begin()); i != mass.end(); ++i)
        for (auto j (mass.cbegin()); j != mass.cend(); ++j)
        {
            if (i == j)
                continue;
            if (isIntxn(*i, *j))
                for (auto k : *j)
                    i->insert(k);
        }

    zeroMass_.clear();
    for (auto& i : mass)
        zeroMass_.insert(std::move(i));

    for (auto& i : zeroMass_)
        switch (i.size()) {
        case 2 :
            break;
        case 3 :
            makeAspCfg(0, i, {}, false);
            break;
        default :
            makeAspCfg(1, i, {}, false);
        }
}


auto AspTable::findAspCfg(int ind, bool mode) -> void
{
    const auto& data (srcAspConfigTable_.at(ind));

    auto aspGroup (data.second->aspGroup);
    const auto& sample (aspSample_[aspGroup]);
    const auto& keySeq (keySeqList_[aspGroup]);

    auto& asp (*data.first);
    auto pointNum (asp.size());
    auto sampleEnd (sample.end());

    std::set<bone_src_t> bones;

    for (auto i : keySeq)
    {
        auto match (std::vector<double>(pointNum, 1));
        auto bone (pointNum > 3
                       ? std::vector<int>{i, NONE, NONE, NONE}
                       : std::vector<int>{i, NONE, NONE});

        for (auto& j : sample)
            if (j.first.contains(i) && j.second->asp == asp[0])
            {
                bone[1] = j.first.other(i);
                match[0] = j.second->orb / asp[0];
                for (auto& k : sample)
                    if (k.first.contains(bone[1]) && k.second->asp == asp[1])
                    {
                        bone[2] = k.first.other(bone[1]);
                        match[1] = k.second->orb / asp[1];
                        if (pointNum > 3)
                        {
                            auto check (sample.find({i, bone[2]}));
                            if (check != sampleEnd && check->second->asp == asp[4])
                            {
                                match[4] = check->second->orb / asp[4];
                                for (auto& h : sample)
                                    if (h.first.contains(bone[2]) && h.second->asp == asp[2])
                                    {
                                        bone[3] = h.first.other(bone[2]);
                                        match[2] = h.second->orb / asp[2];
                                        auto checkA (sample.find({bone[3], i}));
                                        auto checkB (sample.find({bone[3], bone[1]}));
                                        if (checkA != sampleEnd && checkB != sampleEnd &&
                                            checkA->second->asp == asp[3] &&
                                            checkB->second->asp == asp[5])
                                        {
                                            match[3] = checkA->second->orb / asp[3];
                                            match[5] = checkB->second->orb / asp[5];
                                            bones.insert({{bone.begin(), bone.end()}, meanArith(match)});
                                        }
                                    }
                            }
                        }
                        else
                        {
                            auto check (sample.find({i, bone[2]}));
                            if (check != sampleEnd && check->second->asp == asp[2])
                            {
                                match[2] = check->second->orb / asp[2];
                                bones.insert({{bone.begin(), bone.end()}, meanArith(match)});
                            }
                        }
                    }
            }
    }

    for (auto& bone : bones)
    {
        std::set<int> mass;
        for (auto i : bone.first)
        {
            mass.insert(i);
            for (auto& zero : zeroMass_)
                if (zero.find(i) != zero.end())
                {
                    for (auto k : zero)
                        mass.insert(k);
                    break;
                }
        }
        makeAspCfg(ind, mass, bone, mode);
    }
}


auto AspTable::makeAspCfg(int ind, const std::set<int>& mass, const bone_src_t& bone, bool mode) -> void
{
    const auto& data (srcAspConfigTable_.at(ind));

    auto cfg (astroBase_.sortByCrd(mass));
    auto check (aspCfgBone_.find(cfg));

    if (check == aspCfgBone_.end() ||
       (check != aspCfgBone_.end() && check->second.second > bone.second))
    {
        if (bone.first.empty())
        {
            setAspCfgOffset(cfg, 1);
            aspCfgBone_[cfg] = {cfg, 0};
        }
        else
        {
            auto cfgBone (astroBase_.sortByCrd(bone.first));
            setAspCfgOffset(cfgBone, 1);
            aspCfgBone_[cfg] = {std::move(cfgBone), bone.second};
        }
    }

    auto& cell (aspCfgTable_[ind]);
    setAspCfgOffset(cfg, 0);
    cell.first.insert(std::move(cfg));
    cell.second = data.second;

    auto& excepList (data.second->excepList);
    if (!excepList.empty())
        for (auto i : excepList)
        {
            auto excep (aspCfgTable_.find(i));
            if (excep != aspCfgTable_.end())
            {
                std::vector<std::_Rb_tree_const_iterator<std::vector<int>>> buf;

                for (auto j (excep->second.first.begin()); j != excep->second.first.end(); ++j)
                    if (isInclude(*j, mass))
                        buf.push_back(j);

                for (auto j : buf)
                    excep->second.first.erase(j);

                if (excep->second.first.empty())
                    aspCfgTable_.erase(excep);
            }
        }

    if (mode && ind > 1 && ind < 17)
    {
        if (!isMajorCfg_ && data.second->type == 0)
            isMajorCfg_ = !isIntxn(AstroBase::fictPlanet, bone.first);

        if (ind != 5)
            for (auto i : bone.first)
            {
                std::set<int> pair (bone.first);
                pair.erase(i);
                auto aspA (planetX2Table_.at({i, *pair.begin()}).asp);
                auto aspB (planetX2Table_.at({i, *(--pair.end())}).asp);
                if (aspA == aspB)
                {
                    switch (getAspGroupInd(aspA)) {
                    case AspGroup::Red :
                    case AspGroup::Green :
                        aspTabSpec_.topRedGreen.insert(i);
                        break;
                    case AspGroup::Black :
                    case AspGroup::Blue :
                        aspTabSpec_.topBlackBlue.insert(i);
                    }
                    break;
                }
            }
    }
}


auto AspTable::setAspCfgOffset(const std::vector<int>& cfg, int ind) -> void
{
    int length (cfg.size());
    if (aspCfgOffset_[ind] < length)
        aspCfgOffset_[ind] = length;
}


auto AspTable::makePlanetChain(map_view_chain_t& chain) -> void
{
    auto& planetCrd (astroBase_.getPlanetCrd());
    auto& planetSignNo (astroBase_.getPlanetSignNo());

    map_view_link_t sorted;

    for (auto i : AstroBase::noFictPlanet)
        sorted.push_back({planetCrd.at(i), i});

    std::sort(sorted.begin(), sorted.end());

    map_view_link_t buf {sorted[0]};
    for (auto i (++sorted.cbegin()); i != sorted.cend(); ++i)
    {
        if (planetSignNo.at(i->second) - planetSignNo.at((--buf.end())->second) < 3)
            buf.push_back(*i);
        else
        {
            chain.push_back(std::move(buf));
            buf.push_back(*i);
        }
    }
    if (!buf.empty())
        chain.push_back(std::move(buf));

    if (chain.size() > 1)
    {
        auto lastLink (--chain.end());
        if (12 - planetSignNo.at((--lastLink->end())->second) +
                            planetSignNo.at(chain[0][0].second) < 3)
        {
            for (const auto& i : chain[0])
                lastLink->push_back(i);

            chain.erase(chain.begin());
        }
    }

    auto zero (chain[0][0].first);
    for (auto& link : chain)
        for (auto& i : link)
        {
            i.first -= zero;
            if (i.first < 0)
                i.first += 360;
        }
}


auto AspTable::setMapViewSpec(const map_view_chain_t& chain) -> void
{
    auto chainFront (chain.begin());
    auto chainBack (--chain.end());
    auto isFront (chainFront->size() > chainBack->size());

    switch (mapView_) {
    case 4 :
    case 9 :
        auto less (isFront ? chainBack : chainFront);
        for (auto& i : *less)
            aspTabSpec_.arrow.insert(i.second);
    }

    switch (mapView_) {
    case 3 :
    case 4 :
    case 6 :
    case 8 :
    case 9 :
    case 15 :
    case 57 :
        auto more (isFront ? chainFront : chainBack);
        const auto& sample (aspSample_[AspGroup::Union]);

        std::vector<std::vector<int>> nodeSeq;
        std::vector<int> buf {more->begin()->second};

        for (auto i (++more->begin()); i != more->end(); ++i)
        {
            if (sample.find({i->second, *(--buf.end())}) != sample.end())
                buf.push_back(i->second);
            else
            {
                nodeSeq.push_back(std::move(buf));
                buf.push_back(i->second);
            }
        }
        if (!buf.empty())
            nodeSeq.push_back(std::move(buf));

        if (nodeSeq.size() % 2 != 0)
        {
            const auto& bed (nodeSeq[nodeSeq.size() / 2]);
            aspTabSpec_.bed = {bed.begin(), bed.end()};
        }
    }
}


auto AspTable::loadExtCfg() -> void
{
    for (auto ind : {3, 4})
    {
        auto& extCfgList (ind == 3 ? ext3Point_ : ext4Point_);
        auto& extCfgSrc (ind == 3 ? ext3PointR_ : ext4PointR_);

        QFile file (path_ + QString("ext%1point.json").arg(ind));
        QByteArray source;

        if (file.exists() && file.open(QIODevice::ReadOnly))
        {
            source = file.readAll();
            file.close();
        }
        else
        {
            errorLog(file.fileName() + " not found");
            extCfgList = extCfgSrc;
            dumpExtCfg(ind);
            continue;
        }

        QJsonParseError parseError;
        QJsonDocument json (QJsonDocument::fromJson(source, &parseError));

        if (json.isNull())
        {
            errorLog(file.fileName() + " parsing error: " + parseError.errorString());
            extCfgList = extCfgSrc;
            dumpExtCfg(ind);
            continue;
        }

        extCfgList.clear();

        auto isDump (false);
        for (const auto& i : json.array())
        {
            const auto& dec (i.toArray());
            const auto& data (dec[1].toArray());

            std::vector<double> keyAsp;
            std::vector<int> excepList;

            for (const auto& j : dec[0].toArray())
                keyAsp.push_back(j.toDouble());

            for (const auto& j : dec[2].toArray())
                excepList.push_back(j.toInt());

            auto aspGroup (data[0].toInt());
            auto title (data[3].toString());
            auto check (true);

            switch (keyAsp.size()) {
            case 3 :
            case 6 :
                for (auto k : keyAsp)
                    if (k < 0 || k > 180)
                    {
                        check = false;
                        break;
                    }

                if (aspGroup < 0 || aspGroup > 10 || title.isEmpty())
                    check = false;

                if (!check)
                {
                    QString errorStr (file.fileName() + " invalid data for key: ");

                    for (auto k : keyAsp)
                        errorStr += QString("%1 ").arg(k);

                    errorLog(errorStr);
                    isDump = true;
                    continue;
                }
                break;
            default :
                QString errorStr (file.fileName() + " invalid key size: ");

                for (auto k : keyAsp)
                    errorStr += QString("%1 ").arg(k);

                errorLog(errorStr);
                isDump = true;
                continue;
            }

            extCfgList.push_back(
                {std::move(keyAsp), {aspGroup, std::move(excepList),
                 data[1].toDouble(), data[2].toInt(), std::move(title)}});
        }

        if (isDump)
            dumpExtCfg(ind);
    }
}


auto AspTable::dumpExtCfg(int ind) const -> void
{
    QJsonArray aspCfgArray;

    auto& extCfgList (ind == 3 ? ext3Point_ : ext4Point_);
    for (auto& cfg : extCfgList)
    {
        QJsonArray keyAsp;
        QJsonArray singleData {
            cfg.second.aspGroup, cfg.second.baseAsp, cfg.second.type, cfg.second.title
        };
        QJsonArray excepArray;

        for (auto i : cfg.first)
            keyAsp.push_back(i);

        for (auto i : cfg.second.excepList)
            excepArray.push_back(i);

        aspCfgArray.push_back(QJsonArray{keyAsp, singleData, excepArray});
    }

    QFile file (path_ + QString("ext%1point.json").arg(ind));

    if (file.open(QIODevice::WriteOnly))
    {
        file.write(QByteArray(QJsonDocument(aspCfgArray).toJson()));
        file.close();
    }
    else
        errorLog("can not create " + file.fileName());
}


auto AspTable::makePatch(const std::map<KeyPair, AspData>& patch) -> QString
{
    if (patch.empty())
        return {};

    QString res;

    for (auto i (patch.begin()), j (--patch.end()); i != patch.end(); ++i)
        res += i->first + " : " + i->second + (i != j ? "; " : "");

    return res;
}


auto AspTable::makePatch(const QString& patch) -> std::map<KeyPair, AspData>
{
    if (patch.isEmpty())
        return {};

    std::map<KeyPair, AspData> res;

    for (auto& i : patch.split("; "))
    {
        auto pair (i.split(" : "));
        res[pair[0]] = pair[1];
    }

    return res;
}


auto AspTable::getAspGroupInd(double asp) -> int
{
    switch (static_cast<int>(asp)) {
    case NONE:
        return NONE;
    case 0 :
        return AspGroup::Union;
    case 180 :
    case 90 :
    case 135 :
    case 45 :
        return AspGroup::Black;
    case 120 :
    case 60 :
    case 150 :
    case 30 :
        return AspGroup::Red;
    case 72 :
    case 144 :
    case 108 :
    case 36 :
        return AspGroup::Green;
    case 40 :
    case 80 :
    case 160 :
    case 20 :
    case 100 :
    case 140 :
        return AspGroup::Blue;
    default :
        if (asp == ASP_S || asp == ASP_TS || asp == ASP_BS || asp == ASP_PS)
            return AspGroup::Violet;

        return AspGroup::NotMarked;
    }
}


auto KeyPair::operator<(const KeyPair& rhs) const -> bool
{
    return ((less < rhs.less) || (less == rhs.less && more < rhs.more)) ? true : false;
}


auto KeyPair::operator>(const KeyPair& rhs) const -> bool
{
    return ((less > rhs.less) || (less == rhs.less && more > rhs.more)) ? true : false;
}


AspData::AspData(const QString& str)
{
    auto chain (str.split(", "));

    ang = chain[0].toDouble();
    asp = chain[1].toDouble();
    orb = chain[2].toDouble();
    acc = chain[3].toInt();
}


AspData::operator QString() const
{
    QString str;
    QTextStream out (&str);

    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(9);
    out << ang << ", " << asp << ", " << orb << ", " << acc;

    return str;
}


auto AspData::operator==(const AspData& rhs) const -> bool
{
    return ang == rhs.ang && asp == rhs.asp && orb == rhs.orb && acc == rhs.acc;
}

} // namespace napatahti
