#include <QFileInfo>

namespace napatahti {

auto errorLog(const QString& error) -> void
{
    QFile file ("error.log");

    if (file.open(QFileInfo(file).size() < 5e6
            ? QIODevice::WriteOnly | QIODevice::Append : QIODevice::WriteOnly))
    {
        QTextStream out (&file);

        out << QDateTime::currentDateTime().toString(("d.MM.yyyy h:mm:ss"))
            << ' ' << error << '\n';
        file.close();
    }
}


auto makeAsTime(int num) -> std::array<int, 3>
{
    auto hh (num / 3600);
    auto mm ((num - 3600 * hh) / 60);
    auto ss (num - 60 * mm - 3600 * hh);

    if (num < 0)
        return {-hh, -mm, -ss};

    return {hh, mm, ss};
}


auto makeAsTime(double num) -> std::array<int, 3>
{
    auto hh (static_cast<int>(num));
    auto mm (static_cast<int>(60 * (num - hh)));
    auto ss (static_cast<int>(3600 * (num - hh - mm / 60.0)));

    if (num < 0)
        return {-hh, -mm, -ss};

    return {hh, mm, ss};
}


auto checkRelation(double x, double y) -> int
{// Neutral, Dominant, Weak - 0, 1, 2
    if (x < 0)
        x = -x;
    if (y < 0)
        y = -y;

    if (x == 0)
        return y == 0 ? 0 : 2;
    else if (y == 0)
        return 1;
    else if (x / y >= 1.5)
        return 1;
    else if (y / x >= 1.5)
        return 2;
    else
        return 0;
}


auto checkRelation(const std::array<double, 2>& p1, const std::array<double, 2>& p2) -> int
{
    return checkRelation(p1[0] + p1[1], p2[0] + p2[1]);
}


auto roundTo(double src, int num) -> QString
{
    auto load (1);

    for (auto i (0); i < num; ++i)
        load *= 10;

    return QString("%1").arg(std::round(load * src) / load);
}

} // namespace napatahti
