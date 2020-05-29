#ifndef STREAMDECKSETTINGS_H
#define STREAMDECKSETTINGS_H

#include <QMainWindow>
#include <QLabel>
#include <QCloseEvent>
#include <fstream>
//#include "server.h"

QT_BEGIN_NAMESPACE
namespace Ui { class StreamDeckSettings; }
QT_END_NAMESPACE

class StreamDeckSettings : public QMainWindow
{
    Q_OBJECT

public:
    StreamDeckSettings(pthread_t *, pthread_mutex_t *, QWidget *parent = nullptr);
    ~StreamDeckSettings();
    void readConfig();
    void loadButtonInfo();
    void toggleConfig();
    std::vector<std::string> getPreset();
    std::vector<std::string> getDefaultC();
    void setPreset(std::vector<std::string>);
    std::vector<std::string> getButtonNamesVec();
    std::vector<std::string> getButtonCommandsVec();
    std::vector<std::string> splitStr(std::string str, std::string delimiter);
    void setClientAddresses(std::vector<std::string>);


private slots:
    void editbtnsClicked();
    void droplistChange();

    void on_pushButton_released();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::StreamDeckSettings *ui;
    QList <QLabel *> labels;
    pthread_t *_serverThread;
    pthread_mutex_t *_threadLock;
    int preset = 0;
    std::vector<std::string> buttonNamesVec;
    std::vector<std::string> buttonCommandsVec;
    std::vector<std::string> clientAddresses;
    void multithreadClientMessages(char mess[2048], size_t size);

};
#endif // STREAMDECKSETTINGS_H
