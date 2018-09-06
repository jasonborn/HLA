#ifndef EXONNAVIGATORWIDGET_H
#define EXONNAVIGATORWIDGET_H

#include <QWidget>
#include "all_base_struct.h"

typedef struct _tagExon
{
   int i_exonstartpos;    //导航条起始pos
   int i_exonendpos;      //导航条结束pos
   int i_screenstartpos;      //导航条起始x坐标
   int i_screenwidth;        //导航条长度
   int i_exonindex;     //导航条显示index
}Exon;

class ExonNavigatorWidget:public QWidget
{
    Q_OBJECT
public:
    ExonNavigatorWidget(QWidget *parent = nullptr);
    ~ExonNavigatorWidget();
    void SetExonData(QString &str_sample, QString &str_gene);
private:

    void paintEvent(QPaintEvent *event);
    void DrawExonArea(QPainter &paiter);
    void DrawSelectFrame(QPainter &paiter);
    void DrawExonPos(QPainter &paiter);
    void mousePressEvent(QMouseEvent *event);
    int PeakPosToScreenPos(int oldpos);
    int ScreenPosToPeakPos(int newpos);

signals:
    void signalExonFocusPosition(int selectpos,int exonstart, int index);//选中的峰图pos，导航条起始pos,导航条index
private:
    int m_iwidth;
    int m_iheight;
    int m_itop1;
    int m_itop2;
    int m_itop3;
    int m_ih0;
    int m_ih1;
    int m_ih2;
    int m_ih3;
    int m_iSelectPos;
    int m_iMidgap;          //碱基导航条中间间距
    int m_igap;             //碱基导航条两端间距
    int m_isub_index;       //外显子index,最大最小值之差
    int m_isub_pos;         //外显子x坐标，最大最小之差
    double m_dXscale;       //计算每个碱基的间距
    QVector<Exon> m_vec_Exon; //导航条信息
    QVector<int> m_vecExonIndex;
    ExonNavigatorInfo m_Exoninfo;
    QString m_str_SampleName;
    QString m_str_GeneName;
};

#endif // EXONNAVIGATORWIDGET_H
