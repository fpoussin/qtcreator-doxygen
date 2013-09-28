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
    javaDoc = 0,
    qtDoc = 1,
    customDoc = 2
};

enum Files2Comment {
    headers = 0,
    implementations = 1,
    /*qml = 2,*/	// For future use
    /*bothqt = 3,*/ // For future use
    all = 2
};

struct DoxygenSettingsStruct
{
    DoxygenSettingsStruct();
    void fromSettings(QSettings *);
    void toSettings(QSettings *);
    // add options to command line
    QStringList addOptions(const QStringList &args) const;
    // Format arguments for log windows hiding passwords, etc.
    static QString formatArguments(const QStringList &args);

    bool equals(const DoxygenSettingsStruct &s) const;

    QString doxyfileFileName;
    QString doxygenCommand;
    QString doxywizardCommand;
    DoxygenStyle style;
    Files2Comment fcomment;
    bool printBrief;
    bool shortVarDoc;
    bool verbosePrinting;
    bool automaticReturnType;

    bool fileCommentsEnabled;
    bool fileCommentHeaders;
    bool fileCommentImpl;
    QString fileComment;


    // Custom style
    QString customBegin;
    QString customBrief;
    QString customEmptyLine;
    QString customNewLine;
    QString customEnding;
    QString customShortDoc;
    QString customShortDocEnd;

    // Support javadoc, Qt & custom style documentation
    struct DoxygenComment
    {
        QString doxBegin; //               = "/**\n";
        QString doxBrief;  //              = "* @brief";
        QString doxEmptyLine;  //          = "*\n";
        QString doxNewLine;//              " " * @";
        QString doxEnding;//               = " */";
        QString doxShortVarDoc;
        QString doxShortVarDocEnd;
    } DoxyComment;
    void setDoxygenCommentStyle(DoxygenStyle s);
};

inline bool operator==(const DoxygenSettingsStruct &p1, const DoxygenSettingsStruct &p2)
    { return p1.equals(p2); }
inline bool operator!=(const DoxygenSettingsStruct &p1, const DoxygenSettingsStruct &p2)
    { return !p1.equals(p2); }

} // namespace Internal
} // namespace DoxyPlugin
#endif // DOXYGENSETTINGSSTRUCT_H
