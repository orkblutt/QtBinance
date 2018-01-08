#ifndef CANDLESTICKSVIEW_H
#define CANDLESTICKSVIEW_H

#include <QtCharts>
#include <QObject>

class candleSticksView : public QChartView
{
public:
    candleSticksView(QChart *chart, QWidget *parent = 0);
    void setReady(bool ready);
    bool ready(){return _ready;}

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);

private:
    bool _ready;
};

#endif // CANDLESTICKSVIEW_H
