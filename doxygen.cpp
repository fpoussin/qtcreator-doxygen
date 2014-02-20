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
#include <plugins/cpptools/cppmodelmanager.h>
#include <plugins/texteditor/basetexteditor.h>
#include <plugins/coreplugin/icore.h>
#include <plugins/coreplugin/editormanager/ieditor.h>
#include <plugins/coreplugin/editormanager/editormanager.h>
#include <plugins/coreplugin/editormanager/editorview.h>
#include <plugins/projectexplorer/project.h>
#include <plugins/projectexplorer/projectexplorer.h>
#include <plugins/projectexplorer/session.h>
#include <plugins/projectexplorer/projectexplorerconstants.h>
#include <libs/cplusplus/Overview.h>
#include <libs/extensionsystem/pluginmanager.h>
#include <cplusplus/Scope.h>
#include <cplusplus/Symbols.h>
#include <cplusplus/Names.h>
#include <cplusplus/CppDocument.h>

#include <QString>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QProgressDialog>

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
    const Scope *scope = symbol->asScope();
    QStringList scopes;

    if(symbol->isFunction())
    {
        const Name *name = symbol->name();
        Overview overview;
        overview.showArgumentNames = false;
        overview.showReturnTypes = false;
        scopes.prepend(overview.prettyName(name));
        return scopes;
    }

    for (; scope; scope = scope->enclosingScope())
    {
        Symbol *owner = scope->memberAt(0);

        if (owner && owner->name() && ! scope->isEnum())
        {
            const Name *name = owner->name();
            Overview overview;
            overview.showArgumentNames = false;
            overview.showReturnTypes = false;
            scopes.prepend(overview.prettyName(name));
        }
    }
    return scopes;
}

Symbol* currentSymbol(Core::IEditor *editor)
{
    CppTools::CppModelManagerInterface *modelManager = CppTools::CppModelManagerInterface::instance();
    if (!modelManager)
    {
        return 0;
    }

    const Snapshot snapshot = modelManager->snapshot();
    Document::Ptr doc = snapshot.document(editor->document()->filePath());
    if (!doc)
    {
        return 0;
    }

    Symbol* last = doc->lastVisibleSymbolAt(editor->currentLine(), editor->currentColumn());
    return last;
}

// TODO, recode it entirely.
void Doxygen::createDocumentation(const DoxygenSettingsStruct &DoxySettings)
{
    const Core::EditorManager *editorManager = Core::EditorManager::instance();
    Core::IEditor *editor = editorManager->currentEditor();

    // before continuing, test if the editor is actually showing a file.
    if(!editor)
        return;

    // get the widget for later.
    TextEditor::BaseTextEditorWidget *editorWidget = qobject_cast<TextEditor::BaseTextEditorWidget*>(
                editorManager->currentEditor()->widget());

    // get our symbol
    Symbol *lastSymbol = currentSymbol(editor);
    editorWidget->gotoLineStart();
    int lastLine = editor->currentLine();
    int lastColumn = editor->currentColumn();
    while(lastSymbol
          && (lastSymbol->line() != static_cast<unsigned>(lastLine)
              || lastSymbol->column() != static_cast<unsigned>(lastColumn)))
    {
        //qDebug() << "lastSymbol: " << lastSymbol->line() << " " << lastSymbol->column();
        //qDebug() << "lastLine: " << lastLine << " " << lastColumn;
        editorWidget->gotoNextWord();
        // infinite loop prevention
        if(lastLine == editor->currentLine() && lastColumn == editor->currentColumn())
            return;
        lastLine = editor->currentLine();
        lastColumn = editor->currentColumn();
        lastSymbol = currentSymbol(editor);
    }
    //qDebug() << lastLine << " " << lastColumn;
    if (!lastSymbol)
    {
        return;
    }

    QStringList scopes = scopesForSymbol(lastSymbol);
    Overview overview;
    overview.showArgumentNames = true;
    overview.showDefaultArguments = false;
    overview.showTemplateParameters = false;
    overview.showReturnTypes = true;
    overview.showFunctionSignatures = true;
    const Name *name = lastSymbol->name();
    scopes.append(overview.prettyName(name));
    //qDebug() << overview.prettyName(name);
    //qDebug() << overview.prettyType(lastSymbol->type(), name);
    //qDebug() << scopes;

    QString docToWrite;
    // Do we print a short documentation block at end of line?
    bool printAtEnd = false;

    // Get current indentation as per bug #5
    QString indent;
    editorWidget->gotoLineStart();
    editorWidget->gotoLineStartWithSelection();
    QString currentText = editorWidget->textCursor().selectedText();
    QStringList textList = currentText.split(QRegExp("\\b"));
    indent = textList.at(0);

    // quickfix when calling the method on "};" (end class) or "}" (end namespace)
    if(indent.contains(QRegExp("^\\};?")))
    {
        return;
    }

    if(indent.endsWith('~'))
        indent.chop(1);

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
            QString projectRoot = getProjectRoot(editor);
            QString fileName = editor->document()->filePath().remove(0, editor->document()->filePath().lastIndexOf("/") + 1);
            QString fileNameProj = editor->document()->filePath().remove(projectRoot);
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
        docToWrite += indent + DoxySettings.DoxyComment.doxEnding;
    }
    // Here comes the bitch.
    else if(lastSymbol->isDeclaration() || lastSymbol->isFunction())
    {
        overview.showArgumentNames = true;
        overview.showReturnTypes = false;
        overview.showDefaultArguments = false;
        overview.showTemplateParameters = false;
        overview.showFunctionSignatures = true;

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
                docToWrite = DoxySettings.DoxyComment.doxShortVarDoc + "TODO" + DoxySettings.DoxyComment.doxShortVarDocEnd;
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

            QStringList args = arglist.trimmed().split(',', QString::SkipEmptyParts);

            Q_FOREACH(QString singleArg, args)
            {
                singleArg.remove(QRegExp("\\s*=.*")); // FIXME probably don't need the * after \\s but...
                singleArg.replace("*","");
                singleArg.replace("&","");
                docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "param " + singleArg.section(' ',  -1) + "\n";
            }

            // And now check the return type
            overview.showArgumentNames = false;
            overview.showDefaultArguments = false;
            overview.showTemplateParameters = false;
            overview.showReturnTypes = true;
            overview.showFunctionSignatures = false;

            arglist = overview.prettyType(lastSymbol->type(), name);

            // FIXME this check is just insane...
            if( arglist.contains(' ')
                    && ((lastSymbol->isFunction() && !overview.prettyName(name).contains("::~"))
                        || (lastSymbol->isDeclaration() && overview.prettyName(name).at(0) != '~') ))
            {
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
                    if(DoxySettings.automaticReturnType == false)
                    {
                        arglist.clear();
                    }

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

void Doxygen::addFileComment(const DoxygenSettingsStruct &DoxySettings)
{
    const Core::EditorManager *editorManager = Core::EditorManager::instance();
    Core::IEditor *editor = editorManager->currentEditor();

    // before continuing, test if the editor is actually showing a file.
    if(!editor)
        return;

    // get the widget for later.
    TextEditor::BaseTextEditorWidget *editorWidget = qobject_cast<TextEditor::BaseTextEditorWidget*>(
                editorManager->currentEditor()->widget());
    // get our symbol
    editorWidget->gotoLine(1, 0);
    editorWidget->insertPlainText(DoxySettings.fileComment + "\n");
}

void Doxygen::addSymbol(const CPlusPlus::Symbol* symbol, QList<const Symbol*> &symmap)
{
    if(!symbol || symbol->isBaseClass() || symbol->isGenerated())
        return;

    if(symbol->isArgument()
            || symbol->isFunction()
            || symbol->isDeclaration()
            || symbol->isEnum())
    {
        symmap.append(symbol);
        return;
    }
    else if(symbol->isClass())
    {
        symmap.append(symbol);
    }

    const CPlusPlus::Scope* scopedSymbol = symbol->asScope();
    if(scopedSymbol)
    {
        int nbmembers = scopedSymbol->memberCount();
        for(int i=0; i<nbmembers; ++i)
        {
            addSymbol(scopedSymbol->memberAt(i), symmap);
        }
    }
}

void Doxygen::documentFile(const DoxygenSettingsStruct &DoxySettings)
{
    const Core::EditorManager *editorManager = Core::EditorManager::instance();
    Core::IEditor *editor = editorManager->currentEditor();

    // before continuing, test if the editor is actually showing a file.
    if(!editor)
    {
        //qDebug() << "No editor";
        return;
    }

    CppTools::CppModelManagerInterface *modelManager = CppTools::CppModelManagerInterface::instance();
    //ExtensionSystem::PluginManager::instance()->getObject<CPlusPlus::CppModelManagerInterface>();
    if(!modelManager)
    {
        //qDebug() << "No modelManager";
        return;
    }

    const Snapshot snapshot = modelManager->snapshot();
    Document::Ptr doc = snapshot.document(editor->document()->filePath());
    if(!doc)
    {
        //qDebug() << "No document";
        return;
    }

    // TODO : check
    int globalSymbols = doc->globalSymbolCount();
    if(!globalSymbols)
    {
        if(DoxySettings.fileCommentsEnabled)
        {
            addFileComment(DoxySettings);
        }
        //qDebug() << "No global symbols";
        return;
    }


    // check that as well...
    Scope* scope = doc->scopeAt(0,0);
    if(!scope)
    {
        if(DoxySettings.fileCommentsEnabled)
        {
            addFileComment(DoxySettings);
        }
        //qDebug() << "No scope";
        return;
    }

    unsigned symbolcount = scope->memberCount();

    QList<const Symbol*> symmap;
    for(unsigned i=0; i < symbolcount; ++i)
        addSymbol(scope->memberAt(i), symmap);

    // sanity check, it's expensive and ugly but the result isn't pretty on some codes if not done.
    unsigned oldline=0;
    Q_FOREACH(const Symbol* sym, symmap)
    {
        if(sym->line() == oldline)
            symmap.removeOne(sym);
        oldline = sym->line();
    }

    TextEditor::BaseTextEditorWidget *editorWidget = qobject_cast<TextEditor::BaseTextEditorWidget*>(
                editorManager->currentEditor()->widget());

    if (editorWidget)
    {
        QList<const Symbol*>::iterator it = symmap.end();
        for(; it != symmap.begin(); --it)
        {
            const Symbol* sym = *(it-1);
            editorWidget->gotoLine(sym->line());
            createDocumentation(DoxySettings);
        }

        if(DoxySettings.fileCommentsEnabled)
        {
            addFileComment(DoxySettings);
        }
    }
}

// TODO fix this!!!
void Doxygen::documentActiveProject(const DoxygenSettingsStruct &DoxySettings)
{
    documentProject(ProjectExplorer::SessionManager::startupProject(), DoxySettings);
}

void Doxygen::documentOpenedProject(const DoxygenSettingsStruct &DoxySettings)
{
    documentProject(ProjectExplorer::ProjectExplorerPlugin::currentProject(), DoxySettings);
}

void Doxygen::documentProject(ProjectExplorer::Project *p, const DoxygenSettingsStruct &DoxySettings)
{
    Core::EditorManager *editorManager = Core::EditorManager::instance();
    QStringList files = p->files(ProjectExplorer::Project::ExcludeGeneratedFiles);
    QProgressDialog progress("Processing files...", "Cancel", 0, files.size());
    progress.setWindowModality(Qt::WindowModal);
    for(int i = 0 ; i < files.size() ; ++i)
    {
        bool documented = false;
        progress.setValue(i);
        if(progress.wasCanceled()){
            break;
        }

        QFileInfo fileInfo(files[i]);
        QString fileExtension = fileInfo.suffix();
        if(
                (
                    (DoxySettings.fcomment == headers /*|| DoxySettings.fcomment == bothqt*/ ||
                     DoxySettings.fcomment == all)
                    && (fileExtension == "hpp" || fileExtension == "h")
                    )
                || (
                    (DoxySettings.fcomment == implementations /*|| DoxySettings.fcomment == bothqt*/ ||
                     DoxySettings.fcomment == all)
                    && (fileExtension == "cpp" || fileExtension == "c")
                    )
                ) { /*|| ( //TODO: add documentation of QML files (see doxyqml comments interpretation)
                    (DoxySettings.fcomment == qml || DoxySettings.fcomment == all)
                    && fileExtension == "qml"
                    )
                ) {*/
            Core::IEditor *editor = editorManager->openEditor(files[i]);
            if(editor)
            {
                documented = true;
                documentFile(DoxySettings);
            }
        }

        if(DoxySettings.fileCommentsEnabled && documented == false)
        {
            bool commentFile = false;
            //qDebug() << "FileCommentHeaders: " << DoxySettings.fileCommentHeaders;
            //qDebug() << "FileCommentImpl: " << DoxySettings.fileCommentImpl;
            if(DoxySettings.fileCommentHeaders && (fileExtension == "hpp" || fileExtension == "h"))
            {
                commentFile = true;
            }
            else if(DoxySettings.fileCommentImpl && (fileExtension == "cpp" || fileExtension == "c"))
            {
                commentFile = true;
            }

            if(commentFile)
            {
                Core::IEditor *editor = editorManager->openEditor(files[i]);
                if(editor)
                    addFileComment(DoxySettings);
            }
        }
    }
    progress.setValue(files.size());
}

QString Doxygen::getProjectRoot(Core::IEditor* editor)
{
    QString projectRoot;
    ProjectExplorer::Project* proj = ProjectExplorer::SessionManager::projectForFile(editor->document()->filePath());
    if(proj)
    {
        projectRoot = proj->projectDirectory() + "/";
    }
    return projectRoot;
}

