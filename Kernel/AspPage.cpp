#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include "shared.h"
#include "AstroBase.h"
#include "AspPage.h"

namespace napatahti {

auto AspPage::loadPage(const QString& title) -> void
{
    if (title.isEmpty() || title == titleR_)
    {
        restore();
        return;
    }

    QString fileName (path + title + ".json");
    QFile file (fileName);
    QByteArray source;

    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        source = file.readAll();
        file.close();
    }
    else
    {
        restore();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument json (QJsonDocument::fromJson(source, &parseError));

    if (json.isNull())
    {
        errorLog(fileName + " parsing error: " + parseError.errorString());
        restore();
        return;
    }

    title_ = title;
    aspCatalog_.clear();
    aspEnb_.clear();
    aspPoint_.clear();
    aspPen_.clear();
    aspDash_.clear();
    aspType_.clear();
    orbTable_ = {};

    auto part (0);
    for (const auto& i : json.array())
    {
        switch (part) {
        case 0 :
            for (const auto& j : i.toArray())
            {
                const auto& dec (j.toArray());
                auto asp (dec[0].toDouble());

                aspCatalog_[asp] = dec[1].toString();
                aspEnb_[asp] = dec[2].toBool();
                aspPoint_[asp] = dec[3].toDouble();
                aspPen_[asp] = makePen(dec[4].toString(), dec[5].toInt());
                aspDash_[asp] = dec[5].toInt();
                aspType_[asp] = dec[6].toInt();
            }
            break;
        case 1 :
            for (const auto& j : i.toArray())
            {
                const auto& dec (j.toArray());
                orbTable_.planet[{dec[0].toInt(), dec[1].toDouble()}] = dec[2].toDouble();
            }
            break;
        case 2 :
            for (const auto& j : i.toArray())
            {
                const auto& dec (j.toArray());
                orbTable_.asteroid[dec[0].toDouble()] = dec[1].toDouble();
            }
            break;
        case 3 :
            for (const auto& j : i.toArray())
            {
                const auto& dec (j.toArray());
                orbTable_.cuspid[dec[0].toDouble()] = dec[1].toDouble();
            }
            break;
        case 4 :
            auto ind (0);
            for (const auto& j : i.toArray())
            {
                orbTable_.star[ind] = j.toDouble();
                ++ind;
            }
            break;
        }

        ++part;
    }

    if (!validate())
    {
        restore();
        return;
    }
}


auto AspPage::dumpPage(const QString& title) const -> bool
{
    if (title == titleR_ || (title.isEmpty() && title_ == titleR_))
        return false;

    QJsonArray aspProperty;
    QJsonArray planetOrb;
    QJsonArray asteroidOrb;
    QJsonArray cuspidOrb;
    QJsonArray starOrb;

    for (auto& i : orbTable_.planet)
        planetOrb.push_back(QJsonArray{i.first.first, i.first.second, i.second});

    for (auto& i : orbTable_.asteroid)
        asteroidOrb.push_back(QJsonArray{i.first, i.second});

    for (auto& i : orbTable_.cuspid)
        cuspidOrb.push_back(QJsonArray{i.first, i.second});

    for (auto i : orbTable_.star)
        starOrb.push_back(i);

    for (auto& i : aspCatalog_)
        aspProperty.push_back(QJsonArray{
            i.first, i.second, aspEnb_.at(i.first), aspPoint_.at(i.first),
            aspPen_.at(i.first).color().name(), aspDash_.at(i.first), aspType_.at(i.first)});

    QString fileName (path + (title.isEmpty() ? title_ : title) + ".json");
    QFile file (fileName);

    if (file.open(QIODevice::WriteOnly))
    {
        file.write(QByteArray(QJsonDocument(
            {aspProperty, planetOrb, asteroidOrb, cuspidOrb, starOrb}).toJson()));
        file.close();
    }
    else
    {
        errorLog("can not create " + fileName);
        return false;
    }

    return true;
}


auto AspPage::createPage(const QString& title) -> bool
{
    auto result (true);
    auto tempPage(path + "Template.txt");

    if (!QFile::exists(tempPage))
        result = createTempPage();

    if (result)
        result = QFile::copy(tempPage, path + title + ".json");

    return result;
}


auto AspPage::createAsp(const QVariantList& buffer) -> void
{
    auto  asp (buffer[0].toDouble());
    auto& dash (aspDash_[asp]);

    aspCatalog_[asp] = buffer[1].toString();
    aspEnb_[asp] = true;
    aspPoint_[asp] = 0;
    dash = buffer[2].toInt();
    aspType_[asp] = buffer[3].toInt();
    aspPen_[asp] = makePen(buffer[4].toString(), dash);

    for (auto j : AstroBase::planetOrder)
        orbTable_.planet[{j, asp}] = 1;

    orbTable_.asteroid[asp] = 1;
    orbTable_.cuspid[asp] = 1;
}


auto AspPage::deleteAsp(double asp) -> void
{
    aspCatalog_.erase(asp);
    aspEnb_.erase(asp);
    aspPoint_.erase(asp);
    aspDash_.erase(asp);
    aspType_.erase(asp);
    aspPen_.erase(asp);

    for (auto j : AstroBase::planetOrder)
        orbTable_.planet.erase({j, asp});

    orbTable_.asteroid.erase(asp);
    orbTable_.cuspid.erase(asp);
}


auto AspPage::insertAsp(const QVariantList& buffer) -> void
{
    auto asp (buffer[0].toDouble());

    if (contains(asp, aspCatalog_))
        return;

    auto& dash (aspDash_[asp]);

    aspCatalog_[asp] = buffer[1].toString();
    aspEnb_[asp] = buffer[2].toBool();
    aspPoint_[asp] = buffer[3].toDouble();
    dash = buffer[5].toInt();
    aspType_[asp] = buffer[6].toInt();
    aspPen_[asp] = makePen(buffer[4].toString(), dash);

    for (auto i (7); i < 23; ++i)
        orbTable_.planet[{AstroBase::planetOrder[i-7], asp}] = buffer[i].toDouble();

    orbTable_.asteroid[asp] = buffer[23].toDouble();
    orbTable_.cuspid[asp] = buffer[24].toDouble();
}


auto AspPage::copyAsp(double asp) const -> QVariantList
{
    QVariantList result {
        asp, aspCatalog_.at(asp), aspEnb_.at(asp), aspPoint_.at(asp),
        aspPen_.at(asp).color(), aspDash_.at(asp), aspType_.at(asp)
    };

    for (auto i : AstroBase::planetOrder)
        result.push_back(orbTable_.planet.at({i, asp}));

    result.push_back(orbTable_.asteroid.at(asp));
    result.push_back(orbTable_.cuspid.at(asp));

    return result;
}


auto AspPage::setAspEnb(const QVariantList& aspList) -> void
{
    switch (aspList.size()) {
    case 0 :
        for (auto& i : aspEnb_)
            i.second = false;
        return;
    case 1 :
        if (aspList[0] == -1)
        {
            for (auto& i : aspEnb_)
                i.second = true;
            return;
        }
        break;
    }

    for (auto& i : aspEnb_)
        i.second = aspList.contains(i.first);
}


auto AspPage::setAspStyle(const QVariantList& buffer) -> void
{
    auto  asp (buffer[0].toDouble());
    auto& dash (aspDash_[asp]);

    aspCatalog_[asp] = buffer[1].toString();
    dash = buffer[2].toInt();
    aspType_[asp] = buffer[3].toInt();
    aspPen_[asp] = makePen(buffer[4].toString(), dash);
}


auto AspPage::restore() -> void
{
    title_ = titleR_;
    aspCatalog_ = aspCatalog;
    aspEnb_ = aspEnbR_;
    aspPoint_ = aspPointR_;
    aspDash_ = aspDashR_;
    aspType_ = aspTypeR_;

    aspPen_.clear();

    for (auto& i : aspCatalog)
        aspPen_[i.first] = makePen(aspColorR_.at(i.first), aspDashR_.at(i.first));

    orbTable_.reset();
}


auto AspPage::createTempPage() -> bool
{
    auto title (title_);

    dumpPage(title_);

    title_ = "$$$__TEMPLATE__$$$";
    aspCatalog_.clear();
    aspEnb_.clear();
    aspPoint_.clear();
    aspDash_.clear();
    aspType_.clear();
    aspPen_.clear();
    orbTable_.toTemp();

    for (auto i : aspListBone)
    {
        aspCatalog_[i] = aspCatalog.at(i);
        aspEnb_[i] = aspEnbR_.at(i);
        aspPoint_[i] = 0;
        aspDash_[i] = aspDashR_.at(i);
        aspType_[i] = aspTypeR_.at(i);
        aspPen_[i] = makePen(aspColorR_.at(i), aspDashR_.at(i));
    }

    auto result (dumpPage(title_));

    if (!QFile::rename(path + title_ + ".json", path + "Template.txt"))
        QFile::remove(path + title_ + ".json");

    loadPage(title);

    return result;
}


auto AspPage::validate() const -> bool
{
    QString fileName (path + title_ + ".json");

    for (auto& i : aspCatalog_)
    {
        if (i.first < 0 || i.first > 180)
        {
            errorLog(fileName + " invalid asp value " + QString("%1").arg(i.first));
            return false;
        }
        if (i.second.isEmpty() || i.second.size() > 4)
        {
            errorLog(fileName + " invalid symbol size for " + QString("%1").arg(i.first));
            return false;
        }

        auto point (aspPoint_.at(i.first));
        auto pen (aspPen_.at(i.first));
        auto dash (aspDash_.at(i.first));
        auto type (aspType_.at(i.first));

        if (point < 0 || point > 10)
        {
            errorLog(fileName + " invalid point value for " + QString("%1").arg(i.first));
            return false;
        }
        if (pen.color() == QColor::Invalid)
        {
            errorLog(fileName + " invalid color for " + QString("%1").arg(i.first));
            return false;
        }
        if (dash < 0 || dash > 4)
        {
            errorLog(fileName + " invalid dash value for " + QString("%1").arg(i.first));
            return false;
        }
        if (type < 0 || type > 1)
        {
            errorLog(fileName + " invalid type index for " + QString("%1").arg(i.first));
            return false;
        }
    }

    auto planetEnd (orbTable_.planet.end());
    auto asteroidEnd (orbTable_.asteroid.end());
    auto cuspidEnd (orbTable_.cuspid.end());

    for (auto& i : aspCatalog_)
    {
        for (auto j : AstroBase::planetOrder)
        {
            auto check (orbTable_.planet.find({j, i.first}));
            if (check == planetEnd)
            {
                errorLog(fileName + " incomplite planet orb table");
                return false;
            }
            if (check->second < 0 || check->second > 10)
            {
                 errorLog(fileName + " invalid planet orb value for " +
                          QString("{%1, %2}").arg(check->first.first).arg(check->first.second));
                 return false;
            }
        }

        auto check (orbTable_.asteroid.find(i.first));
        if (check == asteroidEnd)
        {
             errorLog(fileName + " incomplite asteroid orb table");
             return false;
        }
        if (check->second < 0 || check->second > 10)
        {
             errorLog(fileName + " invalid asteroid orb value for " +
                      QString("%1").arg(check->first));
             return false;
        }

        check = orbTable_.cuspid.find(i.first);
        if (check == cuspidEnd)
        {
             errorLog(fileName + " incomplite cuspid orb table");
             return false;
        }
        if (check->second < 0 || check->second > 10)
        {
             errorLog(fileName + " invalid cuspid orb value for " +
                      QString("%1").arg(check->first));
             return false;
        }
    }

    if (orbTable_.star.size() != 5)
    {
         errorLog(fileName + " incomplite star orb table");
         return false;
    }
    auto ind (0);
    for (auto i : orbTable_.star)
    {
        if (i < 0 || i > 10)
        {
             errorLog(fileName + " invalid star orb value for " +
                      QString("M%1").arg(ind));
             return false;
        }
        ++ind;
    }

    return true;
}


auto AspPage::makePen(const QColor& color, int dash) -> QPen
{
    switch (dash) {
    case 1 :
    case 2 :
    {
        QPen pen {color, 1, Qt::DashLine, Qt::RoundCap};
        dash == 1 ? pen.setDashPattern({15, 6}) : pen.setDashPattern({5, 2.5});
        return pen;
    }
    case 3 :
    {
        QPen pen {color, 1, Qt::DashDotLine, Qt::RoundCap};
        pen.setDashPattern({15, 3, 2, 3});
        return pen;
    }
    default :
        return {color, 1, Qt::SolidLine, Qt::RoundCap};
    }
}


auto OrbTable::get(int keyA, int keyB, double asp) const -> double
{
    auto check (planet.find({keyA, asp}));
    auto orbA (check != planet.end() ? check->second : asteroid.at(asp));

    check = planet.find({keyB, asp});
    auto orbB (check != planet.end() ? check->second : asteroid.at(asp));

    return orbA > orbB ? orbA : orbB;
}


auto OrbTable::reset() -> void
{
    planet.clear();
    asteroid.clear();
    cuspid.clear();
    star = {1, 0.5, 0.3, 0.2, 0.1};

    const std::map<double, std::set<int>> range {
        {1, {0, 1}}, {0.925, {2, 3, 4}}, {0.85, {5, 11, 24, 12, 56}},
        {0.775, {6, 7, 15}}, {0.7, {8, 9, 57}}
    };

    for (auto& i : AspPage::aspCatalog)
    {
        auto orb (sourceOrb(i.first));

        for (auto& k : range)
            for (auto j : k.second)
                planet[{j, i.first}] = k.first * orb;

        asteroid[i.first] = 0.65 * orb;
        cuspid[i.first] = 0.6 * orb;
    }
}


auto OrbTable::toTemp() -> void
{
    planet.clear();
    asteroid.clear();
    cuspid.clear();
    star = {1, 0.5, 0.3, 0.2, 0.1};

    for (auto i : AspPage::aspListBone)
    {
        for (auto j : AstroBase::planetOrder)
            planet[{j, i}] = 1;

        asteroid[i] = 1;
        cuspid[i] = 1;
    }
}


auto OrbTable::sourceOrb(double asp, double acc, int level) -> double
{
    if (asp == 0)
        return 27.85042735 / level;
    else
        for (auto i (1); i <= 7; ++i)
            for (auto j (1); j <= 18; ++j)
            {
                auto ang (360.0 * i / j);
                if (asp >= ang - acc && asp <= ang + acc)
                    return 28.5 / (level * std::sqrt(i) * (
                                   1 + std::pow(j / 7.0, 2) + std::pow(j / 7.0, 3)));
            }

    return 4.78487518 / level;
}

} // namespace napatahti
