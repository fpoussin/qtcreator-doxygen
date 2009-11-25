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

#include "doxygenplugin.h"
#include "doxygenconstants.h"
#include "doxygen.h"

#include <plugins/cppeditor/cppeditorconstants.h>

#include <plugins/cpptools/cpptoolsconstants.h>

#include <plugins/coreplugin/coreconstants.h>
#include <plugins/coreplugin/actionmanager/actionmanager.h>
#include <plugins/coreplugin/actionmanager/command.h>
#include <plugins/coreplugin/icore.h>
#include <plugins/coreplugin/uniqueidmanager.h>
#include <plugins/coreplugin/editormanager/ieditor.h>
#include <plugins/coreplugin/editormanager/editormanager.h>

#include <QtPlugin>
#include <QAction>
#include <QKeySequence>

using namespace CppHelper;
using namespace CppHelper::Internal;

DoxygenPlugin::DoxygenPlugin()
{
}

DoxygenPlugin::~DoxygenPlugin()
{
}
bool DoxygenPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
    Q_UNUSED(error_message);

    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *am = core->actionManager();
    Core::ActionContainer *contextMenu= am->createMenu(CppEditor::Constants::M_CONTEXT);

    QList<int> context;
    context << core->uniqueIDManager()->uniqueIdentifier(CppEditor::Constants::C_CPPEDITOR);

    QAction* createDocumentationDoxygen = new QAction(tr("Create Doxygen Documentation"),  this);
    Core::Command *cmd = am->registerAction(createDocumentationDoxygen, Constants::CREATE_DOCUMENTATION_DOXYGEN, context);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F3")));
    connect(createDocumentationDoxygen, SIGNAL(triggered()), this, SLOT(createDocumentationDoxygen()));
    am->actionContainer(CppTools::Constants::M_TOOLS_CPP)->addAction(cmd);
    contextMenu->addAction(cmd);

    return true;

}

void DoxygenPlugin::extensionsInitialized()
{
}

void DoxygenPlugin::shutdown()
{
}

void DoxygenPlugin::createDocumentationDoxygen()
{
    Doxygen::instance()->createDocumentationDoxygen();
}

Q_EXPORT_PLUGIN(DoxygenPlugin)
