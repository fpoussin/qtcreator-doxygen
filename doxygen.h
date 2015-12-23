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

#ifndef DOXYGEN_H
#define DOXYGEN_H

#include <3rdparty/cplusplus/Symbols.h>
#include <projectexplorer/project.h>
#include <coreplugin/icore.h>
#include <coreplugin/editormanager/ieditor.h>
#include <QProgressDialog>
#include "doxygensettingsstruct.h"

namespace DoxyPlugin {
namespace Internal {

class Doxygen : public QObject
{
    Q_OBJECT
public:
    static Doxygen* instance();
    static QString getProjectRoot();

public slots:
    bool documentEntity(const DoxygenSettingsStruct &DoxySettings, Core::IEditor *editor);
    bool addFileComment(const DoxygenSettingsStruct &DoxySettings, Core::IEditor *editor);
    uint documentFile(const DoxygenSettingsStruct &DoxySettings, Core::IEditor *editor);
    uint documentProject(ProjectExplorer::Project *p, const DoxygenSettingsStruct &DoxySettings);
    uint documentSpecificProject(const DoxygenSettingsStruct &DoxySettings);
    uint documentCurrentProject(const DoxygenSettingsStruct &DoxySettings);

signals:
    void message(QString);

private slots:
    void cancelOperation(void);

private:
    Doxygen(QObject *parent = 0);
    ~Doxygen();
    void addSymbol(const CPlusPlus::Symbol* symbol, QList<const CPlusPlus::Symbol*> &symmap);

    QProgressDialog* m_projectProgress;
    QProgressDialog* m_fileProgress;
    bool m_cancel;

    static Doxygen* m_instance;
};

} // namespace Internal
} // namespace DoxyPlugin
#endif // DOXYGEN_H
