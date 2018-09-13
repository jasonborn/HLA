#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qscrollarea.h>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>

namespace Ui {
class MainWindow;
}

class SampleTreeWidget;
class MatchListWidget;
class MultiPeakWidget;
class ExonNavigatorWidget;
class BaseAlignTableWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void SetStatusbar();                    //显示底部左侧状态栏信息
    void ConnectSignalandSolt();            //连接信号与槽函数
    void DisConnectSignalandSolt();         //断开信号与槽函数连接

    void InitUI();

public slots:
    void slotSampleTreeItemChanged(QTreeWidgetItem *item, int col);
    void slotExonFocusPosition(int startpos, int selectpos, int exonstartpos, int index);
    void slotAlignTableFocusPosition(QTableWidgetItem *item);
    void slotPeakFocusPosition(int index, int colnum);

    void slotShowSaveDlg();
    void slotShowLoadFileDlg();
    void slotShowOpenDlg();
    void slotShowDeleteDlg();
    void slotShowExportDlg();

    void slotReset();
    void slotMisPosForward();
    void slotMisPosBackward();
    void slotMarkAllSampleApproved();
    void slotMarkAllSampleReviewed();
    void slotAlignPair();
    void slotAlignLab();
    void slotUpdateDatabase();
    void slotControl();
    void slotSetExonTrim();

    void slotyRangeRoomUp();
    void slotyRangeRoomDown();
    void slotyRoomUp();
    void slotyRoomDown();
    void slotxRoomUp();
    void slotxRoomDown();
    void resetRoomSetting();

    void slotApplyOne();
    void slotApplyAll();
    void slotAnalyseLater();
    void slotAnalyseNow();
    void slotanalyse();

    void slotAbout();
    void slotHelp();
private:
    Ui::MainWindow *ui;
    SampleTreeWidget *m_pSampleTreeWidget;
    MatchListWidget *m_pMatchListWidget;
    MultiPeakWidget *m_pMultiPeakWidget;
    ExonNavigatorWidget *m_pExonNavigatorWidget;
    BaseAlignTableWidget *m_pBaseAlignTableWidget;
    QScrollArea *m_pPeak_area;
    QString m_str_SelectFile;           //保存样品列表选中的文件名称
};

#endif // MAINWINDOW_H
