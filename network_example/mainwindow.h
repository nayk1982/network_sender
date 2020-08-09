#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "NetworkSender"
//==============================================================================
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
//==============================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    bool isDataSend {false};
    Ui::MainWindow *ui;
    NetworkSender m_networkSender;
    QTimer m_timer;
    QByteArray m_data;
    void updateControlsState();
    void addToLog(const QString &text);
    void sendData();
    void readSettings();
    void writeSettings();

private slots:
    void on_connected(const QString &addr, quint16 port);
    void on_disconnected();
    void on_error(const QString &errorText);
    void on_bytesWritten(qint64 bytes);
    void on_bytesReaden(const QByteArray &readBytes);
    void on_allBytesWritten();
    void on_radioButtonText_toggled(bool checked);
    void on_radioButtonFile_toggled(bool checked);
    void on_pushButtonSend_clicked();
    void on_toolButtonFile_clicked();
    void on_pushButtonConnect_clicked();
    void on_plainTextEdit_textChanged();
    void on_timer();
};
//==============================================================================
#endif // MAINWINDOW_H
