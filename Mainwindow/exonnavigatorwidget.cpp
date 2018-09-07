#include "exonnavigatorwidget.h"
#include <QPainter>
#include <QtDebug>
#include <QMouseEvent>
#include "DataBase/soaptypingdb.h"

ExonNavigatorWidget::ExonNavigatorWidget(QWidget *parent)
    :QWidget(parent)
{
    m_iheight = 100;
    m_itop1 = m_iheight*0.45;
    m_itop2 = m_iheight*0.5;
    m_itop3 = m_iheight*0.7+1;
    m_ih0 = m_iheight*0.3;
    m_ih1 = m_iheight*0.5;
    m_ih2 = m_iheight*0.4;
    m_ih3 = m_iheight*0.2;
    //m_iSelectPos = 100;
    m_iMidgap = 40;
    m_igap = 20;
    m_isub_index = 0;
    m_isub_pos = 0;
    m_dXscale = 0;
    m_vec_Exon.reserve(4);
}

ExonNavigatorWidget::~ExonNavigatorWidget()
{

}

void ExonNavigatorWidget::SetExonData(QString &str_sample, QString &str_gene)
{
    if(m_str_SampleName !=str_sample || m_str_GeneName != str_gene)
    {
        m_str_SampleName = str_sample;
        m_str_GeneName = str_gene;
    }
    else
    {
        return;
    }

    m_vecExonIndex.clear();
    m_Exoninfo.maxExonIndex = 0;
    m_Exoninfo.minExonIndex = 0;
    m_Exoninfo.vec_editPos.clear();
    m_Exoninfo.vec_frMis.clear();
    m_Exoninfo.vec_frUnEqual.clear();
    m_Exoninfo.vec_pcMis.clear();

    SoapTypingDB::GetInstance()->getExonPositionIndex(str_gene, m_vecExonIndex);

    SoapTypingDB::GetInstance()->getExonNavigatorInfo(str_sample, m_Exoninfo);

    m_isub_index = m_Exoninfo.maxExonIndex - m_Exoninfo.minExonIndex;
    m_isub_pos = m_vecExonIndex[m_Exoninfo.maxExonIndex] - m_vecExonIndex[m_Exoninfo.minExonIndex-1];
    assert(m_isub_pos> 0);

    foreach(int pos,m_Exoninfo.vec_frUnEqual)
    {
        m_set_mispos.insert(pos);
    }

    foreach(int pos,m_Exoninfo.vec_editPos)
    {
        m_set_mispos.insert(pos);
    }

    foreach(int pos,m_Exoninfo.vec_frMis)
    {
        m_set_mispos.insert(pos);
    }

    foreach(int pos,m_Exoninfo.vec_pcMis)
    {
        m_set_mispos.insert(pos);
    }

    CalcExonData();
}

void ExonNavigatorWidget::CalcExonData()
{
    int i_width = width();
    if(m_iwidth != i_width) //窗口大小发生变化
    {
        m_iwidth = i_width;
        m_vec_Exon.clear();
        m_dXscale = (i_width - 2*m_igap - m_isub_index*m_iMidgap)*1.0/m_isub_pos;

        for(int i=m_Exoninfo.minExonIndex; i<=m_Exoninfo.maxExonIndex; i++)
        {
            Exon exon;
            exon.i_exonstartpos = m_vecExonIndex[i-1]+1;
            exon.i_exonendpos= m_vecExonIndex[i];
            exon.i_screenstartpos = m_igap+(m_vecExonIndex[i-1]-m_vecExonIndex[m_Exoninfo.minExonIndex-1])*m_dXscale
                    +(i-m_isub_index)*m_iMidgap;//碱基导航条起始坐标
            exon.i_screenwidth = (m_vecExonIndex[i] - m_vecExonIndex[i-1])*m_dXscale;//碱基导航条长度
            exon.i_exonindex = i;
            m_vec_Exon.push_back(exon);
        }
    }
}

void ExonNavigatorWidget::paintEvent(QPaintEvent *event)
{
    QPainter exonPainter(this);
    int i_width = width();
    exonPainter.setPen(QColor(139,139,139));
    exonPainter.setBrush(Qt::white);
    exonPainter.drawRect(QRect(0,0,i_width-1,m_iheight -1));

    if(m_isub_pos)
    {
        DrawExonArea(exonPainter);
        DrawSelectFrame(exonPainter);
        DrawExonPos(exonPainter);
    }
}

void ExonNavigatorWidget::DrawExonArea(QPainter &exonPainter)
{
    QFont fontRegion;
    QFont fontPos;
    fontRegion.setPointSize(20);
    fontRegion.setWeight(QFont::Black);
    fontRegion.setFamily(tr("微软雅黑"));
    fontPos.setPointSize(12);

    CalcExonData();

    foreach(const Exon &exon, m_vec_Exon)
    {
        int x_pos = exon.i_screenstartpos;
        int width = exon.i_screenwidth;
        int x_pos_index = x_pos + (width-80)/2;//碱基导航条index起始坐标,默认长度为80

        exonPainter.setBrush(QBrush(QColor(235,244,223)));
        exonPainter.drawRect(x_pos, m_itop2, width, m_ih2);
        QRect recti(x_pos_index, 0, 80, m_ih0);
        exonPainter.setBrush(Qt::NoBrush);
        exonPainter.setFont(fontRegion);
        exonPainter.drawText(recti, Qt::AlignHCenter|Qt::AlignCenter, QString("Exon%1").arg(exon.i_exonindex));
        exonPainter.setFont(fontPos);
        QRect recti1(x_pos, 0, width, m_ih1);
        exonPainter.drawText(recti1, Qt::AlignLeft|Qt::AlignBottom, QString::number(exon.i_exonstartpos));
        exonPainter.drawText(recti1, Qt::AlignRight|Qt::AlignBottom, QString::number(exon.i_exonendpos));
    }
}

void ExonNavigatorWidget::DrawSelectFrame(QPainter &exonPainter)
{
    exonPainter.setPen(QColor(136,136,136));
    exonPainter.setBrush(QBrush(QColor(255,255,0,127)));
    exonPainter.drawRect(QRect(m_iSelectPos-15, m_itop1-15, 25, m_ih1+15));
    exonPainter.setBrush(Qt::NoBrush);
    exonPainter.setPen(Qt::black);
    exonPainter.drawText(QRect(m_iSelectPos-15, 0, 25, m_ih1), Qt::AlignBottom|Qt::AlignCenter,
                         QString::number(ScreenPosToPeakPos(m_iSelectPos)));
}

void ExonNavigatorWidget::DrawExonPos(QPainter &exonPainter)
{
    exonPainter.setPen(Qt::NoPen);
    exonPainter.setBrush(QColor(0,0,255));
    foreach(int pos,m_Exoninfo.vec_frUnEqual)
    {
        exonPainter.drawRect(PeakPosToScreenPos(pos), m_itop2, 2, m_ih3);
    }

    exonPainter.setBrush(QColor(0,0,0));
    foreach(int pos,m_Exoninfo.vec_editPos)
    {
        exonPainter.drawRect(PeakPosToScreenPos(pos), m_itop2, 2, m_ih3);
    }

    exonPainter.setBrush(QColor(57,181,74));
    foreach(int pos,m_Exoninfo.vec_frMis)
    {
        exonPainter.drawRect(PeakPosToScreenPos(pos), m_itop2, 2, m_ih3);
    }

    exonPainter.setBrush(QColor(237,28,36));
    foreach(int pos,m_Exoninfo.vec_pcMis)
    {
        exonPainter.drawRect(PeakPosToScreenPos(pos), m_itop2, 2, m_ih3);
    }

    //for(QMap<int, int>::iterator it=typeMisPosition_.begin(); it!=typeMisPosition_.end(); it++)
//    {
//        exonPainter.drawRect(210, m_itop3, 2, m_ih3);
//    }
}

void ExonNavigatorWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        QPoint pos = event->pos();
        foreach(const Exon &exon, m_vec_Exon)
        {
            if(pos.x() >= exon.i_screenstartpos && pos.x() <= exon.i_screenstartpos+exon.i_screenwidth)
            {
                m_iSelectPos = pos.x();
                update();
                emit signalExonFocusPosition(m_vecExonIndex[m_Exoninfo.minExonIndex-1]+1,
                        ScreenPosToPeakPos(m_iSelectPos),
                        exon.i_exonstartpos, exon.i_exonindex);
                break;
            }
        } 
    }
}

int ExonNavigatorWidget::PeakPosToScreenPos(int peakpos) //峰图坐标转换成显示坐标
{
    int screenpos = 0;

    foreach(const Exon &exon, m_vec_Exon)
    {
        if(peakpos >= exon.i_exonstartpos && peakpos <= exon.i_exonendpos)
        {
            screenpos = (peakpos - m_vecExonIndex[m_Exoninfo.minExonIndex - 1]-1)*m_dXscale
                    + m_igap + (exon.i_exonindex-m_isub_index)*m_iMidgap;
            break;
        }
    }
    return screenpos;
}

int ExonNavigatorWidget::ScreenPosToPeakPos(int screenpos) //显示坐标转换成峰图坐标
{
    int peakpos = 0;
    foreach(const Exon &exon, m_vec_Exon)
    {
        if(screenpos >= exon.i_screenstartpos && screenpos <= exon.i_screenstartpos+exon.i_screenwidth)
        {
            peakpos = (screenpos - m_igap - (exon.i_exonindex-m_isub_index)*m_iMidgap)/m_dXscale
                    + m_vecExonIndex[m_Exoninfo.minExonIndex - 1]+1;
            break;
        }
    }
    return peakpos;
}

void ExonNavigatorWidget::SetSelectPos(int colnum, int &selectpos, int &exonstartpos, int &index)
{
    selectpos = colnum + m_vecExonIndex[m_Exoninfo.minExonIndex - 1];
    foreach(const Exon &exon, m_vec_Exon)
    {
        if(selectpos >= exon.i_exonstartpos && selectpos <= exon.i_exonendpos)
        {
            m_iSelectPos = (selectpos - m_vecExonIndex[m_Exoninfo.minExonIndex - 1])*m_dXscale
                    + m_igap + (exon.i_exonindex-m_isub_index)*m_iMidgap;
            exonstartpos = exon.i_exonstartpos;
            index = exon.i_exonindex;
            break;
        }
    }
    update();
}

void ExonNavigatorWidget::setSelectFramePosition(int index, int &startpos, int &selectpos, int &exonstartpos)
{
    if(m_iSelectindex != index)
    {
        m_iSelectindex = index;
    }
    else
    {
        return;
    }

    foreach(const Exon &exon, m_vec_Exon)
    {
        if(exon.i_exonindex == index)
        {
            startpos = m_vecExonIndex[m_Exoninfo.minExonIndex - 1];
            exonstartpos = exon.i_exonstartpos;

            foreach(int peakpos, m_set_mispos)
            {
                peakpos++;
                if(peakpos > exon.i_exonstartpos && peakpos < exon.i_exonendpos)
                {
                    m_iSelectPos = (peakpos - m_vecExonIndex[m_Exoninfo.minExonIndex - 1])*m_dXscale
                            + m_igap + (exon.i_exonindex-m_isub_index)*m_iMidgap;

                    selectpos = peakpos-1;
                    update();
                    return;
                }
            }

            //如果没有错配位点，默认为导航条起始位置
            m_iSelectPos = exon.i_screenstartpos;
            selectpos = exon.i_exonstartpos;
            update();
            break;
        }
    }


}

void ExonNavigatorWidget::SetSelectFramePos(int index, int colnum, int &test)
{
    foreach(const Exon &exon, m_vec_Exon)
    {
        if(exon.i_exonindex == index)
        {
            int selectpos = colnum + exon.i_exonstartpos;
            test = selectpos  - m_vecExonIndex[m_Exoninfo.minExonIndex - 1] -1;
            if(selectpos >= exon.i_exonstartpos && selectpos <= exon.i_exonendpos)
            {
                m_iSelectPos = (selectpos - m_vecExonIndex[m_Exoninfo.minExonIndex - 1])*m_dXscale
                        + m_igap + (exon.i_exonindex-m_isub_index)*m_iMidgap;
                break;
            }
        }
    }
    update();
}
