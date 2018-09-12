#ifndef MULTIPEAKWIDGET_H
#define MULTIPEAKWIDGET_H

#include <QWidget>
#include "Core/fileTablebase.h"
#include <QMenu>

typedef struct _tagGeneLetter
{
    QPoint pos;
    char type;
    char oldtype;
}GeneLetter;

class PeakLine
{
public:
    PeakLine(long size);
    ~PeakLine();
    int getSize();
//    void SetBaseAPoint(int index,double x, double y);
//    void SetBaseTPoint(int index,double x, double y);
//    void SetBaseGPoint(int index,double x, double y);
//    void SetBaseCPoint(int index,double x, double y);
    void SetBasePoint(char type, double x, double y);
    QPolygonF& GetBasePoint(char type);
    //QPointF *getBaseAPoint(char type);
    void AddGeneLetter(GeneLetter &geneletter);
    QVector<GeneLetter>& GetGeneLetter();

    void SetFileName(QString &str);
    QString& GetFileName();

    void SetExcludePos(int left, int right);
    void GetExcludePos(int &left, int &right);

    void SetOffset(int offset);
    int GetOffset();

    void SetGssp(bool isGssp);
    bool GetGssp();
private:
    int m_left_exclude;
    int m_right_exclude;
    long m_lsize;
    long m_loffset;
    bool m_bGssp;
//    QPointF *base_a;
//    QPointF *base_t;
//    QPointF *base_g;
//    QPointF *base_c;
    QPolygonF m_vec_baseA;
    QPolygonF m_vec_baseT;
    QPolygonF m_vec_baseG;
    QPolygonF m_vec_baseC;

    QString m_strFileName;
    QVector<GeneLetter> m_vec_GeneLetter;
};



class MultiPeakWidget:public QWidget
{
    Q_OBJECT

public:
    MultiPeakWidget(QWidget *parent = nullptr);
    ~MultiPeakWidget();
    void SetPeakData(const QString &str_samplename, const QString &str_exon);
    void SetSelectPos(int pos);
    void AdjustPeakHeight(int height);
    void AdjustPeakY(int y);
    void AdjustPeakX(int x);
private:
    void CreateRightMenu();
    void ConnectSignalandSolt();
    void AdjustPeak();
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

private slots:
    void slotDelteThisFile();
    void slotActApplyOne();
    void slotActApplyAll();
    void slotActanalyzeLater();
    void slotActanalyzeNow();
    void slotActanalyze();
    void slotHighLightLeftPart();
    void slotHighLightRightPart();
    void slotResetExclude();

signals:
    //index:当前导航条index colnum：选中的位置和left_exclude的差值
    void signalPeakFocusPosition(int index, int colnum);
    void signalChangeDB();
    void signalActApplyOne();
    void signalActApplyAll();
    void signalActanalyzeLater();
private:
    QVector<Ab1FileTableBase> m_vec_filetable;
    int m_x_step;            //x轴间距
    double m_y_step;         //y轴缩放比例
    QPoint m_select_pos;    //选中碱基的位置
    bool m_bIsSelect;      //是否选中了一个碱基
    int m_index_Select;    //选中碱基的下标
    int m_index_PeakLine;   //当前选中的峰图下标
    long m_l_xSize;         //x轴的长度
    int m_iPeaklinehight;
    QString m_str_SampleName;
    QString m_str_Exon;
    QVector<QSharedPointer<PeakLine>> m_vec_Peakline;

    QMenu *m_pRightMenu;
    QAction *m_pActDelete;
    QAction *m_pActInsertBaseN;
    QAction *m_pActHideTraceDisplay;
//    QAction *m_pActFilterByCurrentBase;
//    QAction *m_pActRemoveLastBaseFilter;
//    QAction *m_pActRemoveAllBaseFilters;
//    QAction *m_pActRemoveLastNullAlleleFilter;
    QAction *m_pActApplyOne;
    QAction *m_pActApplyAll;
    QAction *m_pActanalyzeLater;
    QAction *m_pActanalyzeNow;
    QAction *m_pActanalyze;
    QAction *m_pActExcludeLeft;
    QAction *m_pActExcludeRight;
    QAction *m_pActResetExclude;
    //QAction *m_pActIntelligent_Analysis;
    //QAction *m_pActReset_Analysis;
//    QAction *m_pActChangeToTrace;
//    QAction *m_pActRemoveThisTrace;

};

#endif // MULTIPEAKWIDGET_H
