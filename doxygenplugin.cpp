/**************************************************************************
**
** This file is part of Doxygen plugin for Qt Creator
**
** Copyright (c) 2009 Kevin Tanguy (kofee@kofee.org).
** Copyright (c) 2015 Fabien Poussin (fabien.poussin@gmail.com).
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
#include "doxygen.h"
#include "doxygenconstants.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>
#include <cppeditor/cppeditorconstants.h>
#include <cpptools/cpptoolsconstants.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/session.h>

#include <extensionsystem/pluginmanager.h>
#include <utils/parameteraction.h>
#include <utils/qtcassert.h>
#include <utils/synchronousprocess.h>

#include <QAction>
#include <QFileInfo>
#include <QKeySequence>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QStringList>

#include <QtPlugin>

using namespace ExtensionSystem;
using namespace DoxyPlugin;
using namespace DoxyPlugin::Internal;

// Timeout for building documentation
enum { doxygenTimeOut = 120 };

static const char CMD_ID_DOXYGEN_MAINVIEW[] = "Doxygen.MainView";
static const char CMD_ID_DOXYGEN_MENU[] = "Doxygen.Menu";
static const char CMD_ID_CREATEDOCUMENTATION[] = "Doxygen.CreateDocumentation";
static const char CMD_ID_DOCUMENTFILE[] = "Doxygen.DocumentFile";
static const char CMD_ID_DOCUMENTOPENEDPROJECT[] = "Doxygen.DocumentOpenedProject";
static const char CMD_ID_DOCUMENTACTIVEPROJECT[] = "Doxygen.DocumentActiveProject";
static const char CMD_ID_BUILDDOCUMENTATION[] = "Doxygen.BuildDocumentation";
static const char CMD_ID_DOXYFILEWIZARD[] = "Doxygen.RunWizard";

DoxygenPlugin* DoxygenPlugin::m_instance = nullptr;

DoxygenPlugin::DoxygenPlugin()
{
    m_instance = this;
}

DoxygenPlugin::~DoxygenPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    m_instance = nullptr;
    m_settings->deleteLater();
}

bool DoxygenPlugin::initialize(const QStringList& arguments, QString* errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    using namespace Constants;
    using namespace Core::Constants;
    using namespace ExtensionSystem;

    Q_UNUSED(arguments);
    Q_UNUSED(errorString);

    // settings dialog :)
    m_settings = new DoxygenSettings;
    //addAutoReleasedObject(m_settings);

    Core::ActionManager* am = Core::ActionManager::instance();
    Core::Context globalcontext(C_GLOBAL);
    //Core::Context context(CMD_ID_DOXYGEN_MAINVIEW);
    Core::ActionContainer* toolsContainer = am->actionContainer(Core::Constants::M_TOOLS);
    Core::ActionContainer* doxygenMenu = am->createMenu(Utils::Id(CMD_ID_DOXYGEN_MENU));
    doxygenMenu->menu()->setTitle(tr("&Doxygen"));
    toolsContainer->addMenu(doxygenMenu);

    // put action in our own menu in "Tools"
    // create documentation for symbol under cursor
    Core::Command* command;
    m_doxygenCreateDocumentationAction = new QAction(tr("Document current entity"), this);
    command = am->registerAction(m_doxygenCreateDocumentationAction, CMD_ID_CREATEDOCUMENTATION, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F9")));
    connect(m_doxygenCreateDocumentationAction, SIGNAL(triggered(bool)), this, SLOT(documentEntity()));
    doxygenMenu->addAction(command);
    // Don't forget the contextual menu
    Core::ActionContainer* contextMenu = am->createMenu(CppEditor::Constants::M_CONTEXT);
    contextMenu->addAction(command);

    // create documentation for a whole file
    m_doxygenDocumentFileAction = new QAction(tr("Document current file"), this);
    command = am->registerAction(m_doxygenDocumentFileAction, CMD_ID_DOCUMENTFILE, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F5")));
    connect(m_doxygenDocumentFileAction, SIGNAL(triggered(bool)), this, SLOT(documentFile()));
    doxygenMenu->addAction(command);
    /*
    // create documentation for a whole project of the currently opened file
    m_doxygenDocumentOpenedProjectAction = new QAction(tr("Document whole project of opened file"),  this);
    command = am->registerAction(m_doxygenDocumentOpenedProjectAction,
            CMD_ID_DOCUMENTOPENEDPROJECT, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F7")));
    connect(m_doxygenDocumentOpenedProjectAction, SIGNAL(triggered(bool)),
            this, SLOT(documentOpenedProject()));
    doxygenMenu->addAction(command);
*/
    // create documentation for a whole project
    m_doxygenDocumentActiveProjectAction = new QAction(tr("Document current project"), this);
    command = am->registerAction(m_doxygenDocumentActiveProjectAction,
        CMD_ID_DOCUMENTACTIVEPROJECT, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F8")));
    connect(m_doxygenDocumentActiveProjectAction, SIGNAL(triggered(bool)),
        this, SLOT(documentCurrentProject()));
    doxygenMenu->addAction(command);

    // "compile" documentation action
    m_doxygenBuildDocumentationAction = new QAction(tr("Build Doxygen Documentation"), this);
    command = am->registerAction(m_doxygenBuildDocumentationAction, CMD_ID_BUILDDOCUMENTATION, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F4")));
    connect(m_doxygenBuildDocumentationAction, SIGNAL(triggered(bool)), this, SLOT(buildDocumentation()));
    doxygenMenu->addAction(command);

    // edit Doxyfile action
    m_doxygenDoxyfileWizardAction = new QAction(tr("Edit Doxyfile"), this);
    command = am->registerAction(m_doxygenDoxyfileWizardAction, CMD_ID_DOXYFILEWIZARD, globalcontext);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F6")));
    connect(m_doxygenDoxyfileWizardAction, SIGNAL(triggered(bool)), this, SLOT(doxyfileWizard()));
    doxygenMenu->addAction(command);

    // Internal connections
    Doxygen* dox = Doxygen::instance();

    connect(dox, SIGNAL(message(QString)), this, SLOT(externalString(QString)));

    connect(this, SIGNAL(doxyDocumentEntity(DoxygenSettingsStruct, Core::IEditor*)),
        dox, SLOT(documentEntity(DoxygenSettingsStruct, Core::IEditor*)));
    connect(this, SIGNAL(doxyDocumentFile(DoxygenSettingsStruct, Core::IEditor*)),
        dox, SLOT(documentFile(DoxygenSettingsStruct, Core::IEditor*)));
    connect(this, SIGNAL(doxyDocumentCurrentProject(DoxygenSettingsStruct)),
        dox, SLOT(documentCurrentProject(DoxygenSettingsStruct)));

    // Process connection for Doxygen
    m_process = new QProcess();
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
        this, SLOT(processExited(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(readyRead()),
        this, SLOT(readProcessOutput()));

    return true;
}

void DoxygenPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag DoxygenPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

void DoxygenPlugin::documentEntity()
{
    Core::IEditor* editor = Core::EditorManager::instance()->currentEditor();
    emit doxyDocumentEntity(settings(), editor);
}

void DoxygenPlugin::documentFile()
{
    if (QMessageBox::question((QWidget*)this->parent(),
            "Doxygen", "Document current File?",
            QMessageBox::Yes, QMessageBox::No)
        == QMessageBox::Yes) {
        Core::IEditor* editor = Core::EditorManager::instance()->currentEditor();
        emit doxyDocumentFile(settings(), editor);
    }
}

void DoxygenPlugin::documentSpecificProject()
{
    Doxygen::instance()->documentSpecificProject(settings());
}

void DoxygenPlugin::documentCurrentProject()
{
    emit doxyDocumentCurrentProject(settings());
}

bool DoxygenPlugin::buildDocumentation() // TODO: refactor
{
    // TODO, allow configuration of the command
    // the default here will just run doxygen at the project root

    ProjectExplorer::Project* p = ProjectExplorer::ProjectTree::currentProject();
    if (!p) {
        QMessageBox::warning((QWidget*)parent(),
            tr("Doxygen"),
            tr("You don't have any current project."),
            QMessageBox::Close, QMessageBox::NoButton);
        return false;
    }

    QString projectRoot = Doxygen::getProjectRoot();
    if (!projectRoot.size())
        return false;

    QString doxyFile = projectRoot;
    doxyFile += settings().doxyfileFileName;
    QStringList args;

    // create default Doxyfile if it doesn't exist
    QFileInfo doxyFileInfo(doxyFile);

    if (!doxyFileInfo.exists()) {
        args << "-g" << doxyFile;
        runDoxygen(args, projectRoot);
        return true;
    }
    args << doxyFile;
    runDoxygen(args, projectRoot);
    return false;
}

void DoxygenPlugin::doxyfileWizard() // TODO: refactor
{
    // prevent a crash if user launches this command with no project opened
    // You don't need to have an editor open for that.
    ProjectExplorer::Project* p = ProjectExplorer::ProjectTree::currentProject();
    if (!p) {
        QMessageBox::warning((QWidget*)parent(),
            tr("Doxygen"),
            tr("You don't have any current project."),
            QMessageBox::Close, QMessageBox::NoButton);
        return;
    }

    QString projectRoot = p->projectDirectory().toString();
    QString executable = settings().doxywizardCommand;
    QStringList arglist(settings().doxyfileFileName);

    bool ret = QProcess::startDetached(settings().doxywizardCommand, arglist, projectRoot);

    if (!ret) {
        const QString outputText = tr("Failed to launch %1\n").arg(executable);
        externalString(outputText);
    }
}

void DoxygenPlugin::runDoxygen(const QStringList& arguments, QString workingDirectory)
{
    const QString executable = settings().doxygenCommand;
    if (executable.isEmpty()) {
        externalString(tr("No doxygen executable specified"));
        return;
    }
    const QStringList allArgs = settings().addOptions(arguments);
    const QString outputText = tr("Executing: %1 %2\n").arg(executable).arg(DoxygenSettingsStruct::formatArguments(allArgs));
    externalString(outputText);

    m_process->close();
    m_process->waitForFinished();

    m_process->setWorkingDirectory(workingDirectory);

    m_process->start(executable, allArgs);
    if (!m_process->waitForStarted(5000))
        qDebug("Could not start %s within 5 seconds", executable.toStdString().c_str());

    return;
}

void DoxygenPlugin::externalString(const QString& text)
{
    Core::MessageManager::write(text);
    Core::MessageManager::showOutputPane();
}

void DoxygenPlugin::processExited(int returnCode, QProcess::ExitStatus exitStatus)
{
    DoxygenResponse response;
    response.error = true;
    response.stdErr = QLatin1String(m_process->readAllStandardError());
    response.stdOut = QLatin1String(m_process->readAllStandardOutput());
    switch (exitStatus) {
    case QProcess::NormalExit:
        response.error = false;
        break;
    case QProcess::CrashExit:
        response.message = tr("The process terminated with exit code %1.").arg(returnCode);
        break;
    }
    if (response.error)
        externalString(response.message);
    else
        externalString(tr("Doxygen ran successfully"));
}

void DoxygenPlugin::readProcessOutput()
{
    externalString(QLatin1String(m_process->readAll()));
}

DoxygenSettingsStruct DoxygenPlugin::settings() const
{
    return m_settings->settings();
}

DoxygenPlugin* DoxygenPlugin::instance()
{
    return m_instance;
}
