/**************************************************************************
**
** This file is part of Doxygen plugin for Qt Creator
**
** Copyright (c) 2009 Kevin Tanguy (kofee@kofee.org).
**
** This plugin is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as
** published by the Free Software Foundation, either version 2.1
** of the License, or (at your option) any later version.
**
** This plugin is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Doxygen Plugin. If not, see <http://www.gnu.org/licenses/>.
**/

#include "doxygensettingswidget.h"
#include "ui_doxygensettingswidget.h"

DoxygenSettingsWidget::DoxygenSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DoxygenSettingsWidget)
{
    ui->setupUi(this);
    ui->pathChooser_doxygen->setExpectedKind(Utils::PathChooser::Command);
    ui->pathChooser_doxygen->setPromptDialogTitle(tr("Doxygen Command"));
    ui->pathChooser_wizard->setExpectedKind(Utils::PathChooser::Command);
    ui->pathChooser_wizard->setPromptDialogTitle(tr("DoxyWizard Command"));
    // Why do I *have* to call processEvents() to get my QComboBox display the right value?!
    qApp->processEvents();
    connect(ui->styleChooser, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCustomWidgetPart(int)));
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
    rc.doxygenCommand = ui->pathChooser_doxygen->path();
    rc.doxywizardCommand = ui->pathChooser_wizard->path();
    rc.style = ui->styleChooser->currentIndex();
    rc.printBrief = ui->printBriefTag->isChecked();
    rc.shortVarDoc = ui->shortVariableDocumentation->isChecked();
    rc.verbosePrinting = ui->verbosePrinting->isChecked();
    rc.customBegin = QString(ui->beginTagEdit->text()).replace("\\n", "\n");
    rc.customBrief = QString(ui->briefTagEdit->text()).replace("\\n", "\n");
    rc.customEmptyLine = QString(ui->emptyLineTagEdit->text()).replace("\\n", "\n");
    rc.customEnding = QString(ui->endTagEdit->text()).replace("\\n", "\n");
    rc.customNewLine = QString(ui->newLineEdit->text()).replace("\\n", "\n");
    rc.customShortDoc = QString(ui->shortTagEdit->text()).replace("\\n", "\n");
    rc.customShortDocEnd = QString(ui->shortTagEndEdit->text()).replace("\\\n", "\n");
    return rc;
}

void DoxygenSettingsWidget::setSettings(const DoxygenSettingsStruct &s)
{
    ui->pathChooser_doxygen->setPath(s.doxygenCommand);
    ui->pathChooser_wizard->setPath(s.doxywizardCommand);
    ui->styleChooser->setCurrentIndex(s.style);
    ui->printBriefTag->setChecked(s.printBrief);
    ui->shortVariableDocumentation->setChecked(s.shortVarDoc);
    ui->verbosePrinting->setChecked(s.verbosePrinting);
    ui->beginTagEdit->setText(QString(s.customBegin).replace("\n", "\\n"));
    ui->briefTagEdit->setText(QString(s.customBrief).replace("\n", "\\n"));
    ui->emptyLineTagEdit->setText(QString(s.customEmptyLine).replace("\n", "\\n"));
    ui->endTagEdit->setText(QString(s.customEnding).replace("\n", "\\n"));
    ui->newLineEdit->setText(QString(s.customNewLine).replace("\n", "\\n"));
    ui->shortTagEdit->setText(QString(s.customShortDoc).replace("\n", "\\n"));
    ui->shortTagEndEdit->setText(QString(s.customShortDocEnd).replace("\n", "\\n"));
}

void DoxygenSettingsWidget::updateCustomWidgetPart(int index)
{
    if(index == 2)
    {
        ui->beginTagEdit->setEnabled(true);
        ui->briefTagEdit->setEnabled(true);
        ui->emptyLineTagEdit->setEnabled(true);
        ui->endTagEdit->setEnabled(true);
        ui->newLineEdit->setEnabled(true);
        ui->shortTagEdit->setEnabled(true);
        ui->shortTagEndEdit->setEnabled(true);
    }
    else
    {
        ui->beginTagEdit->setEnabled(false);
        ui->briefTagEdit->setEnabled(false);
        ui->emptyLineTagEdit->setEnabled(false);
        ui->endTagEdit->setEnabled(false);
        ui->newLineEdit->setEnabled(false);
        ui->shortTagEdit->setEnabled(false);
        ui->shortTagEndEdit->setEnabled(false);
    }
}
