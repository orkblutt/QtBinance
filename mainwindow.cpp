#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _buyPrice(0.0),
    _sellPrice(0.013850),
    _ethBal(0.0),
    _wtcBal(0.0),
    _currentPrice(0.0),
    _bReady(false),
    _buyMode(false)
{
    ui->setupUi(this);

    QString path(QDir::currentPath() + QDir::separator() + "key.ini");

    QSettings settings(path, QSettings::IniFormat);

    QString api_key=settings.value("api_key").toString();
    QString secret_key=settings.value("secret_key").toString();
qDebug() << api_key;

    _client = new binanceClient(api_key.toLocal8Bit()
                                , secret_key.toLocal8Bit());

    connect(_client, SIGNAL(priceSignal(double)), this, SLOT(onPriceReply(double)));
    connect(_client, SIGNAL(balanceSignal(double,double)), this, SLOT(onBalanceReply(double,double)));


    _client->getAccount();

    _thread = new priceThread(this);
    connect(_thread, SIGNAL(emitPrice()), this, SLOT(onPrice()));
    _thread->start();
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
        if(_buyPrice > 0.0 && (_buyPrice + ((_buyPrice/100) * 20) <= price)  )
        {
            qDebug() << _buyPrice + ((_buyPrice/100) * 20);

            qDebug() << "SELL";
            _sellPrice = price;
            on_pushButtonSell_clicked();

        }
        if(_sellPrice > 0.0 && (_sellPrice - ((_sellPrice/100) * 15) >= price) )
        {
            qDebug() << "BUY";
            _buyPrice = price;
            on_pushButtonBuy_clicked();
        }
    }
}

void MainWindow::onBalanceReply(double eth, double wtc)
{
    _ethBal = eth;
    _wtcBal = wtc;
    qDebug() << "balance is " << _ethBal << " ETH and " << _wtcBal << " WTC";
    _bReady = true;
}

priceThread::priceThread(QObject *parent) : _parent(parent)
{

}

void priceThread::run()
{
    //QThread::sleep(5);
    while(1)
    {
        emit emitPrice();
        QThread::sleep(1);
    }
}

void MainWindow::on_pushButtonBuy_clicked()
{
    double allIn = qRound(_ethBal / _currentPrice) - 2;
    qDebug() << allIn;
    _client->openOrder("WTCETH", "BUY", "MARKET", allIn, 0 );

}

void MainWindow::on_pushButtonSell_clicked()
{
    qDebug() << _wtcBal;
    double wtc = qRound(_wtcBal) - 1;
    _client->openOrder("WTCETH", "SELL", "MARKET", wtc, 0 );
}


void MainWindow::on_checkBoxBuySell_clicked(bool checked)
{
    _buyMode = checked;
    if(_buyMode)
    {
        if(_sellPrice == 0.0)
        {
            qDebug() << "_sellPrice need to be filled on buy mode";
            _buyMode = false;
            ui->checkBoxBuySell->setChecked(false);
        }
    }
}
