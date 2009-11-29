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

} // Doxygen
} // Internal
#endif // DOXYGENSETTINGSSTRUCT_H
