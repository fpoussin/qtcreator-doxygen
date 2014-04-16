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

#include "doxygensettings.h"
#include "doxygen.h"
#include "doxygenplugin.h"
#include "doxygenconstants.h"
#include <plugins/coreplugin/icore.h>
#include <libs/utils/qtcassert.h>
#include <QtCore/QCoreApplication>
#include <QIcon>

namespace DoxyPlugin {

DoxygenSettings* DoxygenSettings::m_doxygenSettingsInstance = 0;

DoxygenSettings::DoxygenSettings()
{
    m_doxygenSettingsInstance = this;
    if(QSettings *settings = Core::ICore::instance()->settings())
        m_settings.fromSettings(settings);
    setId("A.General");
    setDisplayName(tr("Doxygen"));
    setCategory(Core::Id::fromString(QString(Constants::DOXYGEN_SETTINGS_CATEGORY)));
    setDisplayCategory("Doxygen");
    setCategoryIcon(":/doxygen.png");
}

QWidget* DoxygenSettings::createPage(QWidget *parent)
{
    m_widget = new DoxygenSettingsWidget(parent);
    m_widget->setSettings(settings());
    return m_widget;
}

QWidget* DoxygenSettings::widget()
{
    m_widget = new DoxygenSettingsWidget;
    m_widget->setSettings(settings());
    return m_widget;
}

void DoxygenSettings::apply()
{
    DoxygenPlugin::instance()->setSettings(m_widget->settings());
}

void DoxygenSettings::finish()
{
 delete m_widget;
}

DoxygenSettingsStruct DoxygenSettings::settings() const
{
    return m_settings;
}

void DoxygenSettings::setSettings(const DoxygenSettingsStruct &s)
{
    if (s != m_settings)
    {
        m_settings = s;
        if(QSettings *settings = Core::ICore::instance()->settings())
            m_settings.toSettings(settings);
    }
}

DoxygenSettings* DoxygenSettings::instance()
{
    QTC_ASSERT(m_doxygenSettingsInstance, return m_doxygenSettingsInstance);
    return m_doxygenSettingsInstance;
}
}
