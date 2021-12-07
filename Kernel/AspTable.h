#ifndef ASPTABLE_H
#define ASPTABLE_H

#include <set>
#include <QGraphicsItem>

namespace napatahti {

class Person;
class AstroBase;
class AspPage;
class ConfigDialog;


struct KeyPair {
    KeyPair(int keyA, int keyB)
        : less (keyA < keyB ? keyA : keyB)
        , more (keyA < keyB ? keyB : keyA)
    {}
    KeyPair(const std::array<int, 2>& key)
        : less (key[0] < key[1] ? key[0] : key[1])
        , more (key[0] < key[1] ? key[1] : key[0])
    {}
    KeyPair(const QString& str)
        : less (str.split(", ")[0].toInt())
        , more (str.split(", ")[1].toInt())
    {}

    const int less;
    const int more;

    auto contains(int key) const { return key == less || key == more; }
    auto other(int key) const { return key == less ? more : less; }
    auto empty() const { return false; }
    auto size() const { return static_cast<unsigned long long>(2); }
    auto begin() const { return &less; }
    auto end() const { return &more + 1; }

    operator QString() const { return QString("%1, %2").arg(less).arg(more); }
    auto operator<(const KeyPair& rhs) const -> bool;
    auto operator>(const KeyPair& rhs) const -> bool;
    auto operator==(const KeyPair& rhs) const { return less == rhs.less && more == rhs.more; }
    auto operator!=(const KeyPair& rhs) const { return !operator==(rhs); };
};


struct AspData {
    AspData() {}
    AspData(double ang, double asp, double orb, int acc)
        : ang (ang), asp (asp), orb (orb), acc (acc) {}
    AspData(const QString& str);

    double ang;
    double asp;
    double orb;
    int    acc;

    operator QString() const;
    auto operator==(const AspData& rhs) const -> bool;
    auto operator!=(const AspData& rhs) const { return !operator==(rhs); }
};


struct AspGroup {
    enum {
        Union, Black, Red, Green, Blue, Violet,
        BlackRed, BlackGreen, BlackBlue, RedBlue, NotMarked
    };
    std::vector<int> normal  {Union, Black, Red, Green, Blue, Violet, NotMarked};
    std::vector<int> compute {Union, Black, Red, Green, Blue};
};


struct AspCfgData {
    int              aspGroup;
    std::vector<int> excepList;
    double           baseAsp;
    int              type;
    QString          title;
};


struct AspFictStat {
    std::map<int, double> globa {{11, 0}, {12, 0}, {24, 0}, {56, 0}};
    std::map<int, double> schit {{11, 0}, {12, 0}, {24, 0}, {56, 0}};

    auto begin() { return &globa; }
    auto end() { return &schit + 1; }
    auto begin() const { return &globa; }
    auto end() const { return &schit + 1; }
};


struct MajorAsp {
    std::set<double> globa {180, 90, 120, 60, 72, 108, 40, 100};
    std::set<double> schit {180, 90, 120, 60, 72, 144, 40, 80};
};


struct AspTabSpec {
    enum {RexAsp, InPit, TopRedGreen, TopBlackBlue, Arrow, Bed};

    std::set<int> rexAsp;
    std::set<int> inPit;
    std::set<int> topRedGreen;
    std::set<int> topBlackBlue;
    std::set<int> arrow;
    std::set<int> bed;

    std::vector<std::set<int>> stelliumSign;

    auto begin() { return &rexAsp; }
    auto end() { return &bed + 1; }
    auto begin() const { return &rexAsp; }
    auto end() const { return &bed + 1; }
};


class AspTable
{
public :
    using asp_list_t = std::vector<double>;
    using src_asp_config_list_t = std::vector<std::pair<asp_list_t, AspCfgData>>;
    using src_asp_config_table_t = std::map<int, std::pair<const asp_list_t*, const AspCfgData*>>;

    using asp_sample_t = std::vector<std::map<KeyPair, const AspData*>>;
    using planet_star_table_t = std::map<int, std::array<QString, 2>>;
    using cuspid_star_table_t = std::array<std::array<QString, 2>, 12>;

    using bone_src_t = std::pair<std::set<int>, double>;
    using asp_cfg_seq_t = std::set<std::vector<int>>;
    using asp_config_table_t = std::map<int, std::pair<asp_cfg_seq_t, const AspCfgData*>>;
    using asp_config_bone_t = std::map<std::vector<int>, std::pair<std::vector<int>, double>>;

    using asp_field_t = std::array<std::pair<double, int>, 5>;
    using map_view_link_t = std::vector<std::pair<double, int>>;
    using map_view_chain_t = std::vector<map_view_link_t>;

public :
    explicit AspTable(const Person& person, const AstroBase& astroBase, const AspPage& aspPage);

    auto update(bool mode, int accKey, int viewKey) -> void;
    auto computeStarTable() -> void;
    auto getAspSample(bool bonesOnly, QGraphicsItem* item=nullptr) const -> const asp_sample_t&;

    auto& aspTable() const { return *this; }

    auto& getPlanetX2Table() const { return planetX2Table_; }
    auto& getPlanetStarTable() const { return planetStarTable_; }
    auto& getCuspidStarTable() const { return cuspidStarTable_; }

    auto& getAspCfgTable() const { return aspCfgTable_; }
    auto& getAspCfgBone() const { return aspCfgBone_; }
    auto& getAspCfgOffset() const { return aspCfgOffset_; }

    auto& getAspField() const { return aspField_; }
    auto& getAspFictStat() const { return aspFictStat_; }
    auto& getAspTabSpec() const { return aspTabSpec_; }

    static auto makePatch(const std::map<KeyPair, AspData>& patch) -> QString;
    static auto makePatch(const QString& patch) -> std::map<KeyPair, AspData>;

public :
    static const AspGroup aspGroup;

private :
    const Person&    person_;
    const AstroBase& astroBase_;
    const AspPage&   aspPage_;

    std::array<int, 2> aspConfigEnd_;
    src_asp_config_list_t  ext3Point_;
    src_asp_config_list_t  ext4Point_;
    src_asp_config_table_t srcAspConfigTable_;

    std::map<KeyPair, AspData> planetX2Table_;
    asp_sample_t               aspSample_;
    mutable asp_sample_t       aspSampleMarkCfg_;
    std::vector<std::set<int>> keySeqList_;
    planet_star_table_t        planetStarTable_;
    cuspid_star_table_t        cuspidStarTable_;

    std::set<std::set<int>> zeroMass_;
    asp_config_table_t      aspCfgTable_;
    asp_config_bone_t       aspCfgBone_;
    std::array<int, 2>      aspCfgOffset_;

    asp_field_t aspField_;
    AspFictStat aspFictStat_;
    AspTabSpec  aspTabSpec_;

    int  mapView_;
    bool isMajorCfg_;

    static int mvAcc_;
    static QString path_;

    static const MajorAsp majorAsp_;

    static src_asp_config_list_t classic3Point_;
    static src_asp_config_list_t classic4Point_;
    static src_asp_config_list_t ext3PointR_;
    static src_asp_config_list_t ext4PointR_;

private :
    auto computePlanetX2Table() -> void;
    auto computeAspConfig() -> void;
    auto computeAspField() -> void;
    auto computeRexAspInPit() -> void;
    auto computeAspFictStat() -> void;
    auto computeMapView() -> void;

    auto makeSample() -> void;
    auto makeSample(int accKey) -> void;
    auto addToSample(const KeyPair& key, const AspData* data) -> void;

    auto findZeroCfg() -> void;
    auto findAspCfg(int ind, bool mode) -> void;
    auto makeAspCfg(int ind, const std::set<int>& mass, const bone_src_t& bone, bool mode) -> void;
    auto setAspCfgOffset(const std::vector<int> &cfg, int ind) -> void;

    auto makePlanetChain(map_view_chain_t& chain) -> void;
    auto setMapViewSpec(const map_view_chain_t& chain) -> void;

    auto loadExtCfg() -> void;
    auto dumpExtCfg(int ind) const -> void;

    static auto getAspGroupInd(double asp) -> int;

    friend ConfigDialog;
    friend auto staticInit(QApplication& app) -> void;
};

} // namespace napatahti

#endif // ASPTABLE_H
