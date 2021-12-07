#include <QApplication>
#include "shared.h"
#include "RefBook.h"
#include "AstroBase.h"
#include "AshaTest.h"

namespace napatahti {

auto AshaTest::update() -> void
{
    powerKindness_.clear();
    powerCreative_.clear();
    kindDetailed_.clear();
    ashaSpec_ = {};

    for (auto i : AstroBase::noFictPlanet)
    {
        powerCreative_[i] = {0, 0};
        kindDetailed_[i] = {0, 0};
    }

    auto& planetCrd (astroBase_.getPlanetCrd());
    auto& planetSignNo (astroBase_.getPlanetSignNo());
    auto& planetHouseNo (astroBase_.getPlanetHouseNo());
    auto& cuspidCrd (astroBase_.getCuspidCrd());

    auto rev (cuspidCrd[0] - planetCrd.at(0));

    if (rev > 180)
        rev = -360 - rev;
    else if (rev < -180)
        rev = 360 - rev;

    rev = rev > 0 ? 1 : -1;

    std::array<double, 9> extraPoint {
        cuspidCrd[0], cuspidCrd[9],
        cuspidCrd[0] + rev * (planetCrd.at(1) - planetCrd.at(0)), // Asc, MC, Fortuna
        cuspidCrd[1], cuspidCrd[4],
        cuspidCrd[0] + rev * (planetCrd.at(3) - planetCrd.at(5)), // II, V, Happiness
        cuspidCrd[7], cuspidCrd[10],
        cuspidCrd[0] + rev * (planetCrd.at(4) - planetCrd.at(6))  // VII, XI, Fate
    };

    for (auto i : {2, 5, 8})
    {
        if (extraPoint[i] >= 360)
            extraPoint[i] -= 360;
        else if (extraPoint[i] < 0)
            extraPoint[i] += 360;
    }

    for (auto i : AstroBase::noFictPlanet)
    {
        auto crd (planetCrd.at(i));
        auto sign (planetSignNo.at(i));
        auto houseRuler (RefBook::signRuler[planetHouseNo.at(i)][0]);
        auto signRuler (RefBook::signRuler[sign]);
        auto isBothRuler (signRuler[1] != NONE);
        auto elevator (RefBook::signEssen[sign][0]);
        auto degreeRuler (RefBook::degreeRuler[static_cast<int>(crd)]);
        auto thermRuler (RefBook::thermRuler[static_cast<int>(crd / 5)]);
        auto decanRuler (RefBook::decanRuler[static_cast<int>(crd / 10)]);

        powerCreative_[houseRuler][0] += 6;
        powerCreative_[signRuler[0]][0] += 5;
        if (isBothRuler)
            powerCreative_[signRuler[1]][0] += 4;
        powerCreative_[elevator][0] += 4;
        powerCreative_[degreeRuler][0] += 3;
        powerCreative_[thermRuler][0] += 2;
        powerCreative_[decanRuler][0] += 1;

        switch (i) {
        case 0 :
        case 1 :
            powerCreative_[signRuler[0]][1] += 5;
            if (isBothRuler)
                powerCreative_[signRuler[1]][1] += 4;
            powerCreative_[elevator][1] += 4;
            powerCreative_[degreeRuler][1] += 3;
            powerCreative_[thermRuler][1] += 2;
            powerCreative_[decanRuler][1] += 1;
            break;
        case 3 :
        case 5 :
            kindDetailed_[signRuler[0]][0] += 5;
            if (isBothRuler)
                kindDetailed_[signRuler[1]][0] += 4;
            kindDetailed_[elevator][0] += 4;
            kindDetailed_[degreeRuler][0] += 3;
            kindDetailed_[thermRuler][0] += 2;
            kindDetailed_[decanRuler][0] += 1;
            break;
        case 4 :
        case 6 :
            kindDetailed_[signRuler[0]][1] -= 5;
            if (isBothRuler)
                kindDetailed_[signRuler[1]][1] -= 4;
            kindDetailed_[elevator][1] -= 4;
            kindDetailed_[degreeRuler][1] -= 3;
            kindDetailed_[thermRuler][1] -= 2;
            kindDetailed_[decanRuler][1] -= 1;
        }
    }

    auto ind (0);
    for (auto crd : extraPoint)
    {
        auto sign (static_cast<int>(crd / 30));
        auto signRuler (RefBook::signRuler[sign]);
        auto isBothRuler (signRuler[1] != NONE);
        auto elevator (RefBook::signEssen[sign][0]);
        auto degreeRuler (RefBook::degreeRuler[static_cast<int>(crd)]);
        auto thermRuler (RefBook::thermRuler[static_cast<int>(crd / 5)]);
        auto decanRuler (RefBook::decanRuler[static_cast<int>(crd / 10)]);

        switch (ind) {
        case 0 :
        case 1 :
        case 2 :
            powerCreative_[signRuler[0]][1] += 5;
            if (isBothRuler)
                powerCreative_[signRuler[1]][1] += 4;
            powerCreative_[elevator][1] += 4;
            powerCreative_[degreeRuler][1] += 3;
            powerCreative_[thermRuler][1] += 2;
            powerCreative_[decanRuler][1] += 1;
            break;
        case 3 :
        case 4 :
        case 5 :
            kindDetailed_[signRuler[0]][0] += 5;
            if (isBothRuler)
                kindDetailed_[signRuler[1]][0] += 4;
            kindDetailed_[elevator][0] += 4;
            kindDetailed_[degreeRuler][0] += 3;
            kindDetailed_[thermRuler][0] += 2;
            kindDetailed_[decanRuler][0] += 1;
            break;
        case 6 :
        case 7 :
        case 8 :
            kindDetailed_[signRuler[0]][1] -= 5;
            if (isBothRuler)
                kindDetailed_[signRuler[1]][1] -= 4;
            kindDetailed_[elevator][1] -= 4;
            kindDetailed_[degreeRuler][1] -= 3;
            kindDetailed_[thermRuler][1] -= 2;
            kindDetailed_[decanRuler][1] -= 1;
        }
        ++ind;
    }

    std::array<std::set<double>, 2> buf;
    for (const auto& i : powerCreative_)
    {
        auto sum (pairSum(kindDetailed_.at(i.first)));
        powerKindness_[i.first] = {i.second[0], sum};
        buf[0].insert(i.second[0]);
        buf[1].insert(sum);
    }

    auto maxPower (*(--buf[0].end()));
    auto minMaxKind (std::array<double, 2>{*buf[1].begin(), *(--buf[1].end())});

    for (const auto& i : powerKindness_)
    {
        if (i.second[1] == minMaxKind[0])
            ashaSpec_.anareta.insert(i.first);
        else if (i.second[1] == minMaxKind[1])
            ashaSpec_.alcocoden.insert(i.first);

        if (i.second[0] == maxPower)
            ashaSpec_.dGenitura.insert(i.first);
    }

    for (auto spec (ashaSpec_.begin()); spec != ashaSpec_.end(); ++spec)
    {
        auto k (spec == ashaSpec_.dGenIt() ? 1 : 0);

        if (spec->size() > 1)
        {
            auto maxValue (0.0);
            for (auto i : *spec)
            {
                auto value (powerCreative_.at(i)[k]);
                if (value > maxValue)
                    maxValue = value;
            }

            std::set<int> buf;
            for (auto i : *spec)
            {
                if (powerCreative_.at(i)[k] == maxValue)
                    buf.insert(i);
            }
            *spec = std::move(buf);
        }
    }
}


auto AshaTest::toStrAshaStat() const -> QString
{
    QString str;
    QTextStream out (&str);

    std::array<QString, 3> titles {"Anareta", "Alcocoden", "Dominus Genitura"};

    out << QApplication::tr("Asha test", "dump statistic header")
        << "\n--------------------------------------------------\n\n";

    for (auto& i : AstroBase::noFictPlanet)
    {
        auto& powerKindness (powerKindness_.at(i));
        auto& kindDetailed (kindDetailed_.at(i));

        out.setFieldWidth(10);
        out.setFieldAlignment(QTextStream::AlignRight);
        out << AstroBase::getPlanetName(i);

        out.reset();
        out << ' ';
        out.setFieldWidth(3);
        out.setFieldAlignment(QTextStream::AlignRight);
        out << powerKindness[0];

        out.reset();
        out << ", ";
        out.setFieldWidth(3);
        out.setFieldAlignment(QTextStream::AlignRight);
        if (powerKindness[1] > 0)
            out.setNumberFlags(QTextStream::ForceSign);
        out << powerKindness[1];

        out.reset();
        out << " ( ";
        out.setFieldWidth(3);
        out.setFieldAlignment(QTextStream::AlignRight);
        if (kindDetailed[0] > 0)
            out.setNumberFlags(QTextStream::ForceSign);
        out << kindDetailed[0];

        out.reset();
        out << ", ";
        out.setFieldWidth(3);
        out.setFieldAlignment(QTextStream::AlignRight);
        out << kindDetailed[1];

        out.reset();
        out << " ), ";
        out.setFieldWidth(3);
        out.setFieldAlignment(QTextStream::AlignRight);
        out << powerCreative_.at(i)[1];
        out.reset();
        out << '\n';
    }

    out << "\n--------------------------------------------------\n";

    auto title (titles.cbegin());
    for (auto& spec : ashaSpec_)
    {
        out << '\n' << *title << " : ";

        auto k (*(--spec.end()));
        for (auto i : spec)
            out << AstroBase::getPlanetName(i) << (i != k ? " " : "");

        ++title;
    }

    return str;
}

} // namespace napatahti
