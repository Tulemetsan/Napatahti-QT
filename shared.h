#ifndef SHARED_H
#define SHARED_H

#include <cmath>
#include <map>
#include <QString>

namespace napatahti {

inline auto sinDeg(double angle) { return std::sin(angle * 0.01746031746); }
inline auto cosDeg(double angle) { return std::cos(angle * 0.01746031746); }

auto errorLog(const QString& error) -> void;

auto makeAsTime(int num) -> std::array<int, 3>;
auto makeAsTime(double num) -> std::array<int, 3>;

auto checkRelation(double x, double y) -> int;
auto checkRelation(const std::array<double, 2>& p1, const std::array<double, 2>& p2) -> int;

auto roundTo(double src, int num) -> QString;

template <class T>
inline auto pairSum(const std::array<T, 2>& pair) { return pair[0] + pair[1]; }

template <class T1, class T2>
inline auto contains(const T1& key, const T2& seq)
    { return std::find(seq.begin(), seq.end(), key) != seq.end(); }

template <class T1, class T2>
inline auto contains(const T1& key, const std::map<T1, T2>& map)
    { return map.find(key) != map.end(); }

template <class T>
auto meanArith(const T& numSeq);

template <class T1, class T2>
auto isIntxn(const T1& xTypeSeqA, const T2& xTypeSeqB);

template <class T1, class T2>
auto isInclude(const T1& xTypeSeqA, const T2& xTypeSeqB);

//---------------------------------------------------------------------------//

template <class T>
auto meanArith(const T& numSeq)
{
   if (numSeq.empty())
        return *numSeq.begin() * 0;

    auto sum (*numSeq.begin());
    for (auto i (++numSeq.begin()); i != numSeq.end(); ++i)
        sum += *i;

    return sum / static_cast<int>(numSeq.size());
}


template <class T1, class T2>
auto isIntxn(const T1& xTypeSeqA, const T2& xTypeSeqB)
{
    if (xTypeSeqA.size() < xTypeSeqB.size())
    {
        for (auto& i : xTypeSeqA)
            if (std::find(xTypeSeqB.begin(), xTypeSeqB.end(), i) != xTypeSeqB.end())
                return true;
    }
    else
    {
        for (auto& i : xTypeSeqB)
            if (std::find(xTypeSeqA.begin(), xTypeSeqA.end(), i) != xTypeSeqA.end())
                return true;
    }

    return false;
}


template <class T1, class T2>
auto isInclude(const T1& xTypeSeqA, const T2& xTypeSeqB)
{
    if (xTypeSeqA.empty() || xTypeSeqB.empty())
        return false;

    if (xTypeSeqA.size() < xTypeSeqB.size())
    {
        for (auto& i : xTypeSeqA)
            if (std::find(xTypeSeqB.begin(), xTypeSeqB.end(), i) == xTypeSeqB.end())
                return false;
    }
    else
    {
        for (auto& i : xTypeSeqB)
            if (std::find(xTypeSeqA.begin(), xTypeSeqA.end(), i) == xTypeSeqA.end())
                return false;
    }

    return true;
}

} // namespace napatahti

#endif // SHARED_H
