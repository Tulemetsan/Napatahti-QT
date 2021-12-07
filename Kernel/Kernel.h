#ifndef KERNEL_H
#define KERNEL_H

#include "Person.h"
#include "RefBook.h"
#include "AstroBase.h"
#include "AspPage.h"
#include "AspTable.h"
#include "PrimeTest.h"
#include "AshaTest.h"
#include "CosmicTest.h"

namespace napatahti {

class AppTrigger;


class Kernel
    : public Person
    , public AstroBase
    , public AspPage
    , public AspTable
    , public PrimeTest
    , public AshaTest
    , public CosmicTest
{
public :
    explicit Kernel();

    auto getHeaderStr() const -> const QString& { return headerStr_; }
    auto setHeaderStr() -> void;

    auto getMoonDay() const -> const QStringList&;
    auto toStrMoonCalendar() const -> QString;

    auto getAccKey() const -> int;

    auto getCoreForce(int view) const -> const CoreForce&;
    auto getPlanetStat() const -> const std::map<int, std::array<double, 2>>&;
    auto getFictionStat() const -> const std::map<int, std::pair<double, int>>*;
    auto toStrCoreStat() const -> QString;
    auto toStrPrimeStat() const -> QString;

private :
    const AppTrigger& trigger_;
    const QLocale& locale_;

    QString headerStr_;
};

} // namespace napatahti

#endif // KERNEL_H
