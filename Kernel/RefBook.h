#ifndef REFBOOK_H
#define REFBOOK_H

#include <map>
#include <set>
#include <QString>

#define NONE -1

namespace napatahti {

//---------------------------------------------------------------------------//
// Elements : fire, land, water, air - 0, 1, 2, 3
// Crosses : I, II, III - 0, 1, 2
// Zones : I, II, III - 0, 1, 2
// Quadrants : I, II, III, IV - 0, 1, 2, 3
// Hemispheres : noth, south - 0, 1; est, west - 0, 1
// Bases : I, II, III - 0, 1, 2
// Degree Kad : king, destruct - true, false
//---------------------------------------------------------------------------//

class RefBook
{
public :
    const static QString speedSym;
    const static QString signSym;
    const static std::array<QString, 12> cuspidSym;
    const static std::array<QString, 12> signSymStr;
    const static std::array<QString, 12> cuspidSymStr;
    const static std::array<QString, 12> houseSymStr;

    const static std::map<std::array<int, 2>, int> specState;

    const static std::map<int, int> planetPosElem;
    const static std::map<int, int> planetNegElem;
    const static std::map<int, int> planetPosBase;
    const static std::map<int, int> planetNegBase;
    const static std::map<int, int> planetNegPlanet;
    const static std::map<int, std::vector<std::set<int>>> planetRelative;

    const static std::array<int, 12> signElem;
    const static std::array<int, 12> signCros;
    const static std::array<int, 12> signZone;
    const static std::array<int, 12> signQuad;
    const static std::array<int, 12> signHsns;
    const static std::array<int, 12> signHsew;
    const static std::array<std::array<int, 2>, 12> signRuler;
    const static std::array<std::array<int, 2>, 12> signOppos;
    const static std::array<std::array<int, 2>, 12> signEssen;

    const static std::array<int, 360> degreeRuler;
    const static std::map<int, bool>  degreeKad;
    const static std::map<int, int>   degreeExal;
    const static std::map<int, int>   degreeFall;

    const static std::array<int, 72> thermRuler;
    const static std::array<int, 36> decanRuler;
};

struct SpecState {enum Key {Ruler, Detriment, Exaltation, Fall, Neutral};};
struct ForceFlag {enum Key {Neutral, Dominant, Weak};};

} // namespace napatahti

#endif // REFBOOK_H
