#include <QKeyEvent>
#include "shared.h"
#include "Kernel/AspPage.h"
#include "AspDialog.h"
#include "ui_AspDialog.h"

namespace napatahti {

AspDialog::AspDialog(AspPage& aspPage, QWidget* root)
    : QDialog(root)
    , ui(new Ui::AspDialog)
    , colorDialog_ (new QColorDialog(this))
    , scene_ (new QGraphicsScene)
    , aspPage_ (aspPage)
{
    ui->setupUi(this);

    if (root != nullptr)
        setLocale(root->locale());

    auto font (this->font());
    auto rect (ui->styleView->rect());
    auto offset (QPointF(0, 0.5 * rect.height()));

    ui->aspEdit->setValidator(
        new QRegularExpressionValidator(QRegularExpression(
            "^(((\\d{1,2})|(1[0-7]\\d))\\.\\d*)|(180\\.0*)$"), this));
    ui->symEdit->setValidator(
        new QRegularExpressionValidator(QRegularExpression("^\\S{1,4}$"), this));

    aspFont_ = {font, {"HamburgSymbols", font.pointSize()}};

    ui->styleView->setScene(scene_);
    ui->styleView->setRenderHint(QPainter::Antialiasing);

    for (auto& i : baseColor_)
        colorDialog_->setCustomColor(i.first, i.second);

    scene_->setSceneRect(rect);
    aspLine_ = {rect.topLeft() + offset, rect.bottomRight() - offset};

    connect(ui->resetButton, &QPushButton::clicked, this, &AspDialog::onClickReset);
    connect(ui->fontButton, &QPushButton::clicked, this, &AspDialog::onClickFont);
    connect(ui->colorButton, &QPushButton::clicked, this, &AspDialog::onClickColor);
    connect(ui->saveButton, &QPushButton::clicked, this, &AspDialog::onClickSave);
    connect(ui->deleteButton, &QPushButton::clicked, this, &AspDialog::onClickDelete);
    connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(colorDialog_, &QColorDialog::colorSelected, this, &AspDialog::onSelectColor);

    connect(ui->dashBox, &QComboBox::currentIndexChanged, this, &AspDialog::onSelectDash);
}


AspDialog::~AspDialog()
{
    delete ui;
}


void AspDialog::onClickReset()
{
    if (asp_ >= 0)
    {
        aspPen_ = aspPage_.getAspPen().at(asp_);

        ui->symEdit->setText(aspPage_.getAspCatalog().at(asp_));
        ui->symEdit->setFont(aspFont_[aspPage_.getAspType().at(asp_)]);
        ui->dashBox->setCurrentIndex(aspPage_.getAspDash().at(asp_));

        ui->symEdit->setFocus();
    }
    else
    {
        aspPen_ = baseColor_.at(0);

        ui->aspEdit->setText("");
        ui->symEdit->setText("ø");
        ui->symEdit->setFont(aspFont_[1]);
        ui->dashBox->setCurrentIndex(0);

        ui->aspEdit->setFocus();
    }

    updateLine();
}


void AspDialog::onClickFont()
{
    auto ind (ui->symEdit->font().family() == aspFont_[1].family() ? 0 : 1);

    ui->symEdit->setFont(aspFont_[ind]);
}


void AspDialog::onClickColor()
{
    colorDialog_->setCurrentColor(aspPen_.color());
    colorDialog_->exec();
}


void AspDialog::onSelectColor(const QColor& color)
{
    aspPen_.setColor(color);
    updateLine();
}


void AspDialog::onSelectDash(int index)
{
    auto color (aspPen_.color());

    aspPen_ = aspPage_.makePen(color, index);
    updateLine();
}


void AspDialog::onClickSave()
{
    auto sym (ui->symEdit->text());

    if (sym.isEmpty())
    {
        reject();
        return;
    }

    if (asp_ >= 0)
        aspPage_.setAspStyle({
            asp_, sym,
            ui->dashBox->currentIndex(),
            ui->symEdit->font().family() == aspFont_[1].family() ? 1 : 0,
            aspPen_.color()});
    else
    {
        auto asp (validateAsp(ui->aspEdit->text()));

        if (asp < 0)
        {
            reject();
            return;
        }

        aspPage_.createAsp({
            asp, sym,
            ui->dashBox->currentIndex(),
            ui->symEdit->font().family() == aspFont_[1].family() ? 1 : 0,
            aspPen_.color()});
    }

    accept();
}


void AspDialog::onClickDelete()
{
    aspPage_.deleteAsp(asp_);
    accept();
}


auto AspDialog::execm(double asp) -> int
{
    if (asp >= 0)
    {
        aspPen_ = aspPage_.getAspPen().at(asp);

        setWindowIcon(QIcon(":/24x24/edit.png"));
        setWindowTitle(tr("Edit aspect"));

        ui->aspEdit->setText(QString("%1").arg(asp));
        ui->aspEdit->setEnabled(false);
        ui->symEdit->setText(aspPage_.getAspCatalog().at(asp));
        ui->symEdit->setFont(aspFont_[aspPage_.getAspType().at(asp)]);
        ui->dashBox->setCurrentIndex(aspPage_.getAspDash().at(asp));
        ui->deleteButton->setEnabled(true);

        ui->symEdit->setFocus();
    }
    else
    {
        aspPen_ = baseColor_.at(0);

        setWindowIcon(QIcon(":/24x24/new.png"));
        setWindowTitle(tr("New aspect"));

        ui->aspEdit->setText("");
        ui->aspEdit->setEnabled(true);
        ui->symEdit->setText("ø");
        ui->symEdit->setFont(aspFont_[1]);
        ui->dashBox->setCurrentIndex(0);
        ui->deleteButton->setEnabled(false);

        ui->aspEdit->setFocus();
    }

    asp_ = asp;
    updateLine();

    return exec();
}


auto AspDialog::keyPressEvent(QKeyEvent* event) -> void
{
    switch (event->key()) {
    case Qt::Key_Return :
    case Qt::Key_Enter :
        onClickSave();
        break;
    default :
        QDialog::keyPressEvent(event);
    }
}


auto AspDialog::validateAsp(const QString& text) -> double
{
    auto ok (false);
    auto asp (text.toDouble(&ok));

    if (ok && !contains(asp, aspPage_.getAspCatalog()))
        return std::round(1e9 * asp) / 1e9;

    return -1;
}


auto AspDialog::updateLine() -> void
{
    scene_->clear();
    scene_->addLine(aspLine_.translated(0, -1), aspPen_);
    scene_->addLine(aspLine_, aspPen_);
    scene_->addLine(aspLine_.translated(0, 1), aspPen_);
}

} // namespace napatahti
