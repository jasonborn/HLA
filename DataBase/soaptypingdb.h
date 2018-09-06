#ifndef SOAPTYPINGDB_H
#define SOAPTYPINGDB_H

#include <QSqlDatabase>
#include "all_base_struct.h"
#include "Core/fileTablebase.h"

enum AnalysisType
{
    MATCHTOTAL = 0,
    MATCHRARE,
    MATCHBAD,
    MISMATCH,
    UNMATCH
};

enum MarkType
{
    OWNED = 0,
    PENDING,
    REVIEWED,
    APPROVED
};


class SoapTypingDB
{
public:
    static SoapTypingDB *GetInstance()
    {
        static SoapTypingDB instance;
        return &instance;
    }
    ~SoapTypingDB();
    void GetGsspNames(QStringList &gsspNames);      //获取gssp名称列表
    bool FindExonAndRFByGsspName(const QString &name, QString &exonIndex, QString &RF);
    void GetGsspMapToExonAndFR(QMap<QString, ExonAndRF> &mapToExonAndFR);
    void GetExonInfo(const QString &gene_name, int exon_index, ExonInfoS& exonInfo);
    int  GetExcludePosition(const QString &key, int &lefpos, int &rightpos);
    void InsertOneFileTable(Ab1FileTableBase &fileTable);
    void insertOneGsspFileTable(Ab1FileTableBase &fileTable);
    int getFileInfoFromRealTimeDatabase(const QString &sampleName, QVector<FileInfo> &fileInfos,
                                        ExonInfo &exonInfo);
    int getGsspFileInfoFromRealTimeDatabase(const QString &sampleName, QVector<FileInfo> &gsspFileInfos,
                                                          ExonInfo &exonInfo);
    void compareWithExonInfo(ExonInfo &exonInfo, int exonIndex, int exonStartPos, int exonEndPos);
    void modifySequence(QByteArray &sequence, QSet<int> &editPostion, int exonStartPos, int excludeLeft,
                                      int excludeRight, const QString &editString);
    void insertSampleInfoToRealTimeDatabase(const SampleInfo &sampleInfo);
    void getConsensusSequenceFromStaticDatabase(const QString &geneName, QByteArray &geneSequence,
                                                int exonStartPos, int exonLength);
    void getShieldAllelesFromDatabase(const QString &sampleName, QSet<QString> &shieldAlleles);
    void getAlleleInfosFromStaticDatabase(const QString &geneName, int exonStartPos, int alleleLength,
                                          int minExonIndex, int maxExonIndex, QVector<AlleleInfo> &alleleInfos,
                                          QSet<QString> &shieldAlleles);
    void getGsspAlleleInfosFromStaticDatabase(const QString &geneName, int exonStartPos, int gsspLength,
                                              QVector<GsspAlleleInfo> &gsspAlleleInfos, const QString &gsspName);
    void getGsspPosAndSeqFromGsspDatabase(const QString &gsspName, int &gsspPos, QString &gsspSeq);
    void updateGsspFileResultToRealTimeDatabase(const QString &fileName, int alignResult,
                                                const QString &typeResult, const QString &filterResult);

    void getSampleTreeDataFromRealTimeDatabase(QVector<SampleTreeInfo_t> &sampleTreeInfoList);
    void getFileTreeInfosFromRealTimeDatabase(const QString &sampleName, QVector<FileTreeInfo_t> &fileTreeInfos);
    void getGsspFileTreeInfosFromRealTimeDatabase(const QString &sampleName, QVector<FileTreeInfo_t> &gsspTreeInfos);
    void getResultDataFromsampleTable(const QString &sampleName, bool isCombined, QStringList &typeResult);
    void getResultDataFromGsspTable(const QString &fileName, bool isGsspFilter,
                                                  bool isGssp, QStringList &typeResult);

    void getAlldataFormRealTime(const QString &sampleName, const QString &exonIndex,
                                 QVector<Ab1FileTableBase> &vec_filetable);

    void getExonPositionIndex(const QString &geneName, QVector<int> &position);
    void getExonNavigatorInfo(const QString &sampleName, ExonNavigatorInfo &exonNavigatorInfo);

    void getBaseAlignSampleInfo(const QString &sampleName, BaseAlignSampleInfo &baseAlignSampleInfo);
private:
    bool InitDB();

public:
    SoapTypingDB();
    SoapTypingDB(const SoapTypingDB &){}
    SoapTypingDB& operator=(const SoapTypingDB&){}
private:
    static QSqlDatabase m_SqlDB;
    //QSqlQuery *m_pQuery;
};

#endif // SOAPTYPINGDB_H
