#include "loadfiledlg.h"
#include "ui_loadfiledlg.h"
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include "Core/fileTablebase.h"

const QString RESULTPATH = "Result";

LoadFileDlg::LoadFileDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoadFileDlg)
{
    ui->setupUi(this);
    InitUI();
    ConnectSignalandSolt();

    m_idaysNum = 0;
    getLoadInfo();
    getOkIndex();
    SetTableData();
}

LoadFileDlg::~LoadFileDlg()
{
    delete ui;
}

void LoadFileDlg::InitUI()
{
    QStringList li;
    li<<"Today"<<"Last 1 Days"<<"Last 3 Days"<<"Last 7 Days"<<"Last 30 Days"<<"All";
    ui->comboBox->addItems(li);

    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setColumnWidth(0,200);
    ui->tableWidget->setColumnWidth(1,150);
    ui->tableWidget->setColumnWidth(2,150);
    ui->tableWidget->setColumnWidth(3,200);
    ui->tableWidget->setColumnWidth(5,200);
    QStringList header;
    header <<"Sample Name"<<"Analysis Type"<<"Mark Type"<<"Created Time"<<"Last Modifed Time";
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

void LoadFileDlg::ConnectSignalandSolt()
{
    connect(ui->btnLoad, &QPushButton::clicked, this, &LoadFileDlg::slotClickLoadButton);
    connect(ui->btnExit, &QPushButton::clicked, this, &LoadFileDlg::close);
    connect(ui->checkBox, &QCheckBox::clicked, this, &LoadFileDlg::slotClickCheckAllBox);
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LoadFileDlg::slotdateComboBoxChagned);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &LoadFileDlg::slotLineEditChanged);
}

void LoadFileDlg::slotClickLoadButton()
{
    if(m_vec_index.size()==0)
    {
        close();
        return;
    }

    ui->checkBox->setEnabled(false);
    ui->btnExit->setEnabled(false);
    ui->comboBox->setEditable(false);
    ui->lineEdit->setEnabled(false);


    QVector<QString> samplePaths;
    QVector<QString> filePaths;
    QVector<QString> gsspFilePaths;

    int size = m_vec_index.size();
    for(int i=0; i<size; i++)
    {
        if(ui->tableWidget->item(i, 0)->checkState()==Qt::Checked)
        {
            readListFile(m_vec_LoadInfo.at(m_vec_index.at(i)).listFile, samplePaths, filePaths, gsspFilePaths);
        }
    }

    int fileSize = samplePaths.size()+filePaths.size()+gsspFilePaths.size();
    int value=0;

    ui->btnStatus->setText("Waiting..");
    ui->progressBar->setRange(0, fileSize);
    for(int i=0; i<samplePaths.size();i++)
    {
        loadSample(samplePaths.at(i));
        ui->progressBar->setValue(++value);
    }

    for(int i=0; i<filePaths.size(); i++)
    {
        loadFile(filePaths.at(i));
        ui->progressBar->setValue(++value);
    }

    for(int i=0; i<gsspFilePaths.size(); i++)
    {
        loadGssp(gsspFilePaths.at(i));
        ui->progressBar->setValue(++value);
    }
    close();
}

void LoadFileDlg::slotClickCheckAllBox(bool status)
{
    for(int i=0; i<ui->tableWidget->rowCount(); i++)
    {
        Qt::CheckState state = status ? Qt::Checked: Qt::Unchecked;
        ui->tableWidget->item(i, 0)->setCheckState(state);
    }
}

void LoadFileDlg::slotdateComboBoxChagned(int index)
{
    switch(index)
    {
    case 0:
        m_idaysNum=0;
        break;
    case 1:
        m_idaysNum=1;
        break;
    case 2:
        m_idaysNum=3;
        break;
    case 3:
        m_idaysNum=7;
        break;
    case 4:
        m_idaysNum=30;
        break;
    default:
        m_idaysNum=300000;
        break;
    }

    getOkIndex();
    SetTableData();
}

void LoadFileDlg::slotLineEditChanged(const QString & name)
{
    m_str_SearchName = name;
    getOkIndex();
    SetTableData();
}

void LoadFileDlg::getLoadInfo()
{
    m_vec_LoadInfo.clear();
    QDir dir;
    dir.setPath(RESULTPATH);
    if(!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::Dirs);
    dir.setSorting(QDir::Name|QDir::Time);
    QFileInfoList list=dir.entryInfoList();
    for(int i=0; i<list.size();i++)
    {

        LoadInfo loadInfo;

        loadInfo.sampleName = list.at(i).baseName();
        loadInfo.createdTime = list.at(i).birthTime().toString();
        QString fileName = QString("%1%2%3%4list.txt").arg(RESULTPATH).arg(QDir::separator()).
                arg(loadInfo.sampleName).arg(QDir::separator());
        QFileInfo fileInfo(fileName);
        if(!fileInfo.exists())
            continue;
        loadInfo.dateToNow = fileInfo.lastModified().daysTo(QDateTime::currentDateTime());
        loadInfo.modifiedTime=fileInfo.lastModified().toString();
        loadInfo.listFile = fileInfo.filePath();
        QFile file(fileInfo.filePath());
        file.open(QFile::ReadOnly);
        QTextStream stream(&file);
        loadInfo.analysisType=stream.readLine().split(":").at(1).toInt();
        loadInfo.markType=stream.readLine().split(":").at(1).toInt();
        file.close();
        m_vec_LoadInfo.push_back(loadInfo);
    }
}

QString getAnalysisType_load(int type)
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

QString getMarkType_load(int type)
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

QIcon getIcon_load(int analysisType, int markType)
{
    return QIcon(QString(":/images/filetree%1%2.png").arg(markType).arg(analysisType));
}

void LoadFileDlg::getOkIndex()
{
    m_vec_index.clear();
    for(int i=0; i<m_vec_LoadInfo.size();i++)
    {
        if(m_vec_LoadInfo.at(i).dateToNow > m_idaysNum)
            continue;
        if(!m_str_SearchName.isEmpty() && !m_vec_LoadInfo.at(i).sampleName.contains(m_str_SearchName))
            continue;
        m_vec_index.push_back(i);
    }
}

void LoadFileDlg::SetTableData()
{
    if(m_vec_index.size()<=0)
        return;

    int size = m_vec_index.size();
    ui->tableWidget->setRowCount(size);

    for(int i=0; i<size;i++)
    {
        QTableWidgetItem *itemN = new QTableWidgetItem;
        const LoadInfo &loadInfo = m_vec_LoadInfo.at(m_vec_index.at(i));
        itemN->setCheckState(ui->checkBox->checkState());
        itemN->setIcon(getIcon_load(loadInfo.analysisType, loadInfo.markType));
        itemN->setText(loadInfo.sampleName);
        ui->tableWidget->setItem(i, 0, itemN);
        itemN = new QTableWidgetItem;
        itemN->setText(getAnalysisType_load(loadInfo.analysisType));
        ui->tableWidget->setItem(i, 1, itemN);

        itemN = new QTableWidgetItem;
        itemN->setText(getMarkType_load(loadInfo.markType));
        ui->tableWidget->setItem(i, 2, itemN);

        itemN = new QTableWidgetItem;
        itemN->setText(loadInfo.createdTime);
        ui->tableWidget->setItem(i, 3, itemN);

        itemN = new QTableWidgetItem;
        itemN->setText(loadInfo.modifiedTime);
        ui->tableWidget->setItem(i, 4, itemN);
    }
}

void LoadFileDlg::readListFile(const QString &listFilePath, QVector<QString> &samplePaths,
                  QVector<QString> &filePaths, QVector<QString> &gsspFilePaths)
{
    QFile file(listFilePath);
    file.open(QFile::ReadOnly);
    QTextStream stream(&file);
    while(!stream.atEnd())
    {
        QStringList line = stream.readLine().split(":");
        if(line.at(0)=="sampleTable")
        {
            samplePaths.push_back(line.at(1));
        }
        else if(line.at(0)=="fileTable")
        {
            filePaths.push_back(line.at(1));
        }
        else if(line.at(0)=="gsspFileTable")
        {
            gsspFilePaths.push_back(line.at(1));
        }
    }
    file.close();
}

void LoadFileDlg::loadSample(const QString &samplePath)
{
    Ab1FileTableBase sampletable;

    QFile file(samplePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream stream(&file);

//    sampletable.setSampleName(stream.readLine());
//    sampletable.setGeneName(stream.readLine());
//    sampletable.setFileType(stream.readLine().toInt());
//    sampletable.setMarkType(stream.readLine().toInt());
//    sampletable.setAnalysisType(stream.readLine().toInt());
//    sampletable.setMinExonIndex(stream.readLine().toInt());
//    sampletbale.setMaxExonIndex(stream.readLine().toInt());
//    sampletable.setExonStartPos(stream.readLine().toInt());
//    sampletable.setExonEndPos(stream.readLine().toInt());
//    sampletable.setConsensusSequence(stream.readLine());
//    sampletable.set


//    sampletable.consensusSequence = stream.readLine().toAscii();
//    sampletable.forwardSequence = stream.readLine().toAscii();
//    sampletable.reverseSequence = stream.readLine().toAscii();
//    sampletable.patternSequence = stream.readLine().toAscii();
//    sampletable.mismatchBetweenPC = stream.readLine();
//    sampletable.mismatchBetweenFR = stream.readLine();
//    sampletable.mmismatchBetweenFR = stream.readLine();
//    sampletable.editPostion = stream.readLine();
//    sampletable.typeResult = stream.readLine();
//    sampletable.gsspInfo = stream.readLine();
//    sampletable.shieldAllele = stream.readLine();
//    sampletable.setResult = stream.readLine();
//    sampletable.setNote = stream.readLine();
//    sampletable.setGSSP = stream.readLine();
//    sampletable.combinedResult = stream.readLine();
//    file.close();
//    insertSampleToRealTimeDatabase(sampletable);
    return;
}

void LoadFileDlg::loadFile(const QString &filePath)
{
    Ab1FileTableBase filetable;

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream stream(&file);

//    filetable.fileName = stream.readLine().toAscii();
//    filetable.sampleName = stream.readLine().toAscii();
//    filetable.filePath = stream.readLine();
//    filetable.isExtraFile = stream.readLine().toInt();
//    filetable.geneName = stream.readLine().toAscii();
//    filetable.exonIndex = stream.readLine().toInt();
//    filetable.rOrF = stream.readLine().toAscii();
//    filetable.exonStartPos = stream.readLine().toInt();
//    filetable.exonEndPos = stream.readLine().toInt();
//    filetable.usefulSequence = stream.readLine().toAscii();
//    filetable.baseSequence = stream.readLine().toAscii();
//    filetable.basePostion = stream.readLine();
//    filetable.baseQuality = stream.readLine();
//    filetable.baseNumber = stream.readLine().toInt();
//    filetable.baseASignal = stream.readLine();
//    filetable.baseTSignal = stream.readLine();
//    filetable.baseGSignal = stream.readLine();
//    filetable.baseCSignal = stream.readLine();
//    filetable.signalNumber = stream.readLine().toInt();
//    filetable.maxSignal = stream.readLine().toInt();
//    filetable.maxQuality = stream.readLine().toInt();
//    filetable.averageBaseWidth = stream.readLine().toFloat();
//    filetable.isGood = stream.readLine().toInt();
//    filetable.alignResult = stream.readLine().toInt();
//    filetable.alignStartPos = stream.readLine().toInt();
//    filetable.alignEndPos = stream.readLine().toInt();
//    filetable.alignInfo = stream.readLine();
//    filetable.excludeLeft = stream.readLine().toInt();
//    filetable.excludeRight = stream.readLine().toInt();
//    filetable.editInfo = stream.readLine();

//    insertFileToRealTimeDatabase(filetable);

    file.close();

    return;
}

void LoadFileDlg::loadGssp(const QString &gsspFilePath)
{
    Ab1FileTableBase gsspfiletable;

    QFile file(gsspFilePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream stream(&file);

//    gsspfiletable.fileName = stream.readLine().toAscii();
//    gsspfiletable.sampleName = stream.readLine().toAscii();
//    gsspfiletable.filePath = stream.readLine();
//    gsspfiletable.gsspName = stream.readLine().toAscii();
//    gsspfiletable.geneName = stream.readLine().toAscii();
//    gsspfiletable.exonIndex = stream.readLine().toInt();
//    gsspfiletable.rOrF = stream.readLine().toAscii();
//    gsspfiletable.exonStartPos = stream.readLine().toInt();
//    gsspfiletable.exonEndPos = stream.readLine().toInt();
//    gsspfiletable.usefulSequence = stream.readLine().toAscii();
//    gsspfiletable.baseSequence = stream.readLine().toAscii();
//    gsspfiletable.basePostion = stream.readLine();
//    gsspfiletable.baseQuality = stream.readLine();
//    gsspfiletable.baseNumber = stream.readLine().toInt();
//    gsspfiletable.baseASignal = stream.readLine();
//    gsspfiletable.baseTSignal = stream.readLine();
//    gsspfiletable.baseGSignal = stream.readLine();
//    gsspfiletable.baseCSignal = stream.readLine();
//    gsspfiletable.signalNumber = stream.readLine().toInt();
//    gsspfiletable.maxSignal = stream.readLine().toInt();
//    gsspfiletable.maxQuality = stream.readLine().toInt();
//    gsspfiletable.averageBaseWidth = stream.readLine().toFloat();
//    gsspfiletable.isGood = stream.readLine().toInt();
//    gsspfiletable.alignResult = stream.readLine().toInt();
//    gsspfiletable.alignStartPos = stream.readLine().toInt();
//    gsspfiletable.alignEndPos = stream.readLine().toInt();
//    gsspfiletable.alignInfo = stream.readLine().toAscii();
//    gsspfiletable.excludeLeft = stream.readLine().toInt();
//    gsspfiletable.excludeRight = stream.readLine().toInt();
//    gsspfiletable.editInfo = stream.readLine();
//    gsspfiletable.typeResult = stream.readLine();
//    gsspfiletable.filterResult = stream.readLine();

//    insertGsspFileToRealTimeDatabase(gsspfiletable);

    file.close();

    return;
}
