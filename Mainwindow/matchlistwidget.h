#ifndef MATCHLISTWIDGET_H
#define MATCHLISTWIDGET_H

#include <QTableWidget>

class MatchListWidget: public QTableWidget
{
    Q_OBJECT
public:
    MatchListWidget(QWidget *parent=0);
    ~MatchListWidget();
    void SetTableData(const QString &str_sample, const QString &str_gssp);
private:
    void InitUI();

};

#endif // MATCHLISTWIDGET_H
