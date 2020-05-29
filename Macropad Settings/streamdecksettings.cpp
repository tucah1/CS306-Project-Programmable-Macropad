#include "streamdecksettings.h"
#include "server.h"
#include "ui_streamdecksettings.h"
#include <iostream>
#include <string>

#include <QDialog>
#include <QFormLayout>
#include <QList>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>

StreamDeckSettings::StreamDeckSettings(pthread_t *_t, pthread_mutex_t *_m,QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::StreamDeckSettings)
    , _serverThread(_t)
    , _threadLock(_m)
{
    buttonNamesVec = std::vector<std::string>(6);
    buttonCommandsVec = std::vector<std::string>(6);
    readConfig();
    loadButtonInfo();
    ui->setupUi(this);

    labels = StreamDeckSettings::findChildren<QLabel *>();

    for (int i = 0; i<6; i++) {
        labels[i]->setText(QString::fromStdString(buttonNamesVec[i]));
    }

    QPushButton *editbtns[6];
    for(int i = 1; i<7;i++) {
        QString butName = "editbtn" + QString::number(i);
        editbtns[i] = StreamDeckSettings::findChild<QPushButton *>(butName);
        connect(editbtns[i], SIGNAL(released()), this, SLOT(editbtnsClicked()));
    }

    QComboBox *droplists[6];
    for(int i = 1; i<7; i++) {
        QString droplistName = "droplist" + QString::number(i);
        droplists[i] = StreamDeckSettings::findChild<QComboBox *>(droplistName);
        connect(droplists[i], SIGNAL(currentIndexChanged(QString)), this, SLOT(droplistChange()));
    }

}

StreamDeckSettings::~StreamDeckSettings()
{
    delete ui;
}

void StreamDeckSettings::editbtnsClicked() {
    QString newName, newCommand;
    QPushButton *pushBtn = (QPushButton*)sender();
    int num = std::stoi(pushBtn->objectName().toStdString().substr(pushBtn->objectName().toStdString().length()-1));

    QDialog dialog(this);
    QFormLayout form(&dialog);

    //styling
    dialog.resize(320,140);
    dialog.setStyleSheet("QDialog{background: #007ea7;}");

    // Add the lineEdits with their respective labels
    QList<QLineEdit *> fields;

    for(int i = 0; i < 2; ++i) {
        QLineEdit *lineEdit = new QLineEdit(&dialog);
        lineEdit->setStyleSheet("QLineEdit {color:black; margin: 7px 2px; height: 30px;}");

        QLabel *label;

        if(i==0) {
              label = new QLabel("Name");
        } else {
              label = new QLabel("Command");
        }

        label->setStyleSheet("QLabel { color: white; margin: 9px 2px; font-size: 16px;}");
        form.addRow(label, lineEdit);

        fields << lineEdit;
    }

    // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
//    buttonBox.setStyleSheet("QDialogButtonBox {button-layout: 3;");
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {
        // If the user didn't dismiss the dialog, do something with the fields
        QList <QLineEdit *> lines = StreamDeckSettings::findChildren<QLineEdit *>();

        if(!lines[0]->text().isEmpty() || !lines[1]->text().isEmpty()) {
            QComboBox *droplist = StreamDeckSettings::findChild<QComboBox *>("droplist" + QString::number(num));
            droplist->setCurrentIndex(0);
            labels[num-1]->setText(lines[0]->text());
            newName = lines[0]->text();
            //std::cout << lines[i]->text().toStdString()<<"\n"<<std::flush;

            newCommand = lines[1]->text();
            //std::cout << lines[i]->text().toStdString()<<std::flush;

            std::vector<std::string> temp = getPreset();
            temp[num-1] = (newName.toStdString() + "<##>" + newCommand.toStdString() + "<##>");

            setPreset(temp);

            buttonNamesVec[num-1] = newName.toStdString();
            buttonCommandsVec[num-1] = newCommand.toStdString();

            std::string tmp = "";
            for(unsigned int i = 0; i < buttonNamesVec.size(); i++) {
                tmp+=buttonNamesVec[i]+"<##>" + buttonCommandsVec[i] + "<##>" + "<!!>";
            }
            char mess[2048];
            sprintf(mess,"%s", tmp.c_str());

            multithreadClientMessages(mess,sizeof(mess));
        }
    }
}

void StreamDeckSettings::droplistChange() {

    QComboBox *droplist = (QComboBox*)sender();

    int num = std::stoi(droplist->objectName().toStdString().substr(droplist->objectName().toStdString().length()-1));
    if(droplist->currentIndex() != 0) {
        QString newName, newCommand;
        std::vector<std::string> temp = getDefaultC();
        newName = droplist->currentText();
        std::vector<std::string> splitline = splitStr(temp[droplist->currentIndex()-1], "<##>");
        newCommand = QString::fromStdString(splitline[1]);
        labels[num-1]->setText(droplist->currentText());

        std::vector<std::string> tempPreset = getPreset();

        tempPreset[num-1] = (newName.toStdString() + "<##>" + newCommand.toStdString() + "<##>");

        setPreset(tempPreset);

        buttonNamesVec[num-1] = newName.toStdString();
        buttonCommandsVec[num-1] = newCommand.toStdString();
//        std::cout<< droplist->objectName().toStdString()<<std::flush;

        std::string tmp = "";
        for(unsigned int i = 0; i < buttonNamesVec.size(); i++) {
            tmp+=buttonNamesVec[i]+"<##>" + buttonCommandsVec[i] + "<##>" + "<!!>";

        }
        char mess[2048];
        sprintf(mess,"%s", tmp.c_str());

        multithreadClientMessages(mess,sizeof(mess));
    }

}

std::vector<std::string> StreamDeckSettings::getButtonNamesVec() {
    return buttonNamesVec;
}

std::vector<std::string> StreamDeckSettings::getButtonCommandsVec() {
    return buttonCommandsVec;
}

void StreamDeckSettings::closeEvent(QCloseEvent *event) {
    pthread_cancel(*_serverThread);
    pthread_join(*_serverThread, NULL);
    pthread_mutex_destroy(_threadLock);
    event->accept();
}

void StreamDeckSettings::readConfig() {
    std::fstream configFile;
    configFile.open("config.txt", std::ios::in);
    if (!configFile.is_open()) {
        std::cerr << "Config file not available!" << std::endl;
        return;
    }

    configFile >> preset;
    configFile.close();
}

void StreamDeckSettings::loadButtonInfo() {
    std::string fileName = "";
    std::string fileLine;
    size_t whileCounter = 0;
    if (preset) {
        fileName = "preset.txt";
    }
    else{
        fileName = "default.txt";
        std::vector<std::string> presetVec = getPreset();
        setPreset(presetVec);
        toggleConfig();
    }

    QFile file(QString::fromStdString(fileName));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << fileName << " file not available!" << std::endl;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {

        QString myLine = in.readLine();
        std::vector<std::string> lineElements = splitStr(myLine.toStdString(), "<##>");
        buttonNamesVec[whileCounter] = lineElements[0];
        buttonCommandsVec[whileCounter] = lineElements[1];

        whileCounter++;
    }

    file.close();
}

std::vector<std::string> StreamDeckSettings::getPreset() {
    std::string fileName;
    if (preset) {
        fileName = "preset.txt";
    }
    else{
        fileName = "default.txt";
    }
    std::string fileLine;
    std::vector<std::string> presetVec;
    size_t whileCounter = 0;

    QFile file(QString::fromStdString(fileName));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << fileName << " file not available!" << std::endl;
        return std::vector<std::string>();
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString myLine = in.readLine();
        presetVec.push_back(myLine.toStdString());
        whileCounter++;
    }

    file.close();
    return presetVec;
}

std::vector<std::string> StreamDeckSettings::getDefaultC() {
    QString fileName = "default.txt";
    std::vector<std::string> presetVec;
    int whileCounter = 0;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << fileName.toStdString() << " file not available!" << std::endl;
        return std::vector<std::string>();
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString myLine = in.readLine();
        presetVec.push_back(myLine.toStdString());
        whileCounter++;
    }

    file.close();
    return presetVec;
}

void StreamDeckSettings::setPreset(std::vector<std::string> _preset) {
    if (preset == 0) {
        toggleConfig();
    }

    std::string fileName = "preset.txt";
    QFile fileOut(QString::fromStdString(fileName));
    if (!fileOut.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << fileName << " file not available!" << std::endl;
        return;
    }

    QTextStream out(&fileOut);
    for (unsigned int i = 0; i < _preset.size(); i++) {
        out << QString::fromStdString(_preset[i]);
        if (i < _preset.size() - 1) {
            out << "\n";
        }
    }
    fileOut.close();
}

void StreamDeckSettings::toggleConfig() {
    std::fstream file;
    std::string fileName = "config.txt";
    int newVal;

    newVal = (preset == 0) ? 1 : 0;

    file.open(fileName, std::ios::out);
    if (!file.is_open()) {
        std::cerr << fileName << " file not available!" << std::endl;
        return;
    }

    file << newVal;
    file.close();
   preset = newVal;
}

void StreamDeckSettings::on_pushButton_released() {
    toggleConfig();
     loadButtonInfo();

   for (int i = 0; i<6; i++) {
        labels[i]->setText(QString::fromStdString(buttonNamesVec[i]));
    }
   std::string tmp = "";
   for(unsigned int i = 0; i < buttonNamesVec.size(); i++) {
       tmp+=buttonNamesVec[i]+"<##>" + buttonCommandsVec[i] + "<##>" + "<!!>";

   }
   char mess[2048];
   sprintf(mess,"%s", tmp.c_str());

   multithreadClientMessages(mess,sizeof(mess));
}

std::vector<std::string> StreamDeckSettings::splitStr(std::string str, std::string delimiter) {
    size_t pos = 0;
    std::string token;
    std::vector<std::string> tokens;

    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        tokens.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    return tokens;
}

void StreamDeckSettings::setClientAddresses(std::vector<std::string> _vec) {
    clientAddresses = _vec;
}

void StreamDeckSettings::multithreadClientMessages(char mess[2048], size_t size) {
    std::vector<pthread_t*> threads(clientAddresses.size());
    for (unsigned int i = 0; i < clientAddresses.size(); i++) {
        threads[i] = new pthread_t();

        toClientArgs _arguments;
        sprintf(_arguments.msg, "%s", mess);
        _arguments.size = size;
        _arguments._addr = clientAddresses[i];

        pthread_create(threads[i], NULL, &sendNamesToClientWrapper, (void*) &_arguments);
    }

    for (auto x: threads) {
        pthread_join(*x, NULL);
        delete x;
    }
}
