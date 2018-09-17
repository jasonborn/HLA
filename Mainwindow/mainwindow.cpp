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
#include "Dialog/loadfiledlg.h"
#include "Dialog/deletefiledlg.h"
#include "Dialog/reportdlg.h"
#include "DataBase/soaptypingdb.h"
#include <QMessageBox>
#include "ThreadTask/analysissamplethreadtask.h"
#include "Dialog/allelepairdlg.h"
#include "Dialog/setdlg.h"
#include "Dialog/exontimdlg.h"
#include "Dialog/alignmentdlg.h"
#include <QFileInfo>
#include <QDesktopServices>

QTime g_time_main;
QT_CHARTS_USE_NAMESPACE

const QString VERSION("1.0.6.0");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    InitUI();
    ConnectSignalandSlot();
    SetStatusbar();
    update();
    m_pSampleTreeWidget->SetTreeData();
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

    ui->actionAnalyze->setEnabled(false);
}

void MainWindow::ConnectSignalandSlot()
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
    connect(ui->actionSet_Thread, &QAction::triggered, this, &MainWindow::slotControl);
    connect(ui->actionSet_Exon_Trim, &QAction::triggered, this, &MainWindow::slotSetExonTrim);

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

    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::slotAbout);
    connect(ui->actionHelp_Documents, &QAction::triggered, this, &MainWindow::slotHelp);


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
    OpenFileDialog dlg(this);
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
    QString strfile = item->text(0);
    if(!strfile.contains(".ab1"))
    {
        return;
    }

    m_str_SelectFile = strfile;
    QString str_info = item->text(1);

    QStringList str_list = m_str_SelectFile.split('_');
    m_pMatchListWidget->SetTableData(str_list[0],"");

    m_pExonNavigatorWidget->SetExonData(str_list[0],str_list[1]);

    m_pBaseAlignTableWidget->SetAlignTableData(str_list[0]);

    g_time_main.start();
    m_pMultiPeakWidget->SetPeakData(str_list[0],str_info.left(1));
    qDebug()<<"total time"<<g_time_main.elapsed();

    int startpos;
    int selectpos;
    int exonstartpos;
    int index = str_info.left(1).toInt();
    if(item->text(1).contains("Filter"))//如果是gssp文件
    {
        index = item->data(0,Qt::UserRole).toInt();
    }
    m_pExonNavigatorWidget->setSelectFramePosition(index, startpos, selectpos, exonstartpos);


    int i_columnPos = selectpos - startpos;
    int sliderPos = i_columnPos*25-20;
    m_pBaseAlignTableWidget->selectColumn(i_columnPos);
    m_pBaseAlignTableWidget->horizontalScrollBar()->setSliderPosition(sliderPos);

    int i_sub = selectpos - exonstartpos;
    m_pMultiPeakWidget->SetSelectPos(i_sub);
}

//导航条起始pos,选中的峰图pos，选中的导航条起始pos,选中的导航条index
void MainWindow::slotExonFocusPosition(int startpos, int selectpos, int exonstartpos, int index)
{
    int i_columnPos = selectpos - startpos; //二者之差为表格的第几列
    int sliderPos = i_columnPos*25+8;
    m_pBaseAlignTableWidget->horizontalScrollBar()->setSliderPosition(sliderPos);
    m_pBaseAlignTableWidget->selectColumn(i_columnPos+1);

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

    m_pExonNavigatorWidget->SetSelectPos(i_colnum, selectpos, exonstartpos ,index);

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
    m_pBaseAlignTableWidget->horizontalScrollBar()->setSliderPosition(test*25+8);
}


void MainWindow::slotShowSaveDlg()
{
    SaveFileDlg dlg(this);
    dlg.exec();
}


void MainWindow::slotShowLoadFileDlg()
{
    LoadFileDlg load(this);
    load.exec();
}

void MainWindow::slotShowDeleteDlg()
{
    DeleteFileDlg dlg(this);
    dlg.exec();
}

void MainWindow::slotShowExportDlg()
{
    ReportDlg report(this);
    report.setVersion(VERSION); //新增
    report.exec();
}

void MainWindow::slotReset()
{   
    QStringList str_list = m_str_SelectFile.split('_');

    int isApproved = SoapTypingDB::GetInstance()->getMarkTypeBySampleName(str_list.at(0));
    if(isApproved == APPROVED || isApproved == -1)
    {
        QMessageBox::warning(this, tr("Soap Typing"),tr("Please unlock the sample as it was marked approved"));
        return;
    }
    QString info;
    info.append(QString("Exon %1\n").arg(str_list.at(2)));
    bool bIsGssp = true;
    if(str_list.at(2).contains('R') || str_list.at(2).contains('F'))
    {
        bIsGssp = false;
    }
    else
    {
        info.append("Is Gssp file\n");
    }

    info.append(QString("File: %1\n").arg(m_str_SelectFile));
    info.append(QString("Would you like to reset this file?"));
    QMessageBox informationBox(this);
    informationBox.setWindowTitle(tr("SoapTyping"));
    informationBox.setIcon(QMessageBox::Information);
    informationBox.setText(info);
    informationBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    switch(informationBox.exec())
    {
    case QMessageBox::Yes:
    {
        SoapTypingDB::GetInstance()->resetFileByFileName(str_list.at(0), bIsGssp);

        AnalysisSampleThreadTask *pTask = new AnalysisSampleThreadTask(str_list.at(0));
        pTask->run();
        delete pTask;
        //emit signalFileChanged(signalInfo_, 1);
        break;
    }
    case QMessageBox::No:
        break;
    default:
        break;
    }
}

void MainWindow::slotMisPosForward()
{
    m_pExonNavigatorWidget->ActForward();
}

void MainWindow::slotMisPosBackward()
{
    m_pExonNavigatorWidget->ActBackward();
}

void MainWindow::slotMarkAllSampleApproved()
{
    QMessageBox informationBox(this);
    informationBox.setWindowTitle(tr("Soap Typing"));
    informationBox.setIcon(QMessageBox::Information);
    informationBox.setText(tr("Would you really like to Mark all samples as approved"));
    informationBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    switch(informationBox.exec())
    {
    case QMessageBox::Yes:
        SoapTypingDB::GetInstance()->markAllSampleApproved();
        //emit signalFileChanged(signalInfo_, 1);
        break;
    case QMessageBox::No:
        break;
    default:
        break;
    }
}

void MainWindow::slotMarkAllSampleReviewed()
{
    QMessageBox informationBox(this);
    informationBox.setWindowTitle(tr("Soap Typing"));
    informationBox.setIcon(QMessageBox::Information);
    informationBox.setText(tr("Would you really like to Marked all samples as reviewed?"));
    informationBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    switch(informationBox.exec())
    {
    case QMessageBox::Yes:
    {
        int t = SoapTypingDB::GetInstance()->markAllSampleReviewed();
        if(t==1)
        {
            QMessageBox::warning(this, tr("Soap Typing"),tr("Please unlock the sample witch were marked approved"));
        }
        //emit signalFileChanged(signalInfo_, 1);
        break;
    }
    case QMessageBox::No:
        return;
    default:
        return;
    }

}

void MainWindow::slotAlignPair()
{
    QStringList str_list = m_str_SelectFile.split('_');
    if(str_list.isEmpty())
    {
        return;
    }
    QStringList typeResultList = m_pMatchListWidget->GetMatchList();
    QStringList typeResult;

    AllelePairDlg align(this);
    align.SetData(str_list.at(1));
    align.exec();
    QString allele1, allele2;
    align.getSelectAllele(allele1,allele2);
    if(!allele1.isEmpty()&&!allele2.isEmpty())
    {
        //emit signalAllelePair(allele1, allele2);
    }
}

void MainWindow::slotAlignLab()
{
    AlignmentDlg dlg(this);
    dlg.exec();
}

void MainWindow::slotUpdateDatabase()
{

}

void MainWindow::slotControl()
{
    SetDlg dlg(this);
    dlg.exec();
}

void MainWindow::slotSetExonTrim()
{
    ExonTimDlg exonTrimDlg(this);
    exonTrimDlg.exec();
}

void MainWindow::slotyRangeRoomUp()
{
    m_pMultiPeakWidget->AdjustPeakHeight(20);
}

void MainWindow::slotyRangeRoomDown()
{
    m_pMultiPeakWidget->AdjustPeakHeight(-20);
}

void MainWindow::slotyRoomUp()
{
    m_pMultiPeakWidget->AdjustPeakY(1);
}

void MainWindow::slotyRoomDown()
{
    m_pMultiPeakWidget->AdjustPeakY(-1);
}

void MainWindow::slotxRoomUp()
{
    m_pMultiPeakWidget->AdjustPeakX(14);
}

void MainWindow::slotxRoomDown()
{
    m_pMultiPeakWidget->AdjustPeakX(-14);
}

void MainWindow::resetRoomSetting()
{
    m_pMultiPeakWidget->RestorePeak();
}

void MainWindow::slotApplyOne()
{
    ui->actionApply_All->setIconVisibleInMenu(false);
    ui->actionApply_One->setIconVisibleInMenu(true);
}

void MainWindow::slotApplyAll()
{
    ui->actionApply_All->setIconVisibleInMenu(true);
    ui->actionApply_One->setIconVisibleInMenu(false);
}

void MainWindow::slotAnalyseLater()
{
    ui->actionEdit_Multi->setIconVisibleInMenu(true);
    ui->actionEdit_One->setIconVisibleInMenu(false);
}

void MainWindow::slotAnalyseNow()
{
    ui->actionEdit_Multi->setIconVisibleInMenu(false);
    ui->actionEdit_One->setIconVisibleInMenu(true);
}

void MainWindow::slotanalyse()
{
    ui->actionAnalyze->setEnabled(false);
}

void MainWindow::slotAbout()
{
    QMessageBox message(QMessageBox::NoIcon, "About SoapTyping Software",
                        QString("SoapTyping V%1\nCopyright (C) 2012-2013 BGI").arg(VERSION),
                        QMessageBox::Ok,this);
    message.setIconPixmap(QPixmap(":/png/images/about.png"));

    message.exec();
}

void MainWindow::slotHelp()
{
    QFileInfo info("Documents/Help.pdf");
    if(info.exists())
    {
        bool ok= QDesktopServices::openUrl(QUrl(info.absoluteFilePath(), QUrl::TolerantMode));
        if(!ok)
        {
            QMessageBox::warning(this, "SoapTyping", QString("Can't open %1").arg(info.absoluteFilePath()));
        }
    }
    else
    {
        QMessageBox::warning(this, "SoapTyping", "Documents are missing!");
    }
}
