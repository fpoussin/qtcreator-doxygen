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

#ifndef DOXYGENPLUGIN_H
#define DOXYGENPLUGIN_H

#include <libs/extensionsystem/iplugin.h>
#include "doxygensettings.h"
#include "doxygensettingsstruct.h"

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


class DoxygenPlugin  : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kofee.dev.Doxygen" FILE "Doxygen.json")

public:
    bool initialize(const QStringList &arguments, QString *error_message);
    void extensionsInitialized();
    void shutdown();
    static DoxygenPlugin* instance();
    void setSettings(const DoxygenSettingsStruct &s);
    DoxygenSettingsStruct settings() const;
    DoxygenResponse runDoxygen(const QStringList &arguments, int timeOut,
                               bool showStdOutInOutputWindow, QString workingDirectory = QString(), QTextCodec *outputCodec = 0);

private:
    static DoxygenPlugin *m_doxygenPluginInstance;
    DoxygenSettings* m_settings;
    QAction* m_doxygenCreateDocumentationAction;
    QAction* m_doxygenDocumentFileAction;
    QAction* m_doxygenDocumentOpenedProjectAction;
    QAction* m_doxygenDocumentActiveProjectAction;
    QAction* m_doxygenBuildDocumentationAction;
    QAction* m_doxygenDoxyfileWizardAction;


private slots:
    void createDocumentation();
    void documentFile();
    void documentOpenedProject();
    void documentActiveProject();
    bool buildDocumentation();
    void doxyfileWizard();
    void externalString(const QString&, bool);
};

} // namespace Internal
} // namespace DoxyPlugin

#endif // DOXYGENPLUGIN_H
