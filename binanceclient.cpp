#include <QDebug>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QUrlQuery>
#include <QUrl>

#include <QJsonDocument>
#include <QJsonObject>


#include "binanceclient.h"

binanceClient::binanceClient(QByteArray apiKey, QByteArray secretKey, QObject* parent):
    _apiKey(apiKey)
  , _secretKey(secretKey)
{
    _networkManagerAccount = new QNetworkAccessManager(this);
    _networkManagerPrice = new QNetworkAccessManager(this);
    _networkManagerOrder = new QNetworkAccessManager(this);
    _networkManagerCandle = new QNetworkAccessManager(this);
    _networkManagerSTime = new QNetworkAccessManager(this);

    connect(_networkManagerAccount, &QNetworkAccessManager::finished,
            this, &binanceClient::replyFinishedAccount, Qt::DirectConnection);
    connect(_networkManagerPrice, &QNetworkAccessManager::finished,
            this, &binanceClient::replyFinishedPrice, Qt::DirectConnection);
    connect(_networkManagerOrder, &QNetworkAccessManager::finished,
            this, &binanceClient::replyFinishedOrder, Qt::DirectConnection);

    connect(_networkManagerCandle, &QNetworkAccessManager::finished,
            this, &binanceClient::replyFinishedCandle, Qt::DirectConnection);

    connect(_networkManagerSTime, &QNetworkAccessManager::finished,
            this, &binanceClient::replyFinishedSTime, Qt::DirectConnection);





}

QByteArray binanceClient::getHMAC(const QString & message)
{
    QUrl url(message);
    return QMessageAuthenticationCode::hash(url.toEncoded(), _secretKey, QCryptographicHash::Sha256).toHex();
}


void binanceClient::getServerTime()
{
    QUrl url(QString(API_URL) + QString(TIME_V1));
    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(url);
    _networkManagerSTime->get(netReq);

}

void binanceClient::getAllPrices()
{
    QUrl url(QString(API_URL) + QString(PRICE_V1));
    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(url);
    qDebug() << url;
    //_networkManager->get(netReq);
}

void binanceClient::getSymbolPrice(const QString &name)
{
    QUrl url(QString(API_URL) + QString(PRICE_V3) + QString("?symbol=") + name);
    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(url);
    //qDebug() << url;
    _networkManagerPrice->get(netReq);
}

void binanceClient::getAccount()
{

    QString surl(QString(API_URL) + QString(ACCOUNT_V3) + QString("?"));
    QString query(QString("timestamp=") + QString::number(QDateTime::currentMSecsSinceEpoch()) + QString("&recvWindow=5000"));
    QString signature = QString(getHMAC(query));

    query += QString("&signature=") + signature;

    QUrl url(surl + query);


    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(url);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netReq.setRawHeader("User-Agent", "QTBinance");
    netReq.setRawHeader("X-Custom-User-Agent", "QTBinance");
    netReq.setRawHeader("Accept", "application/json");
    netReq.setRawHeader("X-MBX-APIKEY", _apiKey);


    _networkManagerAccount->get(netReq);

}

void binanceClient::getAllOrders()
{
    QString surl(QString(API_URL) + QString(ALLORDERS) + QString("?"));
    QString query(QString("symbol=WTCETH&timestamp=") + QString::number(QDateTime::currentMSecsSinceEpoch()) + QString("&recvWindow=5000"));
    QString signature = QString(getHMAC(query));

    query += QString("&signature=") + signature;

    QUrl url(surl + query);


    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(url);
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netReq.setRawHeader("User-Agent", "QTBinance");
    netReq.setRawHeader("X-Custom-User-Agent", "QTBinance");
    netReq.setRawHeader("Accept", "application/json");
    netReq.setRawHeader("X-MBX-APIKEY", _apiKey);


    //_networkManager->get(netReq);
}

void binanceClient::candleSticks(const QString &name, qulonglong startTime)
{
    // qDebug() << startTime;
    QUrl url(QString(API_URL) + QString(CANDLESTICK) + QString("?symbol=") + name + QString("&interval=1m&startTime=") + QString::number(startTime));

    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    // netReq.setRawHeader("User-Agent", "QTBinance");
    // netReq.setRawHeader("X-Custom-User-Agent", "QTBinance");
    // netReq.setRawHeader("Accept", "application/json");
    netReq.setUrl(url);
    //qDebug() << url;
    QNetworkReply* reply = _networkManagerCandle->get(netReq);

}

void binanceClient::openOrder(const QString &symbol, const QString &order, const QString &type, double quantity, double price)
{
    QString surl = QString(API_URL) + QString (OPENORDER);

    qDebug() << surl;

    QUrlQuery qquery;
    qquery.addQueryItem("symbol", symbol);
    qquery.addQueryItem("side", order);
    qquery.addQueryItem("type", type);
    //qquery.addQueryItem("timeInForce", "GTC");
    qquery.addQueryItem("quantity", QString::number(quantity));
    //qquery.addQueryItem("price", QString::number(price));
    qquery.addQueryItem("timestamp", QString::number(QDateTime::currentMSecsSinceEpoch()));
    qquery.addQueryItem("signature", QString(getHMAC(qquery.toString())));
    qDebug() << qquery.toString();


    QNetworkRequest netReq;
    netReq.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    netReq.setUrl(QUrl(surl));
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netReq.setRawHeader("User-Agent", "QTBinance");
    netReq.setRawHeader("X-Custom-User-Agent", "QTBinance");
    netReq.setRawHeader("Accept", "application/json");
    netReq.setRawHeader("X-MBX-APIKEY", _apiKey);

    _networkManagerOrder->post(netReq, qquery.toString(QUrl::FullyEncoded).toUtf8());

}

void binanceClient::replyFinishedAccount(QNetworkReply *reply)
{

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }


    QByteArray data = reply->readAll();
    if(!data.isEmpty())
    {
        // qDebug() << data;
        QJsonDocument jsondoc = QJsonDocument::fromJson(data);
        if(!jsondoc.isNull())
        {
            if(jsondoc.isObject())
            {
                double wtc = 0.0;
                double eth = 0.0;
                QJsonObject obj = jsondoc.object();
                foreach(QJsonValue element, obj["balances"].toArray())
                {
                    QJsonObject node = element.toObject();
                    if(node["asset"] == "ETH")
                        eth = node["free"].toString().toDouble();
                    else if(node["asset"] == "WTC")
                        wtc = node["free"].toString().toDouble();

                }

                emit balanceSignal(eth, wtc);
            }
        }
    }

    reply->deleteLater();
}

void binanceClient::replyFinishedPrice(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    if(!data.isEmpty())
    {
        // qDebug() << data;
        QJsonDocument jsondoc = QJsonDocument::fromJson(data);

        if(!jsondoc.isNull())
        {
            if(jsondoc.isObject())
            {
                QJsonObject obj = jsondoc.object();
                // qDebug() << obj["price"].toString();
                emit priceSignal(obj["price"].toString().toDouble());
            }

            else if(jsondoc.isArray())
            {
                foreach(QJsonValue element, jsondoc.array())
                {
                    QJsonObject node = element.toObject();
                    if(node["symbol"].toString() == "WTCETH")
                        qDebug() << node["symbol"].toString() << node["price"].toString().toDouble();
                }
            }
        }
    }

    reply->deleteLater();
}


void binanceClient::replyFinishedOrder(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    if(!data.isEmpty())
    {
        qDebug() << data;
        if(QString(data).indexOf("Error") == -1)
        {
            QJsonDocument jsondoc = QJsonDocument::fromJson(data);
            if(!jsondoc.isNull())
            {
                if(jsondoc.isObject())
                {
                    QJsonObject obj = jsondoc.object();
qDebug() << obj["status"];
                    if(obj["status"].toString() == "FILLED")
                        emit orderStatusSignal(true);
                }
            }
        }
        else
        {
            emit orderStatusSignal(false);
        }
    }

    reply->deleteLater();
}

void binanceClient::replyFinishedCandle(QNetworkReply *reply)
{

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    //qDebug() << data;

    if(!data.isEmpty())
    {
        QJsonDocument jsondoc = QJsonDocument::fromJson(data);

        if(!jsondoc.isNull())
        {
            if(jsondoc.isArray())
            {
                emit candleSticksSignal(jsondoc.array());
            }
        }
    }

    reply->deleteLater();
}

void binanceClient::replyFinishedSTime(QNetworkReply *reply)
{

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument jsondoc = QJsonDocument::fromJson(data);
    if(!jsondoc.isNull())
    {
        if(jsondoc.isObject())
        {
            QJsonObject obj = jsondoc.object();

            emit serverTimeSignal(obj["serverTime"].toVariant().toULongLong());
        }
    }

    reply->deleteLater();
}


