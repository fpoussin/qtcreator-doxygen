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

#include <plugins/coreplugin/icore.h>
#include <plugins/coreplugin/coreconstants.h>
#include <plugins/coreplugin/actionmanager/actionmanager.h>
#include <plugins/coreplugin/actionmanager/command.h>
#include <plugins/coreplugin/uniqueidmanager.h>
#include <plugins/coreplugin/editormanager/ieditor.h>
#include <plugins/coreplugin/editormanager/editormanager.h>
#include <plugins/coreplugin/coreconstants.h>
#include <libs/utils/qtcassert.h>
#include <libs/utils/synchronousprocess.h>
#include <libs/utils/parameteraction.h>

#include <QtPlugin>
#include <QAction>
#include <QKeySequence>

using namespace DoxyPlugin;
using namespace DoxyPlugin::Internal;

static const char * const CMD_ID_DOXYGEN_MENU        = "Doxygen.Menu";
static const char * const CMD_ID_CREATEDOCUMENTATION = "Doxygen.CreateDocumentation";

DoxygenPlugin* DoxygenPlugin::m_doxygenPluginInstance = 0;

DoxygenPlugin::DoxygenPlugin()
{
}

DoxygenPlugin::~DoxygenPlugin()
{
}
bool DoxygenPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    using namespace Constants;
    using namespace Core::Constants;
    using namespace ExtensionSystem;

    Q_UNUSED(arguments);
    Q_UNUSED(error_message);
    m_doxygenPluginInstance = this;

    // settings dialog :)
    m_settings = new DoxygenSettings(this);
    addAutoReleasedObject(m_settings);

    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *am = core->actionManager();
    QList<int> globalcontext;
    globalcontext << core->uniqueIDManager()->uniqueIdentifier(C_GLOBAL);
    Core::ActionContainer *toolsContainer = am->actionContainer(Core::Constants::M_TOOLS);
    Core::ActionContainer *doxygenMenu = am->createMenu(QLatin1String(CMD_ID_DOXYGEN_MENU));
    doxygenMenu->menu()->setTitle(tr("&Doxygen"));
    toolsContainer->addMenu(doxygenMenu);

    // put action in our own menu in "Tools"
    Core::Command *command;
    m_doxygenCreateDocumentationAction = new QAction(tr("Create Doxygen Documentation"),  this);
    command = am->registerAction(m_doxygenCreateDocumentationAction, CMD_ID_CREATEDOCUMENTATION, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F3")));
    connect(m_doxygenCreateDocumentationAction, SIGNAL(triggered()), this, SLOT(createDocumentation()));
    doxygenMenu->addAction(command);
    // Don't forget the contextual menu
    Core::ActionContainer *contextMenu= am->createMenu(CppEditor::Constants::M_CONTEXT);
    contextMenu->addAction(command);

    // TODO "compile" documentation action
    // TODO edit Doxyfile action

    return true;
}

void DoxygenPlugin::extensionsInitialized()
{
}

void DoxygenPlugin::shutdown()
{
}

DoxygenPlugin* DoxygenPlugin::instance()
{
    QTC_ASSERT(m_doxygenPluginInstance, return m_doxygenPluginInstance)
    return m_doxygenPluginInstance;
}

void DoxygenPlugin::createDocumentation()
{
    Doxygen::instance()->createDocumentation();
}

void DoxygenPlugin::setSettings(const DoxygenSettingsStruct &s)
{
    m_settings->setSettings(s);
}
DoxygenSettingsStruct  DoxygenPlugin::settings() const
{
    return m_settings->settings();
}

Q_EXPORT_PLUGIN(DoxygenPlugin)
