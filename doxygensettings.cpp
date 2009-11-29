#include "doxygensettings.h"
#include "doxygen.h"
#include "doxygenplugin.h"
#include <plugins/coreplugin/icore.h>
#include <libs/utils/qtcassert.h>


DoxygenSettings* DoxygenSettings::m_doxygenSettingsInstance = 0;

DoxygenSettings::DoxygenSettings(QObject* parent) : IOptionsPage(parent)
{
    m_doxygenSettingsInstance = this;
    if(QSettings *settings = Core::ICore::instance()->settings())
        m_settings.fromSettings(settings);
}

DoxygenSettings::~DoxygenSettings()
{
}

// implementation of interface Core::IOptionsPage
QString DoxygenSettings::id() const
{
    return "DoxygenPlugin";
}

QString DoxygenSettings::trName() const
{
    return tr("Doxygen");
}

QString DoxygenSettings::category() const
{
    return "Documentation";
}

QString DoxygenSettings::trCategory() const
{
    return tr("Documentation");
}

QWidget* DoxygenSettings::createPage(QWidget *parent)
{
    m_widget = new DoxygenSettingsWidget(parent);
    m_widget->setSettings(settings());
    return m_widget;
}

void DoxygenSettings::apply()
{
    DoxygenPlugin::instance()->setSettings(m_widget->settings());
}

void DoxygenSettings::finish()
{
// TODO
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
    QTC_ASSERT(m_doxygenSettingsInstance, return m_doxygenSettingsInstance)
    return m_doxygenSettingsInstance;
}
