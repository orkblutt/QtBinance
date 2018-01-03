#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _buyPrice(0.01396892),
    _sellPrice(0.0),
    _ethBal(0.0),
    _wtcBal(0.0),
    _currentPrice(0.0),
    _bReady(false),
    _buyMode(false),
    _serverTime(0)
{
    ui->setupUi(this);

    QString path(QDir::currentPath() + QDir::separator() + "key.ini");

    QSettings settings(path, QSettings::IniFormat);

    QString api_key=settings.value("api_key").toString();
    QString secret_key=settings.value("secret_key").toString();


    _client = new binanceClient(api_key.toLocal8Bit()
                                , secret_key.toLocal8Bit());



    connect(_client, SIGNAL(priceSignal(double)), this, SLOT(onPriceReply(double)));
    connect(_client, SIGNAL(balanceSignal(double,double)), this, SLOT(onBalanceReply(double,double)));
    connect(_client, SIGNAL(serverTimeSignal(qulonglong)), this, SLOT(onRefreshSTimeReply(qulonglong)));
    connect(_client, SIGNAL(candleSticksSignal(QJsonArray)), this, SLOT(onCandleReply(QJsonArray)));



    _thread = new priceThread(this);
    connect(_thread, SIGNAL(emitPrice()), this, SLOT(onPrice()));
    connect(_thread, SIGNAL(refreshAccount()), this, SLOT(onRefreshAccount()));
    connect(_thread, SIGNAL(refreshCandles()), this, SLOT(onRefreshCandles()));
    connect(_thread, SIGNAL(refreshSTime()), this, SLOT(onRefreshSTime()));


    _thread->start();

    _wtcethCandles = new QCandlestickSeries();
   // _wtcethCandles->setName("Candlesticks");
    _wtcethCandles->setIncreasingColor(QColor(Qt::green));
    _wtcethCandles->setDecreasingColor(QColor(Qt::red));


    _chart = new QChart();

    _chart->setTitle("WTCETH");
    _chart->setAnimationOptions(QChart::NoAnimation);
    _chart->setTheme(QChart::ChartThemeDark);

    _chart->legend()->setVisible(true);
    _chart->legend()->setAlignment(Qt::AlignBottom);

    _chartView = new QChartView(_chart);
    _chartView->setRenderHint(QPainter::Antialiasing);

    ui->gridLayout_2->addWidget(_chartView);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onPrice()
{
    _client->getSymbolPrice("WTCETH");

}

void MainWindow::onPriceReply(double price)
{

    _currentPrice = price;
    ui->lcdNumberWTCETH->display(price);

    if(_bReady)
    {
        if(_buyPrice > 0.0 && (_buyPrice + ((_buyPrice/100) * 5) <= price)  )
        {
            qDebug() << "SELL";
            _sellPrice = price;
            _buyPrice = 0;
            on_pushButtonSell_clicked();

        }

        if(_sellPrice > 0.0 && (_sellPrice - ((_sellPrice/100) * 3) >= price) )
        {
            qDebug() << "BUY";
            _buyPrice = price;
            _sellPrice = 0;
            on_pushButtonBuy_clicked();
        }
    }
}

void MainWindow::onBalanceReply(double eth, double wtc)
{
    _ethBal = eth;
    _wtcBal = wtc;

    qDebug() << "balance is " << _ethBal << " ETH and " << _wtcBal << " WTC";
    ui->lcdNumberBalEth->display(_ethBal);
    ui->lcdNumberBalWTC->display(_wtcBal);

    _bReady = true;
}

void MainWindow::onRefreshSTimeReply(qulonglong stime)
{
    _serverTime = stime;
    qDebug() << "Server time: " << stime;
}

void MainWindow::onCandleReply(QJsonArray jcandleArray)
{

    QStringList categories;

    _chart->removeSeries(_wtcethCandles);

    _wtcethCandles->clear();

    foreach(QJsonValue element, jcandleArray)
    {
        QJsonArray array = element.toArray();
        QCandlestickSet *candlestickSet = new QCandlestickSet(array[0].toDouble());
        candlestickSet->setOpen(array[1].toString().toDouble());
        candlestickSet->setHigh(array[2].toString().toDouble());
        candlestickSet->setLow(array[3].toString().toDouble());
        candlestickSet->setClose(array[4].toString().toDouble());
        _wtcethCandles->append(candlestickSet);
        categories << QDateTime::fromMSecsSinceEpoch(candlestickSet->timestamp()).toString("hh:mm");
    }

    _chart->addSeries(_wtcethCandles);

    _chart->createDefaultAxes();

    QBarCategoryAxis *axisX = qobject_cast<QBarCategoryAxis *>(_chart->axes(Qt::Horizontal).at(0));
    axisX->setCategories(categories);
/*
    QValueAxis *axisY = qobject_cast<QValueAxis *>(_chart->axes(Qt::Vertical).at(0));
    qDebug() << axisY->max() << " " << axisY->min();
    //axisY->setMax(axisY->max() * 1.01);
    //axisY->setMin(axisY->min() * 0.99);
*/

}

void MainWindow::onRefreshAccount()
{
    _client->getAccount();
}

void MainWindow::onRefreshCandles()
{
    if(_serverTime > 0)
        _client->candleSticks("WTCETH", _serverTime - 1800000);
}

void MainWindow::onRefreshSTime()
{
    _client->getServerTime();
}

priceThread::priceThread(QObject *parent) : _parent(parent)
{

}

void priceThread::run()
{
    int refreshAccCntr = 0;
    int refreshCandleCntr = 0;
    int refreshSTimeCntr = 0;

    while(1)
    {

        if(refreshSTimeCntr % 500 == 0)
        {
            refreshSTimeCntr = 0;
            emit refreshSTime();
            QThread::msleep(500);
        }

        if(refreshAccCntr % 200 == 0)
        {
            refreshAccCntr = 0;
            emit refreshAccount();
            QThread::msleep(500);
        }

        if(refreshCandleCntr % 5 == 0)
        {
            refreshCandleCntr = 0;
            emit refreshCandles();
        }


        emit emitPrice();

        refreshAccCntr++;
        refreshCandleCntr++;
        refreshSTimeCntr++;

        QThread::msleep(500);
    }
}

void MainWindow::on_pushButtonBuy_clicked()
{
    double allIn = qRound(_ethBal / _currentPrice) - 5;
    qDebug() << allIn;
    _client->openOrder("WTCETH", "BUY", "MARKET", allIn, 0 );
    _buyPrice = _currentPrice;
    _sellPrice = 0.0;

}

void MainWindow::on_pushButtonSell_clicked()
{
    qDebug() << _wtcBal;
    double wtc = qRound(_wtcBal) - 1;
    _client->openOrder("WTCETH", "SELL", "MARKET", wtc, 0 );
    _sellPrice = _currentPrice;
    _buyPrice = 0;
}

