#include "core/MainWindow.h"
#include "GraphWidget.h"
#include "DisassemblerGraphView.h"
#include "WidgetShortcuts.h"
#include <QVBoxLayout>

GraphWidget::GraphWidget(MainWindow *main, QAction *action) :
    MemoryDockWidget(CutterCore::MemoryWidgetType::Graph, main, action)
{
    setObjectName(main
                  ? main->getUniqueObjectName(getWidgetType())
                  : getWidgetType());

    setAllowedAreas(Qt::AllDockWidgetAreas);

    auto *layoutWidget = new QWidget(this);
    setWidget(layoutWidget);
    auto *layout = new QVBoxLayout(layoutWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layoutWidget->setLayout(layout);

    header = new QLineEdit(this);
    header->setReadOnly(true);
    layout->addWidget(header);

    graphView = new DisassemblerGraphView(layoutWidget, seekable);
    layout->addWidget(graphView);

    // getting the name of the class is implementation defined, and cannot be
    // used reliably across different compilers.
    //QShortcut *toggle_shortcut = new QShortcut(widgetShortcuts[typeid(this).name()], main);
    QShortcut *toggle_shortcut = new QShortcut(widgetShortcuts["GraphWidget"], main);
    connect(toggle_shortcut, &QShortcut::activated, this, [ = ]() {
            toggleDockWidget(true); 
            main->updateDockActionChecked(action);
    });

    connect(graphView, &DisassemblerGraphView::nameChanged, this, &MemoryDockWidget::updateWindowTitle);

    connect(this, &QDockWidget::visibilityChanged, this, [ = ](bool visibility) {
        main->toggleOverview(visibility, this);
        if (visibility) {
            Core()->setMemoryWidgetPriority(CutterCore::MemoryWidgetType::Graph);
            graphView->onSeekChanged(Core()->getOffset());
        }
    });

    connect(graphView, &DisassemblerGraphView::graphMoved, this, [ = ]() {
        main->toggleOverview(true, this);
    });
    connect(seekable, &CutterSeekable::seekableSeekChanged, this, &GraphWidget::prepareHeader);
    connect(Core(), &CutterCore::functionRenamed, this, &GraphWidget::prepareHeader);
}

QWidget *GraphWidget::widgetToFocusOnRaise()
{
    return graphView;
}

void GraphWidget::closeEvent(QCloseEvent *event)
{
    CutterDockWidget::closeEvent(event);
    emit graphClosed();
}

QString GraphWidget::getWindowTitle() const
{
    return graphView->windowTitle;
}

DisassemblerGraphView *GraphWidget::getGraphView() const
{
    return graphView;
}

QString GraphWidget::getWidgetType()
{
    return "Graph";
}

void GraphWidget::prepareHeader()
{
    QString afcf = Core()->cmd(QString("afcf @%1").arg(seekable->getOffset())).trimmed();
    if (afcf.isEmpty()) {
        header->hide();
        return;
    }
    header->show();
    header->setText(afcf);
}

