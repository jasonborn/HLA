#include "savefiledlg.h"
#include "ui_savefiledlg.h"
#include "DataBase/soaptypingdb.h"
#include <QMessageBox>
#include <QDir>
#include <QDate>
#include <QTextStream>

const QString RESULTPATH = "Result";

SaveFileDlg::SaveFileDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveFileDlg)
{
    ui->setupUi(this);
    InitUI();
}

SaveFileDlg::~SaveFileDlg()
{
    delete ui;
}

void SaveFileDlg::InitUI()
{
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setColumnWidth(0, 200);
    ui->tableWidget->setColumnWidth(1, 150);
    ui->tableWidget->setColumnWidth(2, 150);
    ui->tableWidget->setColumnWidth(3, 70);

    QStringList header;
    header<<"Sample Name"<<"Analysis Type"<<"Mark Type"<<"User";
    ui->tableWidget->setHorizontalHeaderLabels(header);
}

void SaveFileDlg::ConnectSignalandSolt()
{
    connect(ui->checkall, &QCheckBox::clicked, this, &SaveFileDlg::slotClickCheckAllBox);
    connect(ui->savebydate, &QCheckBox::clicked,this, &SaveFileDlg::slotClickCheckSaveByDateBox);
    connect(ui->btnsave, &QPushButton::clicked, this, &SaveFileDlg::slotClickSaveButton);
    connect(ui->btnexit, &QPushButton::clicked, this, &SaveFileDlg::close);
}

void SaveFileDlg::slotClickCheckAllBox(bool ischeck)
{
    for(int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        Qt::CheckState state = ischeck ? Qt::Checked : Qt::Unchecked;
        ui->tableWidget->item(i, 0)->setCheckState(state);
    }
}

void SaveFileDlg::slotClickCheckSaveByDateBox(bool ischeck)
{
    m_bSaveByDate = ischeck;
}

void SaveFileDlg::slotClickSaveButton()
{
    int i_row = ui->tableWidget->rowCount();
    if(i_row > 0)
    {
        QVector<SampleTreeInfo_t> vec_checked;
        for(int i=0; i<i_row;i++)
        {
            if(ui->tableWidget->item(i, 0)->checkState() == Qt::Checked)
            {
                QString str_name = ui->tableWidget->item(i, 0)->text();
                vec_checked.push_back(m_map_SampleTreeInfo[str_name]);
            }
        }

        if(vec_checked.empty())
        {
            QMessageBox::warning(this, tr("Soap Typing"), "Please choose sample to save!");
            return;
        }

        ui->checkall->setEnabled(false);
        ui->savebydate->setEnabled(false);
        ui->btnexit->setEnabled(false);
        ui->btnsave->setEnabled(false);
        saveFileT(vec_checked);
        close();
    }
}

void SaveFileDlg::saveFileT(QVector<SampleTreeInfo_t> &sampleInfos)
{
    QDir dir;
    dir.mkpath(RESULTPATH);
    ui->status->setText("Waiting:..");
    ui->progressBar->setRange(0, sampleInfos.size());
    if(m_bSaveByDate)
    {
        QString dates = QDate::currentDate().toString("yyyymmdd");
        for(int i=0; i<sampleInfos.size(); i++)
        {
            saveSampleTByDate(sampleInfos.at(i), dates);
            ui->progressBar->setValue(i+1);
        }
    }
    else
    {
        for(int i=0; i<sampleInfos.size(); i++)
        {
            //saveSampleT(sampleInfos.at(i));
            ui->progressBar->setValue(i+1);
        }
    }
    ui->status->setText("Ready:");
    return;
}


QString getAnalysisType(int type)
{
    switch(type)
    {
    case 0:
        return "Match(common)";
    case 1:
        return "Match(rare)";
    case 2:
        return "Match(bad quality)";
    case 3:
        return "Mismatch";
    default:
        return "Undefined";
    }
    return "Undefined";
}

QString getMarkType(int type)
{
    switch(type)
    {
    case 0:
        return "OWNED";
    case 1:
        return "PENDING";
    case 2:
        return "REVIEWED";
    case 3:
        return "APPROVED";
    default:
        return "OWNED";
    }
    return "OWNED";
}

QIcon getIcon_dlg(int analysisType, int markType)
{
    return QIcon(QString(":/images/filetree%1%2.png").arg(markType).arg(analysisType));
}

void SaveFileDlg::setTableDefaultData()
{
    SoapTypingDB::GetInstance()->getSampleTreeDataFromSampleTable(m_map_SampleTreeInfo);

    int i = 0;
    foreach(const SampleTreeInfo_t& info, m_map_SampleTreeInfo.values())
    {
        QTableWidgetItem *item = new QTableWidgetItem;

        ui->tableWidget->setRowHeight(i, 20);
        item->setCheckState(Qt::Checked);
        item->setIcon(getIcon_dlg(info.analysisType, info.markType));
        item->setText(info.sampleName);
        ui->tableWidget->setItem(i, 0, item);

        item = new QTableWidgetItem;
        item->setText(getAnalysisType(info.analysisType));
        ui->tableWidget->setItem(i, 1, item);

        item = new QTableWidgetItem;
        item->setText(getMarkType(info.markType));
        ui->tableWidget->setItem(i, 2, item);

        item = new QTableWidgetItem;
        item->setText("default");
        ui->tableWidget->setItem(i, 3, item);
        i++;
    }
}

void SaveFileDlg::saveSampleTByDate(const SampleTreeInfo_t &sampleInfo, QString &date)
{
    QString dirPath = QString("%1%2%3_%4").arg(RESULTPATH).arg(QDir::separator()).arg(sampleInfo.sampleName).arg(date);
//    if(sampleInfo.sampleName.contains('_'))
//    {
//        QStringList sp = sampleInfo.sampleName.split("_",QString::SkipEmptyParts);
//        QString dirPath = QString("%1%2%3_%4").arg(RESULTPATH).arg(QDir::separator()).arg(sp.at(0)).arg(date);
//    }
    QDir dir0(dirPath);
    if(!dir0.exists())
    {
        QDir dir;
        dir.mkpath(dirPath);
    }
    QString listFileName = QString("%1%2list.txt").arg(dirPath).arg(QDir::separator());
    QFile file(listFileName);
    if(file.exists())
    {
        file.remove();
    }
    file.open(QFile::WriteOnly);
    QTextStream stream(&file);
    QString outFile = QString("%1%2%3.txt").arg(dirPath).arg(QDir::separator()).arg(sampleInfo.sampleName);
    stream<<"AType:"<<sampleInfo.analysisType<<"\n";
    stream<<"MType:"<<sampleInfo.markType<<"\n";
    stream<<"sampleTable:"<<outFile<<"\n";
    SoapTypingDB::GetInstance()->saveSample(sampleInfo.sampleName, outFile, date);

    for(int i=0; i<sampleInfo.treeinfo.size(); i++)
    {
        const FileTreeInfo_t &fileInfo = sampleInfo.treeinfo.at(i);
        outFile = QString("%1%2%3.txt").arg(dirPath).arg(QDir::separator()).arg(fileInfo.fileName);
        if (fileInfo.isGssp)
        {
            stream<<"gsspFileTable:"<<outFile<<"\n";
            SoapTypingDB::GetInstance()->saveFile(fileInfo.isGssp, fileInfo.fileName, outFile, dirPath, date);
        }
        else
        {
            stream<<"fileTable:"<<outFile<<"\n";
            SoapTypingDB::GetInstance()->saveFile(fileInfo.isGssp, fileInfo.fileName, outFile, dirPath, date);
        }

    }
    file.close();
}

