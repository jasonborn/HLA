#ifndef BASEALIGNTABLEWIDGET_H
#define BASEALIGNTABLEWIDGET_H

#include <QTableWidget>
#include "all_base_struct.h"

class BaseAlignTableWidget:public QTableWidget
{
    Q_OBJECT
public:
    BaseAlignTableWidget();
    ~BaseAlignTableWidget();
    void SetAlignTableData(QString &str_samplename);
private:
    void InitUI();
    void clearBaseAlignTableSampleItem();
    void getTableHead(QStringList &head, int length, int start);
private:
    int m_iRowNum;
    int m_iColNum;

    QString m_str_SampleName;
    QStringList m_sl_defaulthead;
    BaseAlignSampleInfo m_BaseAlignSampleInfo;
};

#endif // BASEALIGNTABLEWIDGET_H
