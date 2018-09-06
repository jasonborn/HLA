#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qscrollarea.h>
#include <QTreeWidgetItem>

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
    void slotShowOpenDlg();
    void InitUI();

public slots:
    void slotSampleTreeItemChanged(QTreeWidgetItem *item, int col);
    void slotExonFocusPosition(int selectpos, int exonstartpos, int index);

private:
    Ui::MainWindow *ui;
    SampleTreeWidget *m_pSampleTreeWidget;
    MatchListWidget *m_pMatchListWidget;
    MultiPeakWidget *m_pMultiPeakWidget;
    ExonNavigatorWidget *m_pExonNavigatorWidget;
    BaseAlignTableWidget *m_pBaseAlignTableWidget;
    QScrollArea *m_pPeak_area;
};

#endif // MAINWINDOW_H
