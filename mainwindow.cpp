#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _buyPrice(0.0134),
    _sellPrice(0.0),
    _ethBal(0.0),
    _wtcBal(0.0),
    _currentPrice(0.0),
    _bReady(false),
    _buyMode(false),
    _serverTime(0),
    _orderPending(false),
    _priceOrder(0.0),
    _orderMessage(true)
{
    ui->setupUi(this);
    QIcon icon(":/images/binance_logo.png");
    setWindowIcon(icon);

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
    connect(_client, SIGNAL(orderStatusSignal(bool)), this, SLOT(onOrderReply(bool)));

    _thread = new priceThread(this);
    connect(_thread, SIGNAL(emitPrice()), this, SLOT(onPrice()));
    connect(_thread, SIGNAL(refreshAccount()), this, SLOT(onRefreshAccount()));
    connect(_thread, SIGNAL(refreshCandles()), this, SLOT(onRefreshCandles()));
    connect(_thread, SIGNAL(refreshSTime()), this, SLOT(onRefreshSTime()));
    _thread->start();

    _wtcethCandles = new QCandlestickSeries();
    _wtcethCandles->setIncreasingColor(QColor(Qt::green));
    _wtcethCandles->setDecreasingColor(QColor(Qt::red));

    _chart = new QChart();
    _chart->setTitle("WTCETH");
    _chart->setAnimationOptions(QChart::NoAnimation);
    _chart->setTheme(QChart::ChartThemeDark);

    _chart->legend()->setVisible(false);

    _chartView = new candleSticksView(_chart);
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

    if(_bReady && !_orderPending)
    {

        if(_orderMessage)
        {
            if(_buyPrice > 0)
                qDebug() << "Next sell at " << _buyPrice + ((_buyPrice/100) * 10) << " ETH";
            else
                qDebug() << "Next buy at " << _sellPrice - ((_sellPrice/100) * 10) << " ETH";
            _orderMessage = false;
        }


        if(_buyPrice > 0.0 && (_buyPrice + ((_buyPrice/100) * 10) <= price)  )
        {
            qDebug() << "SELL @ " << price;

            _priceOrder = price;
            _orderMessage =true;
            on_pushButtonSell_clicked();
        }

        if(_sellPrice > 0.0 && (_sellPrice - ((_sellPrice/100) * 10) >= price) )
        {
            qDebug() << "BUY @ " << price;

            _priceOrder = price;
            _orderMessage = true;
            on_pushButtonBuy_clicked();
        }
    }
}

void MainWindow::onOrderReply(bool filled)
{
    if(filled)
    {
        qDebug() << "ORDER has been filled";
        if(_buyMode)
        {
            _buyPrice = _priceOrder;
            _sellPrice = 0;
        }
        else
        {
            _sellPrice = _priceOrder;
            _buyPrice = 0;
        }

        qDebug() << "SELL PRICE: " << _sellPrice;
        qDebug() << "BUY PRICE: " << _buyPrice;

        qDebug() << "NEXT MOVE IS A " << ((_sellPrice > 0.0) ? "BUY" : "SELL");

        _buyMode = !_buyMode;

        onRefreshAccount();
    }
    else
        qDebug() << "ORDER IS NOT FILLED!";
    _orderPending = false;
}

void MainWindow::onBalanceReply(double eth, double wtc)
{
    _ethBal = eth;
    _wtcBal = wtc;

    ui->lcdNumberBalEth->display(_ethBal);
    ui->lcdNumberBalWTC->display(_wtcBal);

    ui->label_ETHValue->setText(QString::number(_wtcBal * _currentPrice) + QString(" ETH"));

    _bReady = true;
}

void MainWindow::onRefreshSTimeReply(qulonglong stime)
{
    _serverTime = stime;
}

void MainWindow::onCandleReply(QJsonArray jcandleArray)
{

    QStringList categories;

    _chart->removeSeries(_wtcethCandles);
    _wtcethCandles->clear();

    int count = 0;
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
        count++;
    }

    _chart->addSeries(_wtcethCandles);

    _chart->createDefaultAxes();

    QBarCategoryAxis *axisX = qobject_cast<QBarCategoryAxis *>(_chart->axes(Qt::Horizontal).at(0));
    axisX->setCategories(categories);

    if(!_chartView->ready()) _chartView->setReady(true);

}


void MainWindow::onRefreshAccount()
{
    _client->getAccount();
}

void MainWindow::onRefreshCandles()
{
    if(_serverTime > 0)
        _client->candleSticks("WTCETH", _serverTime - 3600000);
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
        if(refreshSTimeCntr % 120 == 0)
        {
            refreshSTimeCntr = 0;
            emit refreshSTime();
            //QThread::msleep(500);
        }

        if(refreshAccCntr % 50 == 0)
        {
            refreshAccCntr = 0;
            emit refreshAccount();
            //QThread::msleep(500);
        }

        if(refreshCandleCntr % 15 == 0)
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
    double allIn = qRound(_ethBal / _currentPrice) - 15;
    qDebug() << allIn;
    _client->openOrder("WTCETH", "BUY", "MARKET", allIn, 0 );
    _buyMode = true;
    _orderPending = true;
    _priceOrder = _currentPrice;
    _orderMessage = true;

}

void MainWindow::on_pushButtonSell_clicked()
{
    qDebug() << _wtcBal;
    double wtc = qRound(_wtcBal) - 1;
    _client->openOrder("WTCETH", "SELL", "MARKET", wtc, 0 );
    _buyMode = false;
    _orderPending = true;
    _priceOrder = _currentPrice;
    _orderMessage = true;
}

