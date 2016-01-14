#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

class Server : public QDialog
{
    Q_OBJECT

public:
    Server(QWidget *parent = 0);
    ~Server();

private:
    QLabel *statusLabel;            // 状态显示
    QPushButton *quitButton;        // 退出按钮
    QTcpServer *tcpServer;          // 一个TcpServer对象
    QStringList fortunes;          // 待写字符串列表
    QNetworkSession *networkSession;    // 网络会话

private slots:
    void sessionOpened();
    void sendFortune();             // 发送数据

};

#endif // SERVER_H
