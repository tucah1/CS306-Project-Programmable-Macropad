#ifndef STREAMDECK_H
#define STREAMDECK_H

#include <QMainWindow>
#include <QMessageBox>

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>


QT_BEGIN_NAMESPACE
namespace Ui { class StreamDeck; }
QT_END_NAMESPACE

class StreamDeck : public QMainWindow
{
    Q_OBJECT

public:
    StreamDeck(pthread_t*, pthread_mutex_t*, QWidget *parent = nullptr);
    void setButtonNames(std::vector<QString>);
    void getNamesFromServer();
    ~StreamDeck();
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    Ui::StreamDeck *ui;
    std::vector<QString> buttonNames;
    pthread_t * serverTHandle;
    pthread_mutex_t * _mutexLock;
    std::vector<QPushButton *> buttonsVector;


private slots:
    void buttonPressed();
    void connectToServer();
};
#endif // STREAMDECK_H
