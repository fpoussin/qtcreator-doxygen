#ifndef DOXYGEN_H
#define DOXYGEN_H

#include "doxygen_global.h"

#include <extensionsystem/iplugin.h>

namespace Doxygen {
namespace Internal {

class DoxygenPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Doxygen.json")

public:
    DoxygenPlugin();
    ~DoxygenPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

private slots:
    void triggerAction();
};

} // namespace Internal
} // namespace Doxygen

#endif // DOXYGEN_H

