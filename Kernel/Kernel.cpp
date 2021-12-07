#include <QApplication>
#include "shared.h"
#include "Kernel.h"
#include "Appgui/SharedGui.h"
#include "Appgui/MainWindow.h"
#include "Appgui/Canvas.h"
#include "Appgui/PersonDialog.h"

namespace napatahti {

Kernel::Kernel()
    : Person ()
    , AstroBase (person())
    , AspPage ()
    , AspTable (person(), astroBase(), aspPage())
    , PrimeTest (astroBase(), aspTable())
    , AshaTest (astroBase())
    , CosmicTest (astroBase(), aspTable())
    , trigger_ (MainWindow::getTrigger())
    , locale_ (SharedGui::getAppLocale())
{
    setHeaderStr();
    AstroBase::update(Canvas::getPlanetSpace(), locale_);
    AspTable::update(true, getAccKey(), trigger_.aspCfgClassOnly);
    PrimeTest::update();
    AshaTest::update();
    CosmicTest::update();
}


auto Kernel::setHeaderStr() -> void
{
    headerStr_.clear();

    QTextStream out (&headerStr_);
    QString format ("d MMMM yyyy ddd h:mm");

    format += dateTime.time().second() != 0 ? ":ss t" : " t";

    auto uLat (makeAsTime(lat));
    auto uLon (makeAsTime(lon));

    out << name << '\n'
        << locale_.toString(dateTime, format)
        << QString(" %1%2%3")
               .arg(uLat[0])
               .arg(lat < 0 ? 's' : 'n')
               .arg(uLat[1], 2, 10, QChar('0'))
        << QString(" %1%2%3")
               .arg(uLon[0])
               .arg(lon < 0 ? 'w' : 'e')
               .arg(uLon[1], 2, 10, QChar('0'))
        << '\n' << location
        << "\n\n" << PersonDialog::hsysTitle().at(hsys) << "\n\n";
}


auto Kernel::getMoonDay() const -> const QStringList&
{
    if (trigger_.modeWestern)
        return westCache_;
    else
        return jyotCache_;
}


auto Kernel::toStrMoonCalendar() const -> QString
{
    QString str;
    QTextStream out (&str);

    QStringList tech {
        "d MMMM yyyy ddd h:mm:ss", "%1",
        QApplication::tr("Moon calendar"),
        QApplication::tr("Western"), QApplication::tr("Jyotisa"),
        QApplication::tr("New moon"), QApplication::tr("Full moon")
    };

    auto uLat (makeAsTime(lat));
    auto uLon (makeAsTime(lon));
    auto utc (dateTime.offsetFromUtc());

    out << tech[2] << " [ "
        << (trigger_.modeWestern ? tech[3] : tech[4]) << ", "
        << uLat[0] << (lat < 0 ? 's' : 'n')
        << tech[1].arg(uLat[1], 2, 10, QChar('0')) << ' '
        << uLon[0] << (lon < 0 ? 'w' : 'e')
        << tech[1].arg(uLon[1], 2, 10, QChar('0')) << " ]\n\n";

    for (auto& i : trigger_.modeWestern ? westCal_ : jyotCal_)
    {
        switch(i.first) {
        case 0 :
            continue;
        case 31 :
            out << '\n' << tech[5] << " - ";
            break;
        case 32 :
            out << tech[6] << " - ";
            break;
        default:
            out << tech[1].arg(i.first, 2)  << " - ";
        }

        out << locale_.toString(revJulDay(i.second, utc), tech[0]) << '\n';
    }

    return str;
}


auto Kernel::getAccKey() const -> int
{
    for (auto i (&trigger_.accMid); i <= &trigger_.accAny; ++i)
        if (*i)
            return i - &trigger_.accMid;

    return -1;
}


auto Kernel::getCoreForce(int view) const -> const CoreForce&
{
    if (trigger_.modeSchit)
        return *(&coreStat_.schitCos + view);
    else
        return *(&coreStat_.globaCos + view);
}


auto Kernel::getPlanetStat() const -> const std::map<int, std::array<double, 2>>&
{
    if (trigger_.modeAsha)
    {
        if (trigger_.ashaPowKind)
            return powerKindness_;
        else if (trigger_.ashaPowCreat)
            return powerCreative_;
        else
            return kindDetailed_;

    }
    else if (trigger_.modePrime)
    {
        auto mode (trigger_.modeSchit ? 1 : 0);
        auto view (trigger_.primeNormal ? 2 : (trigger_.primeHorDet ? 1 : 0));

        return *(&primeStat_.globaCos + 3 * mode + view);
    }
    else
    {
        if (trigger_.cosmicSum)
            return summary_;
        else
            return detailed_;
    }
}


auto Kernel::getFictionStat() const -> const std::map<int, std::pair<double, int>>*
{
    if (trigger_.modePrime)
    {
        if (trigger_.modeSchit)
            return &primeFict_.schit;
        else
            return &primeFict_.globa;
    }

    return nullptr;
}


auto Kernel::toStrCoreStat() const -> QString
{
    QString str;
    QTextStream out (&str);

    std::array<QString, 8> titles {
        QApplication::tr("Elements"), QApplication::tr("Crosses"),
        QApplication::tr("Quadrants"), QApplication::tr("Zones"),
        QApplication::tr("Hemispheres NS"), QApplication::tr("Hemispheres EW"),
        QApplication::tr("Bases"), QApplication::tr("Core")
    };

    auto width (0);
    for (const auto& i : titles)
        if (i.size() > width)
            width = i.size();

    auto method (trigger_.modeSchit ? 1 : 0);

    out << QApplication::tr("Core statistic") << " [ "
        << (method == 0 ? QApplication::tr("Globa PP") : QApplication::tr("Schitov BB"))
        << " ]\n--------------------------------------------------";

    for (auto view : {0, 1})
    {
        auto& mapType (mapType_[view]);
        auto& coreForce (coreStat_.get(method, view));

        out << "\n\n--------------------------------------------------\n"
            << (view == 0 ? QApplication::tr("Cosmogram") : QApplication::tr("Horoscope"))
            << "\n--------------------------------------------------\n";

        auto title (titles.cbegin());
        for (auto& part : coreForce)
        {
            out.setFieldWidth(width);
            out.setFieldAlignment(QTextStream::AlignRight);
            out << *title;
            out.reset();
            out << " : ";

            auto back (--part.end());
            for (auto i (part.begin()); i != part.end(); ++i)
            {
                out << i->first;
                switch (i->second) {
                case ForceFlag::Dominant :
                    out << '+';
                    break;
                case ForceFlag::Weak :
                    out << '-';
                }
                out << (i != back ? ", " : "\n");
            }

            ++title;
        }

        out.setFieldWidth(width);
        out.setFieldAlignment(QTextStream::AlignRight);
        out << *title;
        out.reset();
        out << " : ";

        if (coreForce.total > 0)
            out.setNumberFlags(QTextStream::ForceSign);

        out << coreForce.type << ' ' << coreForce.total
            << (mapType.second != NONE ? ", " + mapType.first : "");
    }

    return str;
}


auto Kernel::toStrPrimeStat() const -> QString
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

    auto method (trigger_.modeSchit ? 1 : 0);

    out << QApplication::tr("Prime test") << " [ "
        << (method == 0 ? QApplication::tr("Globa PP") : QApplication::tr("Schitov BB"))
        << " ]\n--------------------------------------------------";

    for (auto view : {0, 1})
    {
        auto& primeStat (primeStat_.get(method, view));
        auto& coreStat (coreStat_.get(method, view));

        std::array<std::set<int>, 2> spec;

        out << "\n\n--------------------------------------------------\n"
            << (view == 0 ? QApplication::tr("Cosmogram") : QApplication::tr("Horoscope"))
            << "\n--------------------------------------------------\n";

        for (auto i : AstroBase::noFictPlanet)
        {
            auto& status (primeStat.at(i));
            auto sum (pairSum(status));

            out.setFieldWidth(10);
            out.setFieldAlignment(QTextStream::AlignRight);
            out << AstroBase::getPlanetName(i);

            out.reset();
            if (sum > 0)
                out.setNumberFlags(QTextStream::ForceSign);
            out << ' ';
            out.setFieldWidth(5);
            out << sum;
            out.setFieldWidth(0);
            out << "  ( ";
            status[0] > 0 ? out.setNumberFlags(QTextStream::ForceSign) : out.reset();
            out.setFieldWidth(5);
            out << status[0];
            out.setFieldWidth(0);
            out << ", ";
            status[1] > 0 ? out.setNumberFlags(QTextStream::ForceSign) : out.reset();
            out.setFieldWidth(5);
            out << status[1];
            out.setFieldWidth(0);
            out << " )\n";

            if (sum >= 7 || sum <= -7)
                spec[0].insert(i);
            if (status[0] != status[1] && (status[0] == 0 || status[1] == 0))
                spec[1].insert(i);
        }

        for (auto i (0); i < 3; ++i)
        {
            switch (i) {
            case 0 :
                out.setFieldWidth(width);
                out.setFieldAlignment(QTextStream::AlignRight);
                out << '\n' << title[i];
                out.reset();
                out << " : ";
                if (coreStat.total > 0)
                    out.setNumberFlags(QTextStream::ForceSign);
                out << coreStat.total;
                break;
            default :
                if (!spec[i-1].empty())
                {
                    out.setFieldWidth(width);
                    out.setFieldAlignment(QTextStream::AlignRight);
                    out << '\n' << title[i];
                    out.reset();
                    out  << " : ";

                    auto back (--spec[i-1].end());
                    for (auto j (spec[i-1].begin()); j != spec[i-1].end(); ++ j)
                        out << AstroBase::getPlanetName(*j) << (j != back ? ", " : "");
                }
            }
        }
    }

    out << "\n\n--------------------------------------------------\n"
        << QApplication::tr("Fiction planets")
        << "\n--------------------------------------------------\n";

    out.setFieldAlignment(QTextStream::AlignRight);
    out.setRealNumberNotation(QTextStream::RealNumberNotation::FixedNotation);
    out.setRealNumberPrecision(1);

    auto& primeFict (primeFict_[method]);

    for (auto i : {11, 24, 56, 12})
    {
        auto& status (primeFict.at(i));

        out.setFieldWidth(6);
        out << AstroBase::getPlanetName(i);
        out.setFieldWidth(0);
        out << ' ';
        out.setFieldWidth(5);
        out << status.first;

        out.setFieldWidth(0);
        switch (status.second) {
        case ForceFlag::Dominant :
            out << " +";
            break;
        case ForceFlag::Weak :
            out << " -";
        }
        out << '\n';
    }

    return str;
}

} // namespace napatahti
