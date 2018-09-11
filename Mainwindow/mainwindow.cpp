#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include "Dialog/openfiledialog.h"
#include <QSplitter>
#include "sampletreewidget.h"
#include "matchlistwidget.h"
#include <QScrollArea>
#include "multipeakwidget.h"
#include "exonnavigatorwidget.h"
#include "basealigntablewidget.h"
#include <QtDebug>
#include <QScrollBar>
#include <QTime>
#include "Dialog/savefiledlg.h"

QTime g_time_main;
QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InitUI();

    ConnectSignalandSolt();
    SetStatusbar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitUI()
{
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical);
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical);
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);

    m_pExonNavigatorWidget = new ExonNavigatorWidget;
    m_pExonNavigatorWidget->setMinimumHeight(100);

    m_pBaseAlignTableWidget = new BaseAlignTableWidget;
    m_pBaseAlignTableWidget->setMinimumHeight(220);

    m_pPeak_area = new QScrollArea;
    m_pMultiPeakWidget = new MultiPeakWidget();
    m_pPeak_area->setWidget(m_pMultiPeakWidget);
    m_pMultiPeakWidget->setMinimumHeight(400);

    m_pSampleTreeWidget = new SampleTreeWidget;
    m_pSampleTreeWidget->setMinimumSize(400,320);

    m_pMatchListWidget = new MatchListWidget;

    leftSplitter->addWidget(m_pExonNavigatorWidget);
    leftSplitter->addWidget(m_pBaseAlignTableWidget);
    leftSplitter->addWidget(m_pPeak_area);
    leftSplitter->setStretchFactor(0,10);
    leftSplitter->setStretchFactor(1,22);
    leftSplitter->setStretchFactor(2,68);

    rightSplitter->addWidget(m_pSampleTreeWidget);
    rightSplitter->addWidget(m_pMatchListWidget);
    rightSplitter->setStretchFactor(0,50);
    rightSplitter->setStretchFactor(1,50);

    mainSplitter->addWidget(leftSplitter);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0,75);
    mainSplitter->setStretchFactor(1,25);

    ui->verticalLayout->addWidget(mainSplitter);

    ui->statusbarleft->setStyleSheet("QLabel{border:1px solid rgb(139,139,139);}");
    ui->statusbarright->setStyleSheet("QLabel{border:1px solid rgb(139,139,139);}");

    m_pSampleTreeWidget->SetTreeData();
}

void MainWindow::ConnectSignalandSolt()
{
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::slotShowOpenDlg);
    connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::slotShowLoadFileDlg);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::slotShowSaveDlg);
    connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::slotShowDeleteDlg);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::slotShowExportDlg);

    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::slotReset);
    connect(ui->actionForward, &QAction::triggered, this, &MainWindow::slotMisPosForward);
    connect(ui->actionBackward, &QAction::triggered, this, &MainWindow::slotMisPosBackward);
    connect(ui->actionApproval, &QAction::triggered, this, &MainWindow::slotMarkAllSampleApproved);
    connect(ui->actionReview, &QAction::triggered, this, &MainWindow::slotMarkAllSampleReviewed);
    connect(ui->actionAllele_Comparator, &QAction::triggered, this, &MainWindow::slotAlignPair);
    connect(ui->actionAllele_Alignment, &QAction::triggered, this, &MainWindow::slotAlignLab);
    connect(ui->actionUpdate_Database, &QAction::triggered, this, &MainWindow::slotUpdateDatabase);
    connect(ui->actionSetting, &QAction::triggered, this, &MainWindow::slotControl);

    connect(ui->actionY_Range_Zoom_Increase, &QAction::triggered, this, &MainWindow::slotyRangeRoomUp);
    connect(ui->actionY_Range_Zoom_Reduce, &QAction::triggered, this, &MainWindow::slotyRangeRoomDown);
    connect(ui->actionY_Zoom_Increase, &QAction::triggered, this, &MainWindow::slotyRoomUp);
    connect(ui->actionY_Zoom_Reduce, &QAction::triggered, this, &MainWindow::slotyRoomDown);
    connect(ui->actionX_Zoom_Increase, &QAction::triggered, this, &MainWindow::slotxRoomUp);
    connect(ui->actionX_Zoom_Reduce, &QAction::triggered, this, &MainWindow::slotxRoomDown);
    connect(ui->actionReset_Zoom_Setting, &QAction::triggered, this, &MainWindow::resetRoomSetting);


    connect(ui->actionApply_All, &QAction::triggered, this, &MainWindow::slotApplyAll);
    connect(ui->actionApply_One, &QAction::triggered, this, &MainWindow::slotApplyOne);
    connect(ui->actionEdit_Multi, &QAction::triggered, this, &MainWindow::slotAnalyseLater);
    connect(ui->actionEdit_One, &QAction::triggered, this, &MainWindow::slotAnalyseNow);
    connect(ui->actionAnalyze, &QAction::triggered, this, &MainWindow::slotanalyse);


    connect(m_pSampleTreeWidget, &QTreeWidget::itemClicked, this, &MainWindow::slotSampleTreeItemChanged);
    connect(m_pExonNavigatorWidget, &ExonNavigatorWidget::signalExonFocusPosition,
            this, &MainWindow::slotExonFocusPosition);

    connect(m_pBaseAlignTableWidget, &QTableWidget::itemClicked, this, &MainWindow::slotAlignTableFocusPosition);

    connect(m_pMultiPeakWidget, &MultiPeakWidget::signalPeakFocusPosition, this, &MainWindow::slotPeakFocusPosition);


}

void MainWindow::DisConnectSignalandSolt()
{

}

void MainWindow::slotShowOpenDlg()
{
    OpenFileDialog dlg;
    dlg.exec();
    m_pSampleTreeWidget->SetTreeData();
}

void MainWindow::SetStatusbar()
{
    QString strA("<b><font color='#39B54A' size='5'>A</font></b>");
    QString strC("<b><font color='#0000ff' size='5'>C</font></b>");
    QString strG("<b><font color='#000000' size='5'>G</font></b>");
    QString strT("<b><font color='#ff0000' size='5'>T</font></b>");

    QString strR("<b><font color='#C1272d' size='5'>R-</font></b>");
    QString strY("<b><font color='#C1272d' size='5'>Y-</font></b>");
    QString strK("<b><font color='#C1272d' size='5'>K-</font></b>");
    QString strM("<b><font color='#C1272d' size='5'>M-</font></b>");
    QString strS("<b><font color='#C1272d' size='5'>S-</font></b>");
    QString strW("<b><font color='#C1272d' size='5'>W-</font></b>");
    QString strB("<b><font color='#C1272d' size='5'>B-</font></b>");
    QString strD("<b><font color='#C1272d' size='5'>D-</font></b>");
    QString strH("<b><font color='#C1272d' size='5'>H-</font></b>");
    QString strV("<b><font color='#C1272d' size='5'>V-</font></b>");
    QString strspace("  ");

    QString str = strR + strA+ strG + strspace +
                  strY + strT + strC + strspace +
                  strK + strG + strT + strspace +
                  strM + strA + strC + strspace +
                  strS + strG + strC + strspace +
                  strW + strA + strT + strspace +
                  strB + strG + strT + strC + strspace +
                  strD + strG + strA + strT + strspace +
                  strH + strA + strC + strT + strspace +
                  strV + strG + strC + strA;
    ui->statusbarleft->setText(str);
}

void MainWindow::slotSampleTreeItemChanged(QTreeWidgetItem *item, int col)
{
    QString str_name = item->text(0);
    QString str_info = item->text(1);

    QStringList str_list = str_name.split('_');
    m_pMatchListWidget->SetTableData(str_list[0],"");

    m_pExonNavigatorWidget->SetExonData(str_list[0],str_list[1]);

    m_pBaseAlignTableWidget->SetAlignTableData(str_list[0]);

    g_time_main.start();
    m_pMultiPeakWidget->SetPeakData(str_list[0],str_info.left(1));
    qDebug()<<"total time"<<g_time_main.elapsed();

    int startpos;
    int selectpos;
    int exonstartpos;
    m_pExonNavigatorWidget->setSelectFramePosition(str_info.left(1).toInt(), startpos, selectpos, exonstartpos);

    int i_sub_table = selectpos - startpos;
    m_pBaseAlignTableWidget->selectColumn(i_sub_table);
    m_pBaseAlignTableWidget->horizontalScrollBar()->setSliderPosition((i_sub_table+1)*20+230);//不能和峰图同步

    int i_sub = selectpos - exonstartpos;
    m_pMultiPeakWidget->SetSelectPos(i_sub);
}

//导航条起始pos,选中的峰图pos，选中的导航条起始pos,选中的导航条index
void MainWindow::slotExonFocusPosition(int startpos, int selectpos, int exonstartpos, int index)
{
    int i_sub_table = selectpos - startpos;
    int sliderPos = (i_sub_table+1)*20+230;
    m_pBaseAlignTableWidget->horizontalScrollBar()->setValue(sliderPos);//不能和峰图同步
    m_pBaseAlignTableWidget->selectColumn(i_sub_table+1);

    int i_sub = selectpos - exonstartpos;
    QString str_name;
    m_pSampleTreeWidget->SetSelectItem(index, str_name);
    QStringList str_list = str_name.split('_');
    m_pMultiPeakWidget->SetPeakData(str_list[0],str_list[2].left(1));
    m_pMultiPeakWidget->SetSelectPos(i_sub);
}

void MainWindow::slotAlignTableFocusPosition(QTableWidgetItem *item)
{
    int selectpos;
    int exonstartpos;
    int index;

    int i_colnum = item->column();

    m_pExonNavigatorWidget->SetSelectPos(i_colnum, selectpos, exonstartpos ,index);//不能和峰图同步

    int i_sub = selectpos - exonstartpos;
    QString str_name;
    m_pSampleTreeWidget->SetSelectItem(index, str_name);
    QStringList str_list = str_name.split('_');
    m_pMultiPeakWidget->SetPeakData(str_list[0],str_list[2].left(1));
    m_pMultiPeakWidget->SetSelectPos(i_sub);
}

void MainWindow::slotPeakFocusPosition(int index, int colnum)
{
    int test;
    m_pExonNavigatorWidget->SetSelectFramePos(index, colnum,test);
    m_pBaseAlignTableWidget->selectColumn(test+1);
    m_pBaseAlignTableWidget->horizontalScrollBar()->setSliderPosition((test+1)*20+230);
}


void MainWindow::slotShowSaveDlg()
{
    SaveFileDlg dlg(this);
    dlg.exec();
}


void MainWindow::slotShowLoadFileDlg()
{

}

void MainWindow::slotShowDeleteDlg()
{

}

void MainWindow::slotShowExportDlg()
{

}

void MainWindow::slotReset()
{

}

void MainWindow::slotMisPosForward()
{

}

void MainWindow::slotMisPosBackward()
{

}

void MainWindow::slotMarkAllSampleApproved()
{

}

void MainWindow::slotMarkAllSampleReviewed()
{

}

void MainWindow::slotAlignPair()
{

}

void MainWindow::slotAlignLab()
{

}

void MainWindow::slotUpdateDatabase()
{

}

void MainWindow::slotControl()
{

}

void MainWindow::slotyRangeRoomUp()
{

}

void MainWindow::slotyRangeRoomDown()
{

}

void MainWindow::slotyRoomUp()
{

}

void MainWindow::slotyRoomDown()
{

}

void MainWindow::slotxRoomUp()
{

}

void MainWindow::slotxRoomDown()
{

}

void MainWindow::resetRoomSetting()
{

}

void MainWindow::slotApplyOne()
{

}

void MainWindow::slotApplyAll()
{

}

void MainWindow::slotAnalyseLater()
{

}

void MainWindow::slotAnalyseNow()
{

}

void MainWindow::slotanalyse()
{

}
