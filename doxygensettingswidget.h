#ifndef DOXYGENSETTINGSWIDGET_H
#define DOXYGENSETTINGSWIDGET_H

#include <QWidget>
#include <libs/utils/pathchooser.h>
#include "doxygensettingsstruct.h"

using namespace DoxyPlugin;
using namespace DoxyPlugin::Internal;

namespace Ui {
    class DoxygenSettingsWidget;
}

class DoxygenSettingsWidget : public QWidget {
    Q_OBJECT
public:
    DoxygenSettingsWidget(QWidget *parent = 0);
    ~DoxygenSettingsWidget();
    DoxygenSettingsStruct settings() const;
    void setSettings(const DoxygenSettingsStruct &);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DoxygenSettingsWidget *ui;
};

#endif // DOXYGENSETTINGSWIDGET_H
