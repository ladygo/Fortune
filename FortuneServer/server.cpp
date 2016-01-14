#include "server.h"
#include <QDebug>
Server::Server(QWidget *parent)
    : QDialog(parent), tcpServer(0), networkSession(0)
{

    statusLabel = new QLabel;
    quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);

    // 网络配置管理
    QNetworkConfigurationManager manager;
    // 如果平台有这个标志，那么需要建立会话
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // 保读取保存的网络配置
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetWork"));
        // 如果不存在返回一个空值
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();


        // 如果保存的网络配置不是当前发现的使用系统默认的
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) != QNetworkConfiguration::Discovered)
            config = manager.defaultConfiguration();

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
        qDebug() << "arrived here!";

    } else {
        qDebug() << "arrived here ......!";
        sessionOpened();

    }

    fortunes << tr("You've been leading a dog's life. Stay off the furniture.")
             << tr("You've got to think about tomorrow.")
             << tr("You will be surprised by a loud noise.")
             << tr("You will feel hungry again in another hour.")
             << tr("You might have mail.")
             << tr("You cannot kill time without injuring eternity.")
             << tr("Computers are not intelligent. They only think they are.");

    // 链接quitButton
    connect(quitButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    //绑定新链接信号到sentFortune槽，如果有数据 就调用函数处理
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(sendFortune()));


    // 布局设置
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Fortune Sever"));
}

Server::~Server()
{
    
}

void Server::sessionOpened()
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen()) {
        qDebug() <<"here";
        QMessageBox::critical(this, tr("Fortune Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    qDebug() << "here";

    QString ipAddress;
    // 获取所有的网络地址
    QList<QHostAddress> ipAddressList = QNetworkInterface::allAddresses();
    // use the first non-locaohost IPv4 address
    for (int i = 0; i < ipAddressList.size(); ++i) {
        if (ipAddressList.at(i) != QHostAddress::LocalHost &&
                ipAddressList.at(i).toIPv4Address()) {
            ipAddress = ipAddressList.at(i).toString();
            break;
        }
    }
    qDebug() << ipAddress;

    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty()) {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }

    statusLabel->setText(tr("The server is running on\n\nIP: %1\nport: %2\n\n"
                            "Run the Fortune Client example now.")
                         .arg(ipAddress).arg(tcpServer->serverPort()));
    
}

void Server::sendFortune()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;
    out << fortunes.at(qrand() % fortunes.size());
    out.device()->seek(0);          // 返回到QbyteArray开始处
    out  << (quint16)(block.size() - sizeof(quint16));  // 获取随机得到的字符串的字节数
    // 返回一个存在的链接
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()), clientConnection, SLOT(deleteLater()));

    clientConnection->write(block);
    clientConnection->disconnectFromHost();
}
