#ifndef ASPPAGE_H
#define ASPPAGE_H

#include <set>
#include <QPen>

#define ASP_S  51.428571429
#define ASP_BS 102.857142857
#define ASP_TS 154.285714286
#define ASP_PS 25.714285714

namespace napatahti {

class  AstroBase;


struct OrbTable {
    std::map<std::pair<int, double>, double> planet;
    std::map<double, double>                 asteroid;
    std::map<double, double>                 cuspid;
    std::array<double, 5>                    star;

    auto get(int keyA, int keyB, double asp) const -> double;
    auto reset() -> void;
    auto toTemp() -> void;
    static auto sourceOrb(double asp, double acc=15e-4, int level=4) -> double;
};


class AspPage
{
public :
    explicit AspPage() { loadPage(title_); }

    auto loadPage(const QString& title="") -> void;
    auto dumpPage(const QString& title="") const -> bool;

    auto createPage(const QString& title) -> bool;
    auto createAsp(const QVariantList& buffer) -> void;
    auto deleteAsp(double asp) -> void;
    auto insertAsp(const QVariantList& buffer) -> void;
    auto copyAsp(double asp) const -> QVariantList;

    auto setAspEnb(double asp) { return aspEnb_[asp] ^= true; }
    auto setAspEnb(const QVariantList& aspList) -> void;

    auto setAspStyle(const QVariantList& buffer) -> void;
    auto setAspPoint(double asp, double point) { aspPoint_[asp] = point; }

    auto setPlanetOrb(const std::pair<int, double>& key, double orb) { orbTable_.planet[key] = orb; }
    auto setAsterOrb(double asp, double orb) { orbTable_.asteroid[asp] = orb; }
    auto setCuspidOrb(double asp, double orb) { orbTable_.cuspid[asp] = orb; }
    auto setStarOrb(const std::array<double, 5>& orb) { orbTable_.star = orb; }

    auto& aspPage() { return *this; }

    auto& getAspCatalog() const { return aspCatalog_; }
    auto& getAspEnb() const { return aspEnb_; }
    auto& getAspPoint() const { return aspPoint_; }
    auto& getAspPen() const { return aspPen_; }
    auto& getAspDash() const { return aspDash_; }
    auto& getAspType() const { return aspType_; }
    auto& getOrbTable() const { return orbTable_; }

    static auto getTitle() -> const QString& { return title_; }
    static auto setTitle(const QString& title) -> void { title_ = title; }
    static auto titleR() -> const QString& { return titleR_; }
    static auto setTitleR(const QString& title) -> void { titleR_ = title; }
    static auto makePen(const QColor& color, int dash) -> QPen;

public :
    static const QString path;
    static const std::set<double> aspListBone;
    static const std::map<double, QString> aspCatalog;

protected :
    auto& aspPage() const { return *this; }

private :
    std::map<double, QString> aspCatalog_;
    std::map<double, bool>    aspEnb_;
    std::map<double, double>  aspPoint_;
    std::map<double, QPen>    aspPen_;
    std::map<double, int>     aspDash_;
    std::map<double, int>     aspType_;
    OrbTable orbTable_;

    static QString title_;
    static QString titleR_;

    static const std::map<double, bool>   aspEnbR_;
    static const std::map<double, double> aspPointR_;
    static const std::map<double, QColor> aspColorR_;
    static const std::map<double, int>    aspDashR_;
    static const std::map<double, int>    aspTypeR_;

private :
    auto restore() -> void;
    auto createTempPage() -> bool;
    auto validate() const -> bool;
};

} // namespace napatahti

#endif // ASPPAGE_H
