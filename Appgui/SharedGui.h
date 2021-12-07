#ifndef SHAREDGUI_H
#define SHAREDGUI_H

#include <QLocale>
#include <QMargins>

namespace napatahti {

class ConfigDialog;

class SharedGui
{
public :
    static auto getAppLocale() -> const QLocale& { return locale_; }
    static auto setAppLocale(const QLocale& locale) -> void { locale_ = locale; }
    static auto setAppLocale(const QString& name) -> void { locale_ = QLocale(name); }

    static const auto& getMargins() { return margins_; }

protected :
    static QLocale locale_;
    static QMargins margins_;

protected :
    static auto toStrUtc(int utc, bool mode) -> QString;
    static auto toStrCrd(int crd, bool mode) -> QString;

    static auto toIntUtc(const QString& utc) -> int;
    static auto toIntCrd(const QString& crd) -> int;

    friend ConfigDialog;
};

} // namespace napatahti

#endif // SHAREDGUI_H
