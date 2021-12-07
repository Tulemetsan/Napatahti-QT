#ifndef PERSON_H
#define PERSON_H

#include <QDateTime>
#include <QSqlQuery>

namespace napatahti {

class ConfigDialog;


class Person
{
public :
    explicit Person()
        : name (name_)
        , dateTime (now())
        , sex ('E')
        , location (location_)
        , lat (lat_)
        , lon (lon_)
        , hsys (hsys_)
        , patch ()
        , table ()
    {}

    explicit Person(
        const QString&   name,
        const QDateTime& dateTime,
        int              sex,
        const QString&   location,
        int              lat,
        int              lon,
        int              hsys,
        const QString&   patch,
        const QString&   table
        )
        : name (name)
        , dateTime (dateTime)
        , sex (sex)
        , location (location)
        , lat (lat)
        , lon (lon)
        , hsys (hsys)
        , patch (patch)
        , table (table)
    {}

    explicit Person(const QSqlQuery& query, const QString& table="")
        : name (query.value("name").toString())
        , dateTime (query.value("dateTime").toDateTime())
        , sex (query.value("sex").toInt())
        , location (query.value("location").toString())
        , lat (query.value("lat").toInt())
        , lon (query.value("lon").toInt())
        , hsys (query.value("hsys").toInt())
        , patch (query.value("patch").toString())
        , table (table)
    {}

    auto& person() { return *this; }
    auto& person() const { return *this; }

    auto isCurrent(const QVariantMap& data) const -> bool;
    auto isCurrent(const QVariantMap& key, const QString& tab) const -> bool;
    auto isCurrent(const Person& person) const -> bool;

    static auto now() -> QDateTime;
    static auto here() -> std::tuple<QString, int, int, int>;
    static auto restore() -> void;

    auto operator==(const Person& rhs) const -> bool;
    auto operator!=(const Person& rhs) const { return !operator==(rhs); }

    friend ConfigDialog;

public :
    QString   name;
    QDateTime dateTime;
    int       sex;
    QString   location;
    int       lat;
    int       lon;
    int       hsys;
    QString   patch;
    QString   table;

private :
    static QString name_;
    static QString location_;
    static int     utc_;
    static int     lat_;
    static int     lon_;
    static int     hsys_;
};

} // namespace napatahti

#endif // PERSON_H
