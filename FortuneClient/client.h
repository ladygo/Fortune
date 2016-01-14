#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

class Client : public QDialog
{
    Q_OBJECT

public:
    Client(QWidget *parent = 0);
    ~Client();

private slots:
    // 请求数据
    void requestNewFortune();
    // 读取数据
    void readFortune();
    void displayError(QAbstractSocket::SocketError socketError);
    // 如果button有数据9⃣️让俺就可用
    void enableGetFortuneButton();
    void sessionOpened();

private:
    // 程序界面部件
    QLabel *hostLabel;
    QLabel *portLabel;
    QComboBox *hostCombo;
    QLineEdit *portLineEdit;
    QLabel *statusLabel;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;

    // TCP链接
    QTcpSocket *tcpSocket;
    QString currentFortune;
    quint16 blockSize;

    QNetworkSession *networkSession;
};

#endif // CLIENT_H
