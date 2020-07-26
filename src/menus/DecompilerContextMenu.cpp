#include "DecompilerContextMenu.h"
#include "dialogs/preferences/PreferencesDialog.h"
#include "MainWindow.h"
#include "dialogs/BreakpointsDialog.h"
#include "dialogs/CommentsDialog.h"

#include <QtCore>
#include <QShortcut>
#include <QJsonArray>
#include <QClipboard>
#include <QApplication>
#include <QPushButton>
#include <QInputDialog>

DecompilerContextMenu::DecompilerContextMenu(QWidget *parent, MainWindow *mainWindow)
    :   QMenu(parent),
        curHighlightedWord(QString()),
        offset(0),
        isTogglingBreakpoints(false),
        mainWindow(mainWindow),
        annotationHere(nullptr),
        actionCopy(tr("Copy"), this),
        actionCopyInstructionAddress(tr("Copy instruction address (<address>)"), this),
        actionCopyReferenceAddress(tr("Copy address of [flag] (<address>)"), this),
        actionShowInSubmenu(tr("Show in"), this),
        actionAddComment(tr("Add Comment"), this),
        actionDeleteComment(tr("Delete comment"), this),
        actionRenameThingHere(tr("Rename function at cursor"), this),
        actionDeleteName(tr("Delete <name>"), this),
        actionToggleBreakpoint(tr("Add/remove breakpoint"), this),
        actionAdvancedBreakpoint(tr("Advanced breakpoint"), this),
        breakpointsInLineMenu(new QMenu(this)),
        actionContinueUntil(tr("Continue until line"), this),
        actionSetPC(tr("Set PC"), this)
{
    setActionCopy(); // Sets all three copy actions
    addSeparator();

    setActionShowInSubmenu();
    copySeparator = addSeparator();

    setActionAddComment();
    setActionDeleteComment();

    setActionRenameThingHere();
    setActionDeleteName();

    addSeparator();
    addBreakpointMenu();
    addDebugMenu();

    setShortcutContextInActions(this);

    connect(this, &DecompilerContextMenu::aboutToShow,
            this, &DecompilerContextMenu::aboutToShowSlot);
    connect(this, &DecompilerContextMenu::aboutToHide,
            this, &DecompilerContextMenu::aboutToHideSlot);
}

DecompilerContextMenu::~DecompilerContextMenu()
{
}

void DecompilerContextMenu::setAnnotationHere(RCodeAnnotation *annotation)
{
    this->annotationHere = annotation;
}

void DecompilerContextMenu::setCurHighlightedWord(QString word)
{
    this->curHighlightedWord = word;
}

void DecompilerContextMenu::setOffset(RVA offset)
{
    this->offset = offset;

    // this->actionSetFunctionVarTypes.setVisible(true);
}

void DecompilerContextMenu::setFirstOffsetInLine(RVA firstOffset)
{
    this->firstOffsetInLine = firstOffset;
}

void DecompilerContextMenu::setAvailableBreakpoints(QVector<RVA> offsetList)
{
    this->availableBreakpoints = offsetList;
}

void DecompilerContextMenu::setupBreakpointsInLineMenu()
{
    breakpointsInLineMenu->clear();
    for (auto curOffset : this->availableBreakpoints) {
        QAction *action = breakpointsInLineMenu->addAction(RAddressString(curOffset));
        connect(action, &QAction::triggered, this, [this, curOffset] {
            BreakpointsDialog::editBreakpoint(Core()->getBreakpointAt(curOffset),
                                              this);
        });
    }
}

void DecompilerContextMenu::setCanCopy(bool enabled)
{
    // actionCopy.setVisible(enabled);
    // actionCopyInstructionAddress.setVisible(!enabled);
    if (enabled) {
        actionCopy.setText("Copy");
    } else {
        actionCopy.setText("Copy this line");
    }
}

void DecompilerContextMenu::setShortcutContextInActions(QMenu *menu)
{
    for (QAction *action : menu->actions()) {
        if (action->isSeparator()) {
            //Do nothing
        } else if (action->menu()) {
            setShortcutContextInActions(action->menu());
        } else {
            action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        }
    }
}

void DecompilerContextMenu::setIsTogglingBreakpoints(bool isToggling)
{
    this->isTogglingBreakpoints = isToggling;
}

bool DecompilerContextMenu::getIsTogglingBreakpoints()
{
    return this->isTogglingBreakpoints;
}

void DecompilerContextMenu::aboutToHideSlot()
{
    actionAddComment.setVisible(true);
    actionRenameThingHere.setVisible(true);
    actionDeleteName.setVisible(false);
}

void DecompilerContextMenu::aboutToShowSlot()
{
    if (this->firstOffsetInLine != RVA_MAX) {
        actionShowInSubmenu.setVisible(true);
        QString comment = Core()->cmdRawAt("CC.", this->firstOffsetInLine);
        actionAddComment.setVisible(true);
        if (comment.isEmpty()) {
            actionDeleteComment.setVisible(false);
            actionAddComment.setText(tr("Add Comment"));
        } else {
            actionDeleteComment.setVisible(true);
            actionAddComment.setText(tr("Edit Comment"));
        }
    } else {
        actionShowInSubmenu.setVisible(false);
        actionAddComment.setVisible(false);
        actionDeleteComment.setVisible(false);
    }


    setupBreakpointsInLineMenu();

    // Only show debug options if we are currently debugging
    debugMenu->menuAction()->setVisible(Core()->currentlyDebugging);

    bool hasBreakpoint = !this->availableBreakpoints.isEmpty();
    int numberOfBreakpoints = this->availableBreakpoints.size();
    if (numberOfBreakpoints == 0) {
        actionToggleBreakpoint.setText(tr("Add breakpoint"));
    } else if (numberOfBreakpoints == 1) {
        actionToggleBreakpoint.setText(tr("Remove breakpoint"));
    } else {
        actionToggleBreakpoint.setText(tr("Remove all breakpoints in line"));
    }

    if (numberOfBreakpoints > 1) {
        actionAdvancedBreakpoint.setMenu(breakpointsInLineMenu);
    } else {
        actionAdvancedBreakpoint.setMenu(nullptr);
    }
    actionAdvancedBreakpoint.setText(hasBreakpoint ?
                                     tr("Edit breakpoint") : tr("Advanced breakpoint"));

    QString progCounterName = Core()->getRegisterName("PC").toUpper();
    actionSetPC.setText(tr("Set %1 here").arg(progCounterName));

    if (!annotationHere
            || annotationHere->type ==
            R_CODE_ANNOTATION_TYPE_CONSTANT_VARIABLE) { // To be considered as invalid
        actionRenameThingHere.setVisible(false);
        copySeparator->setVisible(false);
    } else {
        copySeparator->setVisible(true);
        if (annotationHere->type == R_CODE_ANNOTATION_TYPE_FUNCTION_NAME) {
            actionRenameThingHere.setVisible(true);
            actionRenameThingHere.setText(tr("Rename function %1").arg(QString(
                                                                           annotationHere->reference.name)));
        }
        if (annotationHere->type == R_CODE_ANNOTATION_TYPE_GLOBAL_VARIABLE) {
            RFlagItem *flagDetails = r_flag_get_i(Core()->core()->flags, annotationHere->reference.offset);
            if (flagDetails) {
                actionRenameThingHere.setText(tr("Rename %1").arg(QString(flagDetails->name)));
                actionDeleteName.setText(tr("Remove %1").arg(QString(flagDetails->name)));
                actionDeleteName.setVisible(true);
            } else {
                if (Core()->getConfig("r2ghidra.rawptr")== "true") {
                    actionRenameThingHere.setText(tr("Add name"));
                } else {
                    actionRenameThingHere.setText(tr("Rename %1").arg(curHighlightedWord));
                }
            }
        }
    }
    actionCopyInstructionAddress.setText(tr("Copy instruction address (%1)").arg(RAddressString(
                                                                                     offset)));
    bool isReference = false;
    if (annotationHere) {
        isReference = (annotationHere->type == R_CODE_ANNOTATION_TYPE_GLOBAL_VARIABLE
                       || annotationHere->type == R_CODE_ANNOTATION_TYPE_CONSTANT_VARIABLE
                       || annotationHere->type == R_CODE_ANNOTATION_TYPE_FUNCTION_NAME);
    }
    if (isReference) {
        actionCopyReferenceAddress.setVisible(true);
        RVA referenceAddr = annotationHere->reference.offset;
        RFlagItem *flagDetails = r_flag_get_i(Core()->core()->flags, referenceAddr);
        if (annotationHere->type == R_CODE_ANNOTATION_TYPE_FUNCTION_NAME) {
            actionCopyReferenceAddress.setText(tr("Copy address of %1 (%2)").arg
                                               (QString(annotationHere->reference.name), RAddressString(referenceAddr)));
        } else if (flagDetails) {
            actionCopyReferenceAddress.setText(tr("Copy address of %1 (%2)").arg
                                               (flagDetails->name, RAddressString(referenceAddr)));
        } else {
            actionCopyReferenceAddress.setText(tr("Copy address (%1)").arg(RAddressString(referenceAddr)));
        }
    } else {
        actionCopyReferenceAddress.setVisible(false);
    }
    if (actionShowInSubmenu.menu() != nullptr) {
        actionShowInSubmenu.menu()->deleteLater();
    }
    actionShowInSubmenu.setMenu(mainWindow->createShowInMenu(this, offset));
    updateTargetMenuActions();
}

// Set up actions


void DecompilerContextMenu::setActionCopy() // Set all three copy actions
{
    connect(&actionCopy, &QAction::triggered, this, &DecompilerContextMenu::actionCopyTriggered);
    addAction(&actionCopy);
    actionCopy.setShortcut(QKeySequence::Copy);

    connect(&actionCopyInstructionAddress, &QAction::triggered, this,
            &DecompilerContextMenu::actionCopyInstructionAddressTriggered);
    addAction(&actionCopyInstructionAddress);
    // actionCopyInstructionAddress.setShortcut(QKeySequence::Copy);

    connect(&actionCopyReferenceAddress, &QAction::triggered, this,
            &DecompilerContextMenu::actionCopyReferenceAddressTriggered);
    addAction(&actionCopyReferenceAddress);
    actionCopyReferenceAddress.setShortcut({Qt::CTRL + Qt::SHIFT + Qt::Key_C});
}

void DecompilerContextMenu::setActionShowInSubmenu()
{
    addAction(&actionShowInSubmenu);
}

void DecompilerContextMenu::setActionAddComment()
{
    connect(&actionAddComment, &QAction::triggered, this,
            &DecompilerContextMenu::actionAddCommentTriggered);
    addAction(&actionAddComment);
    actionAddComment.setShortcut(Qt::Key_Semicolon);
}

void DecompilerContextMenu::setActionDeleteComment()
{
    connect(&actionDeleteComment, &QAction::triggered, this,
            &DecompilerContextMenu::actionDeleteCommentTriggered);
    addAction(&actionDeleteComment);
}

void DecompilerContextMenu::setActionRenameThingHere()
{
    actionRenameThingHere.setShortcut({Qt::Key_N});
    connect(&actionRenameThingHere, &QAction::triggered, this,
            &DecompilerContextMenu::actionRenameThingHereTriggered);
    addAction(&actionRenameThingHere);
}

void DecompilerContextMenu::setActionDeleteName()
{
    connect(&actionDeleteName, &QAction::triggered, this,
            &DecompilerContextMenu::actionDeleteNameTriggered);
    addAction(&actionDeleteName);
    actionDeleteName.setVisible(false);
}

void DecompilerContextMenu::setActionToggleBreakpoint()
{
    connect(&actionToggleBreakpoint, &QAction::triggered, this,
            &DecompilerContextMenu::actionToggleBreakpointTriggered);
    actionToggleBreakpoint.setShortcuts({Qt::Key_F2, Qt::CTRL + Qt::Key_B});
}

void DecompilerContextMenu::setActionAdvancedBreakpoint()
{
    connect(&actionAdvancedBreakpoint, &QAction::triggered, this,
            &DecompilerContextMenu::actionAdvancedBreakpointTriggered);
    actionAdvancedBreakpoint.setShortcut({Qt::CTRL + Qt::Key_F2});
}

void DecompilerContextMenu::setActionContinueUntil()
{
    connect(&actionContinueUntil, &QAction::triggered, this,
            &DecompilerContextMenu::actionContinueUntilTriggered);
}

void DecompilerContextMenu::setActionSetPC()
{
    connect(&actionSetPC, &QAction::triggered, this, &DecompilerContextMenu::actionSetPCTriggered);
}

// Set up action responses

void DecompilerContextMenu::actionCopyTriggered()
{
    emit copy();
}

void DecompilerContextMenu::actionCopyInstructionAddressTriggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(RAddressString(offset));
}

void DecompilerContextMenu::actionCopyReferenceAddressTriggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(RAddressString(annotationHere->reference.offset));
}

void DecompilerContextMenu::actionAddCommentTriggered()
{
    CommentsDialog::addOrEditComment(this->firstOffsetInLine, this);
}

void DecompilerContextMenu::actionDeleteCommentTriggered()
{
    Core()->delComment(this->firstOffsetInLine);
}

void DecompilerContextMenu::actionRenameThingHereTriggered()
{
    if (!annotationHere) {
        return;
    }
    RCoreLocked core = Core()->core();
    bool ok;
    auto type = annotationHere->type;
    if (type == R_CODE_ANNOTATION_TYPE_FUNCTION_NAME) {
        QString currentName(annotationHere->reference.name);
        RVA func_addr = annotationHere->reference.offset;
        RAnalFunction *func = Core()->functionAt(func_addr);
        if (func == NULL) {
            QString function_name = QInputDialog::getText(this,
                                                          tr("Define this function at %2").arg(RAddressString(func_addr)),
                                                          tr("Function name:"), QLineEdit::Normal, currentName, &ok);
            if (ok && !function_name.isEmpty()) {
                Core()->createFunctionAt(func_addr, function_name);
            }
        } else {
            QString newName = QInputDialog::getText(this, tr("Rename function %2").arg(currentName),
                                                    tr("Function name:"), QLineEdit::Normal, currentName, &ok);
            if (ok && !newName.isEmpty()) {
                Core()->renameFunction(func_addr, newName);
            }
        }

    } else if (type == R_CODE_ANNOTATION_TYPE_GLOBAL_VARIABLE) {
        RVA var_addr = annotationHere->reference.offset;
        RFlagItem *flagDetails = r_flag_get_i(core->flags, var_addr);
        if (flagDetails) {
            QString newName = QInputDialog::getText(this, tr("Rename %2").arg(flagDetails->name),
                                                    tr("Enter name"), QLineEdit::Normal, flagDetails->name, &ok);
            if (ok && !newName.isEmpty()) {
                Core()->renameFlag(flagDetails->name, newName);
            }
        } else {
            if (Core()->getConfig("r2ghidra.rawptr") == "true") {
                QString newName = QInputDialog::getText(this, tr("Add name"), tr("Enter name"), QLineEdit::Normal,
                                                        QString(), &ok);
                if (ok && !newName.isEmpty()) {
                    Core()->addFlag(var_addr, newName, 1);
                }
            } else {
            QString newName = QInputDialog::getText(this, tr("Rename %2").arg(curHighlightedWord),
                                                    tr("Enter name"), QLineEdit::Normal, curHighlightedWord, &ok);
                if (ok && !newName.isEmpty()) {
                    Core()->addFlag(var_addr, newName, 1);
                }
            }
        }

    }
}

void DecompilerContextMenu::actionDeleteNameTriggered()
{
    Core()->delFlag(annotationHere->reference.offset);
}

void DecompilerContextMenu::actionToggleBreakpointTriggered()
{
    if (!this->availableBreakpoints.isEmpty()) {
        setIsTogglingBreakpoints(true);
        for (auto offsetToRemove : this->availableBreakpoints) {
            Core()->toggleBreakpoint(offsetToRemove);
        }
        this->availableBreakpoints.clear();
        setIsTogglingBreakpoints(false);
        return;
    }
    if (this->firstOffsetInLine == RVA_MAX)
        return;

    Core()->toggleBreakpoint(this->firstOffsetInLine);
}

void DecompilerContextMenu::actionAdvancedBreakpointTriggered()
{
    if (!availableBreakpoints.empty()) {
        // Edit the earliest breakpoint in the line
        BreakpointsDialog::editBreakpoint(Core()->getBreakpointAt(this->availableBreakpoints.first()),
                                          this);
    } else {
        // Add a breakpoint to the earliest offset in the line
        BreakpointsDialog::createNewBreakpoint(this->firstOffsetInLine, this);
    }
}

void DecompilerContextMenu::actionContinueUntilTriggered()
{
    Core()->continueUntilDebug(RAddressString(offset));
}

void DecompilerContextMenu::actionSetPCTriggered()
{
    QString progCounterName = Core()->getRegisterName("PC");
    Core()->setRegister(progCounterName, RAddressString(offset).toUpper());
}

// Set up menus

void DecompilerContextMenu::addBreakpointMenu()
{
    breakpointMenu = addMenu(tr("Breakpoint"));

    setActionToggleBreakpoint();
    breakpointMenu->addAction(&actionToggleBreakpoint);
    setActionAdvancedBreakpoint();
    breakpointMenu->addAction(&actionAdvancedBreakpoint);
}

void DecompilerContextMenu::addDebugMenu()
{
    debugMenu = addMenu(tr("Debug"));

    setActionContinueUntil();
    debugMenu->addAction(&actionContinueUntil);
    setActionSetPC();
    debugMenu->addAction(&actionSetPC);
}

void DecompilerContextMenu::updateTargetMenuActions()
{
    for (auto action : showTargetMenuActions) {
        removeAction(action);
        auto menu = action->menu();
        if (menu) {
            menu->deleteLater();
        }
        action->deleteLater();
    }
    showTargetMenuActions.clear();
    RCoreLocked core = Core()->core();
    if (annotationHere && (annotationHere->type == R_CODE_ANNOTATION_TYPE_GLOBAL_VARIABLE
                           || annotationHere->type == R_CODE_ANNOTATION_TYPE_CONSTANT_VARIABLE
                           || annotationHere->type == R_CODE_ANNOTATION_TYPE_FUNCTION_NAME)) {
        QString name;
        if (annotationHere->type == R_CODE_ANNOTATION_TYPE_GLOBAL_VARIABLE
                || annotationHere->type == R_CODE_ANNOTATION_TYPE_CONSTANT_VARIABLE) {
            RVA var_addr = annotationHere->reference.offset;
            RFlagItem *flagDetails = r_flag_get_i(core->flags, var_addr);
            if (flagDetails) {
                name = tr("Show %1 in").arg(flagDetails->name);
            } else {
                name = tr("Show %1 in").arg(RAddressString(annotationHere->reference.offset));
            }
        } else if (annotationHere->type == R_CODE_ANNOTATION_TYPE_FUNCTION_NAME) {
            name = tr("%1 (%2)").arg(QString(annotationHere->reference.name),
                                     RAddressString(annotationHere->reference.offset));
        }
        auto action = new QAction(name, this);
        showTargetMenuActions.append(action);
        auto menu = mainWindow->createShowInMenu(this, annotationHere->reference.offset,
                                                 annotationHere->type);
        action->setMenu(menu);
        insertActions(copySeparator, showTargetMenuActions);
    }
}
