#ifndef SAMPLETREEWIDGET_H
#define SAMPLETREEWIDGET_H

#include <QTreeWidget>

class SampleTreeWidget:public QTreeWidget
{
public:
    SampleTreeWidget(QWidget *parent = 0);
    ~SampleTreeWidget();
    void SetTreeData();
    void SetSelectItem(int index, QString &str_name);
private:
    void InitUI();

    void ConnectSignalandSolt();
};

#endif // SAMPLETREEWIDGET_H
