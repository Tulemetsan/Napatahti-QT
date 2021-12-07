#include "Kernel/Person.h"
#include "Kernel/AstroBase.h"
#include "SharedGui.h"
#include "TCounterDialog.h"
#include "ui_TCounterDialog.h"

namespace napatahti {

TCounterDialog::TCounterDialog(Person* person, QWidget* root)
    : QDialog(root)
    , ui(new Ui::TCounterDialog)
    , personAct_ (person)
    , personRes_ (new Person(*person))
    , epheRange_ (AstroBase::getEpheRange())
{
    ui->setupUi(this);
    setLocale(SharedGui::getAppLocale());

    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);

    bgStepForw_ = new QButtonGroup(this);
    bgStepForw_->addButton(ui->forwButton);
    bgStepForw_->addButton(ui->yyForwButton);
    bgStepForw_->addButton(ui->mhForwButton);
    bgStepForw_->addButton(ui->ddForwButton);
    bgStepForw_->addButton(ui->hhForwButton);
    bgStepForw_->addButton(ui->mmForwButton);
    bgStepForw_->addButton(ui->ssForwButton);

    bgStepBack_ = new QButtonGroup(this);
    bgStepBack_->addButton(ui->backButton);
    bgStepBack_->addButton(ui->yyBackButton);
    bgStepBack_->addButton(ui->mhBackButton);
    bgStepBack_->addButton(ui->ddBackButton);
    bgStepBack_->addButton(ui->hhBackButton);
    bgStepBack_->addButton(ui->mmBackButton);
    bgStepBack_->addButton(ui->ssBackButton);

    connect(bgStepForw_, &QButtonGroup::buttonClicked, this, &TCounterDialog::onClickStep);
    connect(bgStepBack_, &QButtonGroup::buttonClicked, this, &TCounterDialog::onClickStep);

    auto bgMX (new QButtonGroup(this));
    bgMX->addButton(ui->setM0Button);
    bgMX->addButton(ui->setM1Button);

    connect(bgMX, &QButtonGroup::buttonClicked, this, &TCounterDialog::onClickMX);

    auto bgSpec (new QButtonGroup(this));
    bgSpec->addButton(ui->nowButton);
    bgSpec->addButton(ui->resetButton);

    connect(bgSpec, &QButtonGroup::buttonClicked, this, &TCounterDialog::onClickSpec);
}


TCounterDialog::~TCounterDialog()
{
    delete ui;
}


void TCounterDialog::onClickStep(const QAbstractButton* button)
{
    auto  rev (bgStepForw_ == button->group() ? 1 : -1);
    auto& dateTime (personAct_->dateTime);

    if (button == ui->forwButton || button == ui->backButton)
    {
        dateTime = dateTime.addYears(rev * ui->yySpinBox->value());
        dateTime = dateTime.addMonths(rev * ui->mhSpinBox->value());
        dateTime = dateTime.addDays(rev * ui->ddSpinBox->value());
        dateTime = dateTime.addSecs(
            rev * (3600 * ui->hhSpinBox->value() +
                     60 * ui->mmSpinBox->value() +
                          ui->ssSpinBox->value()));
    }
    else if (button == ui->yyForwButton || button == ui->yyBackButton)
        dateTime = dateTime.addYears(rev * ui->yySpinBox->value());
    else if (button == ui->mhForwButton || button == ui->mhBackButton)
        dateTime = dateTime.addMonths(rev * ui->mhSpinBox->value());
    else if (button == ui->ddForwButton || button == ui->ddBackButton)
        dateTime = dateTime.addDays(rev * ui->ddSpinBox->value());
    else if (button == ui->hhForwButton || button == ui->hhBackButton)
        dateTime = dateTime.addSecs(rev * 3600 * ui->hhSpinBox->value());
    else if (button == ui->mmForwButton || button == ui->mmBackButton)
        dateTime = dateTime.addSecs(rev * 60 * ui->mmSpinBox->value());
    else if (button == ui->ssForwButton || button == ui->ssBackButton)
        dateTime = dateTime.addSecs(rev * ui->ssSpinBox->value());

    if (dateTime < epheRange_[0])
        dateTime = epheRange_[0];
    else if (dateTime > epheRange_[1])
        dateTime = epheRange_[1];

    if (dateTime == personRes_->dateTime)
    {
        personAct_->table = personRes_->table;
        personAct_->patch = personRes_->patch;
    }
    else
    {
        if (!personAct_->table.isEmpty())
            personAct_->table.clear();
        if (!personAct_->patch.isEmpty())
            personAct_->patch.clear();
    }

    emit personChanged(0, 0);
    emit personNeedSync();
}


void TCounterDialog::onClickMX(const QAbstractButton* button)
{
    auto value (button == ui->setM1Button ? 1 : 0);

    ui->ssSpinBox->setValue(value);
    ui->mmSpinBox->setValue(value);
    ui->hhSpinBox->setValue(value);
    ui->ddSpinBox->setValue(value);
    ui->mhSpinBox->setValue(value);
    ui->yySpinBox->setValue(value);
}


void TCounterDialog::onClickSpec(const QAbstractButton* button)
{
    if (button == ui->resetButton)
        *personAct_ = *personRes_;
    else
    {
        personAct_->dateTime = Person::now();
        if (!personAct_->table.isEmpty())
            personAct_->table.clear();
        if (!personAct_->patch.isEmpty())
            personAct_->patch.clear();
    }

    emit personChanged(0, 0);
    emit personNeedSync();
}


void TCounterDialog::onDialogSync()
{
    *personRes_ = *personAct_;
}


auto TCounterDialog::modeShow() -> void
{
    *personRes_ = *personAct_;
    showNormal();
    activateWindow();
}

} // namespace napatahti
