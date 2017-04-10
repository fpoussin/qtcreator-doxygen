#include "doxygenfilesdialog.h"
#include "ui_doxygenfilesdialog.h"

DoxygenFilesDialog::DoxygenFilesDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DoxygenFilesDialog)
{
	ui->setupUi(this);

	connect(ui->b_all, SIGNAL(clicked(bool)), this, SLOT(checkAll()));
	connect(ui->b_none, SIGNAL(clicked(bool)), this, SLOT(checkNone()));
	connect(ui->b_cancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
	connect(ui->b_ok, SIGNAL(clicked(bool)), this, SLOT(accept()));

	connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(updateChecks(QTreeWidgetItem *, int)));
}

DoxygenFilesDialog::~DoxygenFilesDialog()
{
	delete ui;
}

void DoxygenFilesDialog::initFileTree(ProjectExplorer::Node *rootNode)
{
	createLeaf(rootNode, new QTreeWidgetItem(ui->treeWidget, QStringList(rootNode->displayName())));
	ui->treeWidget->expandAll();
	ui->treeWidget->setItemsExpandable(false);
}

void DoxygenFilesDialog::createLeaf(ProjectExplorer::Node *parentNode, QTreeWidgetItem *parentItem)
{
	parentItem->setCheckState(0, Qt::Checked);

	if (parentNode->asFolderNode())
	{
		ProjectExplorer::FolderNode *currentFolderNode = parentNode->asFolderNode();

        parentItem->setIcon(0, currentFolderNode->icon());

		foreach(ProjectExplorer::FileNode *fileNode, currentFolderNode->fileNodes())
		{
			QString fileName = fileNode->filePath().toString();

			if ((QRegExp("\\.(h|hpp|c|cpp)$").indexIn(fileName)) != -1)
			{
				QTreeWidgetItem *child = new QTreeWidgetItem(parentItem, QStringList(fileName));
				child->setCheckState(0, Qt::Checked);
				parentItem->addChild(child);
			}
		}

		foreach(ProjectExplorer::FolderNode * folderNode, currentFolderNode->subFolderNodes())
		{
			createLeaf(folderNode, new QTreeWidgetItem(parentItem, QStringList(folderNode->displayName())));
		}
	}
	else if (parentNode->asProjectNode())
	{
		ProjectExplorer::ProjectNode *currentProjectNode = parentNode->asProjectNode();

        parentItem->setIcon(0, currentProjectNode->icon());

		foreach(ProjectExplorer::FileNode *fileNode, currentProjectNode->fileNodes())
		{
			QString fileName = fileNode->filePath().toString();

			if ((QRegExp("\\.(h|hpp|c|cpp)$").indexIn(fileName)) != -1)
			{
				QTreeWidgetItem *child = new QTreeWidgetItem(parentItem, QStringList(fileName));
				child->setCheckState(0, Qt::Checked);
				parentItem->addChild(child);
			}
		}

		foreach(ProjectExplorer::FolderNode *folderNode, currentProjectNode->subFolderNodes())
		{
			createLeaf(folderNode, new QTreeWidgetItem(parentItem, QStringList(folderNode->displayName())));
		}

		foreach(ProjectExplorer::ProjectNode *projectNode, currentProjectNode->subProjectNodes())
		{
			createLeaf(projectNode, new QTreeWidgetItem(parentItem, QStringList(projectNode->displayName())));
		}
	}

	if (parentItem->childCount() == 0)
	{
		delete(parentItem);
	}
}

uint DoxygenFilesDialog::getFilesList(QTreeWidgetItem *parent, QStringList *out, int count)
{
	if (!parent->childCount())
	{
		if (parent->checkState(0) == Qt::Checked)
		{
			out->append(parent->text(0));
			count++;
		}
	}
	else
	{
		for (int i = 0; i < parent->childCount(); i++)
		{
			getFilesList(parent->child(i), out, count);
		}
	}

	return count;
}

uint DoxygenFilesDialog::getFilesList(QStringList *out)
{
	uint count = 0;
	for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
	{
		getFilesList(ui->treeWidget->topLevelItem(i), out, count);
	}
	return count;
}

void DoxygenFilesDialog::changeCheckState(QTreeWidgetItem *parent, Qt::CheckState state)
{
	if (!parent->childCount())
	{
		if (parent->checkState(0) != state)
		{
			parent->setCheckState(0, state);
		}
	}
	else
	{
		for (int i = 0; i < parent->childCount(); i++)
		{
			changeCheckState(parent->child(i), state);
		}
	}
}

void DoxygenFilesDialog::checkAll()
{
	for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
	{
		changeCheckState( ui->treeWidget->topLevelItem(i), Qt::Checked);
	}
}

void DoxygenFilesDialog::checkNone()
{
	for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
	{
		changeCheckState(ui->treeWidget->topLevelItem(i), Qt::Unchecked);
	}
}

void DoxygenFilesDialog::updateChecks(QTreeWidgetItem *item, int column)
{
	bool diff = false;

	if (column != 0 && column != -1)
	{
		return;
	}

	if (item->childCount() != 0 && item->checkState(0) != Qt::PartiallyChecked && column != -1)
	{
		Qt::CheckState checkState = item->checkState(0);
		for (int i = 0; i < item->childCount(); ++i)
		{
			item->child(i)->setCheckState(0, checkState);
		}
	}
	else if (item->childCount() == 0 || column == -1)
	{
		if (item->parent() == 0)
		{
			return;
		}
		for (int j = 0; j < item->parent()->childCount(); ++j)
		{
			if (j != item->parent()->indexOfChild(item) && item->checkState(0) != item->parent()->child(j)->checkState(0))
			{
				diff = true;
			}
		}
		if (diff)
		{
			item->parent()->setCheckState(0, Qt::PartiallyChecked);
		}
		else
		{
			item->parent()->setCheckState(0, item->checkState(0));
		}
		if (item->parent() != 0)
		{
			updateChecks(item->parent(), -1);
		}
	}
}
