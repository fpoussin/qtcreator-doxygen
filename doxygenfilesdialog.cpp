#include "doxygenfilesdialog.h"
#include "ui_doxygenfilesdialog.h"

#include <QListWidget>
#include <QListWidgetItem>

DoxygenFilesDialog::DoxygenFilesDialog(const QStringList &in, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DoxygenFilesDialog)
{
    ui->setupUi(this);

    QStringListIterator it(in);

    while (it.hasNext())
    {
        QListWidgetItem *listItem = new QListWidgetItem(it.next());
        listItem->setCheckState(Qt::Checked);
        ui->listWidget->addItem(listItem);
    }

    connect(ui->b_all, SIGNAL(clicked(bool)), this, SLOT(checkAll()));
    connect(ui->b_none, SIGNAL(clicked(bool)), this, SLOT(checkNone()));
    connect(ui->b_cancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(ui->b_ok, SIGNAL(clicked(bool)), this, SLOT(accept()));

    this->exec();
}

DoxygenFilesDialog::~DoxygenFilesDialog()
{
    delete ui;
}

uint DoxygenFilesDialog::getFilesList(QStringList *out)
{
    uint count = 0;
    for (int i=0; i< ui->listWidget->count(); i++)
    {
        if (ui->listWidget->item(i)->checkState() == Qt::Checked)
        {
            out->append(ui->listWidget->item(i)->text());
            count++;
        }
    }
    return count;
}

void DoxygenFilesDialog::checkAll()
{
    for (int i=0; i< ui->listWidget->count(); i++)
    {
        if (ui->listWidget->item(i)->checkState() == Qt::Unchecked)
            ui->listWidget->item(i)->setCheckState(Qt::Checked);
    }
}

void DoxygenFilesDialog::checkNone()
{
    for (int i=0; i< ui->listWidget->count(); i++)
    {
        if (ui->listWidget->item(i)->checkState() == Qt::Checked)
            ui->listWidget->item(i)->setCheckState(Qt::Unchecked);
    }
}
