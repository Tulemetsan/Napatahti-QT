#ifndef PERSONDIALOG_H
#define PERSONDIALOG_H

#include <QDialog>
#include "SharedGui.h"

namespace Ui {
class PersonDialog;
}

namespace napatahti {

class Person;
class AtlasDialog;
class ConfigDialog;


class PersonDialog : public QDialog, protected SharedGui
{
    Q_OBJECT

public :
    PersonDialog(AtlasDialog* atlas, QWidget* root);
    ~PersonDialog();

    auto modeShow(bool mode, const Person* person) -> void;
    auto modeExec(bool mode=false, const Person* person=nullptr) -> int;

    static auto hsysTitle() -> const std::map<int, QString>& { return hsysTitle_; }

signals :
    void personChanged(const Person& person);
    void personInserted(const Person& person);
    void personUpdated(const QVariantMap& key, const Person& person);
    void personDeleted(const QVariantMap& data);

public slots :
    void onDialogSync();

public :
    static const std::array<int, 3> sexOrder;
    static const std::array<int, 8> hsysOrder;

private :
    const Person* person_;
    const Person* newPerson_ {nullptr};
    const Person* srcPerson_ {nullptr};

    AtlasDialog* atlas_;
    QWidget* root_;
    Ui::PersonDialog* ui;

    bool mode_;

    static QSize atlasSize_;
    static int   leftMgn_;
    static int   topMgn_;
    static int   topHeight_;

    static QWidget* atlasRoot_;

    static std::map<int, QString> sexTitle_;
    static std::map<int, QString> hsysTitle_;

private slots :
    void onClickAtlas();
    void onClickSaveCity();
    void onCitySelect(const QVariantMap& city);

    void onChangeMode();
    void onLoadHere();
    void onLoadNow();

    void onClickApply();
    void onClickSave();
    void onClickDelete();

private :
    auto showEvent(QShowEvent*) -> void;
    auto keyPressEvent(QKeyEvent* event) -> void;

    auto dataToForm() -> void;
    auto dataFromForm() -> Person;
    auto getPersonData(bool table) -> QVariantMap;
    auto setPerson() -> void;
    auto setWindowTop() -> void;

    static auto toDateTime(const QDateTime& dateTime, const QString& utc="UTC") -> QDateTime;

    friend ConfigDialog;
};

} // namespace napatahti

#endif // PERSONDIALOG_H
