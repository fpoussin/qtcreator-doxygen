#include "doxygensettingswidget.h"
#include "ui_doxygensettingswidget.h"

DoxygenSettingsWidget::DoxygenSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DoxygenSettingsWidget)
{
    ui->setupUi(this);
    ui->pathChooser->setExpectedKind(Utils::PathChooser::Command);
    ui->pathChooser->setPromptDialogTitle(tr("Doxygen Command"));
}

DoxygenSettingsWidget::~DoxygenSettingsWidget()
{
    delete ui;
}

void DoxygenSettingsWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

DoxygenSettingsStruct DoxygenSettingsWidget::settings() const
{
    DoxygenSettingsStruct rc;
    rc.doxygenCommand = ui->pathChooser->path();
    rc.style = ui->comboBox->currentIndex();
    rc.printBrief = ui->checkBox->isChecked();
    rc.allowImplementation = ui->checkBox_2->isChecked();
    return rc;
}

void DoxygenSettingsWidget::setSettings(const DoxygenSettingsStruct &s)
{
    ui->pathChooser->setPath(s.doxygenCommand);
    ui->comboBox->setCurrentIndex(s.style);
    ui->checkBox->setChecked(s.printBrief);
    ui->checkBox_2->setChecked(s.allowImplementation);
}
