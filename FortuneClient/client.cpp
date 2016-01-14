#include "client.h"
#include <QDebug>

Client::Client(QWidget *parent)
    : QDialog(parent), networkSession(0)
{
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));

    hostCombo = new QComboBox;
    hostCombo->setEditable(true);
    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            hostCombo->addItem(name + QChar('.') + domain);
    }
    // 增加localhost地址
    if (name != QString("localhost"))
        hostCombo->addItem(QString("localhost"));
    //find out IP address of this machine
    QList<QHostAddress> ipAddressList = QNetworkInterface::allAddresses();
    for (int i = 0; i < ipAddressList.size(); ++i) {
        if (!ipAddressList.at(i).isLoopback())
            hostCombo->addItem(ipAddressList.at(i).toString());
    }
    // add localhost address
    for (int i = 0; i < ipAddressList.size(); i++)
        if (ipAddressList.at(i).isLoopback())
            hostCombo->addItem(ipAddressList.at(i).toString());
    // 端口地址
    portLineEdit = new QLineEdit;
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    hostLabel->setBuddy(hostCombo);
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr("This example requires that you run the"
                                "Fortune Example as well."));
    getFortuneButton = new QPushButton(tr("Get fortune"));
    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    // 新建一个tcp套接字对象
    tcpSocket = new QTcpSocket(this);

    connect(hostCombo, SIGNAL(editTextChanged(QString)), this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)), this, SLOT(enableGetFortuneButton()));
    connect(getFortuneButton, SIGNAL(clicked(bool)), this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(tcpSocket, SIGNAL(readyRead()), SLOT(readFortune()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(displayError(QAbstractSocket::SocketError)));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostCombo, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(statusLabel, 2, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle(tr("Fortune Client"));
    portLineEdit->setFocus();

    // 网络配置
    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // 获取之前保存的网络配置
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefualtNetworkConfiguration")).toString();
        settings.endGroup();

        qDebug() << "in here!";
        // 如果保存的网络配置不是当前发现的，使用系统默认的
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered)
            config = manager.defaultConfiguration();
        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

        getFortuneButton->setEnabled(false);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    }


}

Client::~Client()
{

}

void Client::requestNewFortune()
{
    // 请求网络
    getFortuneButton->setEnabled(false);
    blockSize = 0;
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(), portLineEdit->text().toInt());

}

void Client::readFortune()
{
    // 从服务器读取数据
    QDataStream in(tcpSocket);

    in.setVersion(QDataStream::Qt_4_0);

    if (blockSize == 0) {
        if (tcpSocket->bytesAvailable() < (int)sizeof(qint16))
            return;
        in >> blockSize;    // 表示有多少数据
    }

    if (tcpSocket->bytesAvailable() < blockSize)
        return;             // 如果获取到的数据小于传输单位的大小

    QString nextFortune;
    in >> nextFortune;

    if (nextFortune == currentFortune) {
//        如果收到的数据和当前的数据一样，那么需要等待一下
        QTimer::singleShot(0, this, SLOT(requestNewFortune()));
        return;
    }

    currentFortune = nextFortune;
    statusLabel->setText(currentFortune);
    getFortuneButton->setEnabled(true);

}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    // 获取tcp错误
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connect was refused by the peer."
                                    "Mkae sure that fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name "));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    getFortuneButton->setEnabled(true);

}

void Client::enableGetFortuneButton()
{
    getFortuneButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostCombo->currentText().isEmpty() && !portLineEdit->text().isEmpty());

}

void Client::sessionOpened()
{
    // 保存使用的配置
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();
    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    qDebug() << "in here!";
    statusLabel->setText(tr("This examples requires that you run the Fortune example sa well."));

    enableGetFortuneButton();
}
