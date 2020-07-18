#include "R2GraphWidget.h"
#include "ui_R2GraphWidget.h"

#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

R2GraphWidget::R2GraphWidget(MainWindow *main)
    : CutterDockWidget(main)
    , ui(new Ui::R2GraphWidget)
    , graphView(new GenericR2GraphView(this, main))
{
    ui->setupUi(this);
    ui->verticalLayout->addWidget(graphView);
    connect(ui->refreshButton, &QPushButton::pressed, this, [this]() {
        graphView->refreshView();
    });
    struct GraphType {
        QChar commandChar;
        QString label;
    } types[] = {
        {'a', tr("aga - Data reference graph")},
        {'A', tr("agA - Global data references graph")},
        // {'c', tr("c - Function callgraph")},
        // {'C', tr("C - Global callgraph")},
        // {'f', tr("f - Basic blocks function graph")},
        {'i', tr("agi - Imports graph")},
        {'r', tr("agr - References graph")},
        {'R', tr("agR - Global references graph")},
        {'x', tr("agx - Cross references graph")},
        {'g', tr("agg - Custom graph")},
        {' ', tr("User command")},
    };
    for (auto &graphType : types) {
        if (graphType.commandChar != ' ') {
            ui->graphType->addItem(graphType.label, graphType.commandChar);
        } else {
            ui->graphType->addItem(graphType.label, QVariant());
        }

    }
    connect<void(QComboBox::*)(int)>(ui->graphType, &QComboBox::currentIndexChanged, this, &R2GraphWidget::typeChanged);
    connect(ui->customCommand, &QLineEdit::textEdited, this, [this](){
        graphView->setGraphCommand(ui->customCommand->text());
    });
    connect(ui->customCommand, &QLineEdit::returnPressed, this, [this](){
        graphView->setGraphCommand(ui->customCommand->text());
        graphView->refreshView();
    });
    ui->customCommand->setVisible(false);
}

R2GraphWidget::~R2GraphWidget()
{
}

void R2GraphWidget::typeChanged()
{
    auto currentData = ui->graphType->currentData();
    if (currentData.isNull()) {
        ui->customCommand->setVisible(true);
        graphView->setGraphCommand(ui->customCommand->text());
        ui->customCommand->setFocus();
    } else {
        ui->customCommand->setVisible(false);
        auto command = QString("ag%1").arg(currentData.toChar());
        graphView->setGraphCommand(command);
        graphView->refreshView();
    }
}

GenericR2GraphView::GenericR2GraphView(R2GraphWidget *parent, MainWindow *main)
    : SimpleTextGraphView(parent, main)
    , refreshDeferrer(nullptr, this)
{
    refreshDeferrer.registerFor(parent);
    connect(&refreshDeferrer, &RefreshDeferrer::refreshNow, this, &GenericR2GraphView::refreshView);
}

void GenericR2GraphView::setGraphCommand(QString cmd)
{
    graphCommand = cmd;
}

void GenericR2GraphView::refreshView()
{
    if (!refreshDeferrer.attemptRefresh(nullptr)) {
        return;
    }
    SimpleTextGraphView::refreshView();
}

void GenericR2GraphView::loadCurrentGraph()
{
    blockContent.clear();
    blocks.clear();

    if (graphCommand.isEmpty()) {
        return;
    }

    QJsonDocument functionsDoc = Core()->cmdj(QString("%1j").arg(graphCommand));
    auto nodes = functionsDoc.object()["nodes"].toArray();

    for (const QJsonValueRef &value : nodes) {
        QJsonObject block = value.toObject();
        uint64_t id = block["id"].toVariant().toULongLong();

        QString content;
        QString title = block["title"].toString();
        QString body = block["body"].toString();
        if (!title.isEmpty() && !body.isEmpty()) {
            content = title + "/n" + body;
        } else {
            content = title + body;
        }

        auto edges = block["out_nodes"].toArray();
        GraphLayout::GraphBlock layoutBlock;
        layoutBlock.entry = id;
        for (auto edge : edges) {
            auto targetId = edge.toVariant().toULongLong();
            layoutBlock.edges.emplace_back(targetId);
        }

        addBlock(std::move(layoutBlock), content);
    }

    cleanupEdges(blocks);

    computeGraphPlacement();

    if (graphCommand != lastShownCommand) {
        selectedBlock = NO_BLOCK_SELECTED;
        lastShownCommand = graphCommand;
        center();
    }
}
