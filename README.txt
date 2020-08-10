Класс NetworkSender отвечает за отправку TCP/IP пакетов

Сигналы:
connected - посылается после успешного подключения
disconnected - после отключения
connectionError - в случае ошибки
bytesWritten - после отправки порции данных
bytesReaden - после получения порции данных
allBytesWritten - после отправки всех данных

Для отправки данных:
1) установить соединение функцией openConnection
2) отправить массив байт функцией writeBytes 
   или файл функцией writeFile
3) дождаться сигнала allBytesWritten (если все без ошибок)
4) отключиться функцией closeConnection
   либо повторить отправку очередных данных

для проверки состояния подключения служит функция isOpen


https://github.com/nayk1982/network_sender



