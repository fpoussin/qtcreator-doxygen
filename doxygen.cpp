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

#ifdef __DEBUG__
    unsigned count = scope->symbolCount();

    qDebug();
    Overview overview;
    overview.setShowArgumentNames(true);
    overview.setShowReturnTypes(true);
    overview.setShowFullyQualifiedNamed(true);
    overview.setShowFunctionSignatures(true);

    qDebug() << overview.prettyName(symbol->name());
    qDebug() << overview.prettyType(symbol->type(), symbol->name());
    qDebug() << "number of symbols: " << count;
    qDebug() << "isArgument: " << symbol->isArgument();
    qDebug() << "isBaseClass: " << symbol->isBaseClass();
    qDebug() << "isBlock: " << symbol->isBlock();
    qDebug() << "isClass: " << symbol->isClass();
    qDebug() << "isDeclaration: " << symbol->isDeclaration();
    qDebug() << "isDeprecated: " << symbol->isDeprecated();
    qDebug() << "isEnum: " << symbol->isEnum();
    qDebug() << "isExtern: " << symbol->isExtern();
    qDebug() << "isForwardClassDeclaration: " << symbol->isForwardClassDeclaration();
    qDebug() << "isFriend: " << symbol->isFriend();
    qDebug() << "isFunction: " << symbol->isFunction();
    qDebug() << "isGenerated: " << symbol->isGenerated();
    qDebug() << "isMutable: " << symbol->isMutable();
    qDebug() << "isNamespace: " << symbol->isNamespace();
    qDebug() << "isPrivate: " << symbol->isPrivate();
    qDebug() << "isProtected: " << symbol->isProtected();
    qDebug() << "isPublic: " << symbol->isPublic();
    qDebug() << "isRegister: " << symbol->isRegister();
    qDebug() << "isScopedSymbol: " << symbol->isScopedSymbol();
    qDebug() << "isStatic: " << symbol->isStatic();
    qDebug() << "isTypedef: " << symbol->isTypedef();
    qDebug() << "isTypenameArgument: " << symbol->isTypenameArgument();
    qDebug() << "isUsingDeclaration: " << symbol->isUsingDeclaration();
    qDebug() << "isUsingNamespaceDirective: " << symbol->isUsingNamespaceDirective();
#endif

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
void Doxygen::createDocumentation(const DoxygenSettingsStruct::DoxygenComment &DoxyComment) const
{
    const Core::EditorManager *editorManager = Core::EditorManager::instance();
    Core::IEditor *editor = editorManager->currentEditor();

    // before continuing, test if the editor is actually showing a file.
    if(!editor) return;

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

    /// scopes.at(0) = class name
    /// scopes.at(1) = method name + (...) <-- analyse this
    QStringList scopes = scopesForSymbol(lastSymbol);
    Overview overview;
    overview.setShowArgumentNames(true);
    overview.setShowReturnTypes(true);
    overview.setShowFullyQualifiedNamed(true);
    overview.setShowFunctionSignatures(true);
    const Name *name = lastSymbol->name();
    scopes.append(overview.prettyName(name));

    //    QString genericBeginNoindent = "/**\n* @brief \n*\n";
    //    QString genericBegin         = "    /**\n    * @brief \n    *\n";
    //    QString shortBeginNoindent   = "/** ";
    //    QString shortBegin           = "    /** ";
    QString docToWrite;

    if(lastSymbol->isClass())
    {
        QString fileName = editor->file()->fileName().remove(0, editor->file()->fileName().lastIndexOf("/") + 1);
        QString fileNameProj = editor->file()->fileName().remove(projectRoot);
        docToWrite += DoxyComment.doxGenericBeginNoindent;
        docToWrite += DoxyComment.doxNewLine + "class " + overview.prettyName(name) + " " + fileName + " \"" + fileNameProj + "\"";
        docToWrite += "\n*/\n";
    }
    else if(lastSymbol->isTypedef())
    {
        docToWrite += DoxyComment.doxShortBeginNoindent;
        docToWrite += DoxyComment.doxNewLine + "typedef " + overview.prettyName(name);
        docToWrite += " */\n";
    }
    else if(lastSymbol->isEnum())
    {
        if(lastSymbol->scope()->isClassScope())
        {
            docToWrite += DoxyComment.doxGenericBegin;
            docToWrite += "    " + DoxyComment.doxNewLine + "enum " + overview.prettyName(name);
            docToWrite += "    */\n";
        }
        else
        {
            docToWrite += DoxyComment.doxGenericBeginNoindent;
            docToWrite += DoxyComment.doxNewLine + "enum " + overview.prettyName(name);
            docToWrite += "\n*/\n";
        }
    }
    else if(lastSymbol->isArgument())
    {
        docToWrite += DoxyComment.doxShortBegin;
        docToWrite += "  ARG*/\n";
    }
    // Here comes the bitch.
    else if(lastSymbol->isDeclaration() || lastSymbol->isFunction())
    {
        overview.setShowArgumentNames(true);
        overview.setShowReturnTypes(false);
        overview.setShowFullyQualifiedNamed(true);
        overview.setShowFunctionSignatures(true);
        QString arglist = overview.prettyType(lastSymbol->type(), name);

        docToWrite += DoxyComment.doxGenericBegin;

        // if variable, do it quickly...
        if(!arglist.contains('('))
        {
            docToWrite += "    " + DoxyComment.doxNewLine + "var " + overview.prettyName(name) + "\n    */\n";
        }
        else
        {
            docToWrite += "    " + DoxyComment.doxNewLine + "fn " + overview.prettyName(name) + "\n";
            // Check parameters
            // Do it the naive way first before finding better in the API
            // TODO, check throw()...
            arglist.remove(0, arglist.indexOf("(") + 1);
            arglist.remove(arglist.lastIndexOf(")"), arglist.size() - arglist.lastIndexOf(")"));
            QStringList args = arglist.trimmed().split(',', QString::SkipEmptyParts);

            Q_FOREACH(QString singleArg, args)
            {
                if(singleArg.contains('>'))
                {
                    singleArg.remove(0, singleArg.lastIndexOf('>') + 1);
                }
                if(singleArg.contains('='))
                {
                    singleArg.remove(singleArg.size() - singleArg.lastIndexOf('='));
                }
                singleArg.replace("*","");
                singleArg.replace("&","");
                docToWrite += "    " + DoxyComment.doxNewLine + "param " + singleArg.section(' ', - 1) + "\n";
            }

            // And now check the return type
            overview.setShowArgumentNames(false);
            overview.setShowReturnTypes(true);
            overview.setShowFullyQualifiedNamed(false);
            overview.setShowFunctionSignatures(false);

            arglist = overview.prettyType(lastSymbol->type(), name);

            if( ((overview.prettyName(name) != scopes.front()) && (overview.prettyName(name).at(0) != '~')) || (lastSymbol->isFunction() && !overview.prettyName(name).contains("::~")) )
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
                    docToWrite += "    " + DoxyComment.doxNewLine + "return " +  arglist + "\n";
                }

            }
            docToWrite += "    */\n";
        }
    }


    // Write the documentation in the editor
    TextEditor::BaseTextEditor *editorWidget = qobject_cast<TextEditor::BaseTextEditor*>(
            editorManager->currentEditor()->widget());
    if (editorWidget)
    {
        editorWidget->moveCursor(QTextCursor::StartOfBlock);
        editorWidget->insertPlainText(docToWrite);
    }

}
