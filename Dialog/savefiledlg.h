#ifndef SAVEFILEDLG_H
#define SAVEFILEDLG_H

#include <QDialog>
#include "all_base_struct.h"

namespace Ui {
class SaveFileDlg;
}

class SaveFileDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SaveFileDlg(QWidget *parent = nullptr);
    ~SaveFileDlg();
public:
    static void saveSampleTByDate(const SampleTreeInfo_t &sampleInfo, QString &date);
private:
    void InitUI();
    void ConnectSignalandSolt();
    void setTableDefaultData();
    void saveFileT(QVector<SampleTreeInfo_t> &sampleInfos);


private slots:
    void slotClickCheckAllBox(bool ischeck);
    void slotClickCheckSaveByDateBox(bool ischeck);
    void slotClickSaveButton();

private:
    Ui::SaveFileDlg *ui;
    bool m_bSaveByDate;
    QMap<QString,SampleTreeInfo_t> m_map_SampleTreeInfo;
};

#endif // SAVEFILEDLG_H