#include <QApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include "swelib/src/swephexp.h"
#include "shared.h"
#include "Person.h"
#include "RefBook.h"
#include "AstroBase.h"

namespace napatahti {

AstroBase::AstroBase(const Person& person) : person_ (person)
{
    setEpheRange();
    swe_set_ephe_path(path_);

    auto date (QDate::currentDate());
    auto time (QTime::currentTime());

    julTech_ = swe_julday(
        date.year(), date.month(), date.day(), time.hour() +
        (time.minute() / 60.0) + time.second() / 3600.0, SE_GREG_CAL);

    loadCatalog();
}


auto AstroBase::update(int space, const QLocale& locale) -> void
{
    auto date (person_.dateTime.date());
    auto time (person_.dateTime.time());
    auto utc  (person_.dateTime.offsetFromUtc());

    julDay_ = swe_julday(
        date.year(), date.month(), date.day(), time.hour() +
        (time.minute() / 60.0) + (time.second() - utc) / 3600.0, SE_GREG_CAL);

    computeHouses();
    computePlanets();
    computeCoupling(space);
    computeMoonCalendar();
    computeMoonDayCache(locale);
}


auto AstroBase::computeHouses() -> void
{
    cuspidCrd_    = {};
    cuspidSignNo_ = {};
    cuspidDegree_ = {};
    cuspidCrdStr_ = {};
    houseLength_  = {};

    double cusp[13];
    double asmc[10];

    swe_houses(julDay_, person_.lat / 3600.0, person_.lon / 3600.0,  person_.hsys, cusp, asmc);

    for (auto i (1), j (0); i < 13; ++i, ++j)
    {
        auto& crd (cuspidCrd_[j]);
        auto& sign (cuspidSignNo_[j]);

        crd = cusp[i];
        sign = static_cast<int>(crd / 30);
        cuspidDegree_[j] = static_cast<int>(crd + 1) - 30 * sign;
        cuspidCrdStr_[j] =
            {toStrCrd(crd, true), RefBook::cuspidSymStr[j] + ' ' + toStrCrd(crd, false)};

        if (i == 12)
            houseLength_[j] = cusp[1] - crd;
        else
            houseLength_[j] = cusp[i+1] - crd;

        if (houseLength_[j] < 0)
            houseLength_[j] = -360 - houseLength_[j];
    }
}


auto AstroBase::computePlanets() -> void
{
    planetCrd_.clear();
    planetSpeed_.clear();
    planetSignNo_.clear();
    planetDegree_.clear();
    planetSpeedSym_.clear();
    planetHouseNo_.clear();
    planetCrdStr_.clear();

    int    flag (SEFLG_SWIEPH | SEFLG_SPEED);
    double res[6];
    char   serr[AS_MAXCH];

    for (const auto& i : planetCatalog_)
    {
        if (i.first == 24)
            continue;

        if (swe_calc_ut(julDay_, i.first, flag, res, serr) != Code::SweCalcOK)
            errorLog("sweph error: " + QString(serr));

        auto& crd (planetCrd_[i.first]);
        auto& sign (planetSignNo_[i.first]);
        auto& speed (planetSpeed_[i.first]);

        crd = res[0];
        sign = static_cast<int>(res[0] / 30);
        speed = res[3];
        planetDegree_[i.first] = static_cast<int>(crd + 1) - 30 * sign;
        planetSpeedSym_[i.first] = getSpeedChar(i.first, speed);

        if (i.first == 11)
        {
            auto& ketuCrd (planetCrd_[24]);

            ketuCrd = crd < 180 ? crd + 180 : crd - 180;
            planetSpeed_[24] = speed;
            planetSignNo_[24] = static_cast<int>(ketuCrd / 30);
            planetDegree_[24] = planetDegree_.at(11);
            planetSpeedSym_[24] = planetSpeedSym_.at(11);
        }
    }

    computePlanetHouseNo();

    for (const auto& i : planetCrd_)
        planetCrdStr_[i.first] = {
            toStrCrd(i.second, true),
            QString("%1 %2, %3")
                .arg(getPlanetName(i.first))
                .arg(toStrCrd(i.second, false))
                .arg(RefBook::houseSymStr[planetHouseNo_.at(i.first)])
        };
}


auto AstroBase::computePlanetHouseNo(std::set<int> keys) -> void
{
    std::set<int> buf;

    if (keys.empty())
        for (const auto& i : planetCatalog_)
            keys.insert(i.first);

    for (auto i (0); i < 12; ++i)
    {
        for (auto j : buf)
            keys.erase(j);

        if (keys.empty())
            break;

        buf.clear();

        for (auto j : keys)
        {
            auto asp (planetCrd_.at(j) - cuspidCrd_[i]);
            if (asp < 0)
                asp = -360 - asp;

            if (asp == 0)
            {
                planetHouseNo_[j] = i;
                buf.insert(j);
            }
            else if (houseLength_[i] > 0)
            {
                if (asp > 0 && asp < houseLength_[i])
                {
                    planetHouseNo_[j] = i;
                    buf.insert(j);
                }
            }
            else
            { // Cuspid, planet crossed 0 Ari: first, both
                if ((asp > 0 && asp < -houseLength_[i]) ||
                    (asp < 0 && asp > houseLength_[i]))
                {
                    planetHouseNo_[j] = i;
                    buf.insert(j);
                }
            }
        }
    }
}


auto AstroBase::computeCoupling(int space) -> void
{
    planetOrder_.clear();
    normalizeCrd_.clear();

    for (auto i : AstroBase::planetOrder)
        if (planetEnb_.at(i).crd)
            planetOrder_.push_back(i);

    for (const auto& i : planetCatalog_)
        if (!contains(i.first, AstroBase::planetOrder))
            if (planetEnb_.at(i.first).crd)
                planetOrder_.push_back(i.first);

    if (planetOrder_.size() < 2)
    {
        normalizeCrd_[planetOrder_[0]] = planetCrd_.at(planetOrder_[0]);
        return;
    }

    std::vector<std::pair<int, int>> sector;
    std::vector<std::pair<double, int>> crdKey;

    for (auto i : planetOrder_)
    {
        auto crd (planetCrd_.at(i));
        sector.push_back({static_cast<int>(crd / space), i});
        crdKey.push_back({crd, i});
    }

    std::sort(crdKey.begin(), crdKey.end());

    int maxSectorNo (360 / space);
    int planetNum (sector.size());

    while (true)
    {
        auto stop (true);
        std::sort(sector.begin(), sector.end());

        for (auto i (0); i < planetNum; ++i)
        {
            if (i < planetNum - 1)
            {
                if (sector[i].first == sector[i+1].first)
                {
                    if (sector[i+1].first == maxSectorNo)
                    {
                        if (i != planetNum - 2)
                        {
                            std::vector<std::pair<double, int>> buffer;

                            for (auto j (i); j < planetNum; ++j)
                                buffer.push_back(crdKey[j]);
                            for (auto j (0); j < i; ++j)
                                buffer.push_back(crdKey[j]);

                            crdKey = std::move(buffer);
                            sector.clear();

                            for (const auto& j : crdKey)
                                sector.push_back({static_cast<int>(j.first / space), j.second});
                            for (auto j (0); j < planetNum - i; ++j)
                                sector[j].first = 0;

                            stop = false;
                            break;
                        }
                        else
                        {
                            sector[i+1].first = 0;
                            auto crdBack (crdKey[i+1]);
                            crdKey.pop_back();
                            crdKey.insert(crdKey.begin(), crdBack);
                            stop = false;
                        }
                    }
                    else
                    {
                        sector[i+1].first += 1;
                        stop = false;
                    }
                }
            }
            else
            {
                if (sector[i].first == sector[0].first)
                {
                    sector[0].first += 1;
                    stop = false;
                }
            }
        }

        if (stop)
            break;
    }

    for (auto i (0); i < planetNum; ++i)
        normalizeCrd_[crdKey[i].second] = space * (sector[i].first + 0.5);
}


auto AstroBase::computeMoonCalendar() -> void
{
    jyotCal_.clear();
    westCal_.clear();

    auto& jyotDay (jyotCal_[0]);
    auto& westDay (westCal_[0]);

    // Jyotisa ------------------------------------------------------------- //
    auto ang (planetCrd_.at(1) - planetCrd_.at(0));
    if (ang < 0)
        ang += 360;

    jyotDay = 1 + static_cast<int>(ang / 12);
    for (auto i (2); i < 31; ++i)
    {
        if (i > jyotDay)
            jyotCal_[i] = getMoonJulDay(1, i - 1);
        else
            jyotCal_[i] = getMoonJulDay(-1, i);
    }

    jyotCal_[1]  = getMoonJulDay(-1);
    jyotCal_[31] = getMoonJulDay(1);
    jyotCal_[32] = jyotCal_.at(16);

    // Western ------------------------------------------------------------- //
    westDay = 0;
    westCal_[1]  = jyotCal_.at(1);
    westCal_[31] = jyotCal_.at(31);
    westCal_[32] = jyotCal_.at(32);

    double beg (westCal_.at(1));
    double sec (1.0 / 86400.0);
    int    rsmi (SE_CALC_RISE | SE_BIT_DISC_CENTER | SE_BIT_NO_REFRACTION);
    double pos[3] {person_.lon / 3600.0, person_.lat / 3600.0, 0};
    double next;
    char   serr[AS_MAXCH];

    for (auto i (2); i < 31; ++i)
    {
        switch (swe_rise_trans(beg + sec, 1, 0, 2, rsmi, pos, 1013.25, 15, &next, serr)) {
        case Code::SweRiseTransOK :
            break;
        case Code::BeyondPolarCircle :
            westCal_ = jyotCal_;
            return;
        default :
            errorLog("moon rise error: " + QString(serr));
        }

        if (i < 30 || (i == 30 && next < westCal_.at(31)))
        {
            westCal_[i] = next;
            if (julDay_ > westCal_.at(i - 1) && julDay_ < next)
                westDay = i - 1;
        }

        beg = next;
    }

    if (westDay == 0)
        westDay = westCal_.size() - 3;
}


auto AstroBase::computeMoonDayCache(const QLocale& locale) -> void
{
    QStringList tech {
        "%1 %2", "%1: %2\n%3: %4\n%5: %6",
        "d MMMM yyyy ddd h:mm:ss",
        QApplication::tr("m.d."), QApplication::tr("Moon day"),
        QApplication::tr("Begin"), QApplication::tr("End"),
    };

    auto utc (person_.dateTime.offsetFromUtc());
    auto src (&jyotCal_);

    for (auto cache (&jyotCache_); cache <= &westCache_; ++cache)
    {
        auto day (src->find(src->at(0)));

        *cache = {
            tech[0].arg(day->first).arg(tech[3]),
            tech[1].arg(tech[4]).arg(day->first)
                   .arg(tech[5]).arg(locale.toString(revJulDay(day->second, utc), tech[2]))
                   .arg(tech[6]).arg(locale.toString(revJulDay((++day)->second, utc), tech[2]))
        };

        ++src;
    }
}


auto AstroBase::sortByCrd(const std::set<int>& keys) const -> std::vector<int>
{
    std::vector<int> result;
    std::vector<std::pair<double, int>> buffer;

    for (auto i : keys)
        buffer.push_back({planetCrd_.at(i), i});

    std::sort(buffer.begin(), buffer.end());

    for (const auto& i : buffer)
        result.push_back(i.second);

    return result;
}


auto AstroBase::createPlanet(const QStringList& data) -> bool
{
    auto ind (data[1].toInt());

    if (contains(ind, planetOrder_))
        return false;

    int    flag (SEFLG_SWIEPH | SEFLG_SPEED);
    double res[6];
    char   serr[AS_MAXCH];

    if (swe_calc_ut(julDay_, ind, flag, res, serr) != Code::SweCalcOK)
        return false;

    planetCatalog_[ind] = data[0];
    planetEnb_[ind] = {};

    auto& crd (planetCrd_[ind]);
    auto& sign (planetSignNo_[ind]);
    auto& speed (planetSpeed_[ind]);

    crd = res[0];
    sign = static_cast<int>(res[0] / 30);
    speed = res[3];
    planetDegree_[ind] = static_cast<int>(crd + 1) - 30 * sign;
    planetSpeedSym_[ind] = getSpeedChar(ind, speed);

    computePlanetHouseNo({ind});

    planetCrdStr_[ind] = {
        toStrCrd(crd, true),
        QString("%1 %2, %3")
            .arg(getPlanetName(ind))
            .arg(toStrCrd(crd, false))
            .arg(RefBook::houseSymStr[planetHouseNo_.at(ind)])
    };

    return true;
}


auto AstroBase::updatePlanet(const QStringList& data) -> bool
{
    planetCatalog_[data[1].toInt()] = data[0];
    return true;
}


auto AstroBase::deletePlanet(int ind) -> bool
{
    if (contains(ind, planetOrder))
        return false;

    planetCatalog_.erase(ind);
    planetEnb_.erase(ind);

    planetCrd_.erase(ind);
    planetSpeed_.erase(ind);
    planetSignNo_.erase(ind);
    planetDegree_.erase(ind);
    planetSpeedSym_.erase(ind);
    planetHouseNo_.erase(ind);
    planetCrdStr_.erase(ind);

    auto res (std::find(planetOrder_.begin(), planetOrder_.end(), ind));

    if (res != planetOrder_.end())
    {
        planetOrder_.erase(res);
        normalizeCrd_.erase(ind);
    }

    return true;
}


auto AstroBase::switchEnb(int ind, bool mode) -> void
{
    if (mode)
        planetEnb_[ind].crd ^= true;
    else
        planetEnb_[ind].asp ^= true;
}


auto AstroBase::getPlanetName(int key) -> QString
{
    switch (key) {
    case 11 :
        return "Rahu";
    case 24 :
        return "Ketu";
    case 12 :
        return "Lilith";
    case 56 :
        return "Selena";
    default :
        char name[20];
        swe_get_planet_name(key, name);
        return name;
    }
}


auto AstroBase::getMoonJulDay(int rev, int day, double acc) const -> double
{
    auto offset (day > 0 ? (rev == 1 ? 12 * day : 12 * (day - 1)) : 0);

    auto sunCrd (planetCrd_.at(0) + offset);
    auto moonCrd (planetCrd_.at(1));
    if (sunCrd >= 360)
        sunCrd -= 360;

    auto angBeg (rev * (sunCrd - moonCrd));
    if (angBeg < 0)
        angBeg += 360;

    if (angBeg < acc)
        return julDay_;

    auto sunSpeed (planetSpeed_.at(0));
    auto moonSpeed (planetSpeed_.at(1));
    auto moonJulDayBeg (angBeg / (moonSpeed - sunSpeed));
    auto moonJulDayEnd (julDay_ + rev * moonJulDayBeg);

    int    flag (SEFLG_SWIEPH | SEFLG_SPEED);
    double res[6];
    char   serr[AS_MAXCH];

    while (true)
    {
        if (swe_calc_ut(moonJulDayEnd, SE_SUN, flag, res, serr) != Code::SweCalcOK)
        {
            errorLog("moon julday error: " + QString(serr));
            return 0;
        }
        sunCrd = res[0] + offset;
        sunSpeed = res[3];

        if (sunCrd >= 360)
            sunCrd -= 360;

        if (swe_calc_ut(moonJulDayEnd, SE_MOON, flag, res, serr) != Code::SweCalcOK)
        {
            errorLog("moon julday error: " + QString(serr));
            return 0;
        }
        moonCrd = res[0];
        moonSpeed = res[3];

        auto angEnd (std::abs(sunCrd - moonCrd));
        if (angEnd > 180)
            angEnd = 360 - angEnd;

        if (angEnd < acc)
            return moonJulDayEnd;

        moonJulDayBeg = angEnd / (moonSpeed - sunSpeed);
        if (angEnd < angBeg)
            moonJulDayEnd += rev * moonJulDayBeg;
        else
            moonJulDayEnd -= rev * moonJulDayBeg;
        angBeg = angEnd;
    }
}


auto AstroBase::revJulDay(double jujDay, int utc) const -> QDateTime
{
    int yy, mn, dd;
    double hour;

    swe_revjul(jujDay + utc / 86400.0, SE_GREG_CAL, &yy, &mn, &dd, &hour);

    auto hh (static_cast<int>(hour));
    auto mm (static_cast<int>(60 * (hour - hh)));
    auto ss (static_cast<int>(3600 * (hour - hh - mm / 60.0)));

    return QDateTime(QDate(yy, mn, dd), QTime(hh, mm, ss), Qt::OffsetFromUTC, utc);
}


auto AstroBase::loadCatalog() -> void
{
    QFile file ("catalog.json");
    QByteArray source;

    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        source = file.readAll();
        file.close();
    }
    else
    {
        errorLog("catalog.json not found");
        restore();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument json (QJsonDocument::fromJson(source, &parseError));

    if (json.isNull())
    {
        errorLog("catalog.json parsing error: " + parseError.errorString());
        restore();
        return;
    }

    planetCatalog_.clear();
    planetEnb_.clear();

    std::set<int> mustHave;
    auto isDump (false);

    for (const auto& i : json.array())
    {
        const auto& dec (i.toArray());
        auto key (dec[0].toInt());
        auto sym (dec[1].toString());

        if (contains(key, planetOrder))
            mustHave.insert(key);
        else if (!checkPlanet(key))
        {
            errorLog(QString("catalog.json invalid planet index %1").arg(key));
            isDump = true;
            continue;
        }
        if (sym.size() != 1)
        {
            sym = sym.isEmpty() ? '@' : sym[0];
            isDump = true;
        }

        planetCatalog_[key] = sym;
        planetEnb_[key] = {dec[2].toBool(), dec[3].toBool()};
    }

    if (mustHave.size() != planetOrder.size())
    {
        errorLog("catalog.json not all base planet are present");
        restore();
        return;
    }

    if (isDump)
        dumpCatalog();
}


auto AstroBase::dumpCatalog() const -> void
{
    QFile file ("catalog.json");
    QJsonArray catalog;

    for (auto& i : planetCatalog_)
    {
        auto& enb (planetEnb_.at(i.first));
        catalog.push_back(QJsonArray{
            i.first, i.second, enb.crd, enb.asp});
    }

    if (file.open(QIODevice::WriteOnly))
    {
        file.write(QJsonDocument(catalog).toJson());
        file.close();
    }
    else
        errorLog("can not create catalog.json");
}


auto AstroBase::restore() -> void
{
    planetCatalog_ = planetCatalogR_;
    planetEnb_ = planetEnbR_;

    dumpCatalog();
}


auto AstroBase::setEpheRange() -> void
{
    auto now (QDateTime::currentDateTimeUtc());
    epheRange_ = {now.addMonths(-6), now.addMonths(6)};

    QDir epheDir (path_);
    if (!epheDir.exists())
        QDir().mkdir(path_);
    if (epheDir.isEmpty())
    {
        errorLog("swiss ephemeris not found");
        return;
    }

    QString sepl ("sepl_%1.se1");
    QString semo ("semo_%1.se1");
    QString seas ("seas_%1.se1");
    QSet<QString> ephe;

    for (const auto& i : epheDir.entryInfoList(QDir::Files))
        ephe.insert(i.fileName());

    if (!ephe.contains("sefstars.txt") || !ephe.contains("seorbel.txt"))
        errorLog("swiss ephemeris incomplete");

    auto ind (18);
    auto isCheck (false);
    while (true)
    {
        if (ephe.contains(sepl.arg(ind)) &&
            ephe.contains(semo.arg(ind)) &&
            ephe.contains(seas.arg(ind)))
        {
            epheRange_[0] = QDateTime(
                QDate(100 * ind + 1, 1, 1), QTime(0,0,0), Qt::OffsetFromUTC, 0);
            ind -= 6;
            isCheck = true;
        }
        else break;
    }

    ind = 18;
    while (true)
    {
        if (ephe.contains(sepl.arg(ind)) &&
            ephe.contains(semo.arg(ind)) &&
            ephe.contains(seas.arg(ind)))
        {
            epheRange_[1] = QDateTime(
                QDate(100 * (ind + 6) - 2, 1, 1), QTime(0,0,0), Qt::OffsetFromUTC, 0);
            ind += 6;
            isCheck = true;
        }
        else break;
    }

    if (!isCheck)
        errorLog("swiss ephemeris not found");
}


auto AstroBase::checkPlanet(int key) const -> bool
{
    if (key < 0)
        return false;

    double res[6];
    char   serr[AS_MAXCH];

    return swe_calc_ut(julTech_, key, SEFLG_SWIEPH | SEFLG_SPEED, res, serr) == Code::SweCalcOK;
}


auto AstroBase::toStrCrd(double crd, bool mode) -> QString
{
    auto uCrd (makeAsTime(crd));
    auto signNo (uCrd[0] / 30);
    auto mask (mode ? QString("%1; %2' %3\"  %4") : QString("%1°%2'%3\" %4"));

    return mask.arg(uCrd[0] - 30 * signNo, 2, 10, QChar('0'))
               .arg(uCrd[1], 2, 10, QChar('0'))
               .arg(uCrd[2], 2, 10, QChar('0'))
               .arg(mode ? RefBook::signSym[signNo] : RefBook::signSymStr[signNo]);
}


auto AstroBase::getSpeedChar(int key, double speed) -> QChar
{
    auto speedAbs (std::abs(speed));
    auto mid (AstroBase::middleSpeed.find(key));

    if (mid != AstroBase::middleSpeed.end())
    {
        if (speedAbs < mid->second / 10)
            return RefBook::speedSym[0];
    }
    else
    {
        if (speedAbs < 2.77778e-4)
            return RefBook::speedSym[0];
    }

    return speed > 0
               ? key == 11 || key == 24 ? RefBook::speedSym[1] : ' '
               : key == 11 || key == 24 ? ' ' : RefBook::speedSym[2];
}

} // namespace napatahti
