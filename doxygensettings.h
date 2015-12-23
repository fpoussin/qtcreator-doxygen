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

    QWidget *widget();
    void apply();
    void finish();
    DoxygenSettingsStruct settings() const;
    void setSettings(const DoxygenSettingsStruct &s);

private:
    DoxygenSettingsStruct m_settings;
    DoxygenSettingsWidget* m_widget;
};

} // namespace Internal
} // namespace DoxyPlugin

#endif // DOXYGENSETTINGS_H
