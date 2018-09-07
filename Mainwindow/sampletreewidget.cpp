#include "sampletreewidget.h"
#include <QHeaderView>
#include "all_base_struct.h"
#include "DataBase/soaptypingdb.h"
#include <QDebug>

SampleTreeWidget::SampleTreeWidget(QWidget *parent)
    :QTreeWidget(parent)
{
    InitUI();
    ConnectSignalandSolt();
}

SampleTreeWidget::~SampleTreeWidget()
{

}

void SampleTreeWidget::ConnectSignalandSolt()
{

}

void SampleTreeWidget::InitUI()
{
    setColumnCount(3);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    header()->hide();
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
    //setStyleSheet("QTreeView::branch {image:none;}");
    setIconSize(QSize(20,20));
}

QIcon getIcon(int analysisType, int markType)
{
    return QIcon(QString(":/png/images/filetree%1%2.png").arg(markType).arg(analysisType));
}

void SampleTreeWidget::SetTreeData()
{
    QVector<SampleTreeInfo_t> sampleTreeInfoList;
    SoapTypingDB::GetInstance()->getSampleTreeDataFromRealTimeDatabase(sampleTreeInfoList);

    int sampleSize = sampleTreeInfoList.size();
    if(sampleSize == 0)
    {
        return;
    }

    QBrush brush;
    brush.setColor(Qt::blue);
    for(int i=0; i<sampleSize; i++)
    {
        int gsspSize = 0;
        const SampleTreeInfo_t &sampleTreeInfo = sampleTreeInfoList.at(i);
        QTreeWidgetItem *top = new QTreeWidgetItem;
        top->setText(0, sampleTreeInfo.sampleName);
        top->setText(1, sampleTreeInfo.geneName);
        top->setForeground(1, brush);
        top->setIcon(0, getIcon(sampleTreeInfo.analysisType, sampleTreeInfo.markType));
        top->setSizeHint(0,QSize(200,15));
        int treeSize=sampleTreeInfo.treeinfo.size();
        for(int j=0; j<treeSize; j++)
        {
            const FileTreeInfo_t &fileTreeInfo = sampleTreeInfo.treeinfo.at(j);
            QTreeWidgetItem *child = new QTreeWidgetItem;
            child->setText(0, fileTreeInfo.fileName);
            child->setSizeHint(0,QSize(350,25));
            if(!fileTreeInfo.isGssp)
            {
                if(fileTreeInfo.analysisType == 1)
                {
                    child->setText(1, "UN");
                    child->setIcon(0, QIcon(":/png/images/filetreeFile2.png"));
                }
                else
                {
                    child->setText(1, QString("%1%2").arg(fileTreeInfo.exonIndex).arg(fileTreeInfo.rOrF));
                    child->setIcon(0, QIcon(QString(":/png/images/filetreeFile%1.png").arg(fileTreeInfo.isGood)));
                }
            }
            else
            {
                if(fileTreeInfo.analysisType == 4) //UNMATCH
                {
                    child->setText(1, QString("Filter:%1").arg(fileTreeInfo.gsspName));//2
                    child->setIcon(0, QIcon(":/png/images/filetreeFile2.png"));
                }
                else
                {
                    child->setText(1, QString("Filter:%1").arg(fileTreeInfo.gsspName));//2
                    child->setIcon(0, getIcon(fileTreeInfo.analysisType, 0));
                }
                gsspSize++;
            }
            top->addChild(child);
        }
        if(gsspSize > 1)
        {
            QTreeWidgetItem *child = new QTreeWidgetItem;
            child->setText(0, "Combined");
            child->setIcon(0, getIcon(sampleTreeInfo.analysisType, 0));
            top->addChild(child);
        }
        addTopLevelItem(top);
    }
    topLevelItem(0)->setSelected(true);
    expandItem(topLevelItem(0));
}

void SampleTreeWidget::SetSelectItem(int index, QString &str_name)
{
    QString str_last_select = currentItem()->text(0);
    QStringList str_list = str_last_select.split('_');

    QString str = QString("%1F").arg(index);
    QList<QTreeWidgetItem *> item_list = findItems(str,Qt::MatchRecursive,1);
    foreach(QTreeWidgetItem *item, item_list)
    {
        if(item->text(0).contains(str_list[0]))
        {
            setCurrentItem(item);
            str_name = item->text(0);
            //emit itemClicked(item,0);
            break;
        }
    }
}
