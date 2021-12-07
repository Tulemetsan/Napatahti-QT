#include <QApplication>
#include "shared.h"
#include "RefBook.h"
#include "AstroBase.h"
#include "AspTable.h"
#include "CosmicTest.h"

namespace napatahti {

auto CosmicTest::update() -> void
{
    computeRecepTree();
    computeCosmicSpec();
    computeCosmicStat();

    cosmicSum_ = 0;
    summary_.clear();

    for (const auto& i : detailed_)
    {
        auto sum (pairSum(i.second));
        summary_[i.first] = {0, sum};
        cosmicSum_ += sum;
    }
}


auto CosmicTest::toStrCosmicStat() const -> QString
{
    QString str;
    QTextStream out (&str);

    std::array<QString, 3> title {
        QApplication::tr("Total"), QApplication::tr("Royal"), QApplication::tr("Absolute")
    };

    auto width (0);
    for (const auto& i : title)
        if (width < i.size())
            width = i.size();

    std::array<std::set<int>, 2> spec;

    out << QApplication::tr("Cosmic test")
        <<"\n--------------------------------------------------\n\n";

    for (auto i : AstroBase::noFictPlanet)
    {
        auto& status (detailed_.at(i));

        out.setFieldWidth(10);
        out.setFieldAlignment(QTextStream::AlignRight);
        out << AstroBase::getPlanetName(i);

        out.reset();
        out << ' ';
        out.setFieldWidth(5);
        auto sum (summary_.at(i)[1]);
        if (sum > 0)
            out.setNumberFlags(QTextStream::ForceSign);
        out << sum;

        out.reset();
        out << "  ( ";
        out.setFieldWidth(5);
        if (status[0] > 0)
            out.setNumberFlags(QTextStream::ForceSign);
        out << status[0];

        out.reset();
        out << ", ";
        out.setFieldWidth(5);
        out << status[1];
        out.reset();
        out << " )\n";

        if (sum >= 10 || sum <= -10)
            spec[0].insert(i);
        if (status[0] != status[1] && (status[0] == 0 || status[1] == 0))
            spec[1].insert(i);
    }

    out <<"\n--------------------------------------------------\n\n";

    for (auto i : {0, 1, 2})
        switch (i) {
        case 0 :
            out.setFieldWidth(width);
            out.setFieldAlignment(QTextStream::AlignRight);
            out << title[i];
            out.reset();
            out << " : " << cosmicSum_;
            break;
        default :
            if (spec[i-1].empty())
                continue;
            out.setFieldWidth(width);
            out.setFieldAlignment(QTextStream::AlignRight);
            out << '\n' << title[i];
            out.reset();
            out << " : ";

            auto k (--spec[i-1].end());
            for (auto j (spec[i-1].begin()); j != spec[i-1].end(); ++j)
                out << AstroBase::getPlanetName(*j) << (j != k ? ", " : "");
        }

    return str;
}


auto CosmicTest::computeCosmicStat() -> void
{
    detailed_.clear();

    auto& planetCrd (astroBase_.getPlanetCrd());
    auto& planetSignNo (astroBase_.getPlanetSignNo());
    auto& planetDegree (astroBase_.getPlanetDegree());
    auto& aspTabSpec (aspTable_.getAspTabSpec());
    auto& aspField (aspTable_.getAspField());

    auto ang (planetCrd.at(1) - planetCrd.at(0));
    if (ang < 0)
        ang += 360;

    std::array<int, 2> moonPhaseElem {NONE, NONE};
    switch (static_cast<int>(ang / 90)) {
    case 0 :
        moonPhaseElem = {2, 1};
        break;
    case 1 :
        moonPhaseElem = {0, 3};
        break;
    case 2 :
        moonPhaseElem = {1, 2};
        break;
    case 3 :
        moonPhaseElem = {3, 0};
    }

    // Part I
    for (auto i : AstroBase::noFictPlanet)
    {
        auto& status (detailed_[i]);

        auto crd (planetCrd.at(i));
        auto sign (planetSignNo.at(i));
        auto degreeA (static_cast<int>(crd));
        auto degreeB (planetDegree.at(i));
        auto posElem (RefBook::planetPosElem.at(i));
        auto negElem (RefBook::planetNegElem.at(i));
        auto signElem (RefBook::signElem[sign]);
        auto negPlanet (RefBook::planetNegPlanet.at(i));
        auto degreeRuler (RefBook::degreeRuler[degreeA]);
        auto thermRuler (RefBook::thermRuler[static_cast<int>(crd / 5)]);
        auto decanRuler (RefBook::decanRuler[static_cast<int>(crd / 10)]);

        auto spec (RefBook::specState.find({i, sign}));
        if (spec != RefBook::specState.end())
        {
            switch (spec->second) {
            case SpecState::Ruler :
                status[0] += 5;
                break;
            case SpecState::Detriment :
                status[1] -= 5;
                break;
            case SpecState::Exaltation :
                status[0] += 4;
                break;
            case SpecState::Fall :
                status[1] -= 4;
            }
            if ((i == 2 && sign == 5) || (i == 57 && sign == 2))
                status[0] += 3; // exception (Mer, Vir), (Pro, Gem): 7
            else if ((i == 2 && sign == 11) || (i == 57 && sign == 8))
                status[1] -= 3; // exception (Mer, Psc), (Pro, Sag): -7
        }

        if (posElem == signElem)
            status[0] += 4;
        else if (negElem == signElem)
            status[1] -= 4;

        if (i != 1)
        {
            if (posElem == moonPhaseElem[0])
                status[0] += 1;
            else if (negElem == moonPhaseElem[0])
                status[1] -= 1;
        }
        else
        {
            if (signElem == moonPhaseElem[0])
                status[0] += 2;
            else if (signElem == moonPhaseElem[1])
                status[1] -= 2;
        }

        auto kad (RefBook::degreeKad.find(degreeA));
        if (kad != RefBook::degreeKad.end())
        {
            if (kad->second)
                status[0] += 3;
            else
                status[1] -= 3;
        }

        auto exal (RefBook::degreeExal.find(degreeA));
        if (exal != RefBook::degreeExal.end())
        {
            if (i == exal->second)
                status[0] += 3;
            else
            {
                status[1] -= 1;
                detailed_[exal->second][0] += 1;
            }
        }

        auto fall (RefBook::degreeFall.find(degreeA));
        if (fall != RefBook::degreeFall.end())
        {
            if (i == fall->second)
                status[1] -= 3;
            else
            {
                status[0] += 1;
                detailed_[fall->second][1] -= 1;
            }
        }

        switch (RefBook::planetPosBase.at(i)) {
        case 0 :
        {
            switch (degreeB) {
            case 1 :
                status[0] += 2;
                break;
            case 30 :
                status[1] -= 2;
            }

            switch (aspField[AspGroup::Black].second) {
            case ForceFlag::Dominant :
                status[0] += 2;
                break;
            case ForceFlag::Weak :
                status[1] -= 2;
            }

            switch (aspField[AspGroup::Green].second) {
            case ForceFlag::Dominant :
                status[0] += 1;
                break;
            case ForceFlag::Weak :
                status[1] -= 1;
            }
        }
            break;
        case 1 :
        {
            switch (degreeB) {
            case 1 :
                status[1] -= 2;
                break;
            case 30 :
                status[0] += 2;
            }

            switch (aspField[AspGroup::Red].second) {
            case ForceFlag::Dominant :
                status[0] += 2;
                break;
            case ForceFlag::Weak :
                status[1] -= 2;
            }

            switch (aspField[AspGroup::Blue].second) {
            case ForceFlag::Dominant :
                status[0] += 1;
                break;
            case ForceFlag::Weak :
                status[1] -= 1;
            }
        }
            break;
        case 2 :
        {
            if (i == 2 && sign == 5)
                switch (degreeB) {
                case 16 :
                    status[0] += 2;
                    break;
                case 17 :
                    status[1] -= 2;
                }
            else
                switch (degreeB) {
                case 15 :
                    status[0] += 2;
                    break;
                case 16 :
                    status[1] -= 2;
                }

            if (aspField[AspGroup::Black].second == ForceFlag::Neutral)
                status[0] += 2;
            if (aspField[AspGroup::Green].second == ForceFlag::Neutral)
                status[0] += 1;
        }
            break;
        }

        if (i == degreeRuler)
            status[0] += 2;
        else
        {
            auto relative (RefBook::planetRelative.at(i));
            if (contains(degreeRuler, relative[0]))
                status[0] += 1;
            else if (contains(degreeRuler, relative[1]))
                status[1] -= 1;
        }

        if (i == thermRuler)
            status[0] += 1;
        else if (negPlanet == thermRuler)
            status[1] -= 1;

        if (i == decanRuler)
            status[0] += 1;
        else if (negPlanet == decanRuler)
            status[1] -= 1;
    }

    // Part II
    auto ind (0);
    for (const auto& spec : cosmicSpec_)
    {
        if (spec.empty())
        {
            ++ind;
            continue;
        }

        switch (ind) {
        case CosmicSpec::Atlas :
        case CosmicSpec::Sisyphus :
        case CosmicSpec::Antaeus :
        case CosmicSpec::Icarus :
        case CosmicSpec::Doriphoros :
        case CosmicSpec::Auriga :
        {
            auto k (spec.size() == 1 ? 3 : 1.5);
            for (auto i : spec)
                switch (ind) {
                case CosmicSpec::Atlas :
                case CosmicSpec::Antaeus :
                case CosmicSpec::Doriphoros :
                    detailed_[i][0] += k;
                    break;
                default :
                    detailed_[i][1] -= k;
                }
        }
            break;
        case CosmicSpec::Heart :
        case CosmicSpec::Burning :
        case CosmicSpec::Est :
        case CosmicSpec::West :
        case CosmicSpec::FreeZone :
        case CosmicSpec::SunZone :
        {
            for (auto i : spec)
            {
                switch (i) {
                case 11 :
                case 12 :
                case 24 :
                case 56 :
                    continue;
                }

                switch (ind) {
                case CosmicSpec::Heart :
                    detailed_[i][0] += 3;
                    break;
                case CosmicSpec::Burning :
                    detailed_[i][1] -= 3;
                    break;
                default :
                    switch (RefBook::planetPosBase.at(i)) {
                    case 0 :
                        switch (ind) {
                        case CosmicSpec::Est :
                            detailed_[i][0] += 3;
                            break;
                        case CosmicSpec::West :
                            detailed_[i][1] -= 3;
                        }
                        break;
                    case 1 :
                        switch (ind) {
                        case CosmicSpec::Est :
                            detailed_[i][1] -= 3;
                            break;
                        case CosmicSpec::West :
                            detailed_[i][0] += 3;
                        }
                        if (i == 1)
                            switch (ind) {
                            case CosmicSpec::FreeZone :
                                detailed_[i][0] += 1;
                                break;
                            case CosmicSpec::SunZone :
                                detailed_[i][1] -= 1;
                            }
                        break;
                    case 2 :
                        switch (ind) {
                        case CosmicSpec::FreeZone :
                            detailed_[i][0] += 3;
                            break;
                        case CosmicSpec::SunZone :
                            detailed_[i][1] -= 3;
                        }
                    }
                }
            }
        }
            break;
        }

        ++ind;
    }

    // Part III
    ind = 0;
    for (auto& spec : aspTabSpec)
    {
        if (spec.empty())
        {
            ++ind;
            continue;
        }

        for (auto i : spec)
        {
            switch (i) {
            case 11 :
            case 12 :
            case 24 :
            case 56 :
                continue;
            }

            switch (ind) {
            case AspTabSpec::RexAsp :
                detailed_[i][0] += (spec.size() == 1 ? 3 : 1.5);
                break;
            case AspTabSpec::InPit :
                detailed_[i][1] -= 3;
                break;
            case AspTabSpec::TopRedGreen :
            case AspTabSpec::Arrow :
                detailed_[i][0] += 1;
                break;
            case AspTabSpec::TopBlackBlue :
            case AspTabSpec::Bed :
                detailed_[i][1] -= 1;
            }
        }

        ++ind;
    }

    for (auto& signSeq : aspTabSpec.stelliumSign)
        for (auto i : signSeq)
        {
            detailed_[RefBook::signRuler[i][0]][0] += 3;
            detailed_[RefBook::signEssen[i][0]][0] += 1;
            detailed_[RefBook::signOppos[i][0]][1] -= 3;
            detailed_[RefBook::signEssen[i][1]][1] -= 1;
        }

    // Part IV
    ind = 0;
    for (const auto& branch : recepTree_)
    {
        if (branch.empty())
        {
            ++ind;
            continue;
        }

        auto k (ind == RecepTree::ByRuler || ind == RecepTree::ByDetriment ? 2 : 1);
        for (auto& i : branch)
            switch (ind) {
            case RecepTree::ByRuler :
            case RecepTree::ByExaltation :
            case RecepTree::ByDegree :
                detailed_[i.less][0] += k;
                detailed_[i.more][0] += k;
                break;
            default :
                detailed_[i.less][1] -= k;
                detailed_[i.more][1] -= k;
            }

        ++ind;
    }

    // Part V
    std::array<std::set<int>, 4> posRuler;
    std::array<std::set<int>, 4> negRuler;

    for (auto i : {0, 1, 12, 56})
    {
        auto sign (planetSignNo.at(i));
        switch (i) {
        case 0 :
        case 56 :
            posRuler[0].insert(RefBook::signRuler[sign][0]);
            posRuler[2].insert(RefBook::signEssen[sign][0]);

            negRuler[0].insert(RefBook::signOppos[sign][0]);
            negRuler[2].insert(RefBook::signEssen[sign][1]);
            break;
        case 1 :
            posRuler[1].insert(RefBook::signRuler[sign][0]);
            posRuler[3].insert(RefBook::signEssen[sign][0]);

            negRuler[1].insert(RefBook::signOppos[sign][0]);
            negRuler[3].insert(RefBook::signEssen[sign][1]);
            break;
        case 12 :
            posRuler[0].insert(RefBook::signOppos[sign][0]);
            posRuler[2].insert(RefBook::signEssen[sign][1]);

            negRuler[0].insert(RefBook::signRuler[sign][0]);
            negRuler[2].insert(RefBook::signEssen[sign][0]);
            break;
        }
    }

    for (auto i (0); i < 4; ++i)
    {
        for (auto j : posRuler[i])
            detailed_[j][0] += 4 - i;
        for (auto j : negRuler[i])
            detailed_[j][1] -= 4 - i;
    }
}


auto CosmicTest::computeRecepTree() -> void
{
    recepTree_ = {};
    planetChain_ = {};

    auto& planetSignNo (astroBase_.getPlanetSignNo());
    auto& planetCrd (astroBase_.getPlanetCrd());

    for (auto i : AstroBase::noFictPlanet)
    {
        auto sign (planetSignNo.at(i));
        auto ruler (RefBook::signRuler[sign]);
        auto oppos (RefBook::signOppos[sign]);
        auto essen (RefBook::signEssen[sign]);

        addToRecepTree(i, ruler[0], RecepTree::ByRuler);
        addToRecepTree(i, ruler[1], RecepTree::ByRuler);
        addToPlanetChain(i, ruler[0], RecepTree::ByRuler);
        addToPlanetChain(i, ruler[1], RecepTree::ByRuler);

        addToRecepTree(i, oppos[0], RecepTree::ByDetriment);
        addToRecepTree(i, oppos[1], RecepTree::ByDetriment);
        addToPlanetChain(i, oppos[0], RecepTree::ByDetriment);
        addToPlanetChain(i, oppos[1], RecepTree::ByDetriment);

        addToRecepTree(i, essen[0], RecepTree::ByExaltation);
        addToPlanetChain(i, essen[0], RecepTree::ByExaltation);
        addToRecepTree(i, essen[1], RecepTree::ByFall);
        addToPlanetChain(i, essen[1], RecepTree::ByFall);

        addToRecepTree(i, RefBook::degreeRuler[
            static_cast<int>(planetCrd.at(i))], RecepTree::ByDegree);
    }

    for (auto& i : recepTree_)
        recepTree_.total += i.size();

    for (auto& chain : planetChain_)
    {
        for (auto& i : chain)
            for (auto& j : chain)
                if (isIntxn(i, j))
                    for (auto k : i)
                        j.insert(k);

        auto chainBuf (std::move(chain));
        for (auto& inside : chainBuf)
            if (inside.size() >= 8)
            {
                chain = {std::move(inside)};
                break;
            }
    }
}


auto CosmicTest::computeCosmicSpec() -> void
{
    cosmicSpec_ = {};

    auto& planetCrd (astroBase_.getPlanetCrd());
    auto& planetSignNo (astroBase_.getPlanetSignNo());
    auto& planetX2table (aspTable_.getPlanetX2Table());

    // Part I
    std::array<std::set<int>, 4> specPlanet;

    for (auto i : AstroBase::noFictPlanet)
    {
        auto sign (planetSignNo.at(i));
        auto spec (RefBook::specState.find({i, sign}));
        if (spec != RefBook::specState.end())
        {
            specPlanet[spec->second].insert(i);
            if (i == 2 && (sign == 5 || sign == 11))
                specPlanet[spec->second].insert(i);
        }
    }

    auto ind (0);
    for (const auto& chain : planetChain_)
    {
        if (chain.empty())
        {
            ++ind;
            continue;
        }

        std::set<int> buf;
        if (!specPlanet[ind].empty())
        {
            for (auto i : specPlanet[ind])
                if (contains(i, chain[0]))
                    buf.insert(i);
        }
        else
        {
            for (auto& i : recepTree_[ind])
                if (isInclude(i, chain[0]))
                {
                    buf.insert(i.less);
                    buf.insert(i.more);
                }
        }

        switch (buf.size())
        {
            case 1 :
            case 2 :
                cosmicSpec_[ind] = std::move(buf);
        }

        ++ind;
    }

    // Part II
    std::vector<std::pair<double, int>> buf;

    for (auto i : AstroBase::noFictPlanet)
        buf.push_back({planetCrd.at(i), i});

    std::sort(buf.begin(), buf.end());

    auto sunNo (0);
    for (const auto& i : buf)
    {
        if (i.second == 0)
            break;

        ++sunNo;
    }

    auto zp (buf[sunNo].first);
    for (const auto& i : buf)
        if (i.second != 0)
        {
            auto ang (i.first - zp);
            if (ang < 0)
                ang += 360;

            if (ang > 0 && ang < 180)
                cosmicSpec_.west.insert(i.second);
            else if (ang > 180)
                cosmicSpec_.est.insert(i.second);
        }

    std::array<int, 2> doriph;
    std::array<int, 2> auriga;

    switch (sunNo)
    {
        case 0 :
            doriph = {buf[11].second, buf[10].second};
            auriga = {buf[sunNo+1].second, buf[sunNo+2].second};
            break;
        case 1 :
            doriph = {buf[0].second, buf[11].second};
            auriga = {buf[sunNo+1].second, buf[sunNo+2].second};
            break;
        case 10 :
            doriph = {buf[sunNo-1].second, buf[sunNo-2].second};
            auriga = {buf[11].second, buf[0].second};
            break;
        case 11 :
            doriph = {buf[sunNo-1].second, buf[sunNo-2].second};
            auriga = {buf[0].second, buf[1].second};
            break;
        default :
            doriph = {buf[sunNo-1].second, buf[sunNo-2].second};
            auriga = {buf[sunNo+1].second, buf[sunNo+2].second};
    }

    auto& dataDoriph (planetX2table.at(doriph));
    auto& dataAuriga (planetX2table.at(auriga));

    cosmicSpec_.doriph.insert(doriph[0]);
    if (dataDoriph.asp == 0 && dataDoriph.acc == 1)
        cosmicSpec_.doriph.insert(doriph[1]);

    cosmicSpec_.auriga.insert(auriga[0]);
    if (dataAuriga.asp == 0 && dataAuriga.acc == 1)
        cosmicSpec_.auriga.insert(auriga[1]);

    // Part III
    for (auto i : AstroBase::planetOrder)
    {
        if (i == 0)
            continue;

        auto ang (planetX2table.at({0, i}).ang);

        if (ang < 0.2833334)
            cosmicSpec_.heart.insert(i);
        if (ang >= 0.283334 && ang < 3 && !contains(i, AstroBase::fictPlanet))
            cosmicSpec_.burning.insert(i);

        if (i != 2 && ang >= 3 && ang < 30)
            cosmicSpec_.sunZone.insert(i);
        else if (i == 2 && ang >= 15 && ang < 19)
            cosmicSpec_.sunZone.insert(i);

        if (ang > 160)
            cosmicSpec_.freeZone.insert(i);
        else if (i == 2 && ang >= 25)
            cosmicSpec_.freeZone.insert(i);
    }
}


auto CosmicTest::addToRecepTree(int keyA, int keyB, int type) -> void
{
    if (keyA == keyB || keyB == NONE || keyA == NONE)
        return;

    switch (type)
    {
    case RecepTree::ByRuler :
    case RecepTree::ByDetriment :
    {
        auto& planetSignNo (astroBase_.getPlanetSignNo());
        auto ruler (type == RecepTree::ByRuler
            ? RefBook::signRuler[planetSignNo.at(keyB)]
            : RefBook::signOppos[planetSignNo.at(keyB)]);

        if (keyA == ruler[0])
            recepTree_[type].insert({keyA, keyB});
        if (keyA == ruler[1])
            recepTree_[type].insert({keyA, keyB});
    }
        break;
    case RecepTree::ByExaltation :
    case RecepTree::ByFall :
    {
        auto& planetSignNo (astroBase_.getPlanetSignNo());
        auto essen (type == RecepTree::ByExaltation
            ? RefBook::signEssen[planetSignNo.at(keyB)][0]
            : RefBook::signEssen[planetSignNo.at(keyB)][1]);

        if (keyA == essen)
            recepTree_[type].insert({keyA, keyB});
    }
        break;
    case RecepTree::ByDegree :
        auto degree (static_cast<int>(astroBase_.getPlanetCrd().at(keyB)));

        if (keyA == RefBook::degreeRuler[degree])
            recepTree_[type].insert({keyA, keyB});
    }
}


auto CosmicTest::addToPlanetChain(int keyA, int keyB, int chainNo) -> void
{
    if (keyB == NONE || keyA == NONE)
        return;

    auto& chain (planetChain_[chainNo]);

    if (chain.empty())
        chain.push_back({keyA, keyB});
    else
    {
        auto isInside (false);
        for (auto& link : chain)
        {
            if (link.find(keyA) != link.end())
            {
                link.insert(keyB);
                isInside = true;
            }
            else if (link.find(keyB) != link.end())
            {
                link.insert(keyA);
                isInside = true;
            }
        }
        if (!isInside)
            chain.push_back({keyA, keyB});
    }
}

} // namespace napatahti
