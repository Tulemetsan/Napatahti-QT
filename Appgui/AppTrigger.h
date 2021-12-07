#ifndef APPTRIGGER_H
#define APPTRIGGER_H

#include <QAction>

namespace napatahti {

class AppTrigger
{
public :
    AppTrigger();

    auto set(const QAction* action) -> bool;

    auto key(bool* item) -> int { return item - &viewToolBar; }
    auto get(int key) { return &viewToolBar + key; }
    auto excBeg() { return &modeJyotisa; }

    auto begin() { return &viewToolBar; }
    auto end() { return &modeSchit + 1; }

public :
    bool viewToolBar {true};
    bool viewHeader {true};
    bool viewRecepTree {true};
    bool viewEsseStat {true};
    bool viewCircle {true};
    bool viewCrdTable {true};
    bool viewLineBar {true};
    bool viewAspCfg {true};
    bool viewAspStat {true};
    bool viewCoreStat {true};
    bool viewStatusBar {true};

    bool bySunSpec {true};
    bool byEsseSpec {true};
    bool byAspSpec {true};
    bool dorAuriga {true};
    bool byAshaSpec {true};

    bool circleDegrees {true};
    bool circleSpeeds {true};
    bool circleCuspids {true};
    bool circleColors {false};

    bool fixedStars {true};

    bool aspCfgClassOnly {false};
    bool aspCfgBonesOnly {false};

    bool coreStatCosEnb {true};
    bool coreStatHorEnb {true};
    bool coreStatInvert {false};

    bool modeJyotisa {false};
    bool modeWestern {true};

    bool accMid {false};
    bool accMax {false};
    bool accMin {false};
    bool accAny {true};

    bool zeroAngAsc {true};
    bool zeroAngAri {false};

    bool modePrime {false};
    bool modeAsha {true};
    bool modeCosmic {false};

    bool primeCosDet {false};
    bool primeHorDet {false};
    bool primeNormal {true};

    bool ashaPowKind {true};
    bool ashaPowCreat {false};
    bool ashaKindDet {false};

    bool cosmicSum {true};
    bool cosmicDet {false};

    bool modeGloba {false};
    bool modeSchit {true};

private :
    QList<QList<int>> groupList_;
};

}

#endif // APPTRIGGER_H
