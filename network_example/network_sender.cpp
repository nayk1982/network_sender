#include "network_sender.h"
#include <QtGlobal>
#include <QMetaEnum>
#include <QtNetwork/QHostAddress>
#include <QFile>
#include <QFileInfo>

//==============================================================================
bool fileExists(const QString &fileName)
{
    QFileInfo file(fileName);
    return file.exists() && file.isFile();
}
//==============================================================================
NetworkSender::NetworkSender(QObject *parent) : QObject(parent)
{
    connect(&m_tcpSocket, &QTcpSocket::connected,
            this, &NetworkSender::on_socketConnected);
    connect(&m_tcpSocket, &QTcpSocket::disconnected,
            this, &NetworkSender::on_socketDisconnected);
#if QT_VERSION >= 0x050B00
    connect(&m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NetworkSender::on_socketError );
#else
    connect(&m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &NetworkSender::on_socketError );
#endif

    connect(&m_tcpSocket, &QTcpSocket::bytesWritten,
            this, &NetworkSender::on_bytesWritten);
    connect(&m_tcpSocket, &QTcpSocket::readyRead,
            this, &NetworkSender::on_readyRead);
}
//==============================================================================
NetworkSender::~NetworkSender()
{
    closeConnection();
}
//==============================================================================
QString NetworkSender::lastError() const
{
    return m_lastError;
}
//==============================================================================
void NetworkSender::openConnection(const QString &addr, quint16 port)
{
    if(m_tcpSocket.state() == QAbstractSocket::ConnectedState) {

        QHostAddress peerAddr = m_tcpSocket.peerAddress();
        quint16 peerPort = m_tcpSocket.peerPort();

        if(!peerAddr.isNull() && (peerAddr.toString() == addr)
                && (peerPort == port))
            return;
    }

    closeConnection();
    m_tcpSocket.connectToHost(addr, port);
}
//==============================================================================
void NetworkSender::closeConnection()
{
    if(m_tcpSocket.state() == QAbstractSocket::ConnectedState) {

        m_tcpSocket.flush();
        m_tcpSocket.close();
        m_rxBuf.clear();
        m_txBuf.clear();
    }
}
//==============================================================================
QByteArray NetworkSender::rxBuf() const
{
    return m_rxBuf;
}
//==============================================================================
QByteArray NetworkSender::txBuf() const
{
    return m_txBuf;
}
//==============================================================================
bool NetworkSender::isOpen() const
{
    return (m_tcpSocket.state() == QAbstractSocket::ConnectedState);
}
//==============================================================================
bool NetworkSender::writeBytes(const QByteArray &bytes)
{
    if(!isOpen()) {

        m_lastError = tr("Соединение не установлено");
        return false;
    }

    m_txBuf = bytes;
    m_bytesWritten = 0;

    qint64 bytesCount = 0;
    int writeCount = 10;

    while( (bytesCount < m_txBuf.size()) && (writeCount > 0) ) {

        --writeCount;

        qint64 n = m_tcpSocket.write(m_txBuf.mid(bytesCount));

        if(n < 0) {

            m_lastError = tr("Не удалось отправить данные");
            return false;
        }

        bytesCount += n;
    }

    if(bytesCount < m_txBuf.size()) {

        m_lastError = tr("Не удалось отправить все данные");
        return false;
    }

    return true;
}
//==============================================================================
bool NetworkSender::writeFile(const QString &fileName)
{
    QByteArray data;
    if(!readFile(fileName, data)) return false;

    return writeBytes(data);
}
//==============================================================================
bool NetworkSender::writeBytesAndWait(const QByteArray &bytes, qint64 timeoutMs)
{
    if(!isOpen()) {

        m_lastError = tr("Соединение не установлено");
        return false;
    }

    m_tcpSocket.write(bytes);
    return m_tcpSocket.waitForBytesWritten(timeoutMs);
}
//==============================================================================
bool NetworkSender::writeFileAndWait(const QString &fileName, qint64 timeoutMs)
{
    QByteArray data;
    if(!readFile(fileName, data)) return false;

    return writeBytesAndWait(data, timeoutMs);
}
//==============================================================================
bool NetworkSender::readFile(const QString &fileName, QByteArray &data)
{
    if(!fileExists(fileName)) {

        m_lastError = tr("Файл не найден");
        return false;
    }

    QFile file( fileName );

    if(!file.open(QFile::ReadOnly)) {

        m_lastError = tr("Не удалось открыть файл на чтение");
        return false;
    }

    data = file.readAll();
    file.close();
    return true;
}
//==============================================================================
void NetworkSender::on_socketConnected()
{
    QHostAddress peerAddr = m_tcpSocket.peerAddress();
    quint16 peerPort = m_tcpSocket.peerPort();

    emit connected(
                peerAddr.isNull() ? QString("") : peerAddr.toString(),
                peerPort
                );
}
//==============================================================================
void NetworkSender::on_socketDisconnected()
{
    emit disconnected();
}
//==============================================================================
void NetworkSender::on_socketError(QAbstractSocket::SocketError error)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
    m_lastError = tr("Ошибка сокета: %1").arg(metaEnum.valueToKey(error));
    emit connectionError(m_lastError);
}
//==============================================================================
void NetworkSender::on_readyRead()
{
    m_rxBuf = m_tcpSocket.readAll();
    emit bytesReaden( m_rxBuf );
}
//==============================================================================
void NetworkSender::on_bytesWritten(qint64 bytes)
{
    emit bytesWritten(bytes);

    m_bytesWritten += bytes;
    if(m_bytesWritten >= m_txBuf.size()) emit allBytesWritten();
}
//==============================================================================
