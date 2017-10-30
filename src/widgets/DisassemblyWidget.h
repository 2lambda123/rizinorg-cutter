#ifndef DISASSEMBLYVIEW_H
#define DISASSEMBLYVIEW_H

#include "cutter.h"
#include <QDockWidget>
#include <QTextEdit>
#include <QShortcut>

class DisassemblyWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DisassemblyWidget(QWidget *parent = nullptr);
    explicit DisassemblyWidget(const QString &title, QWidget *parent = nullptr);
    QWidget* getTextWidget();

signals:

public slots:
    void highlightCurrentLine();
    void disasmScrolled();
    void showDisasContextMenu(const QPoint &pt);
    void cursorPositionChanged();
    void on_seekChanged(RVA offset);
    void refreshDisasm();
    void fontsUpdatedSlot();
    void showXrefsDialog();

private:
    QTextEdit *mDisasTextEdit;

    QString readDisasm(RVA offset, bool backwards = false, bool skipFirstInstruction = false);
    RVA readCurrentDisassemblyOffset();
    bool loadMoreDisassembly();
    void highlightDisasms();
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // DISASSEMBLYVIEW_H
