#include "candlesticksview.h"

candleSticksView::candleSticksView(QChart *chart, QWidget *parent) :
    QChartView(chart, parent)
  , _ready(false)
{
    setRubberBand(QChartView::RectangleRubberBand);
    QCursor cursor;
    cursor.setShape(Qt::CrossCursor);
    setCursor(cursor);
}

void candleSticksView::setReady(bool ready)
{
    _ready = ready;
}


void candleSticksView::mouseMoveEvent(QMouseEvent *event)
{
    if(_ready)
    {
        QPointF point = chart()->mapToValue(event->pos());

        QBarCategoryAxis *axisX = qobject_cast<QBarCategoryAxis *>(chart()->axes(Qt::Horizontal).at(0));

        QToolTip::showText(event->globalPos(),
                           QString((point.x() > 0 && qRound(point.x()) < axisX->categories().size()) ? axisX->categories().at(qRound(point.x())) : "")
                           + "\r\n" +
                           QString::number(point.y()),
                           this, rect() );
    }
    QChartView::mouseMoveEvent(event);
}


void candleSticksView::wheelEvent(QWheelEvent *event)
{
    if(event->angleDelta().y() > 0)
        chart()->zoomIn();
    else
        chart()->zoomOut();

    event->accept();
}
