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

QTime g_time;
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
    connect(m_pSampleTreeWidget, &QTreeWidget::itemClicked, this, &MainWindow::slotSampleTreeItemChanged);
    connect(m_pExonNavigatorWidget, &ExonNavigatorWidget::signalExonFocusPosition,
            this, &MainWindow::slotExonFocusPosition);
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

    g_time.start();
    m_pMultiPeakWidget->SetPeakData(str_list[0],str_info.left(1));
    qDebug()<<g_time.elapsed();
}

void MainWindow::slotExonFocusPosition(int startpos, int selectpos, int exonstartpos, int index)
{
    int i_sub_table = selectpos - startpos;
    m_pBaseAlignTableWidget->selectColumn(i_sub_table+1);
    m_pBaseAlignTableWidget->horizontalScrollBar()->setSliderPosition((i_sub_table+1)*20+230);//不能和峰图同步

    int i_sub = selectpos - exonstartpos;
    m_pSampleTreeWidget->SetSelectItem(index);
    m_pMultiPeakWidget->SetSelectPos(i_sub);


}
