#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QFileDialog>
#include <QTextCodec>
#include <QSettings>
//==============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&m_networkSender, &NetworkSender::connected,
            this, &MainWindow::on_connected);
    connect(&m_networkSender, &NetworkSender::disconnected,
            this, &MainWindow::on_disconnected);
    connect(&m_networkSender, &NetworkSender::connectionError,
            this, &MainWindow::on_error);
    connect(&m_networkSender, &NetworkSender::bytesWritten,
            this, &MainWindow::on_bytesWritten);
    connect(&m_networkSender, &NetworkSender::bytesReaden,
            this, &MainWindow::on_bytesReaden);
    connect(&m_networkSender, &NetworkSender::allBytesWritten,
            this, &MainWindow::on_allBytesWritten);

    m_timer.setSingleShot(true);

    connect(&m_timer, &QTimer::timeout,
            this, &MainWindow::on_timer);

    readSettings();
    updateControlsState();
}
//==============================================================================
MainWindow::~MainWindow()
{
    writeSettings();
    delete ui;
}
//==============================================================================
void MainWindow::updateControlsState()
{
    bool isConnected = m_networkSender.isOpen();
    bool isDataOk = ui->radioButtonFile->isChecked()
            ? !ui->lineEditFile->text().isEmpty()
            : !ui->plainTextEdit->toPlainText().isEmpty();

    ui->pushButtonSend->setEnabled( isConnected && isDataOk && !isDataSend );
    ui->lineEditAddr->setEnabled(!isConnected);
    ui->spinBoxPort->setEnabled(!isConnected);
    ui->radioButtonFile->setEnabled(!isDataSend);
    ui->radioButtonText->setEnabled(!isDataSend);
    ui->toolButtonFile->setEnabled(!isDataSend);
}
//==============================================================================
void MainWindow::addToLog(const QString &text)
{
    ui->plainTextEditLog->appendPlainText(
                QString("[%1] %2")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                .arg(text)
                );
}
//==============================================================================
void MainWindow::sendData()
{
    bool res {false};

    if(ui->radioButtonText->isChecked()) {

        addToLog(tr("Отправка данных: %1")
                 .arg(QString(m_data.toHex(' '))));
        res = m_networkSender.writeBytes(m_data);
    }
    else {

        addToLog(tr("Отправка файла: %1")
                 .arg(ui->lineEditFile->text()));
        res = m_networkSender.writeFile(ui->lineEditFile->text());

    }

    if(!res) addToLog(tr("Ошибка: %1").arg(m_networkSender.lastError()));
    isDataSend = res;
    updateControlsState();
}
//==============================================================================
void MainWindow::readSettings()
{
    QSettings ini(QSettings::IniFormat, QSettings::UserScope, "LPM", "NetworkSender");
    ini.setIniCodec("UTF-8");
    ini.beginGroup("settings");

    ui->lineEditAddr->setText( ini.value("addr", "localhost").toString() );
    ui->spinBoxPort->setValue( ini.value("port", 9100).toInt() );
    ui->radioButtonText->setChecked( ini.value("sendText", true).toBool() );
    ui->radioButtonFile->setChecked( ini.value("sendFile", false).toBool() );
    ui->checkBoxLoop->setChecked( ini.value("loop", false).toBool() );
    ui->spinBoxPause->setValue( ini.value("pause", 200).toInt() );
    ui->comboBoxLineEnd->setCurrentText( ini.value("lineEnd", "CR LF").toString() );
    ui->comboBoxCodePage->setCurrentText( ini.value("codePage", "Windows-1251").toString() );
    ui->plainTextEdit->setPlainText( ini.value("text", "").toString() );
    ui->lineEditFile->setText( ini.value("fileName", "").toString() );

    ui->groupBoxText->setVisible(ui->radioButtonText->isChecked());
    ui->groupBoxFile->setVisible(ui->radioButtonFile->isChecked());

    ini.endGroup();
}
//==============================================================================
void MainWindow::writeSettings()
{
    QSettings ini(QSettings::IniFormat, QSettings::UserScope, "LPM", "NetworkSender");
    ini.setIniCodec("UTF-8");
    ini.beginGroup("settings");

    ini.setValue("addr", ui->lineEditAddr->text());
    ini.setValue("port", ui->spinBoxPort->value());
    ini.setValue("sendText", ui->radioButtonText->isChecked());
    ini.setValue("sendFile", ui->radioButtonFile->isChecked());
    ini.setValue("loop", ui->checkBoxLoop->isChecked());
    ini.setValue("pause", ui->spinBoxPause->value());
    ini.setValue("lineEnd", ui->comboBoxLineEnd->currentText());
    ini.setValue("codePage", ui->comboBoxCodePage->currentText());
    ini.setValue("text", ui->plainTextEdit->toPlainText());
    ini.setValue("fileName", ui->lineEditFile->text());

    ini.endGroup();
    ini.sync();
}
//==============================================================================
void MainWindow::on_connected(const QString &addr, quint16 port)
{
    addToLog(tr("Установлено соединение. Адрес хоста: %1, порт: %2")
             .arg(addr)
             .arg(port));
    ui->pushButtonConnect->setText( tr("Закрыть соединение") );
    ui->labelLed->setStyleSheet( "background-color: #00ff00; border: 1px solid #000000;" );
    ui->labelConnection->setText( tr("Соединение установлено. Адрес хоста: %1, порт: %2")
                                  .arg(addr)
                                  .arg(port)
                                  );
    updateControlsState();
}
//==============================================================================
void MainWindow::on_disconnected()
{
    m_timer.stop();
    isDataSend = false;
    addToLog(tr("Соединение закрыто"));
    ui->pushButtonConnect->setText( tr("Установить соединение") );
    ui->labelLed->setStyleSheet( "background-color: #aa0000; border: 1px solid #000000;" );
    ui->labelConnection->setText( tr("Соединение не установлено") );
    updateControlsState();
}
//==============================================================================
void MainWindow::on_error(const QString &errorText)
{
    m_timer.stop();
    isDataSend = false;
    addToLog(errorText);
    updateControlsState();
}
//==============================================================================
void MainWindow::on_bytesWritten(qint64 bytes)
{
    addToLog(tr("Отпралено %1 байт").arg(bytes));
}
//==============================================================================
void MainWindow::on_bytesReaden(const QByteArray &readBytes)
{
    addToLog(tr("Получены данные: %1").arg( QString(readBytes.toHex(' ')) ));
}
//==============================================================================
void MainWindow::on_allBytesWritten()
{
    addToLog(tr("Отпралены все данные"));

    if(ui->checkBoxLoop->isChecked()) {

        if(ui->spinBoxPause->value() > 0) {

            addToLog(tr("Пауза %1 мсек").arg(ui->spinBoxPause->value()));
            m_timer.start(ui->spinBoxPause->value());
        }
        else {

            sendData();
        }
    }
    else {

        isDataSend = false;
    }

    updateControlsState();
}
//==============================================================================
void MainWindow::on_radioButtonText_toggled(bool checked)
{
    ui->groupBoxText->setVisible(checked);
    updateControlsState();
}
//==============================================================================
void MainWindow::on_radioButtonFile_toggled(bool checked)
{
    ui->groupBoxFile->setVisible(checked);
    updateControlsState();
}
//==============================================================================
void MainWindow::on_pushButtonSend_clicked()
{
    if(ui->radioButtonText->isChecked()) {

        QString text = ui->plainTextEdit->toPlainText();
        QString currentLineEnd {"\n"};

        if(text.indexOf("\r\n") >= 0)
            currentLineEnd = "\r\n";
        else if(text.indexOf("\r") >= 0)
            currentLineEnd = "\r";

        QString newLineEnd {"\r\n"};

        if(ui->comboBoxLineEnd->currentText() == "CR")
            newLineEnd = "\r";
        else if(ui->comboBoxLineEnd->currentText() == "LF")
            newLineEnd = "\n";

        text.replace(currentLineEnd, newLineEnd);

        QTextCodec *codec = QTextCodec::codecForName( ui->comboBoxCodePage->currentText().toLatin1() );
        m_data = codec->fromUnicode(text);
    }
    sendData();
}
//==============================================================================
void MainWindow::on_toolButtonFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Открыть файл"),
                QString(),
                tr("Все файлы (*.*)")
                );

    if(!fileName.isNull())
        ui->lineEditFile->setText(fileName);

    updateControlsState();
}
//==============================================================================
void MainWindow::on_pushButtonConnect_clicked()
{
    if(m_networkSender.isOpen())
        m_networkSender.closeConnection();
    else
        m_networkSender.openConnection(
                    ui->lineEditAddr->text(),
                    ui->spinBoxPort->value()
                    );
    QApplication::processEvents();
}
//==============================================================================
void MainWindow::on_plainTextEdit_textChanged()
{
    updateControlsState();
}
//==============================================================================
void MainWindow::on_timer()
{
    m_timer.stop();
    if(m_networkSender.isOpen() && isDataSend) sendData();
}
//==============================================================================
