#include "multipeakwidget.h"
#include <QPainter>
#include "DataBase/soaptypingdb.h"
#include <QDebug>
#include <QMouseEvent>
#include <QtAlgorithms>
#include <QScrollArea>
#include <QScrollBar>

const int PEAKLINEHIGHT = 200;
const int HLINEHIGHT = 20;

PeakLine::PeakLine(long size):m_lsize(size)
{
    m_loffset = 0;
    base_a = new QPointF[size];
    base_t = new QPointF[size];
    base_g = new QPointF[size];
    base_c = new QPointF[size];
}

PeakLine::~PeakLine()
{
    qDebug()<<"~PeakLine";
    delete[] base_a;
    delete[] base_t;
    delete[] base_g;
    delete[] base_c;
}

int PeakLine::getSize()
{
    return m_lsize;
}

void PeakLine::SetBaseAPoint(int index,double x, double y)
{
    base_a[index].setX(x);
    base_a[index].setY(y);
}

void PeakLine::SetBaseTPoint(int index,double x, double y)
{
    base_t[index].setX(x);
    base_t[index].setY(y);
}

void PeakLine::SetBaseGPoint(int index,double x, double y)
{
    base_g[index].setX(x);
    base_g[index].setY(y);
}

void PeakLine::SetBaseCPoint(int index,double x, double y)
{
    base_c[index].setX(x);
    base_c[index].setY(y);
}

QPointF * PeakLine::getBaseAPoint(char type)
{
    switch (type)
    {
    case 'A': return base_a;
    case 'T': return base_t;
    case 'G': return base_g;
    case 'C': return base_c;
    }

}

void PeakLine::AddGeneLetter(GeneLetter &geneletter)
{
    m_vec_GeneLetter.push_back(geneletter);
}

QVector<GeneLetter>& PeakLine::GetGeneLetter()
{
    return m_vec_GeneLetter;
}

void PeakLine::SetFileName(QString &str)
{
    m_strFileName = str;
}

QString& PeakLine::GetFileName()
{
    return m_strFileName;
}

void PeakLine::SetExcludePos(int left, int right)
{
    m_left_exclude = left;
    m_right_exclude = right;
}

void PeakLine::GetExcludePos(int &left, int &right)
{
    left = m_left_exclude;
    right  = m_right_exclude;
}

void PeakLine::SetOffset(int offset)
{
    m_loffset = offset;
}

int PeakLine::GetOffset()
{
    return m_loffset;
}

MultiPeakWidget::MultiPeakWidget(QWidget *parent)
    :QWidget(parent)
{
    m_x_step = 28;
    m_y_step = 0.05;
    m_bIsSelect = false;
    m_l_xSize = 0;
    setFocusPolicy(Qt::StrongFocus);//如果不调用，keyPressEvent不响应
    setAutoFillBackground(true);
}

MultiPeakWidget::~MultiPeakWidget()
{

}

void MultiPeakWidget::SetPeakData(const QString &str_samplename, const QString &str_exon)
{
    if(m_str_SampleName != str_samplename || m_str_Exon != str_exon)
    {
        m_str_SampleName = str_samplename;
        m_str_Exon = str_exon;
    }
    else
    {
        return;
    }

    m_vec_filetable.clear();
    QVector<QSharedPointer<PeakLine>>().swap(m_vec_Peakline);//clear无法释放内存

    SoapTypingDB::GetInstance()->getAlldataFormRealTime(str_samplename, str_exon, m_vec_filetable);
    int i_count_peak = m_vec_filetable.size();
    QVector<int> vec_left,vec_right; //以AlignEndPos为界，计算左右两边的长度
    for(int i=0;i<i_count_peak;i++)
    {
        Ab1FileTableBase &table = m_vec_filetable[i];
        QStringList baseposion = table.getBasePostion().split(":");

        int i_AlginEndPos = table.getAlignEndPos();
        int i_right = table.getBaseNumber() - i_AlginEndPos;
        vec_left.push_back(i_AlginEndPos);
        vec_right.push_back(i_right);
    }

    QVector<int> vec_left_copy(vec_left);
    std::sort(vec_left.begin(),vec_left.end());
    std::sort(vec_right.begin(),vec_right.end());

    m_l_xSize = *(vec_left.crbegin()) + *(vec_right.crbegin());//取左右最大值，相加为总长度
    m_l_xSize *= m_x_step;

    for(int k=0;k<i_count_peak;k++)
    {
        Ab1FileTableBase &table = m_vec_filetable[k];
        long l_size = table.getSignalNumber();
        int i_offset = (*vec_left.crbegin() - vec_left_copy[k])*m_x_step;

        QSharedPointer<PeakLine> pPeakLine = QSharedPointer<PeakLine>(new PeakLine(l_size));
        pPeakLine->SetFileName(table.getFileName());
        pPeakLine->SetExcludePos(table.getAlignStartPos(),table.getAlignEndPos());

        QStringList A_list = table.getBaseASignal().split(":");
        QStringList T_list = table.getBaseTSignal().split(":");
        QStringList G_list = table.getBaseGSignal().split(":");
        QStringList C_list = table.getBaseCSignal().split(":");


        QStringList baseposion = table.getBasePostion().split(":");
        int i_basenum = table.getBaseNumber();
        int pos_i = 0;
        double pos_j = 0.0;
        int i_index = 0;
        int begin = 0;
        int height_tmp = (k+1)*PEAKLINEHIGHT;
        for(int i=0;i<i_basenum;i++)
        {
            int end = baseposion[i].toInt();
            int num = end - begin;
            pos_i = i*m_x_step + i_offset;
            for(int j = 0; j < num; j++)
            {
                pos_j = pos_i + j*m_x_step/num;
                double a_y = height_tmp-A_list[i_index].toInt()*m_y_step;
                pPeakLine->SetBaseAPoint(i_index,pos_j,a_y);

                double t_y = height_tmp-T_list[i_index].toInt()*m_y_step;
                pPeakLine->SetBaseTPoint(i_index,pos_j,t_y);

                double g_y = height_tmp-G_list[i_index].toInt()*m_y_step;
                pPeakLine->SetBaseGPoint(i_index,pos_j,g_y);

                double c_y = height_tmp-C_list[i_index].toInt()*m_y_step;
                pPeakLine->SetBaseCPoint(i_index,pos_j,c_y);

                i_index++;
            }
            begin = end;
        }

        int num = l_size - i_index;
        for(int j = 0;i_index < l_size;i_index++,j++)
        {
            pos_j = pos_i + j*m_x_step/num;
            double a_y = height_tmp-A_list[i_index].toInt()*m_y_step;
            pPeakLine->SetBaseAPoint(i_index,pos_j,a_y);

            double t_y = height_tmp-T_list[i_index].toInt()*m_y_step;
            pPeakLine->SetBaseTPoint(i_index,pos_j,t_y);

            double g_y = height_tmp-G_list[i_index].toInt()*m_y_step;
            pPeakLine->SetBaseGPoint(i_index,pos_j,g_y);

            double c_y = height_tmp-C_list[i_index].toInt()*m_y_step;
            pPeakLine->SetBaseCPoint(i_index,pos_j,c_y);
        }


        QByteArray baseseq = table.getBaseSequence();
        for(int i=0;i<i_basenum;i++)
        {
            GeneLetter geneletter;
            geneletter.pos.setX((i+1)*m_x_step + i_offset);
            geneletter.pos.setY(35 + k*PEAKLINEHIGHT);
            geneletter.type = QChar(baseseq[i]);
            geneletter.oldtype = ' ';
            pPeakLine->AddGeneLetter(geneletter);
        }

        m_vec_Peakline.push_back(pPeakLine);
    }


    resize(m_l_xSize, i_count_peak*PEAKLINEHIGHT+40);//如果不调用，paintEvent不响应
}

void MultiPeakWidget::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter pter(this);
    DrawExcludeArea(&pter);
    DrawSelectFrame(&pter);
    DrawPeakLines(&pter);
    DrawHLines(&pter);
    DrawPeakHead(&pter);
}

void MultiPeakWidget::DrawPeakLines(QPainter *pter)
{
    foreach(const QSharedPointer<PeakLine> & peakline, m_vec_Peakline)
    {
        long l_size = peakline->getSize();
        pter->setPen(Qt::green);
        pter->drawPolyline(peakline->getBaseAPoint('A'), l_size);

        pter->setPen(Qt::red);
        pter->drawPolyline(peakline->getBaseAPoint('T'), l_size);

        pter->setPen(Qt::black);
        pter->drawPolyline(peakline->getBaseAPoint('G'), l_size);

        pter->setPen(Qt::blue);
        pter->drawPolyline(peakline->getBaseAPoint('C'), l_size);
    }
}

void MultiPeakWidget::DrawHLines(QPainter *pter)
{
    QPen pen(Qt::darkGray);
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);
    pter->setPen(pen);

    for(int i=0;i<m_vec_Peakline.size();i++)
    {
        int i_height = i*PEAKLINEHIGHT; //每个峰图的高度

        pter->drawLine(0,20+i_height,m_l_xSize,20+i_height);

        pter->drawLine(0,40+i_height,m_l_xSize,40+i_height);

        pter->drawLine(0,60+i_height,m_l_xSize,60+i_height);

        pter->drawLine(0,PEAKLINEHIGHT+i_height,m_l_xSize,PEAKLINEHIGHT+i_height);
    }
}

void MultiPeakWidget::GetBaseColor(QPainter *pter, const QChar &base)
{
    switch(base.toLatin1())
    {
    case 'A':
        pter->setPen(Qt::green);
        break;
    case 'T':
        pter->setPen(Qt::red);
        break;
    case 'G':
        pter->setPen(Qt::black);
        break;
    case 'C':
        pter->setPen(Qt::blue);
        break;
    default:
        pter->setPen(Qt::darkRed);
    }
}

void MultiPeakWidget::DrawPeakHead(QPainter *pter)
{
    for(int i=0;i<m_vec_Peakline.size();i++)
    {
        pter->drawText(3,15+i*PEAKLINEHIGHT, m_vec_Peakline[i]->GetFileName());
        int i_index = 0;
        foreach(const GeneLetter &letter, m_vec_Peakline[i]->GetGeneLetter())
        {
            GetBaseColor(pter, letter.type);
            pter->drawText(letter.pos, letter.type);
            if(letter.oldtype != ' ')
            {
                GetBaseColor(pter, letter.oldtype);
                pter->drawText(letter.pos.x(),letter.pos.y()+HLINEHIGHT, letter.oldtype);
            }

            if(i_index%10 == 0)
            {
                pter->drawText(letter.pos.x()-3,letter.pos.y()-HLINEHIGHT, QString::number(i_index));
            }
            i_index++;//从0开始计数？
        }
    }
}

void MultiPeakWidget::DrawSelectFrame(QPainter *pter)
{
    pter->setBrush(Qt::yellow);
    pter->setPen(Qt::NoPen);
    pter->drawRect(m_select_pos.x()-5,0,14,m_vec_Peakline.size()*PEAKLINEHIGHT); //绘制比对框

    if(m_bIsSelect)
    {
        pter->setBrush(Qt::NoBrush);
        pter->setPen(Qt::black);
        pter->drawRect(m_select_pos.x()-5,m_select_pos.y()-13,16,16); //绘制编辑碱基框
    }
}

void MultiPeakWidget::DrawExcludeArea(QPainter *pter)
{
    pter->setPen(Qt::NoPen);
    pter->setBrush(Qt::darkGray);

    for(int i=0;i<m_vec_Peakline.size();i++)
    {
        int i_height = i*PEAKLINEHIGHT;
        int left_exclude,right_exclude;
        m_vec_Peakline[i]->GetExcludePos(left_exclude, right_exclude);

        QVector<GeneLetter> &vec_geneLetter = m_vec_Peakline[i]->GetGeneLetter();
        int w_left =  vec_geneLetter[left_exclude].pos.x();
        pter->drawRect(0,60+i_height, w_left,140);
        int w_right = vec_geneLetter[right_exclude-1].pos.x();
        pter->drawRect(w_right,60+i_height,m_l_xSize,140);
    }
}


void MultiPeakWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
    {
        return;
    }
    qDebug()<<"mousePressEvent";
    QPoint pos = event->pos();
    m_bIsSelect = false;

    m_index_PeakLine = 0;
    for(int i=0; i<m_vec_Peakline.size(); i++)
    {
        if(pos.y() > i*PEAKLINEHIGHT && pos.y() < (i+1)*PEAKLINEHIGHT)
        {
            m_index_PeakLine = i;
            break;
        }
    }

    QVector<GeneLetter> &vec_GeneLetter = m_vec_Peakline[m_index_PeakLine]->GetGeneLetter();
    int left_exclude,right_exclude;
    m_vec_Peakline[m_index_PeakLine]->GetExcludePos(left_exclude, right_exclude);

    int w_left = vec_GeneLetter[left_exclude].pos.x();
    int w_right = vec_GeneLetter[right_exclude - 1].pos.x();

    if(pos.x() > w_left && pos.x() < w_right)
    {
        for(int i=left_exclude; i<right_exclude+1; i++)
        {
            int i_low = vec_GeneLetter[i-1].pos.x();
            int i_high = vec_GeneLetter[i].pos.x();
            int i_mid = (i_low+i_high)/2;
            if(pos.x() > i_low && pos.x() < i_mid)
            {
                if(pos.y() > HLINEHIGHT + m_index_PeakLine*PEAKLINEHIGHT &&
                   pos.y() < 2*HLINEHIGHT + m_index_PeakLine*PEAKLINEHIGHT)
                {
                    m_bIsSelect = true;
                    m_index_Select = i-1;
                }
                m_select_pos = vec_GeneLetter[i-1].pos;
            }
            else if (pos.x() > i_mid && pos.x() < i_high)
            {
                if(pos.y() > HLINEHIGHT + m_index_PeakLine*PEAKLINEHIGHT &&
                   pos.y() < 2*HLINEHIGHT +m_index_PeakLine*PEAKLINEHIGHT)
                {
                    m_bIsSelect = true;
                    m_index_Select = i;
                }
                m_select_pos = vec_GeneLetter[i].pos;
            }
        }
        update();
    }
}

void MultiPeakWidget::keyPressEvent(QKeyEvent *event)
{
    if(m_bIsSelect)
    {
        char type = event->key();
        qDebug()<<"keyPressEvent"<<type;

        QByteArray byetary("ATGCRYKMSWBDHVN-");
        QVector<GeneLetter> &vec_GeneLetter = m_vec_Peakline[m_index_PeakLine]->GetGeneLetter();
        if(byetary.contains(type) && vec_GeneLetter[m_index_Select].type != type)
        {
            vec_GeneLetter[m_index_Select].oldtype = vec_GeneLetter[m_index_Select].type;
            vec_GeneLetter[m_index_Select].type = type;
            update();
        }
    }
}

void MultiPeakWidget::SetSelectPos(int pos)
{
    int left_exclude,right_exclude;
    m_vec_Peakline[0]->GetExcludePos(left_exclude, right_exclude);
    QVector<GeneLetter> &vec_GeneLetter = m_vec_Peakline[0]->GetGeneLetter();

    int i_tmp = left_exclude + pos -1;
    m_select_pos = vec_GeneLetter[i_tmp].pos;
    update();

    QScrollArea *pParent = dynamic_cast<QScrollArea*>(parentWidget()->parentWidget());
    pParent->horizontalScrollBar()->setSliderPosition(m_select_pos.x()-230);
}
