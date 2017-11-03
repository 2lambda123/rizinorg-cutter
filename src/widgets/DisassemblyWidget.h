#ifndef DISASSEMBLYWIDGET_H
#define DISASSEMBLYWIDGET_H

#include "cutter.h"
#include <QDockWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QShortcut>


class DisassemblyTextEdit;
class DisassemblyScrollArea;
class DisassemblyContextMenu;

class DisassemblyWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DisassemblyWidget(QWidget *parent = nullptr);
    explicit DisassemblyWidget(const QString &title, QWidget *parent = nullptr);
    QWidget* getTextWidget();

public slots:
    void highlightCurrentLine();
    void showDisasContextMenu(const QPoint &pt);
    void on_seekChanged(RVA offset);
    void refreshDisasm(RVA offset = RVA_INVALID);
    void fontsUpdatedSlot();

private slots:
    void scrollInstructions(int count);
    void updateMaxLines();

    void cursorPositionChanged();

private:
    DisassemblyContextMenu *mCtxMenu;
    DisassemblyScrollArea *mDisasScrollArea;
    DisassemblyTextEdit *mDisasTextEdit;

    RVA topOffset;
    RVA bottomOffset;
    int maxLines;

    QString readDisasm(const QString &cmd, bool stripLastNewline);
    RVA readCurrentDisassemblyOffset();
    bool eventFilter(QObject *obj, QEvent *event);

    void updateCursorPosition();

    void connectCursorPositionChanged(bool disconnect);
};

class DisassemblyScrollArea : public QAbstractScrollArea
{
    Q_OBJECT

public:
    explicit DisassemblyScrollArea(QWidget *parent = nullptr);

signals:
    void scrollLines(int lines);
    void disassemblyResized();

protected:
    bool viewportEvent(QEvent *event) override;

private:
    void resetScrollBars();
};


class DisassemblyTextEdit: public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit DisassemblyTextEdit(QWidget *parent = nullptr)
            : QPlainTextEdit(parent),
              lockScroll(false) {}

    void setLockScroll(bool lock)           { this->lockScroll = lock; }

protected:
    bool viewportEvent(QEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;

private:
    bool lockScroll;
};

#endif // DISASSEMBLYWIDGET_H
