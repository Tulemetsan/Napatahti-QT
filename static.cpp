#include <QApplication>
#include <QTranslator>
#include <QDir>
#include "Kernel/AspTable.h"
#include "Appgui/AspPageDialog.h"
#include "Appgui/AtlasDialog.h"
#include "Appgui/ConfigDialog.h"

namespace napatahti {

auto staticInit(QApplication& app) -> void
{
    auto isLoad (ConfigDialog::loadBase());

    QString locDir ("locale");
    QString aspDir ("aspects");
    QString locName (isLoad ? ConfigDialog::locale_.name() : "en_US");

    if (!QDir(locDir).exists())
        QDir().mkdir(locDir);
    if (!QDir(locDir + '/' + locName).exists())
        QDir(locDir).mkdir(locName);
    if (!QDir(locDir + '/' + locName + '/' + aspDir).exists())
        QDir(locDir + '/' + locName).mkdir(aspDir);

    AspTable::path_ = locDir + '/' + locName + '/' + aspDir + '/';
    AspPageDialog::sampPath_ = AspTable::path_;
    AtlasDialog::dbaseName_ = locDir + '/' + locName + '/' + "atlas.sqlite";

    auto qTranslator (new QTranslator);
    if (qTranslator->load(locDir + '/' + "Napatahti_" + locName))
        app.installTranslator(qTranslator);

    if (isLoad)
        ConfigDialog::loadProFile();
    else
    {
        ConfigDialog::restore(RestoreMode::Full);
        ConfigDialog::dumpBase();
    }

    ConfigDialog::loadExt();
}

} // namespace napatahti
