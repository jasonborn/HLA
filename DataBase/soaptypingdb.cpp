#include "soaptypingdb.h"
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QSqlError>
#include <QDebug>
#include <QMutex>
#include <QTime>
#include <QThread>
#include "all_base_struct.h"

QMutex g_mutex;
QSqlDatabase SoapTypingDB::m_SqlDB;

SoapTypingDB::SoapTypingDB()
{
    InitDB();
}

SoapTypingDB::~SoapTypingDB()
{
    m_SqlDB.close();
}

bool SoapTypingDB::InitDB()
{
    //QMutexLocker locker(&g_mutex);
    QString str = QTime::currentTime().toString("hh_mm_ss_zzz");
    QString str_connect = QString("connect-%1").arg(str);
    m_SqlDB = QSqlDatabase::addDatabase("QSQLITE",str_connect);
    m_SqlDB.setDatabaseName("D:/workspace/build-HLA-Desktop_Qt_5_11_1_MinGW_32bit-Debug/SoapTypingDB.db");

    bool bRet = m_SqlDB.open();
    if(!bRet)
    {
        QSqlError sql_err  = m_SqlDB.lastError();
        qDebug()<<sql_err.text();
    }
    return bRet;
}

void SoapTypingDB::GetGsspNames(QStringList &gsspNames)
{
    QSqlQuery query(m_SqlDB);
    QString str_sql("SELECT gsspName FROM gsspTable");
    bool bRet = query.prepare(str_sql);
    if(bRet)
    {
        bRet = query.exec(str_sql);
        if(bRet)
        {
            while(query.next())
            {
                gsspNames.push_back(query.value(0).toString());
            }
        }
        else
        {
            QSqlError err = query.lastError();
            QString str_err = err.text();
            qDebug()<<str_err;
        }
    }
}

//根据gssp名称查询F/R和外显子序号，所以geneName一列没啥影响
bool SoapTypingDB::FindExonAndRFByGsspName(const QString &name, QString &exonIndex, QString &RF)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT exonIndex,fOrR FROM commonGsspTable WHERE gsspName=?");
    query.bindValue(0, name);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        if(query.next())
        {
            exonIndex = query.value(0).toString();
            RF = query.value(1).toString();
            return true;
        }
    }
    else
    {
        return false;
    }
}

void SoapTypingDB::GetGsspMapToExonAndFR(QMap<QString, ExonAndRF> &mapToExonAndFR)
{
    QSqlQuery query(m_SqlDB);
    QString str_sql("SELECT gsspName,exonIndex,rOrF FROM gsspTable");
    query.prepare(str_sql);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            ExonAndRF exonAndRF;
            exonAndRF.exonIndex = query.value(1).toString();
            exonAndRF.rOrF = query.value(2).toString();
            mapToExonAndFR.insert(query.value(0).toString(), exonAndRF);
        }
    }
}

void SoapTypingDB::GetExonInfo(const QString &gene_name, int exon_index, ExonInfoS& exonInfo)
{
    QMutexLocker locker(&g_mutex);
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT geneSequence,exonPositionIndex FROM oldGeneTable WHERE geneName = ?");
    query.bindValue(0, gene_name);
    bool isSuccess = query.exec();
    if (isSuccess)
    {
        while(query.next())
        {
            exonInfo.exonStartPos = query.value(1).toString().split(":")[exon_index-1].toInt();
            exonInfo.exonEndPos = query.value(1).toString().split(":")[exon_index].toInt();
            exonInfo.consensusSeq = query.value(0).toByteArray().mid(exonInfo.exonStartPos,
                                                                     exonInfo.exonEndPos-exonInfo.exonStartPos);
        }
    }
}

int SoapTypingDB::GetExcludePosition(const QString &key, int &lefpos, int &rightpos)
{
    QMutexLocker locker(&g_mutex);
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT exonStart,exonEnd,excludeLeft,excludeRight FROM exonTrimTable WHERE etKey=?");
    query.bindValue(0, key);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        if(query.next())
        {
            int start = query.value(0).toInt();
            int end = query.value(1).toInt();
            int l_pos = query.value(2).toInt();
            int r_pos = query.value(3).toInt();
            if(l_pos <= 0 || l_pos > end-start)
            {
                lefpos = -1;
            }
            else
            {
                lefpos = start + l_pos - 1;
            }

            if(r_pos <= 0 || r_pos > end-start)
            {
                rightpos = -1;
            }
            else
            {
                rightpos = end - r_pos;
            }
        }
    }
    return -1;
}


void SoapTypingDB::InsertOneFileTable(Ab1FileTableBase &fileTable)
{
    QMutexLocker locker(&g_mutex);
    QSqlQuery query(m_SqlDB);
    query.prepare("insert or replace INTO fileTable (fileName,"
                  "sampleName,"
                  "filePath,"
                  "isExtraFile,"
                  "geneName,"
                  "exonIndex,"
                  "rOrF,"
                  "exonStartPos,"
                  "exonEndPos,"
                  "usefulSequence,"
                  "baseSequence,"
                  "basePostion,"
                  "baseQuality,"
                  "baseNumber,"
                  "baseASignal,"
                  "baseTSignal,"
                  "baseGSignal,"
                  "baseCSignal,"
                  "signalNumber,"
                  "maxSignal,"
                  "maxQuality,"
                  "averageBaseWidth,"
                  "isGood,"
                  "alignResult,"
                  "alignStartPos,"
                  "alignEndPos,"
                  "alignInfo,"
                  "excludeLeft,"
                  "excludeRight,"
                  "editInfo,"
                  "avgSignal)"
                  "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.bindValue(0, fileTable.getFileName());
    query.bindValue(1, fileTable.getSampleName());
    query.bindValue(2, fileTable.getFilePath());
    query.bindValue(3, fileTable.getExtraFile());
    query.bindValue(4, fileTable.getGeneName());
    query.bindValue(5, fileTable.getExonIndex());
    query.bindValue(6, fileTable.getROrF());
    query.bindValue(7, fileTable.getExonStartPos());
    query.bindValue(8, fileTable.getExonEndPos());
    query.bindValue(9, fileTable.getUsefulSequence());
    query.bindValue(10, fileTable.getBaseSequence());
    query.bindValue(11, fileTable.getBasePostion());
    query.bindValue(12, fileTable.getBaseQuality());
    query.bindValue(13, fileTable.getBaseNumber());
    query.bindValue(14, fileTable.getBaseASignal());
    query.bindValue(15, fileTable.getBaseTSignal());
    query.bindValue(16, fileTable.getBaseGSignal());
    query.bindValue(17, fileTable.getBaseCSignal());
    query.bindValue(18, fileTable.getSignalNumber());
    query.bindValue(19, fileTable.getMaxSignal());
    query.bindValue(20, fileTable.getMaxQuality());
    query.bindValue(21, fileTable.getAverageBaseWidth());
    query.bindValue(22, fileTable.getIsGood());
    query.bindValue(23, fileTable.getAlignResult());
    query.bindValue(24, fileTable.getAlignStartPos());
    query.bindValue(25, fileTable.getAlignEndPos());
    query.bindValue(26, fileTable.getAlignInfo());
    query.bindValue(27, fileTable.getExcludeLeft());
    query.bindValue(28, fileTable.getExcludeRight());
    query.bindValue(29, fileTable.getEditInfo());
    query.bindValue(30, fileTable.getAvgsignal());
    bool isSuccess = query.exec();
    if(!isSuccess)
    {
         qDebug()<<"insert fileTable error";
    }
}

void SoapTypingDB::insertOneGsspFileTable( Ab1FileTableBase &fileTable)
{
    QMutexLocker locker(&g_mutex);
    QSqlQuery query(m_SqlDB);
    query.prepare("INSERT or replace INTO gsspFileTable (fileName,"
                  "sampleName,"
                  "filePath,"
                  "gsspName,"
                  "geneName,"
                  "exonIndex,"
                  "rOrF,"
                  "exonStartPos,"
                  "exonEndPos,"
                  "usefulSequence,"
                  "baseSequence,"
                  "basePostion,"
                  "baseQuality,"
                  "baseNumber,"
                  "baseASignal,"
                  "baseTSignal,"
                  "baseGSignal,"
                  "baseCSignal,"
                  "signalNumber,"
                  "maxSignal,"
                  "maxQuality,"
                  "averageBaseWidth,"
                  "isGood,"
                  "alignResult,"
                  "alignStartPos,"
                  "alignEndPos,"
                  "alignInfo,"
                  "excludeLeft,"
                  "excludeRight,"
                  "editInfo,"
                  "typeResult,"
                  "filterResult)"
                  "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.bindValue(0, fileTable.getFileName());
    query.bindValue(1, fileTable.getSampleName());
    query.bindValue(2, fileTable.getFilePath());
    query.bindValue(3, fileTable.getGsspName());
    query.bindValue(4, fileTable.getGeneName());
    query.bindValue(5, fileTable.getExonIndex());
    query.bindValue(6, fileTable.getROrF());
    query.bindValue(7, fileTable.getExonStartPos());
    query.bindValue(8, fileTable.getExonEndPos());
    query.bindValue(9, fileTable.getUsefulSequence());
    query.bindValue(10, fileTable.getBaseSequence());
    query.bindValue(11, fileTable.getBasePostion());
    query.bindValue(12, fileTable.getBaseQuality());
    query.bindValue(13, fileTable.getBaseNumber());
    query.bindValue(14, fileTable.getBaseASignal());
    query.bindValue(15, fileTable.getBaseTSignal());
    query.bindValue(16, fileTable.getBaseGSignal());
    query.bindValue(17, fileTable.getBaseCSignal());
    query.bindValue(18, fileTable.getSignalNumber());
    query.bindValue(19, fileTable.getMaxSignal());
    query.bindValue(20, fileTable.getMaxQuality());
    query.bindValue(21, fileTable.getAverageBaseWidth());
    query.bindValue(22, fileTable.getIsGood());
    query.bindValue(23, fileTable.getAlignResult());
    query.bindValue(24, fileTable.getAlignStartPos());
    query.bindValue(25, fileTable.getAlignEndPos());
    query.bindValue(26, fileTable.getAlignInfo());
    query.bindValue(27, fileTable.getExcludeLeft());
    query.bindValue(28, fileTable.getExcludeRight());
    query.bindValue(29, fileTable.getEditInfo());
    query.bindValue(30, fileTable.getTypeResult());
    query.bindValue(31, fileTable.getFilterResult());
    bool isSuccess = query.exec();
    if(!isSuccess)
    {
         qDebug()<<"insert gsspFileTable error";
    }
}


int SoapTypingDB::getFileInfoFromRealTimeDatabase(const QString &sampleName, QVector<FileInfo> &fileInfos,
                                                  ExonInfo &exonInfo)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT isExtraFile,"
                  "geneName,"
                  "exonIndex,"
                  "rOrF,"
                  "exonStartPos,"
                  "exonEndPos,"
                  "usefulSequence,"
                  "alignResult,"
                  "excludeLeft,"
                  "excludeRight,"
                  "editInfo,"
                  "isGood "
                  "FROM fileTable WHERE sampleName=?");
    query.bindValue(0, sampleName);
    bool isSuccess = query.exec();
    int flag = 0;
    if(isSuccess)
    {

        while(query.next()){
            if(query.value(7).toInt() != 0)
            {
                flag = 1;
                continue;
            }
            FileInfo fileInfo;
            fileInfo.geneName = query.value(1).toByteArray();
            fileInfo.rOrF = query.value(3).toByteArray();
            fileInfo.exonStartPos = query.value(4).toInt();
            fileInfo.exonEndPos = query.value(5).toInt();
            fileInfo.sequence = query.value(6).toByteArray();
            fileInfo.isGood = query.value(11).toInt();
            modifySequence(fileInfo.sequence, fileInfo.editPostion, fileInfo.exonStartPos, query.value(8).toInt(),
                           query.value(9).toInt(), query.value(10).toString());
            compareWithExonInfo(exonInfo, query.value(2).toInt(), fileInfo.exonStartPos, fileInfo.exonEndPos);
            fileInfos.push_back(fileInfo);
        }
    }
    return flag;
}

int SoapTypingDB::getGsspFileInfoFromRealTimeDatabase(const QString &sampleName, QVector<FileInfo> &gsspFileInfos,
                                                      ExonInfo &exonInfo)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT gsspName,"
                  "geneName,"
                  "exonIndex,"
                  "rOrF,"
                  "exonStartPos,"
                  "exonEndPos,"
                  "usefulSequence,"
                  "alignResult,"
                  "excludeLeft,"
                  "excludeRight,"
                  "editInfo,"
                  "isGood,"
                  "fileName "
                  "FROM gsspFileTable WHERE sampleName=?");
    query.bindValue(0, sampleName);
    bool isSuccess = query.exec();
    int flag = 0;
    if(!isSuccess)
    {
         qDebug()<<"getGsspFileInfoFromRealTimeDatabase error:" << sampleName;
        return -1;
    }
    else
    {
        while(query.next())
        {
            if(query.value(7).toInt() == UNMATCH)
            {
                flag = 1;
                continue;
            }
            FileInfo fileInfo;
            fileInfo.gsspName = query.value(0).toByteArray();
            fileInfo.geneName = query.value(1).toByteArray();
            fileInfo.rOrF = query.value(3).toByteArray();
            fileInfo.exonStartPos = query.value(4).toInt();
            fileInfo.exonEndPos = query.value(5).toInt();
            fileInfo.sequence = query.value(6).toByteArray();
            fileInfo.isGood = query.value(11).toInt();
            fileInfo.fileName = query.value(12).toByteArray();
            modifySequence(fileInfo.sequence, fileInfo.editPostion, fileInfo.exonStartPos, query.value(8).toInt(),
                           query.value(9).toInt(), query.value(10).toString());
            compareWithExonInfo(exonInfo, query.value(2).toInt(), fileInfo.exonStartPos, fileInfo.exonEndPos);
            gsspFileInfos.push_back(fileInfo);
        }
    }
    return flag;
}

void SoapTypingDB::compareWithExonInfo(ExonInfo &exonInfo, int exonIndex, int exonStartPos, int exonEndPos)
{
    if(exonIndex < exonInfo.minExonIndex)
        exonInfo.minExonIndex = exonIndex;
    if(exonInfo.maxExonIndex < exonIndex)
        exonInfo.maxExonIndex = exonIndex;
    if(exonStartPos < exonInfo.exonStartPosition)
        exonInfo.exonStartPosition = exonStartPos;
    if(exonInfo.exonEndPostion < exonEndPos)
        exonInfo.exonEndPostion = exonEndPos;
}

void SoapTypingDB::modifySequence(QByteArray &sequence, QSet<int> &editPostion, int exonStartPos, int excludeLeft,
                                  int excludeRight, const QString &editString)
{
    if(editString.size()>0)
    {
        QStringList edits = editString.split(";", QString::SkipEmptyParts);
        for(int i=0; i<edits.size(); i++)
        {
            QStringList posBase = edits.at(i).split(":", QString::SkipEmptyParts);
            int pos = posBase.at(0).toInt();
            sequence[pos-exonStartPos] = posBase.at(1).at(0).toLatin1();
            editPostion.insert(pos);
        }
    }
    if(excludeLeft >= 0)
    {
        int i = excludeLeft-exonStartPos+1;
        while(sequence[i]=='.')
        {
            sequence[i]= '-';
            i++;
        }
        for(int i=excludeLeft-exonStartPos; i>=0; i--)
        {
            sequence[i]='-';
        }
    }
    if(excludeRight >= 0)
    {
        int i = excludeRight - exonStartPos -1;
        while(sequence[i]=='.')
        {
            sequence[i] = '-';
            i--;
        }
        for(int i=excludeRight-exonStartPos; i<sequence.size(); i++)
        {
            sequence[i]='-';
        }
    }
}

void SoapTypingDB::insertSampleInfoToRealTimeDatabase(const SampleInfo &sampleInfo)
{
    QMutexLocker locker(&g_mutex);
    QSqlQuery query(m_SqlDB);
    query.prepare("INSERT or replace INTO sampleTable (sampleName,"
                  "geneName,"
                  "analysisType,"
                  "minExonIndex,"
                  "maxExonIndex,"
                  "exonStartPos,"
                  "exonEndPos,"
                  "consensusSequence,"
                  "forwardSequence,"
                  "reverseSequence,"
                  "patternSequence,"
                  "mismatchBetweenPC,"
                  "mismatchBetweenFR,"
                  "mmismatchBetweenFR,"
                  "editPostion,"
                  "typeResult,"
                  "combinedResult,"
                  "markType)"
                  "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.bindValue(0, sampleInfo.sampleName);
    query.bindValue(1, sampleInfo.geneName);
    query.bindValue(2, sampleInfo.analysisType);
    query.bindValue(3, sampleInfo.minExonIndex);
    query.bindValue(4, sampleInfo.maxExonIndex);
    query.bindValue(5, sampleInfo.exonStartPos);
    query.bindValue(6, sampleInfo.exonEndPos);
    query.bindValue(7, sampleInfo.consensusSequence);
    query.bindValue(8, sampleInfo.forwardSequence);
    query.bindValue(9, sampleInfo.reverseSequence);
    query.bindValue(10, sampleInfo.patternSequence);
    query.bindValue(11, sampleInfo.mismatchBetweenPC);
    query.bindValue(12, sampleInfo.mismatchBetweenFR);
    query.bindValue(13, sampleInfo.mmismatchBetweenFR);
    query.bindValue(14, sampleInfo.editPostion);
    query.bindValue(15, sampleInfo.typeResult);
    query.bindValue(16, sampleInfo.combinedResult);
    query.bindValue(17, OWNED);
    bool isSuccess = query.exec();
    if(!isSuccess)
    {
         qDebug()<<"insertSampleInfoToRealTimeDatabaseT error:"<<sampleInfo.sampleName;
    }
}

void SoapTypingDB::getConsensusSequenceFromStaticDatabase(const QString &geneName, QByteArray &geneSequence,
                                            int exonStartPos, int exonLength)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT geneSequence FROM geneTable WHERE geneName=?");
    query.bindValue(0, geneName);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            geneSequence = query.value(0).toByteArray().mid(exonStartPos, exonLength);
        }
    }
}

void SoapTypingDB::getShieldAllelesFromDatabase(const QString &sampleName, QSet<QString> &shieldAlleles)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT shieldAllele FROM sampleTable WHERE sampleName=?");
    query.bindValue(0, sampleName);
    bool isSuccess = query.exec();
    if(!isSuccess)
    {
         qDebug()<<"getShieldAllelesFromDatabase error:"<<sampleName;
    }
    else
    {
        while(query.next())
        {
            QStringList ss = query.value(0).toString().split(";", QString::SkipEmptyParts);
            for(int i=0; i<ss.size(); i++)
            {
                shieldAlleles.insert(ss.at(i));
            }
        }
    }
}

void SoapTypingDB::getAlleleInfosFromStaticDatabase(const QString &geneName, int exonStartPos, int alleleLength,
                                      int minExonIndex, int maxExonIndex, QVector<AlleleInfo> &alleleInfos,
                                      QSet<QString> &shieldAlleles)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT alleleName,alleleSequence,isRare,isIndel,noStar FROM alleleTable WHERE geneName=?");
    query.bindValue(0, geneName);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            AlleleInfo alleleInfo;
            alleleInfo.alleleName = query.value(0).toString();
            if(shieldAlleles.contains(alleleInfo.alleleName))
                continue;
            alleleInfo.alleleSequence = query.value(1).toByteArray().mid(exonStartPos, alleleLength);
            int isRare = query.value(2).toInt();
            if(isRare)
            {
                alleleInfo.isRare = "r";
            }
            alleleInfo.isIndel = query.value(3).toInt();
            QStringList star = query.value(4).toString().split("", QString::SkipEmptyParts);
            int min = star.at(0).toInt();
            int max = star.at(1).toInt();
            if(min>max)
            {
                for(int i=minExonIndex; i<=maxExonIndex; i++)
                {
                    alleleInfo.starInfo.append(QString("*%1").arg(i));
                }
            }
            else
            {
                if(minExonIndex >= min && maxExonIndex<=max)
                {
                    alleleInfo.starInfo.append("-");
                }
                else
                {
                    for(int i=minExonIndex; i<min; i++)
                    {
                        alleleInfo.starInfo.append(QString("*%1").arg(i));
                    }
                    for(int i=maxExonIndex; i>max; i--)
                    {
                        alleleInfo.starInfo.append(QString("*%1").arg(i));
                    }
                }
            }
            alleleInfos.push_back(alleleInfo);
        }
    }
}

void SoapTypingDB::getGsspPosAndSeqFromGsspDatabase(const QString &gsspName, int &gsspPos, QString &gsspSeq)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT position,base FROM gsspTable WHERE gsspName =?");
    query.bindValue(0, gsspName.toLatin1());
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            gsspPos = query.value(0).toInt();
            gsspSeq = query.value(1).toString();
        }
    }
}

void SoapTypingDB::getGsspAlleleInfosFromStaticDatabase(const QString &geneName, int exonStartPos, int gsspLength,
                                          QVector<GsspAlleleInfo> &gsspAlleleInfos, const QString &gsspName)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT alleleName,alleleSequence FROM alleleTable WHERE geneName=?");
    query.bindValue(0, geneName);
    bool isSuccess = query.exec();

    int gssp_pos;
    QString gssp_seq;

    getGsspPosAndSeqFromGsspDatabase(gsspName, gssp_pos, gssp_seq);

    if(isSuccess)
    {
        while(query.next())
        {
            GsspAlleleInfo gsspAlleleInfo;
            gsspAlleleInfo.alleleName = query.value(0).toString();
            QByteArray alleleSequence = query.value(1).toByteArray();
            for(int i=0; i<gssp_seq.size(); i++)
            {
                if(alleleSequence[i+gssp_pos]!=gssp_seq[i].toLatin1())
                    goto WHILE_LABLE;
            }
            gsspAlleleInfo.alleleSequence = alleleSequence.mid(exonStartPos, gsspLength);
            gsspAlleleInfos.push_back(gsspAlleleInfo);//i2++;
WHILE_LABLE:;
        }
    }
}

void SoapTypingDB::updateGsspFileResultToRealTimeDatabase(const QString &fileName, int alignResult,
                                            const QString &typeResult, const QString &filterResult)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("UPDATE gsspFileTable set "
                  "alignResult=?,"
                  "typeResult=?,"
                  "filterResult=? "
                  "WHERE fileName=?");
    query.bindValue(0, alignResult);
    query.bindValue(1, typeResult);
    query.bindValue(2, filterResult);
    query.bindValue(3, fileName);
    bool isSuccess = query.exec();
    if(!isSuccess)
    {
         qDebug()<<"updateGsspFileResultToRealTimeDatabase error"<<fileName;
    }
}

void SoapTypingDB::getSampleTreeDataFromRealTimeDatabase(QVector<SampleTreeInfo_t> &sampleTreeInfoList)
{
    QSqlQuery query(m_SqlDB);
    bool isSuccess = query.exec("SELECT sampleName,geneName,analysisType,markType FROM sampleTable ORDER BY sampleName DESC");
    if(isSuccess)
    {
        while(query.next())
        {
            SampleTreeInfo_t sampleTreeInfo;
            sampleTreeInfo.sampleName = query.value(0).toString();
            sampleTreeInfo.geneName = query.value(1).toString();
            sampleTreeInfo.analysisType = query.value(2).toInt();
            sampleTreeInfo.markType = query.value(3).toInt();
            getFileTreeInfosFromRealTimeDatabase(sampleTreeInfo.sampleName, sampleTreeInfo.treeinfo);
            getGsspFileTreeInfosFromRealTimeDatabase(sampleTreeInfo.sampleName, sampleTreeInfo.treeinfo);
            sampleTreeInfoList.push_front(sampleTreeInfo);

        }
    }
}

void SoapTypingDB::getFileTreeInfosFromRealTimeDatabase(const QString &sampleName, QVector<FileTreeInfo_t> &fileTreeInfos)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT fileName,exonIndex,rOrF,isGood,alignResult FROM fileTable WHERE sampleName=? "
                  "ORDER BY exonIndex DESC,rOrF DESC, fileName DESC");
    query.bindValue(0, sampleName);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            FileTreeInfo_t fileTreeInfo;
            fileTreeInfo.isGssp = false;
            fileTreeInfo.fileName = query.value(0).toString();
            fileTreeInfo.exonIndex= query.value(1).toInt();
            fileTreeInfo.rOrF = query.value(2).toString();
            fileTreeInfo.isGood = query.value(3).toInt();
            fileTreeInfo.analysisType = query.value(4).toInt();
            fileTreeInfos.push_front(fileTreeInfo);
        }
    }
}

void SoapTypingDB::getGsspFileTreeInfosFromRealTimeDatabase(const QString &sampleName, QVector<FileTreeInfo_t> &gsspTreeInfos)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT fileName,gsspName,exonIndex,rOrF,isGood,alignResult FROM gsspFileTable WHERE sampleName=? "
                  "ORDER BY exonIndex DESC, rOrF DESC, fileName DESC");
    query.bindValue(0, sampleName);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            FileTreeInfo_t gsspTreeInfo;
            gsspTreeInfo.isGssp = true;
            gsspTreeInfo.fileName = query.value(0).toString();
            gsspTreeInfo.gsspName = query.value(1).toString();
            gsspTreeInfo.exonIndex= query.value(2).toInt();
            gsspTreeInfo.rOrF = query.value(3).toString();
            gsspTreeInfo.isGood = query.value(4).toInt();
            gsspTreeInfo.analysisType = query.value(5).toInt();
            gsspTreeInfos.push_front(gsspTreeInfo);
        }
    }
}


void SoapTypingDB::getResultDataFromsampleTable(const QString &sampleName, bool isCombined, QStringList &typeResult)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT typeResult FROM sampleTable WHERE sampleName=?");
    query.bindValue(0, sampleName);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            if(isCombined)
            {
                //typeResult = query.value(1).toString().split(";", QString::SkipEmptyParts);
            }
            else
            {
                typeResult = query.value(0).toString().split(";", QString::SkipEmptyParts);
            }
            break;
        }
    }
}

void SoapTypingDB::getResultDataFromGsspTable(const QString &fileName, bool isGsspFilter,
                                              bool isGssp, QStringList &typeResult)
{
    if(isGsspFilter || isGssp)
    {
        QSqlQuery query(m_SqlDB);
        query.prepare("SELECT typeResult, filterResult FROM gsspFileTable WHERE fileName=?");
        query.bindValue(0, fileName);
        bool isSuccess = query.exec();
        if(isSuccess)
        {
            while(query.next())
            {
                if(isGssp)
                {
                    typeResult = query.value(0).toString().split(";", QString::SkipEmptyParts);
                }
                else if (isGsspFilter)
                {
                    typeResult = query.value(1).toString().split(";", QString::SkipEmptyParts);
                }
                break;
            }
        }
    }
}


void SoapTypingDB::getAlldataFormRealTime(const QString &sampleName, const QString &exonIndex,
                                           QVector<Ab1FileTableBase> &vec_filetable)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT * FROM fileTable WHERE sampleName=? and exonIndex=?  order by fileName");
    query.bindValue(0, sampleName);
    query.bindValue(1, exonIndex);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            Ab1FileTableBase table;
            table.setFileName(query.value(0).toString());
            table.setBaseSequence(query.value(10).toByteArray());
            table.setBasePostion(query.value(11).toString());
            table.setBaseQuality(query.value(12).toString());
            table.setBaseNumber(query.value(13).toInt());
            table.setBaseASignal(query.value(14).toString());
            table.setBaseTSignal(query.value(15).toString());
            table.setBaseGSignal(query.value(16).toString());
            table.setBaseCSignal(query.value(17).toString());
            table.setSignalNumber(query.value(18).toInt());
            table.setMaxSignal(query.value(19).toInt());
            table.setAlignStartPos(query.value(24).toInt());
            table.setAlignEndPos(query.value(25).toInt());

            vec_filetable.push_back(table);
        }
    }

    QSqlQuery query_gssp(m_SqlDB);
    query_gssp.prepare("SELECT * from gsspFileTable WHERE sampleName=? and exonIndex=? order by fileName");
    query_gssp.bindValue(0, sampleName);
    query_gssp.bindValue(1, exonIndex);
    isSuccess = query_gssp.exec();
    if(isSuccess)
    {
        while(query_gssp.next())
        {
            Ab1FileTableBase table;
            table.setFileName(query_gssp.value(0).toString());
            table.setBaseSequence(query_gssp.value(10).toByteArray());
            table.setBasePostion(query_gssp.value(11).toString());
            table.setBaseQuality(query_gssp.value(12).toString());
            table.setBaseNumber(query_gssp.value(13).toInt());
            table.setBaseASignal(query_gssp.value(14).toString());
            table.setBaseTSignal(query_gssp.value(15).toString());
            table.setBaseGSignal(query_gssp.value(16).toString());
            table.setBaseCSignal(query_gssp.value(17).toString());
            table.setSignalNumber(query_gssp.value(18).toInt());
            table.setMaxSignal(query_gssp.value(19).toInt());
            table.setAlignStartPos(query_gssp.value(24).toInt());
            table.setAlignEndPos(query_gssp.value(25).toInt());

            vec_filetable.push_back(table);
        }
    }

}

void SoapTypingDB::getExonPositionIndex(const QString &geneName, QVector<int> &vec_index)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT exonPositionIndex FROM geneTable WHERE geneName = ?");
    query.bindValue(0, geneName);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            QStringList str_list = query.value(0).toString().split(":", QString::SkipEmptyParts);
            foreach(const QString &str, str_list)
            {
                vec_index.push_back(str.toInt());
            }
        }
    }
}

void SoapTypingDB::getExonNavigatorInfo(const QString &sampleName, ExonNavigatorInfo &exonNavigatorInfo)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT minExonIndex,maxExonIndex,mismatchBetweenPC,mismatchBetweenFR,mmismatchBetweenFR,editPostion"
                  " FROM sampleTable WHERE sampleName=?");
    query.bindValue(0, sampleName);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            exonNavigatorInfo.minExonIndex = query.value(0).toInt();
            exonNavigatorInfo.maxExonIndex = query.value(1).toInt();
            QStringList sl_pcMis = query.value(2).toString().split(":", QString::SkipEmptyParts);
            foreach(const QString &str, sl_pcMis)
            {
                exonNavigatorInfo.vec_pcMis.push_back(str.toInt());
            }
            QStringList sl_frMis = query.value(3).toString().split(":", QString::SkipEmptyParts);
            foreach(const QString &str, sl_frMis)
            {
                exonNavigatorInfo.vec_frMis.push_back(str.toInt());
            }
            QStringList sl_frUnEqual = query.value(4).toString().split(":",QString::SkipEmptyParts);
            foreach(const QString &str, sl_frUnEqual)
            {
                exonNavigatorInfo.vec_frUnEqual.push_back(str.toInt());
            }
            QStringList sl_editPos = query.value(5).toString().split(":", QString::SkipEmptyParts);
            foreach(const QString &str, sl_editPos)
            {
                exonNavigatorInfo.vec_editPos.push_back(str.toInt());
            }
        }
    }
}


void SoapTypingDB::getBaseAlignSampleInfo(const QString &sampleName, BaseAlignSampleInfo &baseAlignSampleInfo)
{
    QSqlQuery query(m_SqlDB);
    query.prepare("SELECT exonStartPos,exonEndPos,consensusSequence,forwardSequence,reverseSequence,"
                  "patternSequence,mismatchBetweenPC,mismatchBetweenFR,editPostion "
                  "FROM sampleTable WHERE sampleName=?");
    query.bindValue(0, sampleName);
    bool isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            baseAlignSampleInfo.alignStartPos = query.value(0).toInt();
            baseAlignSampleInfo.alignEndPos = query.value(1).toInt();
            baseAlignSampleInfo.consensusSeq = query.value(2).toString();
            baseAlignSampleInfo.forwardSeq = query.value(3).toString();
            baseAlignSampleInfo.reverseSeq = query.value(4).toString();
            baseAlignSampleInfo.patternSeq = query.value(5).toString();

            QStringList pcMis = query.value(6).toString().split(":", QString::SkipEmptyParts);
            for(int i=0; i<pcMis.size(); i++)
            {
                baseAlignSampleInfo.pcMisMatchPostion.insert(pcMis.at(i).toInt());
            }
            QStringList frMis = query.value(7).toString().split(":", QString::SkipEmptyParts);
            for(int i=0; i<frMis.size(); i++)
            {
                baseAlignSampleInfo.frMisMatchPostion.insert(frMis.at(i).toInt());
            }
            QStringList edit = query.value(8).toString().split(":", QString::SkipEmptyParts);
            for(int i=0; i<edit.size(); i++)
            {
                baseAlignSampleInfo.editPostion.insert(edit.at(i).toInt());
            }
        }
    }

    query.prepare("SELECT fileName,gsspName,exonStartPos,usefulSequence, excludeLeft, excludeRight, editInfo "
                  "FROM gsspFileTable WHERE sampleName=? AND alignResult != 4");
    query.bindValue(0, sampleName);
    isSuccess = query.exec();
    if(isSuccess)
    {
        while(query.next())
        {
            QSet<int> pos;
            BaseAlignGsspInfo gsspInfo;
            gsspInfo.gsspName = query.value(1).toString();
            gsspInfo.gsspFileAlignStartPos = query.value(2).toInt();
            QByteArray seq = query.value(3).toByteArray();
            modifySequence(seq, pos, gsspInfo.gsspFileAlignStartPos, query.value(4).toInt(), query.value(5).toInt(), query.value(6).toString());
            gsspInfo.gsspFileSeq = seq;
            getGsspPosAndSeqFromGsspDatabase(gsspInfo.gsspName, gsspInfo.gsspPostion, gsspInfo.gsspSeq);
            baseAlignSampleInfo.gsspInfoMap.insert(query.value(0).toString(), gsspInfo);
        }
    }
}
