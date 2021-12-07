#ifndef PRIMETEST_H
#define PRIMETEST_H

#include <map>
#include <QString>

namespace napatahti {

class AstroBase;
class AspTable;
class ConfigDialog;


struct CoreInd {
    const std::array<int, 16> globa {0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1};
    const std::array<int, 16> schit {4, 4, 6, 3, 3, 3, 6, 3, 2, 2, 2, 2, 2, 2, 1, 1};

    auto& operator[](int key) const { return *(&globa + key); }
};


struct CoreForce {
    CoreForce(QChar initType, double initTotal)
        : type (initType)
        , total (initTotal)
    {}

    std::vector<std::pair<double, int>> elem;
    std::vector<std::pair<double, int>> cros;
    std::vector<std::pair<double, int>> quad;
    std::vector<std::pair<double, int>> zone;
    std::vector<std::pair<double, int>> hsNS;
    std::vector<std::pair<double, int>> hsEW;
    std::vector<std::pair<double, int>> base;
    std::vector<std::pair<double, int>> elemBin;
    std::vector<std::pair<double, int>> quadBin;

    QChar  type;
    double total;

    auto makeCompare(int key, const std::vector<double>& src) -> void;
    auto elemScale() const { return 100 / (hsEW[0].first + hsEW[1].first); }
    auto baseScale() const { return 100 / (base[0].first + base[1].first + base[2].first); }

    auto begin() { return &elem; }
    auto end() { return &elemBin; }
    auto begin() const { return &elem; }
    auto end() const { return &elemBin; }
    auto cbegin() const { return &elem; }
    auto cend() const { return &quadBin + 1; }
    auto baseIt() const { return &base; }
};


struct CoreStat {
    CoreForce globaCos {'S', 0};
    CoreForce globaHor {'S', 0};
    CoreForce schitCos {'A', 0};
    CoreForce schitHor {'A', 0};

    auto& get(int method, int view) { return *(&globaCos + 2 * method + view); }
    auto& get(int method, int view) const { return *(&globaCos + 2 * method + view); }
};


struct MapType {
    std::pair<QString, int> viewCos;
    std::pair<QString, int> viewHor;

    auto& operator[](int key) { return *(&viewCos + key); }
    auto& operator[](int key) const { return *(&viewCos + key); }
};


struct PrimeInd {
    const std::map<int, double> globa {
        {0, 2}, {1, 2}, {2, 1.5}, {3, 1.5}, {4, 1.5}, {5, 1}, {6, 1}, {7, 1}, {8, 1},
        {9, 1}, {11, 0.5}, {24, 0.5}, {12, 0.5}, {56, 0.5}, {57, 1}, {15, 0.8}
    };
    const std::map<int, double> schit {
        {0, 2}, {1, 2}, {2, 1.5}, {3, 1.5}, {4, 1.5}, {5, 1}, {6, 1}, {7, 1}, {8, 1},
        {9, 1}, {11, 0.5}, {24, 0.5}, {12, 0.5}, {56, 0.5}, {57, 1}, {15, 1}
    };

    auto& operator[](int key) const { return *(&globa + key); }
};


struct PrimeStat {
    std::map<int, std::array<double, 2>> globaCos;
    std::map<int, std::array<double, 2>> globaHor;
    std::map<int, std::array<double, 2>> globaBoth;
    std::map<int, std::array<double, 2>> schitCos;
    std::map<int, std::array<double, 2>> schitHor;
    std::map<int, std::array<double, 2>> schitBoth;

    auto& get(int method, int view) { return *(&globaCos + 3 * method + view); }
    auto& get(int method, int view) const { return *(&globaCos + 3 * method + view); }
};


struct PrimeFict {
    std::map<int, std::pair<double, int>> globa;
    std::map<int, std::pair<double, int>> schit;

    auto& operator[](int key) { return *(&globa + key); }
    auto& operator[](int key) const { return *(&globa + key); }
};


class PrimeTest
{
public :
    explicit PrimeTest(const AstroBase& astroBase, const AspTable& aspTable)
        : astroBase_ (astroBase)
        , aspTable_ (aspTable)
    {}

    auto update() -> void;

    auto& getMapType(int view) const { return mapType_[view]; }

protected :
    CoreStat  coreStat_;
    PrimeStat primeStat_;
    PrimeFict primeFict_;
    MapType   mapType_;

private :
    const AstroBase& astroBase_;
    const AspTable&  aspTable_;

    std::map<std::array<int, 2>, bool> almuten_;

    enum {Elem, Cros, Quad, Zone, HsNS, HsEW, Base, ElemBin, QuadBin};

    static bool almuHard_;

    const static bool almuHardR_;
    const static CoreInd  coreInd_;
    const static PrimeInd primeInd_;

private :
    auto computeCoreStat(int method, int view) -> void;
    auto computePrimeStat(int method, int view) -> void;
    auto computeAlmuten() -> void;
    auto computeMapType(int view) -> void;

    static auto setStatus(std::array<double, 2>& status, int multip, int forceFlag) -> void;

    friend ConfigDialog;
};

} // namespace napatahti

#endif // PRIMETEST_H
