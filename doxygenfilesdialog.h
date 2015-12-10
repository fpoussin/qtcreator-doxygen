#ifndef DOXYGENFILESDIALOG_H
#define DOXYGENFILESDIALOG_H

#include <QDialog>

namespace Ui {
class DoxygenFilesDialog;
}

class DoxygenFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DoxygenFilesDialog(const QStringList &in, QWidget *parent = 0);
    ~DoxygenFilesDialog();
    uint getFilesList(QStringList *out);

private:
    Ui::DoxygenFilesDialog *ui;

private slots:
    void checkAll();
    void checkNone();
};

#endif // DOXYGENFILESDIALOG_H
