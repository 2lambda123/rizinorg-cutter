#include "SdbDock.h"
#include "ui_SdbDock.h"

#include "MainWindow.h"

#include <QDebug>
#include <QTreeWidget>


SdbDock::SdbDock(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::SdbDock)
{
    ui->setupUi(this);

    path = "";

    connect(Core(), SIGNAL(refreshAll()), this, SLOT(reload()));
    reload(nullptr);
}

void SdbDock::reload(QString _path)
{
    if (!_path.isNull())
    {
        path = _path;
    }

    ui->lineEdit->setText(path);
    /* insert root sdb keyvalue pairs */

    ui->treeWidget->clear();
    QList<QString> keys;
    /* key-values */
    keys = CutterCore::getInstance()->sdbListKeys(path);
    foreach (QString key, keys)
    {
        QTreeWidgetItem *tempItem = new QTreeWidgetItem();
        tempItem->setText(0, key);
        tempItem->setText(1, CutterCore::getInstance()->sdbGet(path, key));
        tempItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable);
        ui->treeWidget->insertTopLevelItem(0, tempItem);
    }
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);
    /* namespaces */
    keys = CutterCore::getInstance()->sdbList(path);
    keys.append("..");
    foreach (QString key, keys)
    {
        QTreeWidgetItem *tempItem = new QTreeWidgetItem();
        tempItem->setText(0, key + "/");
        tempItem->setText(1, "");
        ui->treeWidget->insertTopLevelItem(0, tempItem);
    }
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);
}


void SdbDock::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QString newpath;

    if (column == 0)
    {
        if (item->text(0) == "../")
        {
            int idx = path.lastIndexOf("/");
            if (idx != -1)
            {
                newpath = path.mid(0, idx);
            }
            else
            {
                newpath = "";
            }
            reload(newpath);

        }
        else if (item->text(0).indexOf("/") != -1)
        {
            if (path != "")
            {
                newpath = path + "/" + item->text(0).replace("/", "");
            }
            else
            {
                newpath = path + item->text(0).replace("/", "");
            }
            // enter directory
            reload(newpath);
        }
    }
}

SdbDock::~SdbDock() {}

void SdbDock::on_lockButton_clicked()
{
    if (ui->lockButton->isChecked())
    {
        this->setAllowedAreas(Qt::NoDockWidgetArea);
        ui->lockButton->setIcon(QIcon(":/lock"));
    }
    else
    {
        this->setAllowedAreas(Qt::AllDockWidgetAreas);
        ui->lockButton->setIcon(QIcon(":/unlock"));
    }
}

void SdbDock::on_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    Core()->sdbSet(path, item->text(0), item->text(column));
}
