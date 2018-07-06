#pragma once

#include <QJsonObject>
#include <memory>
#include <QStandardItem>
#include <QTableView>

#include "Cutter.h"
#include "CutterDockWidget.h"

class MainWindow;

namespace Ui {
class BacktraceWidget;
}

class BacktraceWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit BacktraceWidget(MainWindow *main);
    ~BacktraceWidget();

private slots:
    void updateContents();
    void setBacktraceGrid();

private:
    std::unique_ptr<Ui::BacktraceWidget> ui;
    QStandardItemModel *modelBacktrace = new QStandardItemModel(1, 5, this);
    QTableView *viewBacktrace = new QTableView;
};