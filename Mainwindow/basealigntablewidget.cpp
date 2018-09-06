#include "basealigntablewidget.h"
#include <QHeaderView>
#include "DataBase/soaptypingdb.h"
#include "Core/core.h"
#include <QDebug>

BaseAlignTableWidget::BaseAlignTableWidget()
{
    m_iRowNum = 9;
    m_iColNum = 1200;
    InitUI();
}


BaseAlignTableWidget::~BaseAlignTableWidget()
{

}

void BaseAlignTableWidget::InitUI()
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectColumns);
    verticalHeader()->setVisible(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    horizontalHeader()->setStretchLastSection(true);
    setStyleSheet("selection-background-color:rgb(255,255,0,127);QTableView::item:selected{color:black;}");
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    verticalHeader()->setDefaultSectionSize(20); //设置行高
    horizontalHeader()->setFixedHeight(20); //设置表头的高度
    horizontalHeader()->setHighlightSections(false);
    setRowCount(m_iRowNum);
    setColumnCount(m_iColNum);

    for(int i=1; i<m_iColNum; i++)
    {
        this->setColumnWidth(i,20);
        m_sl_defaulthead<<"";
    }

    setColumnWidth(0,230);
    setHorizontalHeaderLabels(m_sl_defaulthead);


    QTableWidgetItem *itemArray = new QTableWidgetItem[m_iRowNum * m_iColNum];
    for(int i=0; i<m_iRowNum; i++)
    {
        for(int j=0; j<m_iColNum; j++)
        {

            if(j == 0)
            {
                itemArray->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
            }
            else {
                itemArray->setTextAlignment(Qt::AlignCenter);
            }
            this->setItem(i, j, itemArray);

            if (i == 0)
            {
                itemArray->setBackgroundColor(QColor(0, 169,157,51));
                itemArray->setTextColor(QColor(0, 169,157));
            }
            else if (i == 1)
            {
                itemArray->setBackgroundColor(QColor(140,98,57,51));
                itemArray->setTextColor(QColor(140,98,57));
            }
            else if (i == 2)
            {
                itemArray->setBackgroundColor(QColor(0,113,188,51));
                itemArray->setTextColor(QColor(0,113,188));
            }
            else if (i == 3)
            {
                itemArray->setBackgroundColor(QColor(247,147,30,51));
                itemArray->setTextColor(QColor(247,147,30));
            }
            else if (i == 4)
            {
                itemArray->setBackgroundColor(QColor(102,45,145,51));
                itemArray->setTextColor(QColor(102,45,145));
            }
            else if (i == 5)
            {
                itemArray->setBackgroundColor(QColor(57,181,74,51));
                itemArray->setTextColor(QColor(57,181,74));
            }
            else if (i == 6)
            {
                itemArray->setBackgroundColor(QColor(237,30,121,51));
                itemArray->setTextColor(QColor(237,30,121));
            }

            itemArray++;
        }
    }

    QFont font;
    font.setBold(true);
    font.setPointSize(12);
    for(int i=0; i<9; i++)
    {
        item(i, 0)->setFont(font);
    }


    item(0, 0)->setText("  Consensus Sequence");
    item(1, 0)->setText("  Forward Sequence");
    item(2, 0)->setText("  Reverse Sequence");
    item(3, 0)->setText("  GSSP Sequence");
    item(4, 0)->setText("  Consensus Alignment");
    item(5, 0)->setText("  Pattern Sequence");

}

void BaseAlignTableWidget::SetAlignTableData(QString &str_samplename)
{
    if(m_str_SampleName != str_samplename)
    {
        m_str_SampleName = str_samplename;
    }
    else
    {
        return;
    }

    if(!m_BaseAlignSampleInfo.consensusSeq.isEmpty())
    {
        clearBaseAlignTableSampleItem();
        m_BaseAlignSampleInfo.clear();
    }

    SoapTypingDB::GetInstance()->getBaseAlignSampleInfo(str_samplename, m_BaseAlignSampleInfo);
    int i_startColumn = 1;
    if(m_BaseAlignSampleInfo.alignStartPos == m_BaseAlignSampleInfo.alignEndPos)
    {
        return;
    }

    QStringList header;
    header<<str_samplename;
    getTableHead(header, m_BaseAlignSampleInfo.alignEndPos - m_BaseAlignSampleInfo.alignStartPos, m_BaseAlignSampleInfo.alignStartPos);
    setHorizontalHeaderLabels(header);
    //ui->m_MarkTableWidget->setHorizontalHeaderLabels(header);//hcj
    QString &line = m_BaseAlignSampleInfo.consensusSeq;
    for(int i=0; i<line.size(); i++)
    {
        item(0, i+i_startColumn)->setText(line.at(i));
        item(4, i+i_startColumn)->setText("-");
    }

    line = m_BaseAlignSampleInfo.forwardSeq;
    for(int i=0; i<line.size(); i++)
    {
        if(line.at(i)!='-')
        {
            item(1, i+i_startColumn)->setText(line.at(i));
        }
    }
    line = m_BaseAlignSampleInfo.reverseSeq;
    for(int i=0; i<line.size(); i++)
    {
        if(line.at(i)!='-')
        {
            this->item(2, i+i_startColumn)->setText(line.at(i));
        }
    }
    line = m_BaseAlignSampleInfo.patternSeq;
    for(int i=0; i<line.size(); i++)
    {
        if(line.at(i)!='-')
        {
            item(5, i+i_startColumn)->setText(line.at(i));
        }
    }

    for(QSet<int>::iterator it=m_BaseAlignSampleInfo.editPostion.begin(); it!=m_BaseAlignSampleInfo.editPostion.end(); it++)
    {
        char A = m_BaseAlignSampleInfo.forwardSeq[(*it)-m_BaseAlignSampleInfo.alignStartPos].toLatin1();
        char B = m_BaseAlignSampleInfo.reverseSeq[(*it)-m_BaseAlignSampleInfo.alignStartPos].toLatin1();
        char C = Core::GetInstance()->mergeBases(A, B);
        item(4, (*it)-m_BaseAlignSampleInfo.alignStartPos+i_startColumn)->setText(QString("%1").arg(C));
    }

    for(QSet<int>::iterator it=m_BaseAlignSampleInfo.frMisMatchPostion.begin(); it!=m_BaseAlignSampleInfo.frMisMatchPostion.end(); it++)
    {
        char A = m_BaseAlignSampleInfo.forwardSeq[(*it)-m_BaseAlignSampleInfo.alignStartPos].toLatin1();
        char B = m_BaseAlignSampleInfo.reverseSeq[(*it)-m_BaseAlignSampleInfo.alignStartPos].toLatin1();
        char C = Core::GetInstance()->mergeBases(A, B);
        item(4, (*it)-m_BaseAlignSampleInfo.alignStartPos+i_startColumn)->setText(QString("%1").arg(C));
    }

    for(QSet<int>::iterator it=m_BaseAlignSampleInfo.pcMisMatchPostion.begin(); it!=m_BaseAlignSampleInfo.pcMisMatchPostion.end(); it++)
    {
        char A = m_BaseAlignSampleInfo.forwardSeq[*it-m_BaseAlignSampleInfo.alignStartPos].toLatin1();
        char B = m_BaseAlignSampleInfo.reverseSeq[*it-m_BaseAlignSampleInfo.alignStartPos].toLatin1();
        char C = Core::GetInstance()->mergeBases(A, B);
        item(4, (*it)-m_BaseAlignSampleInfo.alignStartPos+i_startColumn)->setText(QString("%1").arg(C));
    }

    for(QMap<QString, BaseAlignGsspInfo>::iterator it=m_BaseAlignSampleInfo.gsspInfoMap.begin(); it!=m_BaseAlignSampleInfo.gsspInfoMap.end(); it++)
    {
        BaseAlignGsspInfo &info = it.value();
        line = info.gsspFileSeq;
        for(int i=0; i<line.size(); i++)
        {
            if(line.at(i)!='-')
            {
                item(3, i+info.gsspFileAlignStartPos - m_BaseAlignSampleInfo.alignStartPos + i_startColumn)->setText(line.at(i));
            }
        }
        line = info.gsspSeq;
        for(int i=0; i<line.size(); i++)
        {
            item(3, i+info.gsspPostion - m_BaseAlignSampleInfo.alignStartPos + i_startColumn)->setText(line.at(i));
        }
    }
}

void BaseAlignTableWidget::clearBaseAlignTableSampleItem()
{
    this->setHorizontalHeaderLabels(m_sl_defaulthead);
    for(int i=1; i<m_iColNum; i++)
    {
        item(0, i)->setText("");
        item(1, i)->setText("");
        item(2, i)->setText("");
        item(3, i)->setText("");
        item(4, i)->setText("");
        item(5, i)->setText("");
        item(4, i)->setBackgroundColor(QColor(223, 223, 223));
    }
}

void BaseAlignTableWidget::getTableHead(QStringList &head, int length, int start)
{
    bool startWithNull = false;
    start += 1;
    for(int i=0; i<length; i++)
    {
        int pos = i + start;
        int TPos = (pos+2) / 1000;
        int HPos = ((pos+1) % 1000) / 100;
        int DPos = (pos % 100) / 10;
        int Ipos = (pos-1) % 10;
        if(pos/100)
        {
            Ipos = (pos-2) % 10;
        }
        switch(Ipos)
        {
        case 1:
            if(startWithNull)
            {
                head<<QString("%1").arg(1);
            }
            else
            {
                head<<QString("");
            }
            break;
        case 0:
            if(startWithNull&&(HPos>0 || TPos >0 || DPos >0))
            {
                head<<QString("%1").arg(DPos);
            }
            else
            {
                head<<"";
            }
            break;
        case 9:
            if(startWithNull&&(TPos>0 || HPos>0))
            {
                head<<QString("%1").arg(HPos);
            }
            else
            {
                head<<"";
            }
            break;
        case 8:
            if(TPos>0&&startWithNull)
            {
                head<<QString("%1").arg(TPos);
            }
            else
            {
                head<<"";
            }
            break;
        default:
            startWithNull = true;
            head<<"";
        }
    }

    if(m_iColNum>length+1)
    {
        for(int i=length+1; i<m_iColNum; i++)
        {
            head<<"";
        }
    }
}
