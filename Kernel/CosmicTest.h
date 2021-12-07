#ifndef COSMICTEST_H
#define COSMICTEST_H

#include <set>
#include <map>
#include <QString>

namespace napatahti {

struct KeyPair;
class  AstroBase;
class  AspTable;


struct RecepTree {
    enum {ByRuler, ByDetriment, ByExaltation, ByFall, ByDegree};

    int total {0};
    std::set<KeyPair> byRuler;
    std::set<KeyPair> byDetr;
    std::set<KeyPair> byExal;
    std::set<KeyPair> byFall;
    std::set<KeyPair> byDegree;

    auto& operator[](int key) { return *(&byRuler + key); }
    auto& operator[](int key) const { return *(&byRuler + key); }

    auto begin() { return &byRuler; }
    auto end() { return &byDegree + 1; }
    auto begin() const { return &byRuler; }
    auto end() const { return &byDegree + 1; }
};


struct CosmicSpec {
    enum {
        Atlas, Sisyphus, Antaeus, Icarus, Est, West, Doriphoros, Auriga,
        Heart, Burning, SunZone, FreeZone};

    std::set<int> atlas;
    std::set<int> sisyphus;
    std::set<int> antaeus;
    std::set<int> icarus;
    std::set<int> est;
    std::set<int> west;
    std::set<int> doriph;
    std::set<int> auriga;
    std::set<int> heart;
    std::set<int> burning;
    std::set<int> sunZone;
    std::set<int> freeZone;

    auto& operator[](int key) { return *(&atlas + key); }
    auto& operator[](int key) const { return *(&atlas + key); }

    auto begin() { return &atlas; }
    auto end() { return &freeZone + 1; }
    auto begin() const { return &atlas; }
    auto end() const { return &freeZone + 1; }
};


class CosmicTest
{
public :
    using planet_chain_t = std::array<std::vector<std::set<int>>, 4>;

public :
    explicit CosmicTest(const AstroBase& astroBase, const AspTable& aspTable)
        : astroBase_ (astroBase), aspTable_ (aspTable)
    {}

    auto update() -> void;
    auto toStrCosmicStat() const -> QString;

    auto  getCosmicSum() const { return cosmicSum_; }
    auto& getRecepTree() const { return recepTree_; }
    auto& getCosmicSpec() const { return cosmicSpec_; }

protected :
    std::map<int, std::array<double, 2>> summary_;
    std::map<int, std::array<double, 2>> detailed_;

private :
    const AstroBase&  astroBase_;
    const AspTable&   aspTable_;

    double cosmicSum_;

    RecepTree  recepTree_;
    CosmicSpec cosmicSpec_;

    planet_chain_t planetChain_;

private :
    auto computeCosmicStat() -> void;
    auto computeRecepTree() -> void;
    auto computeCosmicSpec() -> void;
    auto addToRecepTree(int keyA, int keyB, int type) -> void;
    auto addToPlanetChain(int keyA, int keyB, int chainNo) -> void;
};

} // namespace napatahti

#endif // COSMICTEST_H
