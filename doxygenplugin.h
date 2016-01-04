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

#ifndef DOXYGENPLUGIN_H
#define DOXYGENPLUGIN_H

#include "doxygen_global.h"

#include <coreplugin/icore.h>
#include <coreplugin/editormanager/ieditor.h>
#include <extensionsystem/iplugin.h>
#include "doxygensettings.h"
#include "doxygensettingsstruct.h"
#include <QProcess>

namespace DoxyPlugin {
namespace Internal {

struct DoxygenResponse
{
    DoxygenResponse() : error(false) {}
    bool error;
    QString stdOut;
    QString stdErr;
    QString message;
};

class DoxygenPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Doxygen.json")

public:
    DoxygenPlugin();
    ~DoxygenPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

    void setSettings(const DoxygenSettingsStruct &s);
    DoxygenSettingsStruct settings() const;
    void runDoxygen(const QStringList &arguments,
                               QString workingDirectory = QString());

    static DoxygenPlugin* instance();

private:
    static DoxygenPlugin* m_instance;
    DoxygenSettings* m_settings;
    QAction* m_doxygenCreateDocumentationAction;
    QAction* m_doxygenDocumentFileAction;
    QAction* m_doxygenDocumentOpenedProjectAction;
    QAction* m_doxygenDocumentActiveProjectAction;
    QAction* m_doxygenBuildDocumentationAction;
    QAction* m_doxygenDoxyfileWizardAction;
    QProcess m_process;

signals:
    void doxyDocumentEntity(const DoxygenSettingsStruct &DoxySettings, Core::IEditor *editor);
    void doxyDocumentFile(const DoxygenSettingsStruct &DoxySettings, Core::IEditor *editor);
    void doxyDocumentSpecificProject(const DoxygenSettingsStruct &DoxySettings);
    void doxyDocumentCurrentProject(const DoxygenSettingsStruct &DoxySettingss);

private slots:
    void documentEntity();
    void documentFile();
    void documentSpecificProject();
    void documentCurrentProject();

    bool buildDocumentation();
    void doxyfileWizard();
    void externalString(const QString&);

    void processExited(int returnCode, QProcess::ExitStatus exitStatus);
    void readProcessOutput(void);
};

} // namespace Internal
} // namespace Doxygen

#endif // DOXYGENPLUGIN_H

