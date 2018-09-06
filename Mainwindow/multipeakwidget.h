#ifndef MULTIPEAKWIDGET_H
#define MULTIPEAKWIDGET_H

#include <QWidget>
#include "Core/fileTablebase.h"

typedef struct _tagGeneLetter
{
    QPoint pos;
    QChar type;
    QChar oldtype;
}GeneLetter;

class PeakLine
{
public:
    PeakLine(long size);
    ~PeakLine();
    int getSize();
    void SetBaseAPoint(int index,double x, double y);
    void SetBaseTPoint(int index,double x, double y);
    void SetBaseGPoint(int index,double x, double y);
    void SetBaseCPoint(int index,double x, double y);
    QPointF *getBaseAPoint(char type);
    void AddGeneLetter(GeneLetter &geneletter);
    QVector<GeneLetter>& GetGeneLetter();

    void SetFileName(QString &str);
    QString& GetFileName();

    void SetExcludePos(int left, int right);
    void GetExcludePos(int &left, int &right);

    void SetOffset(int offset);
    int GetOffset();
private:
    int m_left_exclude;
    int m_right_exclude;
    long m_lsize;
    long m_loffset;
    QPointF *base_a;
    QPointF *base_t;
    QPointF *base_g;
    QPointF *base_c;
    QString m_strFileName;
    QVector<GeneLetter> m_vec_GeneLetter;
};



class MultiPeakWidget:public QWidget
{
public:
    MultiPeakWidget(QWidget *parent = nullptr);
    ~MultiPeakWidget();
    void SetPeakData(const QString &str_samplename, const QString &str_exon);
    void SetSelectPos(int pos);
private:

    void paintEvent(QPaintEvent *event);
    void DrawPeakLines(QPainter *pter);
    void DrawHLines(QPainter *pter);
    void DrawPeakHead(QPainter *pter);
    void DrawBasePosition(QPainter *pter);
    void DrawSelectFrame(QPainter *pter);
    void DrawExcludeArea(QPainter *pter);
    void GetBaseColor(QPainter *pter, const QChar &base);

    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
private:
    QVector<Ab1FileTableBase> m_vec_filetable;
    int m_x_step;            //x轴间距
    double m_y_step;         //y轴缩放比例
    QPoint m_select_pos;    //选中碱基的位置
    bool m_bIsSelect;      //是否选中了一个碱基
    int m_index_Select;    //选中碱基的下标
    int m_index_PeakLine;   //当前选中的峰图下标
    long m_l_xSize;         //x轴的长度

    QString m_str_SampleName;
    QString m_str_Exon;
    QVector<QSharedPointer<PeakLine>> m_vec_Peakline;
};

#endif // MULTIPEAKWIDGET_H
