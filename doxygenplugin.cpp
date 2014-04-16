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
#include <plugins/coreplugin/actionmanager/actioncontainer.h>
#include <plugins/coreplugin/actionmanager/command.h>
#include <plugins/coreplugin/editormanager/ieditor.h>
#include <plugins/coreplugin/editormanager/editormanager.h>
#include <plugins/coreplugin/coreconstants.h>
#include <plugins/coreplugin/messagemanager.h>
#include <plugins/projectexplorer/project.h>
#include <plugins/projectexplorer/projectexplorer.h>
#include <plugins/projectexplorer/session.h>
#include <plugins/projectexplorer/projectexplorerconstants.h>
#include <libs/utils/qtcassert.h>
#include <libs/utils/synchronousprocess.h>
#include <libs/utils/parameteraction.h>
#include <libs/extensionsystem/pluginmanager.h>

#include <QtPlugin>
#include <QAction>
#include <QMenu>
#include <QKeySequence>
#include <QStringList>
#include <QFileInfo>

using namespace ExtensionSystem;
using namespace DoxyPlugin;
using namespace DoxyPlugin::Internal;

// Timeout for building documentation
enum { doxygenTimeOut = 120000};

static const char * const CMD_ID_DOXYGEN_MAINVIEW       = "Doxygen.MainView";
static const char * const CMD_ID_DOXYGEN_MENU           = "Doxygen.Menu";
static const char * const CMD_ID_CREATEDOCUMENTATION    = "Doxygen.CreateDocumentation";
static const char * const CMD_ID_DOCUMENTFILE           = "Doxygen.DocumentFile";
static const char * const CMD_ID_DOCUMENTOPENEDPROJECT  = "Doxygen.DocumentOpenedProject";
static const char * const CMD_ID_DOCUMENTACTIVEPROJECT  = "Doxygen.DocumentActiveProject";
static const char * const CMD_ID_BUILDDOCUMENTATION     = "Doxygen.BuildDocumentation";
static const char * const CMD_ID_DOXYFILEWIZARD         = "Doxygen.RunWizard";


DoxygenPlugin* DoxygenPlugin::m_doxygenPluginInstance = 0;

bool DoxygenPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    //qDebug() << "INITIALIZE";

    using namespace Constants;
    using namespace Core::Constants;
    using namespace ExtensionSystem;

    Q_UNUSED(arguments);
    Q_UNUSED(error_message);
    m_doxygenPluginInstance = this;

    // settings dialog :)
    m_settings = new DoxygenSettings;
    addAutoReleasedObject(m_settings);

    Core::ActionManager *am = Core::ActionManager::instance();
    Core::Context globalcontext(C_GLOBAL);
    //Core::Context context(CMD_ID_DOXYGEN_MAINVIEW);
    Core::ActionContainer *toolsContainer = am->actionContainer(Core::Constants::M_TOOLS);
    Core::ActionContainer *doxygenMenu = am->createMenu(Core::Id(CMD_ID_DOXYGEN_MENU));
    doxygenMenu->menu()->setTitle(tr("&Doxygen"));
    toolsContainer->addMenu(doxygenMenu);

    // put action in our own menu in "Tools"
    // create documentation for symbol under cursor
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
    // create documentation for a whole file
    m_doxygenDocumentFileAction = new QAction(tr("Document whole file"),  this);
    command = am->registerAction(m_doxygenDocumentFileAction, CMD_ID_DOCUMENTFILE, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F5")));
    connect(m_doxygenDocumentFileAction, SIGNAL(triggered()), this, SLOT(documentFile()));
    doxygenMenu->addAction(command);

    // create documentation for a whole project of the currently opened file
    m_doxygenDocumentOpenedProjectAction = new QAction(tr("Document whole project of opened file"),  this);
    command = am->registerAction(m_doxygenDocumentOpenedProjectAction,
            CMD_ID_DOCUMENTOPENEDPROJECT, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F7")));
    connect(m_doxygenDocumentOpenedProjectAction, SIGNAL(triggered()),
            this, SLOT(documentOpenedProject()));
    doxygenMenu->addAction(command);

    // create documentation for a whole project
    m_doxygenDocumentActiveProjectAction = new QAction(tr("Document active project"),  this);
    command = am->registerAction(m_doxygenDocumentActiveProjectAction,
            CMD_ID_DOCUMENTACTIVEPROJECT, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F8")));
    connect(m_doxygenDocumentActiveProjectAction, SIGNAL(triggered()),
            this, SLOT(documentActiveProject()));
    doxygenMenu->addAction(command);


    // "compile" documentation action
    m_doxygenBuildDocumentationAction = new QAction(tr("Build Doxygen Documentation"),  this);
    command = am->registerAction(m_doxygenBuildDocumentationAction, CMD_ID_BUILDDOCUMENTATION, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F4")));
    connect(m_doxygenBuildDocumentationAction, SIGNAL(triggered()), this, SLOT(buildDocumentation()));
    doxygenMenu->addAction(command);

    // edit Doxyfile action
    m_doxygenDoxyfileWizardAction = new QAction(tr("Edit Doxyfile"),  this);
    command = am->registerAction(m_doxygenDoxyfileWizardAction, CMD_ID_DOXYFILEWIZARD, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F6")));
    connect(m_doxygenDoxyfileWizardAction, SIGNAL(triggered()), this, SLOT(doxyfileWizard()));
    doxygenMenu->addAction(command);

    return true;
}

void DoxygenPlugin::extensionsInitialized()
{
    //qDebug() << "INITIALIZED";
}

void DoxygenPlugin::shutdown()
{
    //qDebug() << "SHUTDOWN";
}

DoxygenPlugin* DoxygenPlugin::instance()
{
    //qDebug() << "ASSERT";
    QTC_ASSERT(m_doxygenPluginInstance, return m_doxygenPluginInstance);
    return m_doxygenPluginInstance;
}

void DoxygenPlugin::createDocumentation()
{
    Doxygen::instance()->createDocumentation(settings());
}

void DoxygenPlugin::documentFile()
{
    Doxygen::instance()->documentFile(settings());
}

void DoxygenPlugin::documentOpenedProject()
{
    Doxygen::instance()->documentOpenedProject(settings());
}

void DoxygenPlugin::documentActiveProject()
{
    Doxygen::instance()->documentActiveProject(settings());
}

bool DoxygenPlugin::buildDocumentation() // TODO: refactor
{
    // TODO, allow configuration of the command
    // the default here will just run doxygen at the project root
    const Core::EditorManager *editorManager = Core::EditorManager::instance();
    Core::IEditor *editor = editorManager->currentEditor();

    // prevent a crash if user launches this command with no editor opened
    if(!editor)
        return false;

    QString projectRoot = Doxygen::getProjectRoot(editor);
    if(!projectRoot.size())
        return false;

    QString doxyFile = projectRoot;
    doxyFile += settings().doxyfileFileName;
    QStringList args;

    // create default Doxyfile if it doesn't exist
    QFileInfo doxyFileInfo(doxyFile);

    if(!doxyFileInfo.exists())
    {
        args << "-g" << doxyFile;
        DoxygenResponse response = runDoxygen(args, doxygenTimeOut, true, projectRoot);
        if(!response.error)
            args.clear();
        else return !response.error;
    }
    args << doxyFile;
    DoxygenResponse response = runDoxygen(args, doxygenTimeOut, true, projectRoot);
    return !response.error;
}

void DoxygenPlugin::doxyfileWizard() // TODO: refactor
{
    //qDebug() << "DoxygenPlugin::doxyfileWizard()";
    const Core::EditorManager *editorManager = Core::EditorManager::instance();
    Core::IEditor *editor = editorManager->currentEditor();

    // prevent a crash if user launches this command with no editor opened
    if(!editor)
        return;

    QString projectRoot = Doxygen::getProjectRoot(editor);
    if(!projectRoot.size())
        return;

    QString executable = settings().doxywizardCommand;
    QStringList arglist(settings().doxyfileFileName);

    Core::MessageManager* msgManager = dynamic_cast<Core::MessageManager*>(Core::MessageManager::instance());

    bool ret = QProcess::startDetached(settings().doxywizardCommand, arglist, projectRoot);

    if(!ret)
    {
        const QString outputText = tr("Failed to launch %1\n").arg(executable);
        msgManager->showOutputPane();
        msgManager->write(outputText,Core::MessageManager::WithFocus);
    }
}


void DoxygenPlugin::setSettings(const DoxygenSettingsStruct &s)
{
    m_settings->setSettings(s);
}

DoxygenSettingsStruct DoxygenPlugin::settings() const
{
    return m_settings->settings();
}

DoxygenResponse DoxygenPlugin::runDoxygen(const QStringList &arguments, int timeOut,
                                          bool showStdOutInOutputWindow, QString workingDirectory,
                                          QTextCodec *outputCodec)
{
    const QString executable = settings().doxygenCommand;
    DoxygenResponse response;
    if(executable.isEmpty())
    {
        response.error = true;
        response.message = tr("No doxygen executable specified");
        return response;
    }
    const QStringList allArgs = settings().addOptions(arguments);

    // TODO, get a better output with printError...
    Core::MessageManager* msgManager = dynamic_cast<Core::MessageManager*>(Core::MessageManager::instance());
    msgManager->showOutputPane();

    const QString outputText = tr("Executing: %1 %2\n").arg(executable).arg(DoxygenSettingsStruct::formatArguments(allArgs));
    msgManager->write(outputText,Core::MessageManager::WithFocus);

    // Run, connect stderr to the output window
    Utils::SynchronousProcess process;
    if(!workingDirectory.isEmpty())
        process.setWorkingDirectory(workingDirectory);
    process.setTimeout(timeOut);
    process.setCodec(outputCodec);

    process.setStdErrBufferedSignalsEnabled(true);
    connect(&process, SIGNAL(stdErrBuffered(QString,bool)), this, SLOT(externalString(const QString&, bool)));

    // connect stdout to the output window if desired
    if (showStdOutInOutputWindow) {
        process.setStdOutBufferedSignalsEnabled(true);
        connect(&process, SIGNAL(stdOutBuffered(QString,bool)), this, SLOT(externalString(const QString&, bool)));
    }

    const Utils::SynchronousProcessResponse sp_resp = process.run(executable, allArgs);
    response.error = true;
    response.stdErr = sp_resp.stdErr;
    response.stdOut = sp_resp.stdOut;
    switch (sp_resp.result)
    {
    case Utils::SynchronousProcessResponse::Finished:
        response.error = false;
        break;
    case Utils::SynchronousProcessResponse::FinishedError:
        response.message = tr("The process terminated with exit code %1.").arg(sp_resp.exitCode);
        break;
    case Utils::SynchronousProcessResponse::TerminatedAbnormally:
        response.message = tr("The process terminated abnormally.");
        break;
    case Utils::SynchronousProcessResponse::StartFailed:
        response.message = tr("Could not start doxygen '%1'. Please check your settings in the preferences.").arg(executable);
        break;
    case Utils::SynchronousProcessResponse::Hang:
        response.message = tr("Doxygen did not respond within timeout limit (%1 ms).").arg(timeOut);
        break;
    }
    if (response.error)
        msgManager->write(response.message,Core::MessageManager::WithFocus);
    else msgManager->write(tr("All good mate!"),Core::MessageManager::WithFocus);

    return response;
}

void DoxygenPlugin::externalString(const QString& text, bool)
{
    Core::MessageManager::write(text);
}


Q_EXPORT_PLUGIN(DoxygenPlugin)
