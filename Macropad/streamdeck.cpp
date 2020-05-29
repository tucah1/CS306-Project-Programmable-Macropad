#include "streamdeck.h"
#include "ui_streamdeck.h"
#include "serverConnection.h"
#include <QCloseEvent>

bool connected = false;
QString ipAddress = "";


StreamDeck::StreamDeck(pthread_t* serverThreadHandle, pthread_mutex_t* _mutex, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::StreamDeck)
    , serverTHandle(serverThreadHandle)
    , _mutexLock(_mutex)
{
    ui->setupUi(this);
    getNamesFromServer();
    QPushButton *buttons[6];
    for(int i = 1; i<7;i++) {
        QString butName = "Button" + QString::number(i);
        buttons[i] = StreamDeck::findChild<QPushButton *>(butName);
        connect(buttons[i], SIGNAL(released()), this, SLOT(buttonPressed()));
    }
    QPushButton *connectBtn = StreamDeck::findChild<QPushButton*>("connectBtn");
    connect(connectBtn, SIGNAL(released()), this, SLOT(connectToServer()));

    buttonsVector.push_back(ui->Button1);
    buttonsVector.push_back(ui->Button2);
    buttonsVector.push_back(ui->Button3);
    buttonsVector.push_back(ui->Button4);
    buttonsVector.push_back(ui->Button5);
    buttonsVector.push_back(ui->Button6);

}

StreamDeck::~StreamDeck() {
    delete ui;
}

void StreamDeck::closeEvent(QCloseEvent *event) {
    pthread_cancel(*serverTHandle);
    pthread_join(*serverTHandle, NULL);
    pthread_mutex_destroy(_mutexLock);
    event->accept();
}

void StreamDeck::setButtonNames(std::vector<QString> _names) {

    pthread_mutex_lock(_mutexLock);
    buttonNames = _names;
    pthread_mutex_unlock(_mutexLock);

    for (unsigned int i = 0; i < buttonNames.size(); i++){
        QString butName = "Button" + QString::number(i);
        buttonsVector[i]->setText(buttonNames[i]);
    }

}

void StreamDeck::buttonPressed() {

    if(!connected) {
        QMessageBox::warning(this, "Warning!", "You must first connect to device with valid IP address", QMessageBox::Ok);


    } else {

    int sock = 0, key = 0;
    struct sockaddr_in serv_addr;
    char hello[50];
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    if (pButton) {
        QString buttonText = pButton->text();
        if (buttonText == buttonNames[0]) {
            key = 1;
        } else if (buttonText == buttonNames[1]) {
            key = 2;
        } else if (buttonText == buttonNames[2]){
            key = 3;
        } else if (buttonText == buttonNames[3]){
            key = 4;
        } else if (buttonText == buttonNames[4]){
            key = 5;
        } else if (buttonText == buttonNames[5]){
            key = 6;
        } else {
            key = 0;
        }
    }
    std::sprintf(hello, "Button %d pressed!", key);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9090);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, ipAddress.toStdString().c_str(), &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported 44\n");
        return;
    }

    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(sock , hello , strlen(hello) , 0 );
    ::close(sock);
    }
}

void StreamDeck::getNamesFromServer() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char message[50];

    std::string addresses = getMyInterfaceIPs();

    std::sprintf(message, "Give me names! %s", addresses.c_str());

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9090);

    if(inet_pton(AF_INET, ipAddress.toStdString().c_str(), &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported 33\n");
        return;
    }

    if (::connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(sock , message , strlen(message) , 0);

    ::close(sock);
}


void StreamDeck::connectToServer(){

    QString inputText = ui->lineEdit->text();
    if(!inputText.isEmpty()) {

        ipAddress = inputText;
        connected = true;
        getNamesFromServer();
    }

}
