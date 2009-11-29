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

#ifndef DOXYGENSETTINGSSTRUCT_H
#define DOXYGENSETTINGSSTRUCT_H

#include <QSettings>

namespace DoxyPlugin {
namespace Internal {

enum DoxygenStyle {
    javaDoc,
    qtDoc
};

struct DoxygenSettingsStruct
{
    DoxygenSettingsStruct();
    void fromSettings(QSettings *);
    void toSettings(QSettings *) const;
    // add options to command line
    QStringList addOptions(const QStringList &args) const;
    // Format arguments for log windows hiding passwords, etc.
    static QString formatArguments(const QStringList &args);

    bool equals(const DoxygenSettingsStruct &s) const;

    QString doxygenCommand;
    unsigned int style;
    bool printBrief;
    bool allowImplementation;
};

inline bool operator==(const DoxygenSettingsStruct &p1, const DoxygenSettingsStruct &p2)
    { return p1.equals(p2); }
inline bool operator!=(const DoxygenSettingsStruct &p1, const DoxygenSettingsStruct &p2)
    { return !p1.equals(p2); }

} // namespace Internal
} // namespace DoxyPlugin
#endif // DOXYGENSETTINGSSTRUCT_H
