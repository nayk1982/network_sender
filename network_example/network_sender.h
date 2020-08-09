#ifndef NETWORKSENDER_H
#define NETWORKSENDER_H

#include <QObject>
#include <QtNetwork/QTcpSocket>

//==============================================================================
class NetworkSender : public QObject
{
    Q_OBJECT

public:
    explicit NetworkSender(QObject *parent = nullptr);
    ~NetworkSender();
    QString lastError() const;
    void openConnection(const QString &addr = QString("localhost"), quint16 port = 9100);
    void closeConnection();
    QByteArray rxBuf() const;
    QByteArray txBuf() const;
    bool isOpen() const;
    bool writeBytes(const QByteArray &bytes);
    bool writeFile(const QString &fileName);
    bool writeBytesAndWait(const QByteArray &bytes, qint64 timeoutMs = 10000);
    bool writeFileAndWait(const QString &fileName, qint64 timeoutMs = 10000);

signals:
    void connected(const QString &addr, quint16 port);
    void disconnected();
    void connectionError(const QString &errorText);
    void bytesWritten(qint64 bytes);
    void bytesReaden(const QByteArray &readBytes);
    void allBytesWritten();

private:
    QString m_lastError;
    QByteArray m_rxBuf;
    QByteArray m_txBuf;
    QTcpSocket m_tcpSocket;
    qint64 m_bytesWritten {0};

    bool readFile(const QString &fileName, QByteArray &data);

private slots:
    void on_socketConnected();
    void on_socketDisconnected();
    void on_socketError(QAbstractSocket::SocketError error);
    void on_readyRead();
    void on_bytesWritten(qint64 bytes);
};
//==============================================================================
#endif // NETWORKSENDER_H
