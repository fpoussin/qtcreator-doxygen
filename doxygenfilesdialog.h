#ifndef DOXYGENFILESDIALOG_H
#define DOXYGENFILESDIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <projectexplorer/projectnodes.h>

namespace Ui {
class DoxygenFilesDialog;
}

class DoxygenFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DoxygenFilesDialog(QWidget *parent = 0);
    ~DoxygenFilesDialog();
    uint getFilesList(QStringList *out);

    void initFileTree(ProjectExplorer::Node *rootNode);

private:
    Ui::DoxygenFilesDialog *ui;

    void createLeaf(ProjectExplorer::Node *parentNode, QTreeWidgetItem *parentItem);
    uint getFilesList(QTreeWidgetItem* parent, QStringList *out, int count);
    void changeCheckState(QTreeWidgetItem* parent, Qt::CheckState state);

private slots:
    void checkAll();
    void checkNone();
    void updateChecks(QTreeWidgetItem* item, int column);
};

#endif // DOXYGENFILESDIALOG_H
