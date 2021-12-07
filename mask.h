#ifndef MASK_H
#define MASK_H

namespace napatahti {

struct KernelMask { enum Key {
    AstroBase    = 1,
    Coupling     = 2,
    AspTable     = 4,
    AspCfg       = 8,
    AspTableStar = 16,
    PrimeTest    = 32,
    AshaTest     = 64,
    CosmicTest   = 128,

    AspPageFull   = AspTable | PrimeTest | CosmicTest,
    PlanetCatFull = Coupling | AspCfg,
    AspTableFull  = AspTable | PrimeTest | CosmicTest,
    MapViewAcc    = AspTable | CosmicTest
};};


struct CanvasMask { enum Key {
    Header    = 1,
    RecepTree = 2,
    EsseStat  = 4,
    Circle    = 8,
    Aspects   = 16,
    CrdTable  = 32,
    LineBar   = 64,
    AspCfg    = 128,
    AspStat   = 256,
    CoreStat  = 512,
    FontCache = 1024,
    PenCache  = 2048,

    AspPageFull    = EsseStat | Aspects | CrdTable | LineBar | AspCfg | AspStat,
    AspPageED      = Aspects | AspCfg,
    PlanetCatFull  = Circle | CrdTable | LineBar | AspCfg,
    PlanetCatAsp   = Aspects | AspCfg,
    AspTableFull   = Aspects | EsseStat | LineBar | AspCfg | AspStat,
    CrdTableShort  = CrdTable | LineBar,
    CrdTableLong   = CrdTable | LineBar | AspCfg,
    CacheFull      = Header | RecepTree | EsseStat | Circle | CrdTable |
                     LineBar | AspCfg | AspStat | CoreStat | FontCache | PenCache,
    ColorSpecUpd   = Circle | CrdTable,
    ColorForceUpd  = AspStat | CoreStat,
    PenUpd         = Circle | LineBar | AspStat | CoreStat,
    AlmutenHard    = LineBar | CoreStat
};};

} // namespace napatahti

#endif // MASK_H
