#include <QApplication>
#include "Person.h"

namespace napatahti {

auto Person::isCurrent(const QVariantMap& data) const -> bool
{
    return name == data["name"] && dateTime == data["dateTime"] &&
           sex == data["sex"] && table == data["table"];
}


auto Person::isCurrent(const QVariantMap& key, const QString& tab) const -> bool
{
    return name == key["name"] && dateTime == key["dateTime"] &&
           sex == key["sex"] && table == tab;
}


auto Person::isCurrent(const Person& person) const -> bool
{
    return name == person.name && dateTime == person.dateTime &&
           sex == person.sex && table == person.table;
}


auto Person::now() -> QDateTime
{
    return QDateTime::fromSecsSinceEpoch(
        QDateTime::currentSecsSinceEpoch(), Qt::OffsetFromUTC, utc_);
}


auto Person::here() -> std::tuple<QString, int, int, int>
{
    return {location_, utc_, lat_, lon_};
}


auto Person::restore() -> void
{
    name_ = QApplication::tr("Today");
    location_ = QApplication::tr("Krasnodar, Krasnodar Territory, Russia");
    utc_ = 10800;
    lat_ = 162120;
    lon_ = 140400;
    hsys_ = 'P';
}


auto Person::operator==(const Person& rhs) const -> bool
{
    return name == rhs.name && dateTime == rhs.dateTime &&
           sex == rhs.sex && location == rhs.location &&
           lat == rhs.lat && lon == rhs.lon && hsys == rhs.hsys &&
           patch == rhs.patch && table == rhs.table;
}

} // namespace napatahti
