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
    rc.style = ui->styleChooser->currentIndex();
    rc.printBrief = ui->printBriefTag->isChecked();
    rc.shortVarDoc = ui->shortVariableDocumentation->isChecked();
    rc.verbosePrinting = ui->verbosePrinting->isChecked();
    return rc;
}

void DoxygenSettingsWidget::setSettings(const DoxygenSettingsStruct &s)
{
    ui->pathChooser->setPath(s.doxygenCommand);
    ui->styleChooser->setCurrentIndex(s.style);
    ui->printBriefTag->setChecked(s.printBrief);
    ui->shortVariableDocumentation->setChecked(s.shortVarDoc);
    ui->verbosePrinting->setChecked(s.verbosePrinting);
}
