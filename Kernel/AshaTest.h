#ifndef ASHATEST_H
#define ASHATEST_H

#include <set>
#include <map>
#include <QString>

namespace napatahti {

class AstroBase;


struct AshaSpec {
    std::set<int> anareta;
    std::set<int> alcocoden;
    std::set<int> dGenitura;

    auto begin() { return &anareta; }
    auto end() { return &dGenitura + 1; }
    auto begin() const { return &anareta; }
    auto end() const { return &dGenitura + 1; }
    auto dGenIt() { return &dGenitura; }
};


class AshaTest
{
public :
    explicit AshaTest(const AstroBase& astroBase) : astroBase_ (astroBase) {}

    auto update() -> void;
    auto toStrAshaStat() const -> QString;

    auto& getAshaSpec() const { return ashaSpec_; }

protected :
    std::map<int, std::array<double, 2>> powerKindness_;
    std::map<int, std::array<double, 2>> powerCreative_;
    std::map<int, std::array<double, 2>> kindDetailed_;

private :
    const AstroBase&  astroBase_;

    AshaSpec ashaSpec_;
};

} // namespace napatahti

#endif // ASHATEST_H
