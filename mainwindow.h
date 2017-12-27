#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMutex>

#include "binanceclient.h"

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
private slots:
    void on_pushButtonBuy_clicked();
    void on_pushButtonSell_clicked();
    void on_checkBoxBuySell_clicked(bool checked);
};

#endif // MAINWINDOW_H
