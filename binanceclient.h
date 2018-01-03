#ifndef BINANCECLIENT_H
#define BINANCECLIENT_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>



#define    API_URL "https://api.binance.com/api"
#define    WITHDRAW_API_URL "https://api.binance.com/wapi"
#define    WEBSITE_URL "https://www.binance.com"
#define    PUBLIC_API_VERSION "v1"
#define    PRIVATE_API_VERSION "v3"
#define    WITHDRAW_API_VERSION "v3"

#define     TIME_V1 "/v1/time"
#define     PRICE_V1 "/v1/ticker/allPrices"
#define     PRICE_V3 "/v3/ticker/price"
#define     ACCOUNT_V3 "/v3/account"
#define     ALLORDERS "/v3/allOrders"
#define     OPENORDER "/v3/order"
#define     CANDLESTICK "/v1/klines"

#define    SYMBOL_TYPE_SPOT "SPOT"

#define    ORDER_STATUS_NEW "NEW"
#define    ORDER_STATUS_PARTIALLY_FILLED "PARTIALLY_FILLED"
#define    ORDER_STATUS_FILLED "FILLED"
#define    ORDER_STATUS_CANCELED "CANCELED"
#define    ORDER_STATUS_PENDING_CANCEL "PENDING_CANCEL"
#define    ORDER_STATUS_REJECTED "REJECTED"
#define    ORDER_STATUS_EXPIRED "EXPIRED"

#define    KLINE_INTERVAL_1MINUTE "1m"
#define    KLINE_INTERVAL_3MINUTE "3m"
#define    KLINE_INTERVAL_5MINUTE "5m"
#define    KLINE_INTERVAL_15MINUTE "15m"
#define    KLINE_INTERVAL_30MINUTE "30m"
#define    KLINE_INTERVAL_1HOUR "1h"
#define    KLINE_INTERVAL_2HOUR "2h"
#define    KLINE_INTERVAL_4HOUR "4h"
#define    KLINE_INTERVAL_6HOUR "6h"
#define    KLINE_INTERVAL_8HOUR "8h"
#define    KLINE_INTERVAL_12HOUR "12h"
#define    KLINE_INTERVAL_1DAY "1d"
#define    KLINE_INTERVAL_3DAY "3d"
#define    KLINE_INTERVAL_1WEEK "1w"
#define    KLINE_INTERVAL_1MONTH "1M"

#define    SIDE_BUY "BUY"
#define    SIDE_SELL "SELL"

#define    ORDER_TYPE_LIMIT "LIMIT"
#define    ORDER_TYPE_MARKET "MARKET"
#define    ORDER_TYPE_STOP_LOSS "STOP_LOSS"
#define    ORDER_TYPE_STOP_LOSS_LIMIT "STOP_LOSS_LIMIT"
#define    ORDER_TYPE_TAKE_PROFIT "TAKE_PROFIT"
#define    ORDER_TYPE_TAKE_PROFIT_LIMIT "TAKE_PROFIT_LIMIT"
#define    ORDER_TYPE_LIMIT_MAKER "LIMIT_MAKER"

#define    TIME_IN_FORCE_GTC "GTC"  // Good till cancelled
#define    TIME_IN_FORCE_IOC "IOC"  // Immediate or cancel
#define    TIME_IN_FORCE_FOK "FOK"  // Fill or kill

#define    ORDER_RESP_TYPE_ACK "ACK"
#define    ORDER_RESP_TYPE_RESULT "RESULT"
#define    ORDER_RESP_TYPE_FULL "FULL"




class binanceClient : public QObject
{
    Q_OBJECT
public:
    binanceClient(QByteArray apiKey, QByteArray secretKey, QObject* parent = NULL);

    QByteArray getHMAC(const QString&);

    void getServerTime();
    void getAllPrices();
    void getSymbolPrice(const QString& name);
    void getAccount();
    void getAllOrders();
    void candleSticks(const QString& name, qulonglong startTime);

    void openOrder(const QString& symbol, const QString& order, const QString& type, double quantity, double price);

private:
    QByteArray _secretKey;
    QByteArray _apiKey;

    QNetworkAccessManager* _networkManagerSTime;
    QNetworkAccessManager* _networkManagerAccount;
    QNetworkAccessManager* _networkManagerPrice;
    QNetworkAccessManager* _networkManagerOrder;
    QNetworkAccessManager* _networkManagerCandle;

public slots:
    void replyFinishedAccount(QNetworkReply* reply);
    void replyFinishedPrice(QNetworkReply* reply);
    void replyFinishedOrder(QNetworkReply* reply);
    void replyFinishedCandle(QNetworkReply* reply);
    void replyFinishedSTime(QNetworkReply *reply);

signals:

    void priceSignal(double price);
    void balanceSignal(double, double);
    void refreshAccount();
    void serverTimeSignal(qulonglong stime);
    void candleSticksSignal(QJsonArray);
};

#endif // BINANCECLIENT_H
