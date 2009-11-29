#include <QStringList>

#include "doxygensettingsstruct.h"
using namespace DoxyPlugin;
using namespace DoxyPlugin::Internal;

static const char *groupC = "Doxygen";
static const char *commandKeyC = "Command";
static const char *styleKeyC = "Style";
static const char *printBriefKeyC = "PrintBrief";
static const char *allowImplementationKeyC = "AllowImplementation";

static QString defaultCommand()
{
    QString rc;
    rc = QLatin1String("doxygen");
#if defined(Q_OS_WIN32)
    rc.append(QLatin1String(".exe"));
#endif
    return rc;
}

DoxygenSettingsStruct::DoxygenSettingsStruct() :
    doxygenCommand(defaultCommand()),
    style(javaDoc),
    printBrief(true),
    allowImplementation(false)
{
}

void DoxygenSettingsStruct::fromSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String(groupC));
    doxygenCommand = settings->value(QLatin1String(commandKeyC), defaultCommand()).toString();
    style = settings->value(QLatin1String(styleKeyC), 0).toInt();
    printBrief = settings->value(QLatin1String(printBriefKeyC), 1).toBool();
    allowImplementation = settings->value(QLatin1String(allowImplementationKeyC), 0).toBool();
    settings->endGroup();
}

void DoxygenSettingsStruct::toSettings(QSettings *settings) const
{
    settings->beginGroup(QLatin1String(groupC));
    settings->setValue(QLatin1String(commandKeyC), doxygenCommand);
    settings->setValue(QLatin1String(styleKeyC), style);
    settings->setValue(QLatin1String(printBriefKeyC), printBrief);
    settings->setValue(QLatin1String(allowImplementationKeyC), allowImplementation);
    settings->endGroup();
}

bool DoxygenSettingsStruct::equals(const DoxygenSettingsStruct &s) const
{

    return
            doxygenCommand      == s.doxygenCommand
         && style               ==  s.style
         && printBrief          == s.printBrief
         && allowImplementation == s.allowImplementation;
}

QStringList DoxygenSettingsStruct::addOptions(const QStringList &args) const
{
    // TODO, look at possible doxygen args in the manual and act here...
    return args;
}
