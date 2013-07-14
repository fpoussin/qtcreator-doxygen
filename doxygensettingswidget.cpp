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
    rc.doxyfileFileName = ui->edit_doxyfileName->text();
    rc.doxywizardCommand = ui->pathChooser_wizard->path();
    rc.style = DoxygenStyle(ui->styleChooser->currentIndex());
    rc.fcomment = Files2Comment(ui->fcommentChooser->currentIndex());
    rc.printBrief = ui->printBriefTag->isChecked();
    rc.shortVarDoc = ui->shortVariableDoc->isChecked();
    rc.verbosePrinting = ui->verbosePrinting->isChecked();
    rc.customBegin = QString(ui->edit_beginTag->text()).replace("\\n", "\n");
    rc.customBrief = QString(ui->edit_briefTag->text()).replace("\\n", "\n");
    rc.customEmptyLine = QString(ui->edit_emptyLineTag->text()).replace("\\n", "\n");
    rc.customEnding = QString(ui->edit_endTag->text()).replace("\\n", "\n");
    rc.customNewLine = QString(ui->edit_newLine->text()).replace("\\n", "\n");
    rc.customShortDoc = QString(ui->edit_shortTag->text()).replace("\\n", "\n");
    rc.customShortDocEnd = QString(ui->edit_shortTagEnd->text()).replace("\\\n", "\n");
    return rc;
}

void DoxygenSettingsWidget::setSettings(const DoxygenSettingsStruct &s)
{
    ui->pathChooser_doxygen->setPath(s.doxygenCommand);
    ui->pathChooser_wizard->setPath(s.doxywizardCommand);
    ui->edit_doxyfileName->setText(s.doxyfileFileName);
    ui->styleChooser->setCurrentIndex(s.style);
    ui->fcommentChooser->setCurrentIndex(s.fcomment);
    ui->printBriefTag->setChecked(s.printBrief);
    ui->shortVariableDoc->setChecked(s.shortVarDoc);
    ui->verbosePrinting->setChecked(s.verbosePrinting);
    ui->edit_beginTag->setText(QString(s.customBegin).replace("\n", "\\n"));
    ui->edit_briefTag->setText(QString(s.customBrief).replace("\n", "\\n"));
    ui->edit_emptyLineTag->setText(QString(s.customEmptyLine).replace("\n", "\\n"));
    ui->edit_endTag->setText(QString(s.customEnding).replace("\n", "\\n"));
    ui->edit_newLine->setText(QString(s.customNewLine).replace("\n", "\\n"));
    ui->edit_shortTag->setText(QString(s.customShortDoc).replace("\n", "\\n"));
    ui->edit_shortTagEnd->setText(QString(s.customShortDocEnd).replace("\n", "\\n"));

	updateCustomWidgetPart(s.style);
}

void DoxygenSettingsWidget::updateCustomWidgetPart(int index)
{
    ui->customCommentsGroupBox->setVisible(index == customDoc);
}
