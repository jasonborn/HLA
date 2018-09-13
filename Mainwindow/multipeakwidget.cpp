#include "multipeakwidget.h"
#include <QPainter>
#include "DataBase/soaptypingdb.h"
#include <QDebug>
#include <QMouseEvent>
#include <QtAlgorithms>
#include <QScrollArea>
#include <QScrollBar>
#include <QTime>
#include <set>
#include "ThreadTask/analysissamplethreadtask.h"

const int PEAKLINEHIGHT = 200;
const int HLINEHIGHT = 20;

QTime g_time_peak;
PeakLine::PeakLine(long size):m_lsize(size)
{
    m_loffset = 0;
    m_vec_baseA.reserve(m_lsize);
    m_vec_baseT.reserve(m_lsize);
    m_vec_baseG.reserve(m_lsize);
    m_vec_baseC.reserve(m_lsize);
    m_vec_GeneLetter.reserve(m_lsize);
}

PeakLine::~PeakLine()
{
    qDebug()<<"~PeakLine";
    m_vec_baseA.clear();
    m_vec_baseT.clear();
    m_vec_baseG.clear();
    m_vec_baseC.clear();
    m_vec_GeneLetter.clear();
}

int PeakLine::getSize()
{
    return m_lsize;
}

void PeakLine::SetBasePoint(char type, double x, double y)
{
    switch (type)
    {
    case 'A':
        m_vec_baseA.push_back(QPointF(x,y));
        break;
    case 'T':
        m_vec_baseT.push_back(QPointF(x,y));
        break;
    case 'G':
        m_vec_baseG.push_back(QPointF(x,y));
        break;
    case 'C':
        m_vec_baseC.push_back(QPointF(x,y));
        break;
    }
}

 QPolygonF& PeakLine::GetBasePoint(char type)
 {
     switch (type)
     {
         case 'A': return m_vec_baseA;
         case 'T': return m_vec_baseT;
         case 'G': return m_vec_baseG;
         case 'C': return m_vec_baseC;
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

void PeakLine::SetGssp(bool isGssp)
{
    m_bGssp = isGssp;
}
bool PeakLine::GetGssp()
{
    return  m_bGssp;
}

MultiPeakWidget::MultiPeakWidget(QWidget *parent)
    :QWidget(parent)
{
    m_x_step = 28;
    m_y_step = 5;
    m_bIsSelect = false;
    m_l_xSize = 0;
    m_iPeakHeight = PEAKLINEHIGHT; //峰图高度
    setFocusPolicy(Qt::StrongFocus);//如果不调用，keyPressEvent不响应
    CreateRightMenu();
    ConnectSignalandSolt();
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

    g_time_peak.start();
    m_vec_filetable.clear();
    m_vec_Peakline.clear();

    SoapTypingDB::GetInstance()->getAlldataFormRealTime(str_samplename, str_exon, m_vec_filetable);
    int i_count_peak = m_vec_filetable.size();
    std::set<int> set_left,set_right; //以AlignEndPos为界，计算左右两边的长度
    QVector<int> vec_left_copy;
    for(int i=0;i<i_count_peak;i++)
    {
        Ab1FileTableBase &table = m_vec_filetable[i];
        long l_size = table.getSignalNumber();
        QSharedPointer<PeakLine> pPeakLine = QSharedPointer<PeakLine>(new PeakLine(l_size));
        pPeakLine->SetFileName(table.getFileName());
        pPeakLine->SetExcludePos(table.getAlignStartPos(),table.getAlignEndPos());
        pPeakLine->SetGssp(table.getIsGssp());

        int i_AlginEndPos = table.getAlignEndPos();
        int i_right = table.getBaseNumber() - i_AlginEndPos;
        vec_left_copy.push_back(i_AlginEndPos);
        set_left.insert(i_AlginEndPos);
        set_right.insert(i_right);


        m_vec_Peakline.push_back(pPeakLine);
    }

    for(int i=0;i<i_count_peak;i++)
    {
        QSharedPointer<PeakLine> pPeakLine = m_vec_Peakline[i];
        int i_offset = *set_left.crbegin() - vec_left_copy[i];
        pPeakLine->SetOffset(i_offset);
    }

    m_l_xSize = *(set_left.crbegin()) + *(set_right.crbegin());//取左右最大值，相加为总长度

    qDebug()<<g_time_peak.restart();
    SetPeakLineData();
    qDebug()<<g_time_peak.restart();


}

//这个地方要注意性能问题
void MultiPeakWidget::SetPeakLineData()
{
    double d_ystep = m_y_step*1.0/100;
    for(int k=0;k<m_vec_Peakline.size();k++)
    {
        Ab1FileTableBase &table = m_vec_filetable[k];
        long l_size = table.getSignalNumber();
        QSharedPointer<PeakLine> pPeakLine = m_vec_Peakline[k];
        int i_offset = (pPeakLine->GetOffset())*m_x_step;

        QVector<QStringRef> A_list = table.getBaseASignal().splitRef(':');
        QVector<QStringRef> T_list = table.getBaseTSignal().splitRef(':');
        QVector<QStringRef> G_list = table.getBaseGSignal().splitRef(':');
        QVector<QStringRef> C_list = table.getBaseCSignal().splitRef(':');

        QStringList baseposion = table.getBasePostion().split(":");
        int i_basenum = table.getBaseNumber();
        int pos_i = 0;
        double pos_j = 0.0;
        int i_index = 0;
        int begin = 0;
        int height_tmp = (k+1)*m_iPeakHeight;

        pPeakLine->GetBasePoint('A').clear();
        pPeakLine->GetBasePoint('T').clear();
        pPeakLine->GetBasePoint('G').clear();
        pPeakLine->GetBasePoint('C').clear();

        for(int i=0;i<i_basenum;i++)
        {
            int end = baseposion[i].toInt();
            int num = end - begin;
            pos_i = i*m_x_step + i_offset;
            for(int j = 0; j < num; j++)
            {
                pos_j = pos_i + j*m_x_step/num;
                double a_y = height_tmp-A_list[i_index].toInt()*d_ystep;
                pPeakLine->SetBasePoint('A',pos_j,a_y);

                double t_y = height_tmp-T_list[i_index].toInt()*d_ystep;
                pPeakLine->SetBasePoint('T',pos_j,t_y);

                double g_y = height_tmp-G_list[i_index].toInt()*d_ystep;
                pPeakLine->SetBasePoint('G',pos_j,g_y);

                double c_y = height_tmp-C_list[i_index].toInt()*d_ystep;
                pPeakLine->SetBasePoint('C',pos_j,c_y);

                i_index++;
            }
            begin = end;
        }

        int num = l_size - i_index;
        for(int j = 0;i_index < l_size;i_index++,j++)
        {
            pos_j = pos_i + j*m_x_step/num;
            double a_y = height_tmp-A_list[i_index].toInt()*d_ystep;
            pPeakLine->SetBasePoint('A',pos_j,a_y);

            double t_y = height_tmp-T_list[i_index].toInt()*d_ystep;
            pPeakLine->SetBasePoint('T',pos_j,t_y);

            double g_y = height_tmp-G_list[i_index].toInt()*d_ystep;
            pPeakLine->SetBasePoint('G',pos_j,g_y);

            double c_y = height_tmp-C_list[i_index].toInt()*d_ystep;
            pPeakLine->SetBasePoint('C',pos_j,c_y);
        }

        if(pPeakLine->GetGeneLetter().isEmpty())
        {
            QByteArray baseseq = table.getBaseSequence();
            for(int i=0;i<i_basenum;i++)
            {
                GeneLetter geneletter;
                geneletter.pos.setX((i+1)*m_x_step + i_offset);
                geneletter.pos.setY(35 + k*m_iPeakHeight);
                geneletter.type = baseseq[i];
                geneletter.oldtype = ' ';
                pPeakLine->AddGeneLetter(geneletter);
            }
        }
        else
        {
            for(int i=0;i<pPeakLine->GetGeneLetter().size();i++)
            {
                GeneLetter &geneletter = pPeakLine->GetGeneLetter()[i];
                geneletter.pos.setX((i+1)*m_x_step + i_offset);
                geneletter.pos.setY(35 + k*m_iPeakHeight);
            }
        }
    }
    resize(m_l_xSize*m_x_step, m_vec_Peakline.size()*m_iPeakHeight+40);//如果不调用，paintEvent不响应
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
        pter->setPen(Qt::green);
        pter->drawPolyline(peakline->GetBasePoint('A'));

        pter->setPen(Qt::red);
        pter->drawPolyline(peakline->GetBasePoint('T'));

        pter->setPen(Qt::black);
        pter->drawPolyline(peakline->GetBasePoint('G'));

        pter->setPen(Qt::blue);
        pter->drawPolyline(peakline->GetBasePoint('C'));
    }
}

void MultiPeakWidget::DrawHLines(QPainter *pter)
{
    QPen pen(Qt::darkGray);
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);
    pter->setPen(pen);


    int i_width = m_l_xSize*m_x_step;
    for(int i=0;i<m_vec_Peakline.size();i++)
    {
        int i_height = i*m_iPeakHeight; //每个峰图的高度

        pter->drawLine(0,20+i_height, i_width,20+i_height);

        pter->drawLine(0,40+i_height, i_width,40+i_height);

        pter->drawLine(0,60+i_height, i_width,60+i_height);

        pter->drawLine(0,m_iPeakHeight+i_height, i_width,m_iPeakHeight+i_height);
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
        pter->drawText(3,15+i*m_iPeakHeight, m_vec_Peakline[i]->GetFileName());
        int i_index = 0;
        foreach(const GeneLetter &letter, m_vec_Peakline[i]->GetGeneLetter())
        {
            GetBaseColor(pter, letter.type);
            pter->drawText(letter.pos, QString(letter.type));
            if(letter.oldtype != ' ')
            {
                GetBaseColor(pter, letter.oldtype);
                pter->drawText(letter.pos.x(),letter.pos.y()+HLINEHIGHT, QString(letter.oldtype));
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
    pter->drawRect(m_select_pos.x()-5,0,14,m_vec_Peakline.size()*m_iPeakHeight); //绘制比对框

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

    int i_width = m_l_xSize*m_x_step;
    for(int i=0;i<m_vec_Peakline.size();i++)
    {
        int i_height = i*m_iPeakHeight;
        int left_exclude,right_exclude;
        m_vec_Peakline[i]->GetExcludePos(left_exclude, right_exclude);

        QVector<GeneLetter> &vec_geneLetter = m_vec_Peakline[i]->GetGeneLetter();
        int w_left =  vec_geneLetter[left_exclude].pos.x();
        pter->drawRect(0,60+i_height, w_left, m_iPeakHeight-60);
        int w_right = vec_geneLetter[right_exclude-1].pos.x();
        pter->drawRect(w_right,60+i_height, i_width, m_iPeakHeight-60);
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
        if(pos.y() > i*m_iPeakHeight && pos.y() < (i+1)*m_iPeakHeight)
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
                if(pos.y() > HLINEHIGHT + m_index_PeakLine*m_iPeakHeight &&
                   pos.y() < 2*HLINEHIGHT + m_index_PeakLine*m_iPeakHeight)
                {
                    m_bIsSelect = true;
                    m_index_Select = i-1;
                }
                m_select_pos = vec_GeneLetter[i-1].pos;
                emit signalPeakFocusPosition(m_str_Exon.toInt(), i-1-left_exclude);
                break;
            }
            else if (pos.x() > i_mid && pos.x() < i_high)
            {
                if(pos.y() > HLINEHIGHT + m_index_PeakLine*m_iPeakHeight &&
                   pos.y() < 2*HLINEHIGHT +m_index_PeakLine*m_iPeakHeight)
                {
                    m_bIsSelect = true;
                    m_index_Select = i;
                }
                m_select_pos = vec_GeneLetter[i].pos;
                emit signalPeakFocusPosition(m_str_Exon.toInt(), i-left_exclude);
                break;
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

    int i_tmp = left_exclude + pos;
    m_select_pos = vec_GeneLetter[i_tmp].pos;
    update();

    QScrollArea *pParent = dynamic_cast<QScrollArea*>(parentWidget()->parentWidget());
    pParent->horizontalScrollBar()->setSliderPosition(m_select_pos.x()-230);

    qDebug()<<"MultiPeakWidget::SetSelectPos"<<pos<<m_select_pos;
}


void MultiPeakWidget::CreateRightMenu()
{
    m_pActDelete = new QAction(tr("Delete Selected File"),this);
    m_pActInsertBaseN = new QAction(tr("Insert Base 'N'"),this);
    m_pActHideTraceDisplay = new QAction(tr("Hide Trace Display"),this);
//    m_pActFilterByCurrentBase = new QAction(tr("Filter By Current Base"),this);
//    m_pActRemoveLastBaseFilter = new QAction(tr("Remove Last Base Filter"),this);
//    m_pActRemoveAllBaseFilters = new QAction(tr("Remove All Base Filters"),this);
//    m_pActRemoveLastNullAlleleFilter = new QAction(tr("Remove Last Null AlleleFilter"),this);

//    m_pActInsertBaseN->setDisabled(true);
//    m_pActHideTraceDisplay->setDisabled(true);
//    m_pActFilterByCurrentBase->setDisabled(true);
//    m_pActRemoveLastBaseFilter->setDisabled(true);
//    m_pActRemoveAllBaseFilters->setDisabled(true);
//    m_pActRemoveLastNullAlleleFilter->setDisabled(true);


    m_pActApplyOne = new QAction(QIcon(":/images/apply.png"),tr("Apply One"),this);
    m_pActApplyOne->setIconVisibleInMenu(false);
    m_pActApplyOne->setShortcut(QKeySequence(Qt::ALT+Qt::Key_E));

    m_pActApplyAll = new QAction(QIcon(":/images/apply.png"),tr("Apply All"),this);
    m_pActApplyAll->setDisabled(true);
    m_pActApplyAll->setShortcut(QKeySequence(Qt::ALT+Qt::Key_R));

    m_pActanalyzeNow = new QAction(QIcon(":/images/apply.png"),tr("Edit One"),this);
    m_pActanalyzeNow->setDisabled(true);
    m_pActanalyzeNow->setShortcut(QKeySequence(Qt::ALT+Qt::Key_O));

    m_pActanalyzeLater = new QAction(QIcon(":/images/apply.png"),tr("Edit Multi"),this);
    m_pActanalyzeLater->setIconVisibleInMenu(false);
    m_pActanalyzeLater->setShortcut(QKeySequence(Qt::ALT+Qt::Key_M));

    m_pActanalyze = new QAction(tr("Analyze"),this);
    m_pActanalyze->setShortcut(QKeySequence(Qt::ALT+Qt::Key_T));
    m_pActanalyze->setDisabled(true);

    m_pActExcludeLeft = new QAction(tr("Exclude left"),this);
    m_pActExcludeRight = new QAction(tr("Exclude right"),this);
    m_pActResetExclude = new QAction(tr("Reset Exclude"),this);

//    m_pActIntelligent_Analysis = new QAction(tr("Smart Analysis"),this);
//    m_pActReset_Analysis = new QAction(tr("Reset Analysis"),this);
//    m_pActChangeToTrace = new QAction(tr("Change Exon Or Direction To This Trace"),this);
//    m_pActRemoveThisTrace = new QAction(tr("Remove This Trace"),this);

//    m_pActChangeToTrace->setDisabled(true);
//    m_pActRemoveThisTrace->setDisabled(true);

//    m_pActIntelligent_Analysis->setVisible(false);
//    m_pActReset_Analysis->setVisible(false);

    m_pRightMenu = new QMenu(this);
    m_pRightMenu->addAction(m_pActDelete);
    m_pRightMenu->addAction(m_pActInsertBaseN);
    m_pRightMenu->addAction(m_pActHideTraceDisplay);
    m_pRightMenu->addSeparator();

    m_pRightMenu->addAction(m_pActApplyOne);
    m_pRightMenu->addAction(m_pActApplyAll);
    m_pRightMenu->addSeparator();

    m_pRightMenu->addAction(m_pActanalyzeNow);
    m_pRightMenu->addAction(m_pActanalyzeLater);
    m_pRightMenu->addSeparator();

    m_pRightMenu->addAction(m_pActanalyze);
    m_pRightMenu->addSeparator();
    m_pRightMenu->addAction(m_pActExcludeLeft);
    m_pRightMenu->addAction(m_pActExcludeRight);
    m_pRightMenu->addAction(m_pActResetExclude);
    m_pRightMenu->addSeparator();

//    m_pRightMenu->addAction(m_pActIntelligent_Analysis);
//    m_pRightMenu->addAction(m_pActReset_Analysis);
}

void MultiPeakWidget::contextMenuEvent(QContextMenuEvent *event)
{
    m_pRightMenu->exec(QCursor::pos());
    event->accept();
}

void MultiPeakWidget::ConnectSignalandSolt()
{
    connect(m_pActDelete, &QAction::triggered, this, &MultiPeakWidget::slotDelteThisFile);
    connect(m_pActApplyOne, &QAction::triggered, this, &MultiPeakWidget::slotActApplyOne);
    connect(m_pActApplyAll, &QAction::triggered, this, &MultiPeakWidget::slotActApplyAll);
    connect(m_pActanalyzeLater, &QAction::triggered, this, &MultiPeakWidget::slotActanalyzeLater);
    connect(m_pActanalyzeNow, &QAction::triggered, this, &MultiPeakWidget::slotActanalyzeNow);
    connect(m_pActanalyze, &QAction::triggered, this, &MultiPeakWidget::slotActanalyze);
    connect(m_pActExcludeLeft, &QAction::triggered, this, &MultiPeakWidget::slotHighLightLeftPart);
    connect(m_pActExcludeRight, &QAction::triggered, this, &MultiPeakWidget::slotHighLightRightPart);
    connect(m_pActResetExclude, &QAction::triggered, this, &MultiPeakWidget::slotResetExclude);
}

void MultiPeakWidget::slotDelteThisFile()
{
    QString &str_file = m_vec_Peakline[m_index_PeakLine]->GetFileName();
    bool isgssp = m_vec_Peakline[m_index_PeakLine]->GetGssp();
    SoapTypingDB::GetInstance()->deleteFile(isgssp, str_file);

    AnalysisSampleThreadTask *pTask = new AnalysisSampleThreadTask(m_str_SampleName);
    pTask->run();
    delete pTask;

    emit signalChangeDB();
}

void MultiPeakWidget::slotActApplyOne()
{
    m_pActApplyOne->setDisabled(true);
    m_pActApplyAll->setDisabled(false);
    m_pActApplyOne->setIconVisibleInMenu(true);
    m_pActApplyAll->setIconVisibleInMenu(false);

    emit signalActApplyOne();
}

void MultiPeakWidget::slotActApplyAll()
{
    m_pActApplyOne->setIconVisibleInMenu(false);
    m_pActApplyAll->setIconVisibleInMenu(true);
    m_pActApplyOne->setDisabled(false);
    m_pActApplyAll->setDisabled(true);

    emit signalActApplyAll();
}

void MultiPeakWidget::slotActanalyzeLater()
{
    m_pActanalyzeNow->setIconVisibleInMenu(false);
    m_pActanalyzeLater->setIconVisibleInMenu(true);
    m_pActanalyzeLater->setDisabled(true);
    m_pActanalyzeNow->setDisabled(false);

    emit signalActanalyzeLater();
}

void MultiPeakWidget::slotActanalyzeNow()
{
    m_pActanalyzeNow->setIconVisibleInMenu(true);
    m_pActanalyzeLater->setIconVisibleInMenu(false);
    m_pActanalyzeLater->setDisabled(false);
    m_pActanalyzeNow->setDisabled(true);
}

void MultiPeakWidget::slotActanalyze()
{
    m_pActanalyze->setDisabled(true);

    AnalysisSampleThreadTask *pTask = new AnalysisSampleThreadTask(m_str_SampleName);
    pTask->run();
    delete pTask;

    emit signalChangeDB();
}

void MultiPeakWidget::slotHighLightLeftPart()
{

}

void MultiPeakWidget::slotHighLightRightPart()
{

}

void MultiPeakWidget::slotResetExclude()
{

}

void MultiPeakWidget::AdjustPeakHeight(int height)
{
    m_iPeakHeight += height;
    if(m_iPeakHeight >= 160 && m_iPeakHeight <= 300)
    {
        SetPeakLineData();
        update();
    }
    else
    {
        m_iPeakHeight -= height;
    }
}

void MultiPeakWidget::AdjustPeakY(int y)
{
    m_y_step += y;
    if(m_y_step >= 2 && m_y_step <= 8)
    {
        SetPeakLineData();
        update();
    }
    else
    {
        m_y_step -= y;
    }
}

void MultiPeakWidget::AdjustPeakX(int x)
{
    m_x_step += x;
    if(m_x_step >= 14 && m_x_step <= 42)
    {
        SetPeakLineData();
        update();
    }
    else
    {
        m_x_step -= x;
    }
}

void MultiPeakWidget::RestorePeak()
{
    m_iPeakHeight = PEAKLINEHIGHT;
    m_x_step = 28;
    m_y_step = 5;
    SetPeakLineData();
    update();
}
