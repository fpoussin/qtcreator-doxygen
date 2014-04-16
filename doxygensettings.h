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

#ifndef DOXYGENSETTINGS_H
#define DOXYGENSETTINGS_H

#include <coreplugin/dialogs/ioptionspage.h>
#include "doxygensettingsstruct.h"
#include "doxygensettingswidget.h"

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace DoxyPlugin {
namespace Internal {

class DoxygenSettings : public Core::IOptionsPage
{
    Q_OBJECT
public:
    DoxygenSettings();

    QWidget *createPage(QWidget *parent);
    QWidget *widget();
    void apply();
    void finish();
    static DoxygenSettings* instance();
    DoxygenSettingsStruct settings() const;
    void setSettings(const DoxygenSettingsStruct &s);

private:
    DoxygenSettingsStruct m_settings;
    static DoxygenSettings* m_doxygenSettingsInstance;
    DoxygenSettingsWidget* m_widget;
};

} // namespace Internal
} // namespace DoxyPlugin

#endif // DOXYGENSETTINGS_H
