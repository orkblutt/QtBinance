#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "binanceclient.h"

#include <QMainWindow>
#include <QThread>
#include <QMutex>

#include <QtCharts>

using namespace QtCharts;



namespace Ui {
class MainWindow;
}


class priceThread: public QThread
{
    Q_OBJECT
public:
    priceThread(QObject* parent);

    void run();
private:

    QObject* _parent;

signals:

    void emitPrice();
    void refreshAccount();
    void refreshCandles();
    void refreshSTime();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    binanceClient* _client;
    priceThread* _thread;
    QCandlestickSeries* _wtcethCandles;
    QChart* _chart = new QChart();
    QChartView* _chartView;


    qulonglong _serverTime;


    bool _bReady;
    double _currentPrice;

    double _buyPrice;
    double _sellPrice;

    double _ethBal;
    double _wtcBal;

    bool _buyMode;



public slots:
    void onPrice();
    void onPriceReply(double price);
    void onBalanceReply(double eth, double wtc);
    void onRefreshSTimeReply(qulonglong stime);
    void onCandleReply(QJsonArray jcandleArray);
    void onRefreshAccount();
    void onRefreshCandles();
    void onRefreshSTime();

private slots:
    void on_pushButtonBuy_clicked();
    void on_pushButtonSell_clicked();
};

#endif // MAINWINDOW_H
