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

#include "doxygen.h"


#include <QObject>
#include <plugins/cppeditor/cppeditorconstants.h>
#include <plugins/cpptools/cpptoolsconstants.h>
#include <plugins/cpptools/cppmodelmanagerinterface.h>
#include <plugins/texteditor/basetexteditor.h>
#include <plugins/coreplugin/icore.h>
#include <plugins/coreplugin/uniqueidmanager.h>
#include <plugins/coreplugin/mimedatabase.h>
#include <plugins/coreplugin/actionmanager/actionmanager.h>
#include <plugins/coreplugin/editormanager/ieditor.h>
#include <plugins/coreplugin/editormanager/editormanager.h>
#include <plugins/projectexplorer/project.h>
#include <plugins/projectexplorer/projectexplorer.h>
#include <plugins/projectexplorer/session.h>
#include <plugins/projectexplorer/projectexplorerconstants.h>

#include <libs/cplusplus/Overview.h>
#include <libs/extensionsystem/pluginmanager.h>
#include <shared/cplusplus/Scope.h>
#include <shared/cplusplus/Symbols.h>
#include <shared/cplusplus/Names.h>
#include <cplusplus/CppDocument.h>
#include <cplusplus/CppBindings.h>


#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegExp>

using namespace CPlusPlus;
using namespace ProjectExplorer;
using namespace DoxyPlugin;
using namespace DoxyPlugin::Internal;

Doxygen* Doxygen::m_instance = 0;

Doxygen::Doxygen()
{
}

Doxygen* Doxygen::instance()
{
    if (!m_instance)
        m_instance = new Doxygen();
    return m_instance;
}

// TODO, get rid of it.
QStringList scopesForSymbol(const Symbol* symbol)
{
    Scope *scope = symbol->scope();
    QStringList scopes;

    if(symbol->isFunction())
    {
        const Name *name = symbol->name();
        Overview overview;
        overview.setShowArgumentNames(false);
        overview.setShowReturnTypes(false);
        scopes.prepend(overview.prettyName(name));
        return scopes;
    }

    for (; scope; scope = scope->enclosingScope())
    {
        Symbol *owner = scope->owner();

        if (owner && owner->name() && ! scope->isEnumScope())
        {
            const Name *name = owner->name();
            Overview overview;
            overview.setShowArgumentNames(false);
            overview.setShowReturnTypes(false);
            scopes.prepend(overview.prettyName(name));
        }
    }
    return scopes;
}

Symbol* currentSymbol(Core::IEditor *editor)
{
    int line = editor->currentLine();
    int column = editor->currentColumn();

    CppTools::CppModelManagerInterface *modelManager =
            ExtensionSystem::PluginManager::instance()->getObject<CppTools::CppModelManagerInterface>();
    if (!modelManager)
        return 0;
    const Snapshot snapshot = modelManager->snapshot();
    Document::Ptr doc = snapshot.document(editor->file()->fileName());
    if (!doc)
        return 0;
    return doc->findSymbolAt(line, column);
}

// TODO, recode it entirely.
void Doxygen::createDocumentation(const DoxygenSettingsStruct &DoxySettings)
{
    const Core::EditorManager *editorManager = Core::EditorManager::instance();
    Core::IEditor *editor = editorManager->currentEditor();

    // before continuing, test if the editor is actually showing a file.
    if(!editor) return;

    // get the widget for later.
    TextEditor::BaseTextEditor *editorWidget = qobject_cast<TextEditor::BaseTextEditor*>(
            editorManager->currentEditor()->widget());

    // TODO, only do that if class and verbosePrinting
    // Catch hold of the plugin-manager
    ExtensionSystem::PluginManager* pm
            = ExtensionSystem::PluginManager::instance();
    // Look for the ProjectExplorerPlugin object
    ProjectExplorer::ProjectExplorerPlugin* projectExplorerPlugin
            = pm->getObject<ProjectExplorer::ProjectExplorerPlugin>();
    // Fetch a list of all open projects
    QList<ProjectExplorer::Project*> projects
            = projectExplorerPlugin->session()->projects();
    // Project root directory
    QString projectRoot;

    // Attempt to find our project
    Q_FOREACH(ProjectExplorer::Project* project, projects)
    {
        QStringList files = project->files(Project::ExcludeGeneratedFiles); // ProjectExplorer::Project::FilesMode::ExcludeGeneratedFiles
        // is it our project ?
        if(files.contains(editor->file()->fileName()))
        {
            // YES! get the .pro and remove the directory part from our filename
            // TODO, check if it is smart... (it's not really.)
            Q_FOREACH(QString f, files)
            {
                if(f.contains(QRegExp(".pro$")))
                {
                    projectRoot = f.section('/', 0, -2);
                    if(projectRoot.size()) projectRoot.append("/");
                    continue;
                }
            }
            if(projectRoot.size()) continue;
        }
    }




    Symbol *lastSymbol = currentSymbol(editor);
    if (!lastSymbol || !lastSymbol->scope())
        return;

    QStringList scopes = scopesForSymbol(lastSymbol);
    Overview overview;
    overview.setShowArgumentNames(true);
    overview.setShowReturnTypes(true);
    overview.setShowFullyQualifiedNamed(true);
    overview.setShowFunctionSignatures(true);
    const Name *name = lastSymbol->name();
    scopes.append(overview.prettyName(name));

    QString docToWrite;

    // Do we print a short documentation block at end of line?
    bool printAtEnd = false;

    // Get current indentation as per bug #5
    QString indent;
    editorWidget->moveCursor(QTextCursor::StartOfLine);
    editorWidget->gotoLineEndWithSelection();
    QString currentText = editorWidget->textCursor().selectedText();
    QStringList textList = currentText.split(QRegExp("\\b"));
    indent = textList.at(0);
    if(indent.endsWith('~'))
    {
        indent.chop(1);
    }

    if(lastSymbol->isClass())
    {
        docToWrite += indent + DoxySettings.DoxyComment.doxBegin;
        if(DoxySettings.printBrief)
        {
            docToWrite += indent + DoxySettings.DoxyComment.doxBrief;
            docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine;
        }
        if(DoxySettings.verbosePrinting)
        {
            QString fileName = editor->file()->fileName().remove(0, editor->file()->fileName().lastIndexOf("/") + 1);
            QString fileNameProj = editor->file()->fileName().remove(projectRoot);
            docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "class " + overview.prettyName(name) + " " + fileName + " \"" + fileNameProj + "\"\n";
        }
        docToWrite += indent + DoxySettings.DoxyComment.doxEnding;
    }
    else if(lastSymbol->isTypedef())
    {
        docToWrite += indent + DoxySettings.DoxyComment.doxBegin;
        if(DoxySettings.printBrief)
        {
            docToWrite += indent + DoxySettings.DoxyComment.doxBrief;
            docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine;
        }
        if(DoxySettings.verbosePrinting)
            docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "typedef " + overview.prettyName(name);
        docToWrite += indent + DoxySettings.DoxyComment.doxEnding;
    }
    else if(lastSymbol->isEnum())
    {
        docToWrite += indent + DoxySettings.DoxyComment.doxBegin;
        if(DoxySettings.printBrief)
            docToWrite += indent + DoxySettings.DoxyComment.doxBrief;
        docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine;
        if(DoxySettings.verbosePrinting)
            docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "enum " + overview.prettyName(name) + "\n";
        docToWrite += DoxySettings.DoxyComment.doxEnding;
    }
    // Here comes the bitch.
    else if(lastSymbol->isDeclaration() || lastSymbol->isFunction())
    {
        overview.setShowArgumentNames(true);
        overview.setShowReturnTypes(false);
        overview.setShowFullyQualifiedNamed(true);
        overview.setShowFunctionSignatures(true);
        QString arglist = overview.prettyType(lastSymbol->type(), name);

        docToWrite += indent + DoxySettings.DoxyComment.doxBegin;
        if(DoxySettings.printBrief)
        {
            docToWrite += indent + DoxySettings.DoxyComment.doxBrief;
            docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine;
        }

        // if variable, do it quickly...
        if(!arglist.contains('('))
        {
            if(DoxySettings.shortVarDoc)
            {
                printAtEnd = true;
                docToWrite = DoxySettings.DoxyComment.doxShortVarDoc + "TODO */";
            }
            else
            {
                if(DoxySettings.verbosePrinting)
                    docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "var " + overview.prettyName(name) + "\n" + indent + DoxySettings.DoxyComment.doxEnding;
                else
                    docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine + indent + DoxySettings.DoxyComment.doxEnding;
            }
        }
        else
        {
            // Never noticed it before, a useless comment block because of the Q_OBJECT macro
            // so let's just ignore that will we?
            if(overview.prettyName(name) == "qt_metacall")
                return;

            if(DoxySettings.verbosePrinting)
                docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "fn " + overview.prettyName(name) + "\n";

            // Check parameters
            // Do it the naive way first before finding better in the API
            arglist.remove(0, arglist.indexOf("(") + 1);
            arglist.remove(arglist.lastIndexOf(")"), arglist.size() - arglist.lastIndexOf(")"));
            int indexfrom, indexto;
            while( ((indexfrom = arglist.indexOf('<'))!= -1) && (indexto = arglist.indexOf('>') != -1) )
            {
                    arglist.remove(indexfrom, indexto - indexfrom + 1);
            }
            QStringList args = arglist.trimmed().split(',', QString::SkipEmptyParts);

            Q_FOREACH(QString singleArg, args)
            {
                singleArg.remove(QRegExp("\\s*=.*")); // FIXME probably don't need the * after \\s but...
                singleArg.replace("*","");
                singleArg.replace("&","");
                docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "param " + singleArg.section(' ',  -1) + "\n";
            }

            // And now check the return type
            overview.setShowArgumentNames(false);
            overview.setShowReturnTypes(true);
            overview.setShowFullyQualifiedNamed(false);
            overview.setShowFunctionSignatures(false);

            arglist = overview.prettyType(lastSymbol->type(), name);

            // FIXME this check is just insane...
            if( arglist.contains(' ') && (((overview.prettyName(name) != scopes.front()) && (overview.prettyName(name).at(0) != '~')) || (lastSymbol->isFunction() && !overview.prettyName(name).contains("::~"))) )
            {
                //qDebug() << arglist;
                QRegExp rx("void *");
                rx.setPatternSyntax(QRegExp::Wildcard);
                if(!rx.exactMatch(arglist))
                {
                    // dirty workarround
                    int last;
                    if(arglist.contains('>'))
                        last = arglist.lastIndexOf('>') + 1;
                    else
                        last = arglist.lastIndexOf(' ');

                    arglist.chop(arglist.size() - last);
                    docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "return " +  arglist + "\n";
                }

            }
            docToWrite += indent + DoxySettings.DoxyComment.doxEnding;
        }
    }


    // Write the documentation in the editor
    if (editorWidget)
    {
        if(printAtEnd)
            editorWidget->moveCursor(QTextCursor::EndOfLine);
        else
            editorWidget->moveCursor(QTextCursor::StartOfBlock);
        editorWidget->insertPlainText(docToWrite);
    }

}

void Doxygen::addSymbol(const CPlusPlus::Symbol* symbol, QMap<unsigned, const CPlusPlus::Symbol*> &symmap)
{
    if(!symbol) return;

    if(symbol->isArgument()
        ||symbol->isFunction()
        || symbol->isDeclaration()
        || symbol->isEnum())
        {
        symmap.insert(symbol->line(), symbol);
        return;
    }
    else if(symbol->isForwardClassDeclaration()
        || symbol->isExtern()
        || symbol->isFriend()
        || symbol->isGenerated()
        || symbol->isUsingNamespaceDirective()
        || symbol->isUsingDeclaration()
        )
        {
        return;
    }
    symmap.insert(symbol->line(), symbol);



    const CPlusPlus::ScopedSymbol *scopedSymbol = symbol->asScopedSymbol();
    if (scopedSymbol)
    {
        CPlusPlus::Scope *scope = scopedSymbol->members();
        if (scope)
        {
            CPlusPlus::Scope::iterator cur = scope->firstSymbol();
            while (cur != scope->lastSymbol())
            {
                const CPlusPlus::Symbol *curSymbol = *cur;
                ++cur;
                if (!curSymbol)
                    continue;
                addSymbol(curSymbol, symmap);
            }
        }
    }

}

void Doxygen::documentFile(const DoxygenSettingsStruct &DoxySettings)
{
    const Core::EditorManager *editorManager = Core::EditorManager::instance();
    Core::IEditor *editor = editorManager->currentEditor();

    // before continuing, test if the editor is actually showing a file.
    if(!editor) return;

    CppTools::CppModelManagerInterface *modelManager =
            ExtensionSystem::PluginManager::instance()->getObject<CppTools::CppModelManagerInterface>();
    if (!modelManager)
        return;
    const Snapshot snapshot = modelManager->snapshot();
    Document::Ptr doc = snapshot.document(editor->file()->fileName());
    if (!doc)
        return;

    Scope *scope = doc->globalSymbols();
    if(!scope)
    {
        return;
    }

    unsigned symbolcount = scope->symbolCount();

    QMap<unsigned, const Symbol*> symmap;
    for(unsigned i = 0; i < symbolcount; ++i)
        addSymbol(scope->symbolAt(i), symmap);

    TextEditor::BaseTextEditor *editorWidget = qobject_cast<TextEditor::BaseTextEditor*>(
            editorManager->currentEditor()->widget());

    if (editorWidget)
    {
        QMap<unsigned, const Symbol*>::iterator it = symmap.end();
        for(; it != symmap.begin(); --it)
        {
            unsigned line = (it-1).key();
            editorWidget->gotoLine(line);
            createDocumentation(DoxySettings);
        }
    }
}

