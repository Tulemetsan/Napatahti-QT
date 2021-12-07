#include <QKeyEvent>
#include "Kernel/Person.h"
#include "Kernel/AstroBase.h"
#include "CityDialog.h"
#include "AtlasDialog.h"
#include "PersonDialog.h"
#include "ui_PersonDialog.h"

namespace napatahti {

PersonDialog::PersonDialog(AtlasDialog* atlas, QWidget* root)
    : QDialog (root)
    , atlas_ (atlas)
    , root_ (root)
    , ui (new Ui::PersonDialog)
{
    ui->setupUi(this);
    setLocale(locale_);

    auto& epheRange (AstroBase::getEpheRange());
    ui->dateTimeEdit->setDateTimeRange(epheRange[0], epheRange[1]);

    ui->latEdit->setValidator(new QRegularExpressionValidator(CityDialog::latReg, this));
    ui->lonEdit->setValidator(new QRegularExpressionValidator(CityDialog::lonReg, this));
    ui->utcEdit->setValidator(new QRegularExpressionValidator(CityDialog::utcReg, this));

    for (auto i : sexOrder)
        ui->sexBox->addItem(sexTitle_.at(i), i);

    for (auto i : hsysOrder)
        ui->hsysBox->addItem(hsysTitle_.at(i), i);

    connect(ui->atlasButton, &QPushButton::clicked, this, &PersonDialog::onClickAtlas);
    connect(ui->saveCityButton, &QPushButton::clicked, this, &PersonDialog::onClickSaveCity);

    connect(atlas_, &AtlasDialog::citySelected, this, &PersonDialog::onCitySelect);

    connect(ui->modeButton, &QPushButton::clicked, this, &PersonDialog::onChangeMode);
    connect(ui->redoButton, &QPushButton::clicked, this, &PersonDialog::dataToForm);
    connect(ui->nowButton, &QPushButton::clicked,  this, &PersonDialog::onLoadNow);
    connect(ui->hereButton, &QPushButton::clicked, this, &PersonDialog::onLoadHere);

    connect(ui->applyButton, &QPushButton::clicked, this, &PersonDialog::onClickApply);
    connect(ui->saveButton, &QPushButton::clicked, this, &PersonDialog::onClickSave);
    connect(ui->deleteButton, &QPushButton::clicked, this, &PersonDialog::onClickDelete);
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);
}


PersonDialog::~PersonDialog()
{
    delete ui;
}


void PersonDialog::onDialogSync()
{
    if (isVisible() && mode_)
    {
        dataToForm();
        setWindowTop();
    }
}


void PersonDialog::onClickAtlas()
{
    auto beg (ui->atlasButton->geometry().bottomRight());
    auto end (screen()->geometry().bottomRight());

    beg.ry() += topHeight_;
    end.rx() -= margins_.right() + 1;
    end.ry() -= margins_.bottom() + 1;

    QRect geo (mapToGlobal(beg), atlasSize_);

    if (geo.right() > end.x())
        geo.moveLeft(end.x() - geo.width());
    if (geo.bottom() > end.y())
        geo.moveTop(end.y() - geo.height());

    atlasRoot_ = this;
    atlas_->setGeometry(geo);
    atlas_->exec();
}


void PersonDialog::onClickSaveCity()
{
    auto utc (ui->utcEdit->text());

    atlas_->onUpdateBase({
        {"title", ui->cityTitleEdit->text()},
        {"utc", (utc == "+00:00" || utc == "-00:00") ? "UTC" : utc},
        {"lat", ui->latEdit->text()},
        {"lon", ui->lonEdit->text()},
        {"mode", false}
    });
}


void PersonDialog::onCitySelect(const QVariantMap& city)
{
    if (atlasRoot_ == this)
    {
        ui->cityTitleEdit->setText(city["title"].toString());
        ui->utcEdit->setText(city["utc"].toString());
        ui->latEdit->setText(city["lat"].toString());
        ui->lonEdit->setText(city["lon"].toString());
    }
}


void PersonDialog::onChangeMode()
{
    if (srcPerson_ == nullptr)
        return;

    mode_ ^= 1;

    setPerson();
    dataToForm();
    setWindowTop();
}


void PersonDialog::onLoadHere()
{
    auto&& [location, utc, lat, lon] (Person::here());

    ui->cityTitleEdit->setText(location);
    ui->latEdit->setText(toStrCrd(lat, true));
    ui->lonEdit->setText(toStrCrd(lon, false));
    ui->utcEdit->setText(toStrUtc(utc, true));
}


void PersonDialog::onLoadNow()
{
    auto dateTime (Person::now());

    ui->dateTimeEdit->setDateTime(
        {dateTime.date(), dateTime.time(), Qt::OffsetFromUTC, 0});
}


void PersonDialog::onClickApply()
{
    auto person (dataFromForm());

    if (person != *person_)
        person.table.clear();

    emit personChanged(person);
    accept();
}


void PersonDialog::onClickSave()
{
    if (person_->table.isEmpty())
        emit personInserted(dataFromForm());
    else
        emit personUpdated(getPersonData(false), dataFromForm());

    accept();
}


void PersonDialog::onClickDelete()
{
    emit personDeleted(getPersonData(true));
    accept();
}


auto PersonDialog::modeShow(bool mode, const Person* person) -> void
{
    if (isVisible())
    {
        mode_ = mode ^ 1;
        onChangeMode();
        activateWindow();
        return;
    }

    mode_ = mode;
    srcPerson_ = person;

    setPerson();
    show();
}


auto PersonDialog::modeExec(bool mode, const Person* person) -> int
{
    mode_ = (person == nullptr ? false : mode);
    srcPerson_ = person;

    setPerson();

    return exec();
}


auto PersonDialog::showEvent(QShowEvent*) -> void
{
    if (topHeight_ < 0)
        topHeight_ = frameGeometry().height() - geometry().height();

    setWindowTop();
    dataToForm();
    ui->nameEdit->setFocus();

    auto beg (root_->geometry().topLeft());
    auto end (screen()->geometry().bottomRight());
    auto size (geometry().size());
    auto scale (root_->height() / 1008.0);

    if (isModal())
    {
        beg.rx() += (root_->width() - size.width()) / 2;
        beg.ry() += (root_->height() - size.height()) / 2;
    }
    else
    {
        beg.rx() += scale * leftMgn_;
        beg.ry() += scale * topMgn_;
    }
    end.rx() -= margins_.right() + 1;
    end.ry() -= margins_.bottom() + 1;

    QRect geo {beg, size};

    if (geo.right() > end.x())
        geo.moveLeft(end.x() - geo.width());
    if (geo.bottom() > end.y())
        geo.moveTop(end.y() - geo.height());

    setGeometry(geo);
}


auto PersonDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_Enter :
    case Qt::Key_Return :
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}


auto PersonDialog::dataToForm() -> void
{
    ui->nameEdit->setText(person_->name);
    ui->dateTimeEdit->setDateTime(toDateTime(person_->dateTime));
    ui->utcEdit->setText(toStrUtc(person_->dateTime.offsetFromUtc(), true));
    ui->sexBox->setCurrentIndex(ui->sexBox->findData(person_->sex));
    ui->cityTitleEdit->setText(person_->location);
    ui->latEdit->setText(toStrCrd(person_->lat, true));
    ui->lonEdit->setText(toStrCrd(person_->lon, false));
    ui->hsysBox->setCurrentIndex(ui->hsysBox->findData(person_->hsys));
}


auto PersonDialog::dataFromForm() -> Person
{
    auto dateTime (toDateTime(ui->dateTimeEdit->dateTime(), ui->utcEdit->text()));

    return Person {
        ui->nameEdit->text(),
        dateTime,
        ui->sexBox->currentData().toInt(),
        ui->cityTitleEdit->text(),
        toIntCrd(ui->latEdit->text()),
        toIntCrd(ui->lonEdit->text()),
        ui->hsysBox->currentData().toInt(),
        dateTime == person_->dateTime ? person_->patch : "",
        person_->table
    };
}


auto PersonDialog::getPersonData(bool table) -> QVariantMap
{
    QVariantMap data {
        {"name", person_->name},
        {"dateTime", person_->dateTime},
        {"sex", person_->sex},
    };

    if (table)
        data["table"] = person_->table;

    return data;
}


auto PersonDialog::setPerson() -> void
{
    if (!mode_)
    {
        if (newPerson_ != nullptr)
            delete newPerson_;

        newPerson_ = new Person;
    }

    person_ = mode_ ? srcPerson_ : newPerson_;
}


auto PersonDialog::setWindowTop() -> void
{
    if (mode_)
    {
        auto mask (tr("Edit") + " <" +
            (person_->table.isEmpty() ? "%1%2>" : person_->table + " :: %1%2>"));

        setWindowIcon(QIcon(mode_ ? ":/24x24/edit.png" : ":/24x24/new.png"));
        setWindowTitle(mask.arg(person_->name).arg(person_->patch.isEmpty() ? "" : " &"));
    }
    else
    {
        setWindowIcon(QIcon(":/24x24/new.png"));
        setWindowTitle(tr("New"));
    }
}


auto PersonDialog::toDateTime(const QDateTime& dateTime, const QString& utc) -> QDateTime
{
    return {dateTime.date(), dateTime.time(), Qt::OffsetFromUTC, toIntUtc(utc)};
}

} // namespace napatahti
