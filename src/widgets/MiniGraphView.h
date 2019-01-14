#ifndef MINIGRAPHVIEW_H
#define MINIGRAPHVIEW_H

#include <QWidget>
#include <QPainter>
#include <QShortcut>
#include <QLabel>
#include <QRect>

#include "widgets/GraphView.h"
#include "menus/DisassemblyContextMenu.h"
#include "common/RichTextPainter.h"
#include "CutterSeekableWidget.h"

class QTextEdit;
class SyntaxHighlighter;

class MiniGraphView : public GraphView
{
    Q_OBJECT

    struct Text {
        std::vector<RichTextPainter::List> lines;

        Text() {}

        Text(const QString &text, QColor color, QColor background)
        {
            RichTextPainter::List richText;
            RichTextPainter::CustomRichText_t rt;
            rt.highlight = false;
            rt.text = text;
            rt.textColor = color;
            rt.textBackground = background;
            rt.flags = rt.textBackground.alpha() ? RichTextPainter::FlagAll : RichTextPainter::FlagColor;
            richText.push_back(rt);
            lines.push_back(richText);
        }

        Text(const RichTextPainter::List &richText)
        {
            lines.push_back(richText);
        }

        QString ToQString() const
        {
            QString result;
            for (const auto &line : lines) {
                for (const auto &t : line) {
                    result += t.text;
                }
            }
            return result;
        }
    };

    struct Instr {
        ut64 addr = 0;
        ut64 size = 0;
        Text text;
        Text fullText;
        QString plainText;
        std::vector<unsigned char> opcode; //instruction bytes
    };

    struct Token {
        int start;
        int length;
        QString type;
        Instr *instr;
        QString name;
        QString content;
    };

    struct DisassemblyBlock {
        Text header_text;
        std::vector<Instr> instrs;
        ut64 entry = 0;
        ut64 true_path = 0;
        ut64 false_path = 0;
        bool terminal = false;
        bool indirectcall = false;
    };

    struct Function {
        bool ready;
        ut64 entry;
        ut64 update_id;
        std::vector<DisassemblyBlock> blocks;
    };

    struct Analysis {
        ut64 entry = 0;
        std::unordered_map<ut64, Function> functions;
        bool ready = false;
        ut64 update_id = 0;
        QString status = "Analyzing...";

        bool find_instr(ut64 addr, ut64 &func, ut64 &instr)
        {
            //TODO implement
            Q_UNUSED(addr);
            Q_UNUSED(func);
            Q_UNUSED(instr);
            return false;
        }

        //dummy class
    };

signals:
    void mouseMoved();

public:
    MiniGraphView(QWidget *parent);
    ~MiniGraphView() override;
    std::unordered_map<ut64, DisassemblyBlock> disassembly_blocks;
    virtual void drawBlock(QPainter &p, GraphView::GraphBlock &block) override;
    virtual void blockClicked(GraphView::GraphBlock &block, QMouseEvent *event, QPoint pos) override;
    virtual void blockDoubleClicked(GraphView::GraphBlock &block, QMouseEvent *event,
                                    QPoint pos) override;
    virtual bool helpEvent(QHelpEvent *event) override;
    virtual void blockHelpEvent(GraphView::GraphBlock &block, QHelpEvent *event, QPoint pos) override;
    virtual GraphView::EdgeConfiguration edgeConfiguration(GraphView::GraphBlock &from,
                                                           GraphView::GraphBlock *to) override;
    virtual void blockTransitionedTo(GraphView::GraphBlock *to) override;

    void loadCurrentGraph();
    QString windowTitle;
    bool isGraphEmpty();
    QTextEdit *header = nullptr;

    void paintEvent(QPaintEvent *event) override;
    QRect rangeRect;

public slots:
    void refreshView();
    void colorsUpdatedSlot();
    void fontsUpdatedSlot();
    void onSeekChanged(RVA addr);
    void toggleSync();

    void zoomIn(QPoint mouse = QPoint(0, 0));
    void zoomOut(QPoint mouse = QPoint(0, 0));
    void zoomReset();

    void takeTrue();
    void takeFalse();

    void nextInstr();
    void prevInstr();

protected:
    virtual void wheelEvent(QWheelEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void seekPrev();

    void on_actionExportGraph_triggered();

private:
    bool first_draw = true;
    bool transition_dont_seek = false;

    Token *highlight_token;
    // Font data
    CachedFontMetrics *mFontMetrics;
    qreal charWidth;
    int charHeight;
    int charOffset;
    int baseline;
    bool emptyGraph;

    DisassemblyContextMenu *mMenu;

    void connectSeekChanged(bool disconnect);

    void initFont();
    void prepareGraphNode(GraphBlock &block);
    void prepareHeader();
    Token *getToken(Instr *instr, int x);
    RVA getAddrForMouseEvent(GraphBlock &block, QPoint *point);
    Instr *getInstrForMouseEvent(GraphBlock &block, QPoint *point);
    DisassemblyBlock *blockForAddress(RVA addr);
    void seekLocal(RVA addr, bool update_viewport = true);
    void seekInstruction(bool previous_instr);
    CutterSeekableWidget *seekable = nullptr;
    QList<QShortcut *> shortcuts;
    QList<RVA> breakpoints;

    QColor disassemblyBackgroundColor;
    QColor disassemblySelectedBackgroundColor;
    QColor disassemblySelectionColor;
    QColor PCSelectionColor;
    QColor disassemblyTracedColor;
    QColor disassemblyTracedSelectionColor;
    QColor jmpColor;
    QColor brtrueColor;
    QColor brfalseColor;
    QColor retShadowColor;
    QColor indirectcallShadowColor;
    QColor mAutoCommentColor;
    QColor mAutoCommentBackgroundColor;
    QColor mCommentColor;
    QColor mCommentBackgroundColor;
    QColor mLabelColor;
    QColor mLabelBackgroundColor;
    QColor graphNodeColor;
    QColor mAddressColor;
    QColor mAddressBackgroundColor;
    QColor mCipColor;
    QColor mBreakpointColor;
    QColor mDisabledBreakpointColor;

    QAction actionExportGraph;
    QAction actionSyncOffset;

    QLabel *emptyText = nullptr;
    SyntaxHighlighter *highlighter = nullptr;
};

#endif // MINIGRAPHVIEW_H
