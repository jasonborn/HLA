#include "matchlistwidget.h"
#include <QHeaderView>
#include <DataBase/soaptypingdb.h>

const int I_COLNUM = 5;
const int I_ROWNUM = 500;

MatchListWidget::MatchListWidget(QWidget *parent)
    :QTableWidget(parent)
{
    InitUI();
}

MatchListWidget::~MatchListWidget()
{

}

void MatchListWidget::InitUI()
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);

    verticalHeader()->setVisible(false);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setFixedHeight(40); //新增，设置表头的高度
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter); //新增
    setShowGrid(false); //新增，设置不显示格子线
    setRowCount(I_ROWNUM);
    setColumnCount(I_COLNUM);

    QStringList defaultHead_;
    defaultHead_<<"  Mis"<<"Allele1"<<"Allele2"<<"Info"<<"GSSP";
    this->setColumnWidth(0,60); //改40->60
    this->setColumnWidth(1,92); //改90->92
    this->setColumnWidth(2,92); //改90->92
    this->setColumnWidth(3,62); //改70->62
    this->setColumnWidth(4,40);
    this->setHorizontalHeaderLabels(defaultHead_);
    QTableWidgetItem *itemArray = new QTableWidgetItem[I_ROWNUM * I_COLNUM];
    for(int i=0; i<I_ROWNUM; i++)
    {
        for(int j=0; j<I_COLNUM; j++)
        {
            itemArray->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter); //新增
            this->setItem(i, j, itemArray++);
            if(i%2==0) //新增
            {
                item(i,j)->setBackgroundColor(QColor(235,244,223)); //新增
            }
            else
            {
                item(i,j)->setBackgroundColor(QColor(255,255,255)); //新增
            }
        }
    }
}


void MatchListWidget::SetTableData(const QString &str_sample, const QString &str_gssp)
{
    QStringList typeResult;
    SoapTypingDB::GetInstance()->getResultDataFromGsspTable(str_gssp,false,false,typeResult);
    SoapTypingDB::GetInstance()->getResultDataFromsampleTable(str_sample,false,typeResult);


    for(int i=0; i<typeResult.size(); i++)
    {
        QStringList line = typeResult.at(i).split(",");
        if(line.at(3).toInt()!=0)
        {
            line[0].append("*");
        }

        this->item(i, 0)->setText("  "+line.at(0));
        this->item(i, 1)->setText(line.at(1));
        this->item(i, 2)->setText(line.at(2));
        this->item(i, 3)->setText(line.at(4));

        if(i >= 500)
            break;
    }


}
