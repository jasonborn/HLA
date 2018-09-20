#ifndef MATCHLISTWIDGET_H
#define MATCHLISTWIDGET_H

#include <QTableWidget>
#include "all_base_struct.h"

class MatchListWidget: public QTableWidget
{
    Q_OBJECT
public:
    MatchListWidget(QWidget *parent=0);
    ~MatchListWidget();
    void SetTableData(bool brefresh, const QString &str_sample, const QString &str_file,
                      const QString &str_info, int col);
    QStringList & GetMatchList();
private:
    void InitUI();
    void CreateRightMenu();
    void ConnectSignalandSlot();
    void contextMenuEvent(QContextMenuEvent *event);
    void setGssp();
    bool processGssp(const QVector<AllelePair> &allelePairs, const QString &sampleName, QVector<QStringList> &result);
    void findUsefulGssp(const char *seq11, const char *seq12, const char *seq21, const char *seq22,
                        int exonStart, int exonEnd, QVector<GsspTable> &gsspTables,
                        QStringList &gssps, QStringList &infos);
    void ClearTable();
private slots:
    void slotClickIndelItem(QTableWidgetItem* itemNow);
    void slotRowChanged(QTableWidgetItem* itemNow);

    void slotShowGsspZCode();
    void slotSetFinalType();
signals:
    void signalIndelPostion(int pos);
    void signalAllelePair(QString &, QString &);
private:
    int m_iOldRow; //上次选中的行数
    int m_iRowCount;//总行数
    int m_iCol;
    QString m_str_file;
    QString m_str_SampleName;
    QMenu *m_pRightMenu;
    QAction *m_pActShowGSSPZCode;
    QAction *m_pActSetFinalType;
    QVector<QStringList> m_vec_gsspInfo;
    QStringList m_strlist_result;
};

#endif // MATCHLISTWIDGET_H
