#include <QApplication>
#include "shared.h"
#include "RefBook.h"
#include "AstroBase.h"
#include "AspTable.h"
#include "PrimeTest.h"

namespace napatahti {

auto PrimeTest::update() -> void
{
    coreStat_ = {};
    primeStat_ = {};
    primeFict_ = {};

    computeAlmuten();
    computeMapType(0);
    computeMapType(1);

    for (auto method : {0, 1})
        for (auto view : {0, 1})
        {
            computeCoreStat(method, view);
            computePrimeStat(method, view);
        }

    auto method (0);
    for (auto& part : aspTable_.getAspFictStat())
    {
        const auto& primeStatHor (primeStat_.get(method, 1));

        auto& primeFict (primeFict_[method]);
        auto& primeStatBoth (primeStat_.get(method, 2));
        auto& primeStatCos (primeStat_.get(method, 0));

        std::map<int, double> normFict;

        for (auto& i : part)
        {
            primeFict[i.first].first += i.second;
            normFict[i.first] = primeFict.at(i.first).first;
        }

        primeFict[11].second = checkRelation(normFict.at(11), normFict.at(24));
        primeFict[24].second = checkRelation(normFict.at(24), normFict.at(11));
        primeFict[56].second = checkRelation(normFict.at(56), normFict.at(12));
        primeFict[12].second = checkRelation(normFict.at(12), normFict.at(56));

        for (auto& i : {std::array<int, 2>{11, 24}, {56, 12}})
        {
            auto statA (std::abs(normFict.at(i[0])));
            auto statB (std::abs(normFict.at(i[1])));

            if (statA > 7 || statB > 7)
            {
                if (statA != 0 && statB != 0)
                {
                    auto k (std::min(statA, statB) / std::max(statA, statB));
                    normFict[i[0]] /= (statA / (statA > statB ? 7 : 7 * k));
                    normFict[i[1]] /= (statB / (statA > statB ? 7 * k : 7));
                }
                else
                    normFict[statA > 0 ? i[0] : i[1]] /= ((statA > 0 ? statA : statB) / 7);
            }
        }

        for (auto i : AstroBase::noFictPlanet)
            primeStatBoth[i] = {pairSum(primeStatHor.at(i)), pairSum(primeStatCos.at(i))};

        for (const auto& i : normFict)
        {
            primeStatBoth[i.first] = {0, i.second};
            if (i.second > 0)
                primeStatCos[i.first] = {i.second, 0};
            else
                primeStatCos[i.first] = {0, i.second};
        }

        ++method;
    }
}


auto PrimeTest::computeCoreStat(int method, int view) -> void
{
    auto& planetSiteNo (view == 0 ? astroBase_.getPlanetSignNo() : astroBase_.getPlanetHouseNo());
    const auto& mapType (mapType_[view]);

    auto& coreInd (coreInd_[method]);
    auto& primeInd (primeInd_[method]);

    auto& coreForce (coreStat_.get(method, view));

    std::vector<double> force (12, 0);
    std::vector<double> elemForce (4, 0);
    std::vector<double> crosForce (3, 0);
    std::vector<double> quadForce (4, 0);
    std::vector<double> zoneForce (3, 0);
    std::vector<double> hsnsForce;
    std::vector<double> hsewForce;
    std::vector<double> baseForce (3, 0);

    for (auto i : AstroBase::planetOrder)
        force[planetSiteNo.at(i)] += primeInd.at(i);

    for (auto i (0), j (0); i < 4; ++i, ++j)
        for (auto k : {0, 4, 8})
            elemForce[i] += force[k+j];

    for (auto i (0), j (0); i < 3; ++i, ++j)
        for (auto k : {0, 3, 6, 9})
            crosForce[i] += force[k+j];

    for (auto i (0), j (0); i < 4; ++i, j += 3)
        for (auto k : {0, 1, 2})
            quadForce[i] += force[k+j];

    for (auto i (0), j (0); i < 3; ++i, j += 4)
        for (auto k : {0, 1, 2, 3})
            zoneForce[i] += force[k+j];

    hsnsForce = {quadForce[2] + quadForce[3], quadForce[0] + quadForce[1]};
    hsewForce = {quadForce[0] + quadForce[3], quadForce[1] + quadForce[2]};

    coreForce.makeCompare(Elem, elemForce);
    coreForce.makeCompare(Cros, crosForce);
    coreForce.makeCompare(Quad, quadForce);
    coreForce.makeCompare(Zone, zoneForce);
    coreForce.makeCompare(HsNS, hsnsForce);
    coreForce.makeCompare(HsEW, hsewForce);
    coreForce.makeCompare(ElemBin, elemForce);
    coreForce.makeCompare(QuadBin, quadForce);

    auto k (0);
    for (auto part (coreForce.cbegin()); part != coreForce.cend(); ++part)
    {
        if (part == coreForce.baseIt())
            continue;

        auto more3 (part->size() > 3);
        auto check (true);
        auto i (0);

        for (auto& j : *part)
        {
            if (j.second == ForceFlag::Dominant)
            {
                baseForce[more3 && i > 1 ? i - 2 : i] += coreInd[k];
                check = false;
                break;
            }
            ++i;
        }
        if (check)
            baseForce[2] += coreInd[k+1];

        k += 2;
    }

    if (method == 1 && mapType.second != NONE)
        baseForce[mapType.second] += 4;

    coreForce.makeCompare(Base, baseForce);
}


auto PrimeTest::computePrimeStat(int method, int view) -> void
{
    auto& planetCrd (astroBase_.getPlanetCrd());
    auto& planetSiteNo (view == 0 ? astroBase_.getPlanetSignNo() : astroBase_.getPlanetHouseNo());

    auto& primeStat (primeStat_.get(method, view));
    auto& primeFict (primeFict_[method]);
    auto& coreForce (coreStat_.get(method, view));

    const auto& elemForce (coreForce.elem);
    const auto& crosForce (coreForce.cros);
    const auto& zoneForce (coreForce.zone);
    const auto& quadForce (coreForce.quad);
    const auto& hsnsForce (coreForce.hsNS);
    const auto& hsewForce (coreForce.hsEW);
    const auto& baseForce (coreForce.base);

    auto isGloba (method == 0);
    auto isCosView (view == 0);

    // Part I
    for (auto i : AstroBase::planetOrder)
    {
        auto sign (planetSiteNo.at(i));
        auto elem (RefBook::signElem[sign]);
        auto cros (RefBook::signCros[sign]);
        auto zone (RefBook::signZone[sign]);

        switch (i) {
        case 11 :
        case 12 :
        case 24 :
        case 56 :
            if (isCosView)
            {
                auto& status (primeFict[i]);

                switch (i) {
                case 12 :
                case 56 :
                {
                    auto spec (RefBook::specState.find({i, sign}));
                    if (spec != RefBook::specState.end())
                    {
                        if (spec->second == SpecState::Exaltation)
                            status.first += 1;
                        else if (!isGloba && spec->second == SpecState::Fall)
                            status.first -= 1;
                    }

                    if (isGloba)
                    {
                        if (elemForce[elem].second == ForceFlag::Dominant)
                            status.first += 1;
                        if (crosForce[cros].second == ForceFlag::Dominant)
                            status.first += 1;
                        if (zoneForce[zone].second == ForceFlag::Dominant)
                            status.first += 1;
                    }
                    else
                    {
                        auto oppSign (i == 12 ? planetSiteNo.at(56) : planetSiteNo.at(12));

                        if (checkRelation(
                                elemForce[elem].first,
                                elemForce[RefBook::signElem[oppSign]].first
                                ) == ForceFlag::Dominant
                            )
                            status.first += 1;
                        if (checkRelation(
                                crosForce[cros].first,
                                crosForce[RefBook::signCros[oppSign]].first
                                ) == ForceFlag::Dominant
                            )
                            status.first += 1;
                        if (checkRelation(
                                zoneForce[zone].first,
                                zoneForce[RefBook::signZone[oppSign]].first
                                ) == ForceFlag::Dominant
                            )
                            status.first += 1;
                    }
                }
                    break;
                case 11 :
                case 24 :
                {
                    auto oppSign (i == 11 ? planetSiteNo.at(24) : planetSiteNo.at(11));

                    if (checkRelation(
                            elemForce[elem].first,
                            elemForce[RefBook::signElem[oppSign]].first
                            ) == ForceFlag::Dominant
                        )
                        status.first += (isGloba ? 2 : 1);
                    if (!isGloba &&
                        checkRelation(
                            zoneForce[zone].first,
                            zoneForce[RefBook::signZone[oppSign]].first
                            ) == ForceFlag::Dominant
                        )
                        status.first += 1;
                    if (hsnsForce[RefBook::signHsns[sign]].second == ForceFlag::Dominant)
                        status.first += 1;
                }
                    break;
                }
            }
            continue;
        }

        auto& status (primeStat[i]);

        auto posBase (RefBook::planetPosBase.at(i));
        auto negBase (RefBook::planetNegBase.at(i));

        auto spec (RefBook::specState.find({i, sign}));
        if (spec != RefBook::specState.end())
        {
            if (spec->second == SpecState::Ruler)
                status[0] += 3;
            else if (spec->second == SpecState::Detriment)
                status[1] -= 3;
            else if (spec->second == SpecState::Exaltation)
                status[0] += 3;
            else if (spec->second == SpecState::Fall)
                status[1] -= 3;
            if ((i == 2 && sign == 5) || (i == 57 && sign == 2))
                status[0] += 1;  // exception (Mer, Vir), (Pro, Gem): 4
            else if ((i == 2 && sign == 11) || (i == 57 && sign == 8))
                status[1] -= 1;  // exception (Mer, Psc), (Pro, Sag): -4
        }

        if (RefBook::planetPosElem.at(i) == elem)
            status[0] += 2;
        else if (RefBook::planetNegElem.at(i) == elem)
            status[1] -= 2;
        if (isGloba && i == 1 && sign == 7)
            status[0] -= 1; // exception (Mon, Sco): 1

        if (posBase == cros)
            status[0] += 2;
        if (isGloba && i == 4 && sign == 3)
            status[0] -= 1; // exception (Mar, Cnc): 1
        if (!isGloba && negBase == cros)
            status[1] -= 2;

        if (posBase == zone)
            status[0] += 1;
        if (!isGloba && negBase == zone)
            status[1] -= 1;

        if (isGloba && !isCosView)
        {
            auto degree (static_cast<int>(planetCrd.at(i)));
            auto ruler (RefBook::degreeRuler[degree]);

            auto almu (almuten_.find({i, sign}));
            if (almu != almuten_.end())
                almu->second ? status[0] += 3 : status[1] -= 3;

            if (ruler == i)
                status[0] += 1;
            else if(ruler == RefBook::planetNegPlanet.at(i))
                status[1] -= 1;

            auto exal (RefBook::degreeExal.find(degree));
            if (exal != RefBook::degreeExal.end())
                status[0] += exal->second == i ? 2 : 0.5;

            auto fall (RefBook::degreeFall.find(degree));
            if (fall != RefBook::degreeFall.end())
                status[1] -= fall->second == i ? 2 : 0.5;
        }

        setStatus(status, 2, elemForce[elem].second);
        setStatus(status, 2, crosForce[cros].second);
        setStatus(status, 1, zoneForce[zone].second);
        setStatus(status, 1, quadForce[RefBook::signQuad[sign]].second);
        setStatus(status, 1, hsnsForce[RefBook::signHsns[sign]].second);
        setStatus(status, 1, hsewForce[RefBook::signHsew[sign]].second);
        setStatus(status, 1, baseForce[posBase].second);

        if (!isGloba)
        {
            if (baseForce[posBase].second == ForceFlag::Weak)
                status[1] += 1;
            if (negBase != NONE && baseForce[negBase].second == ForceFlag::Dominant)
                status[1] -= 1;
        }

        coreForce.total += status[0] + status[1];
    }

    // Part II
    if (isCosView)
        for (auto i : AstroBase::fictPlanet)
        {
            auto& status (primeFict[i]);

            auto sign (planetSiteNo.at(i));
            auto& ruler (RefBook::signRuler[sign]);
            auto& oppos (RefBook::signOppos[sign]);
            auto& essen (RefBook::signEssen[sign]);

            status.first +=
                0.5 * (pairSum(primeStat.at(ruler[0]))) +
                0.25 * (ruler[1] != NONE ? pairSum(primeStat.at(ruler[1])) : 0);
            status.first -=
                0.5 * (pairSum(primeStat.at(oppos[0]))) +
                0.25 * (oppos[1] != NONE ? pairSum(primeStat.at(oppos[1])) : 0);
            status.first += 0.5 * pairSum(primeStat.at(essen[0]));
            status.first -= 0.5 * pairSum(primeStat.at(essen[1]));

            switch (i) {
            case 11 :
            {
                std::vector<std::pair<double, int>> revCrd;

                for (auto j : AstroBase::planetOrder)
                    switch (j) {
                    case 12 :
                    case 56 :
                        continue;
                    default :
                        revCrd.push_back({planetCrd.at(j), j});
                    }

                std::sort(revCrd.begin(), revCrd.end());

                std::pair<double, int> rahuSum;
                std::pair<double, int> ketuSum;
                std::pair<double, int> bufSum;

                auto node (0);
                for (const auto& j : revCrd)
                {
                    switch (j.second) {
                    case 11 :
                        ketuSum.first += bufSum.first;
                        ketuSum.second += bufSum.second;
                        bufSum = {0, 0};
                        node = 11;
                        break;
                    case 24 :
                        rahuSum.first += bufSum.first;
                        rahuSum.second += bufSum.second;
                        bufSum = {0, 0};
                        node = 24;
                        break;
                    default :
                        bufSum.first += pairSum(primeStat.at(j.second));
                        ++bufSum.second;
                    }
                }

                if (node == 11)
                {
                    rahuSum.first += bufSum.first;
                    rahuSum.second += bufSum.second;
                }
                else
                {
                    ketuSum.first += bufSum.first;
                    ketuSum.second += bufSum.second;
                }

                status.first +=
                    rahuSum.second != 0 ? rahuSum.first / rahuSum.second : 0;
                primeFict[24].first +=
                    ketuSum.second != 0 ? ketuSum.first / ketuSum.second : 0;
            }
                break;
            case 56 :
            {
                std::pair<double, int> posSum;
                std::pair<double, int> negSum;

                for (auto j : AstroBase::noFictPlanet)
                {
                    auto primeSum (pairSum(primeStat.at(j)));
                    if (primeSum > 0)
                    {
                        posSum.first += primeSum;
                        ++posSum.second;
                    }
                    else if (primeSum < 0)
                    {
                        negSum.first += primeSum;
                        ++negSum.second;
                    }
                }

                status.first +=
                    posSum.second != 0 ? posSum.first / posSum.second : 0;
                primeFict[12].first -=
                    negSum.second != 0 ? negSum.first / negSum.second : 0;
            }
                break;
            }
        }

    // Part III
    for (auto& i : baseForce)
    {
        if (i.second == ForceFlag::Dominant)
        {
            if (isGloba && coreForce.total >= 0)
                coreForce.type = 'F';
            else if (!isGloba)
                coreForce.type = 'S';
            break;
        }
    }

    if (!isGloba && coreForce.type == 'S')
    {
        auto elemBase (4);
        for (auto i (0); i < 4; ++i)
            if (baseForce[i].second == ForceFlag::Dominant)
            {
                elemBase = i;
                break;
            }
        if (elemBase > 1)
            elemBase -= 2;

        if (zoneForce[elemBase].second == ForceFlag::Dominant &&
            crosForce[elemBase].second == ForceFlag::Dominant)
            coreForce.type = 'F';
    }
}


auto PrimeTest::computeAlmuten() -> void
{
    auto& cuspidCrd (astroBase_.getCuspidCrd());
    auto& cuspidSignNo (astroBase_.getCuspidSignNo());

    almuten_.clear();

    if (!almuHard_)
    {// Compute by cuspid
        for (auto i (0); i < 12; ++i)
        {
            auto sign = cuspidSignNo[i];
            almuten_[{RefBook::signRuler[sign][0], i}] = true;
            almuten_[{RefBook::signOppos[sign][0], i}] = false;
        }
        return;
    }

    for (auto i (0); i < 12; ++i)
    {// Compute by middle point
        auto nextCuspid (i < 11 ? i + 1 : 0);
        auto sign (cuspidSignNo[i]);
        auto almuSign (NONE);

        if (i == 0 || i == 3 || i == 6 || i == 9 ||
            sign == cuspidSignNo[nextCuspid])
            almuSign = sign;
        else
        {
            auto nextSign (sign < 11 ? sign + 1 : 0);
            if (nextSign == cuspidSignNo[nextCuspid])
            {
                auto len (30 * nextSign - cuspidCrd[i]);
                if (len < 0)
                    len += 360;

                if ((30 - len) / len >= 1.5)
                    almuSign = nextSign;
                else
                    almuSign = sign;
            }
            else
                almuSign = nextSign;
        }

        almuten_[{RefBook::signRuler[almuSign][0], i}] = true;
        almuten_[{RefBook::signOppos[almuSign][0], i}] = false;
    }
}


auto PrimeTest::computeMapType(int view) -> void
{
    auto& planetSiteNo (view == 0 ? astroBase_.getPlanetSignNo() : astroBase_.getPlanetHouseNo());

    auto& mapType (mapType_[view]);

    std::vector<double> force (12, 0);
    for (auto i : AstroBase::planetOrder)
        switch (i) {
        case 11 :
        case 12 :
        case 24 :
        case 56 :
            force[planetSiteNo.at(i)] += 0.5;
        break;
        default :
            force[planetSiteNo.at(i)] += 1;
        }

    auto sunSign (planetSiteNo.at(0));
    auto moonSign (planetSiteNo.at(1));
    auto sunForce (force[sunSign]);
    auto moonForce (force[moonSign]);
    auto coreNum (0);

    for (auto i (0); i < 12; ++i)
        if (force[i] >= 3 && i != sunSign && i != moonSign)
            ++coreNum;

    if (sunForce >= 3 && moonForce < 2 && coreNum == 0)
        mapType = {QApplication::tr("S"), 0};
    else if (sunForce < 2 && moonForce >= 3 && coreNum == 0)
        mapType = {QApplication::tr("M"), 1};
    else if (sunForce >= 3 && sunSign == moonSign && coreNum == 0)
        mapType = {QApplication::tr("C"), 2};
    else if (sunForce < 2 && moonForce < 2 && coreNum == 1)
        mapType = {QApplication::tr("P"), 2};
    else if (sunForce < 2 && moonForce < 2 && coreNum == 0)
        mapType = {QApplication::tr("D"), 2};
    else if (sunForce >= 3 && moonForce >= 3 && sunSign != moonSign && coreNum == 0)
        mapType = {QApplication::tr("S-M"), 2};
    else if (sunForce >= 3 && moonForce < 2 && coreNum == 1)
        mapType = {QApplication::tr("S-P"), 0};
    else if (sunForce < 2 && moonForce >= 3 && coreNum == 1)
        mapType = {QApplication::tr("M-P"), 1};
    else if (sunForce >= 3 && sunSign == moonSign && coreNum == 1)
        mapType = {QApplication::tr("C-P"), 2};
    else if (sunForce < 2 && moonForce < 2 && coreNum == 2)
        mapType = {QApplication::tr("P-P"), 2};
    else if (sunForce >= 3 && moonForce >= 3 && sunSign != moonSign && coreNum == 1)
        mapType = {QApplication::tr("S-M-P"), 2};
    else if (sunForce >= 3 && sunSign == moonSign && coreNum == 2)
        mapType = {QApplication::tr("C-P-P"), 2};
    else if (sunForce >= 3 && moonForce < 2 && coreNum == 2)
        mapType = {QApplication::tr("S-P-P"), 0};
    else if (sunForce < 2 && moonForce >= 3 && coreNum == 2)
        mapType = {QApplication::tr("M-P-P"), 1};
    else
        mapType = {"", NONE};
}


auto PrimeTest::setStatus(std::array<double, 2>& status, int multip, int forceFlag) -> void
{
    switch (forceFlag) {
    case ForceFlag::Dominant :
        status[0] += multip;
        break;
    case ForceFlag::Weak :
        status[1] -= multip;
    }
}


auto CoreForce::makeCompare(int key, const std::vector<double>& src) -> void
{
    Q_ASSERT(key >= 0 && key <= 8);

    auto res (begin() + key);
    res->clear();

    auto& data (key < 7 ? src : std::vector<double>{src[0] + src[2], src[1] + src[3]});

    for (auto i (data.begin()); i != data.end(); ++i)
    {
        std::set<int> buf;
        for (auto j (data.begin()); j != data.end(); ++j)
        {
            if (i == j)
                continue;

            buf.insert(checkRelation(*i, *j));
        }
        if (buf.size() == 1)
            res->push_back({*i, *buf.begin()});
        else
            res->push_back({*i, 0});
    }
}

} // namespace napatahti
