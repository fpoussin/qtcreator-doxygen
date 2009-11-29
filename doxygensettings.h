#ifndef DOXYGENSETTINGS_H
#define DOXYGENSETTINGS_H

#include <coreplugin/dialogs/ioptionspage.h>
#include "doxygensettingsstruct.h"
#include "doxygensettingswidget.h"

//using namespace DoxyPlugin;
//using namespace DoxyPlugin::Internal;

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace DoxyPlugin {
namespace Internal {

class DoxygenSettings : public Core::IOptionsPage
{
    Q_OBJECT
public:
    DoxygenSettings(QObject* parent = 0);
    ~DoxygenSettings();
    // IOptionsPage implementation
    QString id() const;
    QString trName() const;
    QString category() const;
    QString trCategory() const;
    QWidget *createPage(QWidget *parent);
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
} // namespace CppHelper

#endif // DOXYGENSETTINGS_H
