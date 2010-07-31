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

#include <QStringList>
#include <QTextStream>
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

    // Support both java and qt styles
    setDoxygenCommentStyle(style);
}

void DoxygenSettingsStruct::toSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String(groupC));
    settings->setValue(QLatin1String(commandKeyC), doxygenCommand);
    settings->setValue(QLatin1String(styleKeyC), style);
    settings->setValue(QLatin1String(printBriefKeyC), printBrief);
    settings->setValue(QLatin1String(allowImplementationKeyC), allowImplementation);
    settings->endGroup();

    // Support both java and qt styles
    setDoxygenCommentStyle(style);
}

bool DoxygenSettingsStruct::equals(const DoxygenSettingsStruct &s) const
{

    return
            doxygenCommand         == s.doxygenCommand
            && style               == s.style
            && printBrief          == s.printBrief
            && allowImplementation == s.allowImplementation;
}

QStringList DoxygenSettingsStruct::addOptions(const QStringList &args) const
{
    // TODO, look at possible doxygen args in the manual and act here...
    return args;
}

QString DoxygenSettingsStruct::formatArguments(const QStringList &args)
{
    // TODO find out if it can really be useful or get rid of it
    QString rc;
    QTextStream str(&rc);
    const int size = args.size();
    for (int i = 0; i < size; i++)
    {
        const QString &arg = args.at(i);
        if (i)
            str << ' ';
        str << arg;
    }

    return rc;
}

void DoxygenSettingsStruct::setDoxygenCommentStyle(const int s)
{
    if(!s) // java
    {
        DoxyComment.doxGenericBeginNoindent = "/**\n* @brief \n*\n";
        DoxyComment.doxGenericBegin         = "    /**\n    * @brief \n    *\n";
        DoxyComment.doxBegin                = "/**\n";
        DoxyComment.doxBrief                = "* @brief \n";
        DoxyComment.doxEmptyLine            = "*\n";
        DoxyComment.doxShortBeginNoindent   = "/** ";
        DoxyComment.doxShortBegin           = "    /** ";
        DoxyComment.doxNewLine              = "* @";
        DoxyComment.doxEnding               = "*/";

    }
    else // qt
    {
        DoxyComment.doxGenericBeginNoindent = "/*!\n  \\brief \n\n";
        DoxyComment.doxGenericBegin         = "    /*!\n      \\brief \n\n";
        DoxyComment.doxBegin                = "/*!\n";
        DoxyComment.doxBrief                = "  \\brief \n";
        DoxyComment.doxEmptyLine            = " \n";
        DoxyComment.doxShortBeginNoindent   = "/*! ";
        DoxyComment.doxShortBegin           = "    /*! ";
        DoxyComment.doxNewLine              = "  \\";
        DoxyComment.doxEnding               = "*/";
    }
}
