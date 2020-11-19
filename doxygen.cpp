/**************************************************************************
**
** This file is part of Doxygen plugin for Qt Creator
**
** Copyright (c) 2009 Kevin Tanguy (kofee@kofee.org).
** Copyright (c) 2015 Fabien Poussin (fabien.poussin@gmail.com).
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
#include "doxygenfilesdialog.h"

#include <QObject>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/editorview.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/icore.h>
#include <cplusplus/CppDocument.h>
#include <cplusplus/Names.h>
#include <cplusplus/Overview.h>
#include <cplusplus/Scope.h>
#include <cplusplus/Symbols.h>
#include <cppeditor/cppeditorconstants.h>
#include <cpptools/cppmodelmanager.h>
#include <cpptools/cpptoolsconstants.h>
#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/session.h>
#include <texteditor/texteditor.h>

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QRegExp>
#include <QString>
#include <QStringList>

using namespace CPlusPlus;
using namespace ProjectExplorer;
using namespace DoxyPlugin;
using namespace DoxyPlugin::Internal;

Doxygen* Doxygen::m_instance = nullptr;

Doxygen::Doxygen(QObject* parent)
    : QObject(parent)
{
    m_cancel = false;

    m_projectProgress = new QProgressDialog();
    m_projectProgress->setWindowModality(Qt::WindowModal);
    m_projectProgress->setMinimumWidth(300);
    m_projectProgress->setMinimum(0);
    m_projectProgress->setWindowTitle("Processing project...");
    m_projectProgress->close();

    m_fileProgress = new QProgressDialog();
    m_fileProgress->setWindowModality(Qt::WindowModal);
    m_fileProgress->setMinimumWidth(300);
    m_fileProgress->setMinimum(0);
    m_fileProgress->setWindowTitle("Processing file...");
    m_fileProgress->close();

    connect(m_projectProgress, SIGNAL(canceled()), this, SLOT(cancelOperation()));
    connect(m_fileProgress, SIGNAL(canceled()), this, SLOT(cancelOperation()));
}

Doxygen::~Doxygen()
{
    disconnect(m_projectProgress, SIGNAL(canceled()), this, SLOT(cancelOperation()));
    disconnect(m_fileProgress, SIGNAL(canceled()), this, SLOT(cancelOperation()));

    delete m_projectProgress;
    delete m_fileProgress;
}

Doxygen* Doxygen::instance()
{
    if (!m_instance)
        m_instance = new Doxygen;
    return m_instance;
}

// TODO, get rid of it.
QStringList scopesForSymbol(const Symbol* symbol)
{
    const Scope* scope = symbol->asScope();
    QStringList scopes;

    if (symbol->isFunction()) {
        const Name* name = symbol->name();
        Overview overview;
        overview.showArgumentNames = false;
        overview.showReturnTypes = false;
        scopes.prepend(overview.prettyName(name));
        return scopes;
    }

    for (; scope; scope = scope->enclosingScope()) {
        Symbol* owner = scope->memberAt(0);

        if (owner && owner->name() && !scope->isEnum()) {
            const Name* name = owner->name();
            Overview overview;
            overview.showArgumentNames = false;
            overview.showReturnTypes = false;
            scopes.prepend(overview.prettyName(name));
        }
    }
    return scopes;
}

Symbol* currentSymbol(Core::IEditor* editor)
{
    CppTools::CppModelManager* modelManager = CppTools::CppModelManager::instance();
    if (!modelManager) {
        return nullptr;
    }

    const Snapshot snapshot = modelManager->snapshot();
    Document::Ptr doc = snapshot.document(editor->document()->filePath());
    if (!doc) {
        return nullptr;
    }

    Symbol* last = doc->lastVisibleSymbolAt(editor->currentLine(), editor->currentColumn());
    return last;
}

// TODO: Recode it entirely.
bool Doxygen::documentEntity(const DoxygenSettingsStruct& DoxySettings, Core::IEditor* editor)
{
    // before continuing, test if the editor is actually showing a file.
    if (!editor)
        return false;

    // get the widget for later.
    TextEditor::TextEditorWidget* editorWidget = qobject_cast<TextEditor::TextEditorWidget*>(
        editor->widget());

    // get our symbol
    Symbol* lastSymbol = currentSymbol(editor);
    editorWidget->gotoLineStart();
    int lastLine = editor->currentLine();
    int lastColumn = editor->currentColumn();
    while (lastSymbol
        && (lastSymbol->line() != static_cast<unsigned>(lastLine)
               || lastSymbol->column() != static_cast<unsigned>(lastColumn))) {
        //qDebug() << "lastSymbol: " << lastSymbol->line() << " " << lastSymbol->column();
        //qDebug() << "lastLine: " << lastLine << " " << lastColumn;
        editorWidget->gotoNextWord();
        // infinite loop prevention
        if (lastLine == editor->currentLine() && lastColumn == editor->currentColumn())
            return false;
        lastLine = editor->currentLine();
        lastColumn = editor->currentColumn();
        lastSymbol = currentSymbol(editor);
    }
    //qDebug() << lastLine << " " << lastColumn;
    if (!lastSymbol) {
        return false;
    }

    // We don't want to document multiple times.
    // TODO: Find a better, faster way.
    QRegExp commentClosing("\\*/");
    QString text(editorWidget->document()->toPlainText());
    QStringList lines(text.split(QRegExp("\n|\r\n|\r")));

    // We check the 4 previous lines for comments block closing.
    for (int i = 1; i <= 4; i++) {
        int prevLine = lastLine - i;
        if (prevLine < 0)
            break;
        QString checkText(lines.at(prevLine));
        if (checkText.contains(commentClosing)) {
            //qDebug() << "Duplicate found in" << editor->document()->filePath().toString() << prevLine;
            //qDebug() << checkText;
            return false;
        }
    }

    QStringList scopes = scopesForSymbol(lastSymbol);
    Overview overview;
    overview.showArgumentNames = true;
    overview.showDefaultArguments = false;
    overview.showTemplateParameters = false;
    overview.showReturnTypes = true;
    overview.showFunctionSignatures = true;
    const Name* name = lastSymbol->name();
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
    if (indent.contains(QRegExp("^\\};?"))) {
        return false;
    }

    if (indent.endsWith('~'))
        indent.chop(1);

    if (lastSymbol->isClass()) {
        docToWrite += indent + DoxySettings.DoxyComment.doxBegin;
        if (DoxySettings.printBrief) {
            docToWrite += indent + DoxySettings.DoxyComment.doxBrief;
            docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine;
        }
        if (DoxySettings.verbosePrinting) {
            QString projectRoot = getProjectRoot();
            QString fileNameStr = editor->document()->filePath().toString();
            QString fileName = fileNameStr.remove(0, fileNameStr.lastIndexOf("/") + 1);
            QString fileNameProj = fileNameStr.remove(projectRoot);
            docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "class " + overview.prettyName(name) + " " + fileName + " \"" + fileNameProj + "\"\n";
        }
        docToWrite += indent + DoxySettings.DoxyComment.doxEnding;
    } else if (lastSymbol->isTypedef()) {
        docToWrite += indent + DoxySettings.DoxyComment.doxBegin;
        if (DoxySettings.printBrief) {
            docToWrite += indent + DoxySettings.DoxyComment.doxBrief;
            docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine;
        }
        if (DoxySettings.verbosePrinting)
            docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "typedef " + overview.prettyName(name);
        docToWrite += indent + DoxySettings.DoxyComment.doxEnding;
    } else if (lastSymbol->isEnum()) {
        docToWrite += indent + DoxySettings.DoxyComment.doxBegin;
        if (DoxySettings.printBrief)
            docToWrite += indent + DoxySettings.DoxyComment.doxBrief;
        docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine;
        if (DoxySettings.verbosePrinting)
            docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "enum " + overview.prettyName(name) + "\n";
        docToWrite += indent + DoxySettings.DoxyComment.doxEnding;
    }
    // Here comes the bitch.
    else if (lastSymbol->isDeclaration() || lastSymbol->isFunction()) {
        overview.showArgumentNames = true;
        overview.showReturnTypes = false;
        overview.showDefaultArguments = false;
        overview.showTemplateParameters = false;
        overview.showFunctionSignatures = true;

        QString arglist = overview.prettyType(lastSymbol->type(), name);
        docToWrite += indent + DoxySettings.DoxyComment.doxBegin;
        if (DoxySettings.printBrief) {
            docToWrite += indent + DoxySettings.DoxyComment.doxBrief;
            docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine;
        }

        // if variable, do it quickly...
        if (!arglist.contains('(')) {
            if (DoxySettings.shortVarDoc) {
                printAtEnd = true;
                docToWrite = DoxySettings.DoxyComment.doxShortVarDoc + "TODO: describe" + DoxySettings.DoxyComment.doxShortVarDocEnd;
            } else {
                if (DoxySettings.verbosePrinting)
                    docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "var " + overview.prettyName(name) + "\n" + indent + DoxySettings.DoxyComment.doxEnding;
                else
                    docToWrite += indent + DoxySettings.DoxyComment.doxEmptyLine + indent + DoxySettings.DoxyComment.doxEnding;
            }
        } else {
            // Never noticed it before, a useless comment block because of the Q_OBJECT macro
            // so let's just ignore that will we?
            if (overview.prettyName(name) == "qt_metacall")
                return false;

            if (DoxySettings.verbosePrinting)
                docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "fn " + overview.prettyName(name) + "\n";

            // Check parameters
            // Do it the naive way first before finding better in the API
            arglist.remove(0, arglist.indexOf("(") + 1);
            arglist.remove(arglist.lastIndexOf(")"), arglist.size() - arglist.lastIndexOf(")"));

            QStringList args = arglist.trimmed().split(',', QString::SkipEmptyParts);

            Q_FOREACH (QString singleArg, args) {
                singleArg.remove(QRegExp("\\s*=.*")); // FIXME probably don't need the * after \\s but...
                singleArg.replace("*", "");
                singleArg.replace("&", "");
                docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "param " + singleArg.section(' ', -1) + "\n";
            }

            // And now check the return type
            overview.showArgumentNames = false;
            overview.showDefaultArguments = false;
            overview.showTemplateParameters = false;
            overview.showReturnTypes = true;
            overview.showFunctionSignatures = false;

            arglist = overview.prettyType(lastSymbol->type(), name);

            // FIXME this check is just insane...
            if (arglist.contains(' ')
                && ((lastSymbol->isFunction() && !overview.prettyName(name).contains("::~"))
                       || (lastSymbol->isDeclaration() && overview.prettyName(name).at(0) != '~'))) {
                QRegExp rx("void *");
                rx.setPatternSyntax(QRegExp::Wildcard);
                if (!rx.exactMatch(arglist)) {
                    // dirty workarround
                    int last;
                    if (arglist.contains('>'))
                        last = arglist.lastIndexOf('>') + 1;
                    else
                        last = arglist.lastIndexOf(' ');

                    arglist.chop(arglist.size() - last);
                    if (DoxySettings.automaticReturnType == false) {
                        arglist.clear();
                    }

                    docToWrite += indent + DoxySettings.DoxyComment.doxNewLine + "return " + arglist + "\n";
                }
            }
            docToWrite += indent + DoxySettings.DoxyComment.doxEnding;
        }
    }

    // Write the documentation in the editor
    if (editorWidget) {
        if (printAtEnd)
            editorWidget->moveCursor(QTextCursor::EndOfLine);
        else
            editorWidget->moveCursor(QTextCursor::StartOfBlock);

        editorWidget->insertPlainText(docToWrite);
        return true;
    }
    return false;
}

bool Doxygen::addFileComment(const DoxygenSettingsStruct& DoxySettings, Core::IEditor* editor)
{
    // before continuing, test if the editor is actually showing a file.
    if (!editor)
        return false;

    // get the widget for later.
    TextEditor::TextEditorWidget* editorWidget = qobject_cast<TextEditor::TextEditorWidget*>(
        editor->widget());

    QString text = DoxySettings.DoxyComment.doxBegin + DoxySettings.DoxyComment.doxNewLine;
    text += "file " + editor->document()->filePath().fileName() + "\n";
    text += DoxySettings.DoxyComment.doxEmptyLine;
    text += DoxySettings.fileComment;
    text += DoxySettings.DoxyComment.doxEnding + "\n";

    // get our symbol
    editorWidget->gotoLine(1, 0);
    editorWidget->insertPlainText(text);

    return true;
}

void Doxygen::addSymbol(const CPlusPlus::Symbol* symbol, QList<const Symbol*>& symmap)
{
    if (!symbol || symbol->isBaseClass() || symbol->isGenerated())
        return;

    if (symbol->isArgument()
        || symbol->isFunction()
        || symbol->isDeclaration()
        || symbol->isEnum()) {
        symmap.append(symbol);
        return;
    } else if (symbol->isClass()) {
        symmap.append(symbol);
    }

    const CPlusPlus::Scope* scopedSymbol = symbol->asScope();
    if (scopedSymbol) {
        int nbmembers = scopedSymbol->memberCount();
        for (int i = 0; i < nbmembers; ++i) {
            addSymbol(scopedSymbol->memberAt(i), symmap);
        }
    }
}

uint Doxygen::documentFile(const DoxygenSettingsStruct& DoxySettings, Core::IEditor* editor)
{
    // before continuing, test if the editor is actually showing a file.
    if (!editor) {
        //qDebug() << "No editor";
        return 0;
    }

    m_cancel = false;

    CppTools::CppModelManager* modelManager = CppTools::CppModelManager::instance();
    //ExtensionSystem::PluginManager::instance()->getObject<CPlusPlus::CppModelManagerInterface>();
    if (!modelManager) {
        //qDebug() << "No modelManager";
        return 0;
    }

    const Snapshot snapshot = modelManager->snapshot();
    Document::Ptr doc = snapshot.document(editor->document()->filePath());
    if (!doc) {
        //qDebug() << "No document";
        return 0;
    }

    // TODO : check
    int globalSymbols = doc->globalSymbolCount();
    if (!globalSymbols) {
        if (DoxySettings.fileCommentsEnabled) {
            addFileComment(DoxySettings, editor);
        }
        //qDebug() << "No global symbols";
        return 0;
    }

    // check that as well...
    Scope* scope = doc->scopeAt(0, 0);
    if (!scope) {
        if (DoxySettings.fileCommentsEnabled) {
            addFileComment(DoxySettings, editor);
        }
        //qDebug() << "No scope";
        return 0;
    }

    unsigned symbolcount = scope->memberCount();

    QList<const Symbol*> symmap;
    for (unsigned i = 0; i < symbolcount; ++i)
        addSymbol(scope->memberAt(i), symmap);

    // sanity check, it's expensive and ugly but the result isn't pretty on some codes if not done.
    unsigned oldline = 0;
    Q_FOREACH (const Symbol* sym, symmap) {
        if (sym->line() == oldline)
            symmap.removeOne(sym);
        oldline = sym->line();
    }

    const int symCount = symmap.size();
    const int symMin = 100;
    TextEditor::TextEditorWidget* editorWidget = qobject_cast<TextEditor::TextEditorWidget*>(editor->widget());
    QString fileName("Processing " + editor->document()->filePath().fileName() + "...");

    uint count = 0;
    emit message(fileName);

    if (symCount > symMin) {
        m_fileProgress->setLabelText(fileName);
        m_fileProgress->setMaximum(symCount);
        m_fileProgress->setValue(0);
        m_fileProgress->show();
    }

    if (editorWidget) {
        QList<const Symbol*>::iterator it = symmap.end();
        for (int i = 0; it != symmap.begin(); --it) {
            if (symCount > symMin && i++ % 20 == 0) // Every n occurences
            {
                if (m_cancel) {
                    break;
                }

                m_fileProgress->setValue(i);
                m_fileProgress->update();
            }
            const Symbol* sym = *(it - 1);
            editorWidget->gotoLine(sym->line());
            if (documentEntity(DoxySettings, editor))
                count++;
        }

        if (DoxySettings.fileCommentsEnabled) {
            if (addFileComment(DoxySettings, editor))
                count++;
        }
    }

    if (symCount > symMin)
        m_fileProgress->setValue(symCount);

    return count;
}

// TODO: fix this! Unused at the moment.
uint Doxygen::documentSpecificProject(const DoxygenSettingsStruct& DoxySettings)
{
    return documentProject(ProjectExplorer::SessionManager::startupProject(), DoxySettings);
}

uint Doxygen::documentCurrentProject(const DoxygenSettingsStruct& DoxySettings)
{
    return documentProject(ProjectExplorer::ProjectTree::currentProject(), DoxySettings);
}

void Doxygen::cancelOperation()
{
    m_cancel = true;
    emit message("Operation canceled");
}

uint Doxygen::documentProject(ProjectExplorer::Project* p, const DoxygenSettingsStruct& DoxySettings)
{
    // prevent a crash if user launches this command with no project opened
    // You don't need to have an editor open for that.
    if (!p) {
        QMessageBox::warning((QWidget*)parent(),
            tr("Doxygen"),
            tr("You don't have any current project."),
            QMessageBox::Close, QMessageBox::NoButton);
        return 0;
    }

    uint count = 0;
    m_cancel = false;
    Core::EditorManager* editorManager = Core::EditorManager::instance();

    QStringList files;

    DoxygenFilesDialog* dialog = new DoxygenFilesDialog();
    dialog->initFileTree(p->rootProjectNode());

    if (dialog->exec() != QDialog::Accepted) {
        delete dialog;
        return 0;
    }
    dialog->getFilesList(&files);
    delete dialog;

    m_projectProgress->setMaximum(files.size());
    m_projectProgress->show();

    for (int i = 0; i < files.size(); ++i) {
        bool documented = false;
        m_projectProgress->setValue(i);
        //qDebug() << "Current file:" << i;
        if (m_cancel) {
            break;
        }

        QFileInfo fileInfo(files[i]);
        QString fileExtension = fileInfo.suffix();
        //qDebug() << "Current file:" << files.at(i);
        if (
            (
                (DoxySettings.fcomment == headers /*|| DoxySettings.fcomment == bothqt*/ || DoxySettings.fcomment == all)
                && (fileExtension == "hpp" || fileExtension == "h"))
            || ((DoxySettings.fcomment == implementations /*|| DoxySettings.fcomment == bothqt*/ || DoxySettings.fcomment == all)
                   && (fileExtension == "cpp" || fileExtension == "c"))) { /*|| ( //TODO: add documentation of QML files (see doxyqml comments interpretation)
                    (DoxySettings.fcomment == qml || DoxySettings.fcomment == all)
                    && fileExtension == "qml"
                    )
                ) {*/
            Core::IEditor* editor = editorManager->openEditor(files[i], Utils::Id(),
                Core::EditorManager::DoNotChangeCurrentEditor
                    | Core::EditorManager::IgnoreNavigationHistory
                    | Core::EditorManager::DoNotMakeVisible);
            if (editor) {
                documented = true;
                count += documentFile(DoxySettings, editor);
            }
        }

        if (DoxySettings.fileCommentsEnabled && documented == false) {
            bool commentFile = false;
            //qDebug() << "FileCommentHeaders: " << DoxySettings.fileCommentHeaders;
            //qDebug() << "FileCommentImpl: " << DoxySettings.fileCommentImpl;
            if (DoxySettings.fileCommentHeaders && (fileExtension == "hpp" || fileExtension == "h")) {
                commentFile = true;
            } else if (DoxySettings.fileCommentImpl && (fileExtension == "cpp" || fileExtension == "c")) {
                commentFile = true;
            }

            if (commentFile) {
                Core::IEditor* editor = editorManager->openEditor(files[i]);
                if (editor)
                    count += addFileComment(DoxySettings, editor);
            }
        }
    }
    m_projectProgress->setValue(files.size());

    QString msg;
    msg.sprintf("Doxygen blocs generated: %u", count);
    emit message(msg);

    return count;
}

QString Doxygen::getProjectRoot()
{
    QString projectRoot;
    Project* proj = ProjectTree::currentProject();
    if (proj) {
        projectRoot = proj->projectDirectory().toString() + "/";
    }

    return projectRoot;
}
