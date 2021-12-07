#include "SharedGui.h"

namespace napatahti {

auto SharedGui::toStrUtc(int utc, bool mode) -> QString
{
    if (utc == 0)
        return mode ? "+00:00" : "UTC";

    auto sg ('+');
    auto hh (utc / 3600);
    auto mm ((utc - 3600 * hh) / 60);

    if (utc < 0)
    {
        sg = '-';
        hh = -hh;
        mm = -mm;
    }

    return QString("%1%2:%3").arg(sg).arg(hh, 2, 10, QChar('0')).arg(mm, 2, 10, QChar('0'));
}


auto SharedGui::toStrCrd(int crd, bool mode) -> QString
{
    auto hh (crd / 3600);
    auto mm ((crd - 3600 * hh) / 60);
    auto ss (crd - 60 * mm - 3600 * hh);

    if (crd < 0)
    {
        hh = -hh;
        mm = -mm;
        ss = -ss;
    }

    return QString("%1º %2' %3\" %4")
               .arg(hh, 3 - mode, 10, QChar('0'))
               .arg(mm, 2, 10, QChar('0'))
               .arg(ss, 2, 10, QChar('0'))
               .arg("EWNS"[2 * mode + (crd < 0)]);
}


auto SharedGui::toIntUtc(const QString& utc) -> int
{
    if (utc == "UTC")
        return 0;

    return (utc[0] == '+' ? 1 : -1) *
        (3600 * utc.mid(1, 2).toInt() + 60 * utc.right(2).toInt());
}


auto SharedGui::toIntCrd(const QString& crd) -> int
{
    int k, s;

    switch (crd.back().toLatin1()) {
    case 'N' :
        k = 1;
        s = 0;
        break;
    case 'S' :
        k = -1;
        s = 0;
        break;
    case 'E' :
        k = 1;
        s = 1;
        break;
    case 'W' :
        k = -1;
        s = 1;
        break;
    default :
        return 0;
    }

    return k * (3600 * crd.left(2 + s).toInt() +
                60 * crd.mid(4 + s, 2).toInt() + crd.mid(8 + s, 2).toInt());
}


} // namespace napatahti
