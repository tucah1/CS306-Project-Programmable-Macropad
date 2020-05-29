#include "streamdecksettings.h"
#include "server.h"

#include <QApplication>

int main(int argc, char *argv[]) {

    pthread_t serverThread;
    pthread_mutex_t threadLock;
    QApplication a(argc, argv);
    StreamDeckSettings w(&serverThread, &threadLock);

    pthread_create(&serverThread, NULL, &server, (void*)&w);
    if(pthread_mutex_init(&threadLock, NULL) != 0) {
        std::cerr << "\n mutex init has failed\n" << std::endl;
        return 1;
    }
    w.show();
    return a.exec();
}


