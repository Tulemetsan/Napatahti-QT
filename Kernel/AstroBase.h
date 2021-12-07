#ifndef ASTROBASE_H
#define ASTROBASE_H

#include <set>
#include <QDateTime>

namespace napatahti {

class Person;


struct PlanetEnb {
    bool crd {false};
    bool asp {false};

    operator bool() const { return crd && asp; }
};


class AstroBase
{
    using planet_string_t = std::map<int, std::array<QString, 2>>;
    using cuspid_string_t = std::array<std::array<QString, 2>, 12>;

public :
    explicit AstroBase(const Person& person);

    auto update(int space, const QLocale& locale) -> void;
    auto sortByCrd(const std::set<int>& keys) const -> std::vector<int>;

    auto createPlanet(const QStringList& data) -> bool;
    auto updatePlanet(const QStringList& data) -> bool;
    auto deletePlanet(int ind) -> bool;
    auto switchEnb(int ind, bool mode) -> void;
    auto computeCoupling(int space) -> void;
    auto dumpCatalog() const -> void;

    auto getJulDay() const { return julDay_; }

    auto& astroBase() { return *this; }
    auto& astroBase() const { return *this; }

    auto& getPlanetCatalog() const { return planetCatalog_; }
    auto& getPlanetEnb() const { return planetEnb_; }

    auto& getPlanetCrd() const { return planetCrd_; }
    auto& getPlanetSpeed() const { return planetSpeed_; }
    auto& getPlanetSignNo() const { return planetSignNo_; }
    auto& getPlanetDegree() const { return planetDegree_; }
    auto& getPlanetSpeedSym() const { return planetSpeedSym_; }
    auto& getPlanetHouseNo() const { return planetHouseNo_; }
    auto& getPlanetCrdStr() const { return planetCrdStr_; }
    auto& getPlanetOrder() const { return planetOrder_; }
    auto& getNormalizeCrd() const { return normalizeCrd_; }

    auto& getCuspidCrd() const { return cuspidCrd_; }
    auto& getCuspidSignNo() const { return cuspidSignNo_; }
    auto& getCuspidDegree() const { return cuspidDegree_; }
    auto& getCuspidCrdStr() const { return cuspidCrdStr_; }

    static auto getPlanetName(int key) -> QString;
    static auto getEpheRange() -> const std::array<QDateTime, 2>& { return epheRange_; }

public :
    static const std::vector<int> planetOrder;
    static const std::set<int>    noFictPlanet;
    static const std::set<int>    fictPlanet;
    static const std::map<int, double> middleSpeed;

protected :
    std::map<int, double> jyotCal_;
    std::map<int, double> westCal_;

    QStringList jyotCache_;
    QStringList westCache_;

    auto  revJulDay(double jujDay, int utc) const -> QDateTime;

private :
    const Person& person_;

    std::map<int, QString>   planetCatalog_;
    std::map<int, PlanetEnb> planetEnb_;

    std::map<int, double>  planetCrd_;
    std::map<int, double>  planetSpeed_;
    std::map<int, int>     planetSignNo_;
    std::map<int, int>     planetDegree_;
    std::map<int, QChar>   planetSpeedSym_;
    std::map<int, int>     planetHouseNo_;
    planet_string_t        planetCrdStr_;
    std::vector<int>       planetOrder_;
    std::map<int, double>  normalizeCrd_;

    std::array<double, 12> cuspidCrd_;
    std::array<int, 12>    cuspidSignNo_;
    std::array<int, 12>    cuspidDegree_;
    cuspid_string_t        cuspidCrdStr_;
    std::array<double, 12> houseLength_;

    double julDay_;
    double julTech_;

    enum Code {SweCalcOK = 258, SweRiseTransOK = 0, BeyondPolarCircle = -2};

    static std::array<QDateTime, 2> epheRange_;

    static char path_[];
    static const std::map<int, QString>   planetCatalogR_;
    static const std::map<int, PlanetEnb> planetEnbR_;

private :
    auto computeHouses() -> void;
    auto computePlanets() -> void;
    auto computePlanetHouseNo(std::set<int> keys={}) -> void;
    auto computeMoonCalendar() -> void;
    auto computeMoonDayCache(const QLocale& locale) -> void;

    auto getMoonJulDay(int rev, int day=0, double acc=1e-6) const -> double;

    auto loadCatalog() -> void;
    auto restore() -> void;
    auto setEpheRange() -> void;
    auto checkPlanet(int key) const -> bool;

    static auto toStrCrd(double crd, bool mode) -> QString;
    static auto getSpeedChar(int key, double speed) -> QChar;
};

} // namespace napatahti

#endif // ASTROBASE_H
