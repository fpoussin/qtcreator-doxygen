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

#include "doxygensettings.h"
#include "doxygen.h"
#include "doxygenconstants.h"
#include "doxygenplugin.h"
#include <QIcon>
#include <QtCore/QCoreApplication>
#include <coreplugin/icore.h>
#include <utils/icon.h>
#include <utils/qtcassert.h>

namespace DoxyPlugin {
namespace Internal {

    DoxygenSettings::DoxygenSettings()
        : m_widget()
    {
        if (QSettings* settings = Core::ICore::instance()->settings())
            m_settings.fromSettings(settings);
        setId("A.General");
        setDisplayName(tr("Doxygen"));
        setCategory(Utils::Id::fromString(QString(Constants::DOXYGEN_SETTINGS_CATEGORY)));
        setDisplayCategory("Doxygen");
        setCategoryIcon(Utils::Icon(":/doxygen.png"));
    }

    QWidget* DoxygenSettings::widget()
    {
        if (!m_widget) {
            m_widget = new DoxygenSettingsWidget;
            m_widget->setSettings(settings());
        }
        return m_widget;
    }

    void DoxygenSettings::apply()
    {
        if (!m_widget)
            return;
        setSettings(m_widget->settings());
    }

    void DoxygenSettings::finish()
    {
    }

    DoxygenSettingsStruct DoxygenSettings::settings() const
    {
        return m_settings;
    }

    void DoxygenSettings::setSettings(const DoxygenSettingsStruct& s)
    {
        if (s != m_settings) {
            m_settings = s;
            if (QSettings* settings = Core::ICore::instance()->settings())
                m_settings.toSettings(settings);
        }
    }
}
}
