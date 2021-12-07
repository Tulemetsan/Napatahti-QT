#ifndef STATIC_H
#define STATIC_H

#include "Kernel/Person.h"
#include "Kernel/AstroBase.h"
#include "Kernel/RefBook.h"
#include "Kernel/AspPage.h"
#include "Kernel/AspTable.h"
#include "Kernel/PrimeTest.h"
#include "Appgui/MainWindow.h"
#include "Appgui/Canvas.h"
#include "Appgui/SharedGui.h"
#include "Appgui/CityDialog.h"
#include "Appgui/AtlasDialog.h"
#include "Appgui/PersonDialog.h"
#include "Appgui/DataBaseDialog.h"
#include "Appgui/AspPageDialog.h"
#include "Appgui/AspDialog.h"
#include "Appgui/AspTableDialog.h"
#include "Appgui/AspPatchDialog.h"
#include "Appgui/ConfigDialog.h"

namespace napatahti {

auto staticInit(QApplication& app) -> void;

//---------------------------------------------------------------------------//

QString Person::name_;
QString Person::location_;
int Person::utc_;
int Person::lat_;
int Person::lon_;
int Person::hsys_;

//---------------------------------------------------------------------------//

std::array<QDateTime, 2> AstroBase::epheRange_;

const std::vector<int> AstroBase::planetOrder {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 24, 12, 56, 57, 15};
const std::set<int> AstroBase::noFictPlanet {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 57};
const std::set<int> AstroBase::fictPlanet {11, 12, 24, 56};
const std::map<int, double> AstroBase::middleSpeed {
    {0, 0.985555}, {1, 13.176389}, {2, 1.303888}, {3, 1.199166}, {4, 0.524166},
    {5, 0.083055}, {6, 0.033333}, {7, 0.0116666}, {8, 0.006111}, {9, 0.003888},
    {11, 0.053055}, {57, 0.001388}, {15, 0.019444}, {24, 0.053055}
};

char AstroBase::path_[] {"ephe"};
const std::map<int, QString> AstroBase::planetCatalogR_ {
    {0, "Q"}, {1, "W"}, {2, "E"}, {3, "R"}, {4, "T"}, {5, "Y"}, {6, "U"},
    {7, "I"}, {8, "O"}, {9, "P"}, {11, "{"}, {12, "`"}, {15, "M"}, {18, "V"},
    {19, "B"}, {20, "N"}, {24, "}"}, {56, "Ñ"}, {57, "Ð"}
};
const std::map<int, PlanetEnb> AstroBase::planetEnbR_ {
    {0, {true, true}}, {1, {true, true}}, {2, {true, true}}, {3, {true, true}},
    {4, {true, true}}, {5, {true, true}}, {6, {true, true}}, {7, {true, true}},
    {8, {true, true}}, {9, {true, true}}, {11, {true, true}}, {12, {true, true}},
    {15, {true, true}}, {18, {false, false}}, {19, {false, false}}, {20, {false, false}},
    {24, {true, true}}, {56, {true, true}}, {57, {true, true}}
};

//---------------------------------------------------------------------------//

const QString RefBook::speedSym {"%*$"};
const QString RefBook::signSym  {"asdfghjklzxc"};
const std::array<QString, 12> RefBook::cuspidSym {
    "Z", "2", "3", "“", "5", "6", "’", "8", "9", "X", "11", "12"
};
const std::array<QString, 12> RefBook::signSymStr {
    "Ari", "Tau", "Gem", "Cnc", "Leo", "Vir", "Lib", "Sco", "Sgr", "Cap", "Aqr", "Psc"
};
const std::array<QString, 12> RefBook::cuspidSymStr {
    "AC", "II", "III", "IC", "V", "VI", "DC", "VIII", "IX", "MC", "XI", "XII"
};
const std::array<QString, 12> RefBook::houseSymStr {
    "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII"
};

const std::map<std::array<int, 2>, int> RefBook::specState {
    {{0, 4}, 0}, {{1, 3}, 0}, {{2, 2}, 0}, {{3, 1}, 0}, {{3, 6}, 0}, {{4, 0}, 0},
    {{4, 7}, 0}, {{5, 8}, 0}, {{5, 11}, 0}, {{6, 9}, 0}, {{6, 10}, 0}, {{7, 10}, 0},
    {{8, 11}, 0}, {{9, 7}, 0}, {{57, 5}, 0}, {{15, 6}, 0},
    {{0, 10}, 1}, {{1, 9}, 1}, {{2, 8}, 1}, {{3, 7}, 1}, {{3, 0}, 1}, {{4, 6}, 1},
    {{4, 1}, 1}, {{5, 2}, 1}, {{5, 5}, 1}, {{6, 3}, 1}, {{6, 4}, 1}, {{7, 4}, 1},
    {{8, 5}, 1}, {{9, 1}, 1}, {{57, 11}, 1}, {{15, 0}, 1},
    {{0, 0}, 2}, {{1, 1}, 2}, {{2, 5}, 2}, {{3, 11}, 2}, {{4, 9}, 2}, {{5, 3}, 2},
    {{6, 6}, 2}, {{7, 7}, 2}, {{8, 10}, 2}, {{9, 4}, 2}, {{11, 2}, 2}, {{24, 8}, 2},
    {{12, 7}, 2}, {{56, 1}, 2}, {{57, 2}, 2}, {{15, 8}, 2},
    {{0, 6}, 3}, {{1, 7}, 3}, {{2, 11}, 3}, {{3, 5}, 3}, {{4, 3}, 3}, {{5, 9}, 3},
    {{6, 0}, 3}, {{7, 1}, 3}, {{8, 4}, 3}, {{9, 10}, 3}, {{11, 8}, 3}, {{24, 2}, 3},
    {{12, 1}, 3}, {{56, 7}, 3}, {{57, 8}, 3}, {{15, 2}, 3}
};

const std::map<int, int> RefBook::planetPosElem {
    {0, 0}, {1, 3}, {2, 2}, {3, 1}, {4, 0}, {5, 0}, {6, 1}, {7, 2}, {8, 3}, {9, 3},
    {57, 1}, {15, 2}
};
const std::map<int, int> RefBook::planetNegElem {
    {0, 3}, {1, 0}, {2, 1}, {3, 2}, {4, 3}, {5, 3}, {6, 2}, {7, 1}, {8, 0}, {9, 0},
    {57, 2}, {15, 1}
};
const std::map<int, int> RefBook::planetPosBase {
    {0, 0}, {1, 1}, {2, 2}, {3, 1}, {4, 0}, {5, 0}, {6, 1}, {7, 2}, {8, 1}, {9, 0},
    {57, 1}, {15, 2}
};
const std::map<int, int> RefBook::planetNegBase {
    {0, 1}, {1, 0}, {2, NONE}, {3, 0}, {4, 1}, {5, 1}, {6, 0}, {7, NONE}, {8, 0}, {9, 1},
    {57, 0}, {15, NONE}
};
const std::map<int, int> RefBook::planetNegPlanet {
    {0, 7}, {1, 6}, {2, 5}, {3, 9}, {4, 15}, {5, 2}, {6, 1}, {7, 0}, {8, 57}, {9, 3},
    {57, 8}, {15, 4}
};
const std::map<int, std::vector<std::set<int>>> RefBook::planetRelative {
    {0, {{4, 5}, {1, 7}}}, {1, {{3, 8}, {0, 6}}}, {2, {{15, 7, 57}, {5}}},
    {3, {{1, 8, 15}, {4, 9}}}, {4, {{0, 5, 9}, {3, 15}}}, {5, {{0, 4, 8, 15}, {2, 6}}},
    {6, {{7, 57}, {1, 5}}}, {7, {{2, 6, 15}, {0}}}, {8, {{1, 3, 5}, {9, 57}}},
    {9, {{4}, {3, 8}}}, {57, {{2, 6}, {8}}}, {15, {{2, 3, 5, 7}, {4}}}
};

const std::array<int, 12> RefBook::signElem {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
const std::array<int, 12> RefBook::signCros {0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2};
const std::array<int, 12> RefBook::signZone {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2};
const std::array<int, 12> RefBook::signQuad {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3};
const std::array<int, 12> RefBook::signHsns {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0};
const std::array<int, 12> RefBook::signHsew {0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0};
const std::array<std::array<int, 2>, 12> RefBook::signRuler {
    std::array<int, 2>{4, 9}, {3, 15}, {2, 57}, {1, NONE}, {0, NONE}, {57, 2},
    {15, 3}, {9, 4}, {5, 8}, {6, 7}, {7, 6}, {8, 5}
};
const std::array<std::array<int, 2>, 12> RefBook::signOppos {
    std::array<int, 2>{15, 3}, {9, 4}, {5, 8}, {6, 7}, {7, 6}, {8, 5}, {4, 9},
    {3, 15}, {2, 57}, {1, NONE}, {0, NONE}, {57, 2}
};
const std::array<std::array<int, 2>, 12> RefBook::signEssen {
    std::array<int, 2>{0, 6}, {1, 7}, {57, 15}, {5, 4}, {9, 8}, {2, 3}, {6, 0},
    {7, 1}, {15, 57}, {4, 5}, {8, 9}, {3, 2}
};

const std::array<int, 360> RefBook::degreeRuler {
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8
};
const std::map<int, bool> RefBook::degreeKad {
    {17, true}, {68, true}, {126, true}, {174, true}, {222, true}, {280, true},
    {329, true}, {22, false}, {72, false}, {159, false}, {180, false}, {228, false},
    {288, false}, {333, false}
};
const std::map<int, int> RefBook::degreeExal {
    {19, 0}, {33, 1}, {165, 2}, {357, 3}, {298, 4}, {105, 5}, {200, 6}, {232, 7},
    {308, 8}, {142, 9}, {64, 57}, {247, 15}
};
const std::map<int, int> RefBook::degreeFall {
    {199, 0}, {213, 1}, {345, 2}, {177, 3}, {118, 4}, {285, 5}, {20, 6}, {52, 7},
    {128, 8}, {322, 9}, {244, 57}, {67, 15}
};

const std::array<int, 72> RefBook::thermRuler {
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0
};
const std::array<int, 36> RefBook::decanRuler {
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9, 0, 8, 7, 1, 57, 15,
    4, 0, 3, 2, 1, 6, 5, 9
};

//---------------------------------------------------------------------------//

QString AspPage::title_;
QString AspPage::titleR_;

const QString AspPage::path {"aspects/"};
const std::set<double> AspPage::aspListBone {0, 40, 60, 72, 80, 90, 120, 144, 180};
const std::map<double, QString> AspPage::aspCatalog {
    {0, "q"}, {180, "w"}, {135, "u"}, {90, "r"}, {45, "y"}, {120, "e"}, {150, "K"},
    {60, "t"}, {30, "i"}, {72, "Q"}, {108, "TD"}, {144, "BQ"}, {36, "D"}, {40, "N"},
    {160, "KN"}, {80, "BN"}, {20, "PN"}, {100, "SG"}, {140, "SD"}, {ASP_S, "S"},
    {ASP_TS, "TS"}, {ASP_BS, "BS"}, {ASP_PS, "PS"}
};

const std::map<double, bool> AspPage::aspEnbR_ {
    {0, true}, {180, true}, {135, true}, {90, true}, {45, true}, {120, true},
    {150, true}, {60, true}, {30, true}, {72, true}, {144, true}, {108, true},
    {36, true}, {40, true}, {80, true}, {160, true}, {20, true}, {100, false},
    {140, false}, {ASP_S, false}, {ASP_TS, false}, {ASP_BS, false}, {ASP_PS, false}
};
const std::map<double, double> AspPage::aspPointR_ {
    {0, 4}, {180, 3}, {135, 0.5}, {90, 2}, {45, 1}, {120, 3}, {150, 0.5}, {60, 2},
    {30, 1}, {72, 6}, {108, 1}, {144, 4}, {36, 2}, {40, 6}, {160, 2}, {80, 4},
    {20, 1}, {100, 0}, {140, 0}, {ASP_S, 0}, {ASP_TS, 0}, {ASP_BS, 0}, {ASP_PS, 0}
};
const std::map<double, QColor> AspPage::aspColorR_ {
    {0, "#008000"}, {180, "#000000"}, {135, "#000000"}, {90, "#000000"},
    {45, "#000000"}, {120, "#f00000"}, {150, "#f00000"}, {60, "#f00000"},
    {30, "#f00000"}, {72, "#00c800"}, {108, "#00c800"}, {144, "#00c800"},
    {36, "#00c800"}, {40, "#0080ff"}, {160, "#0080ff"}, {80, "#0080ff"},
    {20, "#0080ff"}, {100, "#0080ff"}, {140, "#0080ff"}, {ASP_S, "#9400d3"},
    {ASP_TS, "#9400d3"}, {ASP_BS, "#9400d3"}, {ASP_PS, "#9400d3"}
};
const std::map<double, int> AspPage::aspDashR_ {
    {0, 0}, {180, 0}, {135, 1}, {90, 0}, {45, 2}, {120, 0}, {150, 1}, {60, 0},
    {30, 2}, {72, 0}, {108, 1}, {144, 0}, {36, 2}, {40, 0}, {160, 1}, {80, 0},
    {20, 2}, {100, 3}, {140, 3}, {ASP_S, 0}, {ASP_TS, 1}, {ASP_BS, 0}, {ASP_PS, 2}
};
const std::map<double, int> AspPage::aspTypeR_ {
    {0, 1}, {180, 1}, {135, 1}, {90, 1}, {45, 1}, {120, 1}, {150, 0}, {60, 1},
    {30, 1}, {72, 0}, {108, 0}, {144, 0}, {36, 0}, {40, 0}, {160, 0}, {80, 0},
    {20, 0}, {100, 0}, {140, 0}, {ASP_S, 0}, {ASP_TS, 0}, {ASP_BS, 0}, {ASP_PS, 0}
};

//---------------------------------------------------------------------------//

int AspTable::mvAcc_;
QString AspTable::path_;

const AspGroup AspTable::aspGroup {};
const MajorAsp AspTable::majorAsp_ {};

AspTable::src_asp_config_list_t AspTable::classic3Point_;
AspTable::src_asp_config_list_t AspTable::classic4Point_;
AspTable::src_asp_config_list_t AspTable::ext3PointR_;
AspTable::src_asp_config_list_t AspTable::ext4PointR_;

//---------------------------------------------------------------------------//

bool PrimeTest::almuHard_;
const bool PrimeTest::almuHardR_ {true};
const CoreInd PrimeTest::coreInd_ {};
const PrimeInd PrimeTest::primeInd_ {};

//---------------------------------------------------------------------------//

AppTrigger MainWindow::trigger_;

//---------------------------------------------------------------------------//

CanvasFont Canvas::fontSrc_;
CanvasColor Canvas::colorSrc_;
CanvasPen Canvas::penSrc_;

QMap<QString, int> Canvas::headerSizeSrc_;
QMap<QString, int> Canvas::recepSizeSrc_;
QMap<QString, int> Canvas::esseStatSizeSrc_;
QMap<QString, int> Canvas::circleSizeSrc_;
QMap<QString, int> Canvas::crdTableSizeSrc_;
QMap<QString, int> Canvas::lineBarSizeSrc_;
QMap<QString, int> Canvas::aspCfgSizeSrc_;
QMap<QString, int> Canvas::aspStatSizeSrc_;
QMap<QString, int> Canvas::coreStatSizeSrc_;

//---------------------------------------------------------------------------//

QLocale SharedGui::locale_;
QMargins SharedGui::margins_;

//---------------------------------------------------------------------------//

const QRegularExpression CityDialog::utcReg {
    "^(\\+((14:00)|((0\\d)|(1[0-3])):[0-5]\\d))|(\\-((12:00)|((0\\d)|(1[0-1])):[0-5]\\d))$"};
const QRegularExpression CityDialog::latReg {
    "^((90º\\s00'\\s00\")|([0-8]\\dº\\s[0-5]\\d'\\s[0-5]\\d\"))\\s[N,S]$"};
const QRegularExpression CityDialog::lonReg {
    "^((180º\\s00'\\s00\")|(((0\\d)|(1[0-7]))\\dº\\s[0-5]\\d'\\s[0-5]\\d\"))\\s[E,W]$"};

//---------------------------------------------------------------------------//

QString AtlasDialog::dbaseName_;
QVariantMap AtlasDialog::newCity_;

//---------------------------------------------------------------------------//

QSize PersonDialog::atlasSize_;
int PersonDialog::leftMgn_;
int PersonDialog::topMgn_;
int PersonDialog::topHeight_ {-1};
QWidget* PersonDialog::atlasRoot_ {nullptr};

std::map<int, QString> PersonDialog::sexTitle_;
std::map<int, QString> PersonDialog::hsysTitle_;

const std::array<int, 3> PersonDialog::sexOrder {'E', 'M', 'F'};
const std::array<int, 8> PersonDialog::hsysOrder {'P', 'K', 'D', 'A', 'S', 'O', 'C', 'B'};

//---------------------------------------------------------------------------//

QString DataBaseDialog::table_;

const DataBaseDialog::SqlMask DataBaseDialog::mask_;
const QString DataBaseDialog::dbaseName_ {"persons.sqlite"};
const QString DataBaseDialog::tableBack_ {"Builtin"};
const QRegularExpression DataBaseDialog::tableReg_ {"^([a-z]|[A-Z])\\w+$"};

//---------------------------------------------------------------------------//

QString AspPageDialog::sampPath_;
std::array<QBrush, 2> AspPageDialog::brushED_;
QPen LineDelegate::markPen_;

const QRegularExpression AspPageDialog::pageReg_ {"^[^/\\\\:*?<>|\\.]+$"};

//---------------------------------------------------------------------------//

const std::map<int, QColor> AspDialog::baseColor_ {
    {0, "#008000"}, {2, "#000000"}, {4, "#f00000"}, {6, "#00c800"},
    {8, "#0080ff"}, {10, "#9400d3"}, {12, "#daa520"}};

//---------------------------------------------------------------------------//

std::array<QBrush, 3> AspTableDialog::brush_;

//---------------------------------------------------------------------------//

QStringList AspPatchDialog::accDesc_;

//---------------------------------------------------------------------------//

QString ConfigDialog::profileName_;
QString ConfigDialog::profileNameR_;

const QString ConfigDialog::path {"profile/"};

} // namespace napatahti

#endif // STATIC_H
