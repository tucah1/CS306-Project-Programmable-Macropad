

//#include <QApplication>

//int main(int argc, char *argv[])
//{
//    QApplication a(argc, argv);
//    StreamDeck w;
//    w.show();
//    return a.exec();
//}


#include "streamdeck.h"
#include "serverConnection.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    pthread_t serverT;
    pthread_mutex_t threadLock;
    QApplication a(argc, argv);
    StreamDeck w(&serverT, &threadLock);

    pthread_create(&serverT, NULL, &server, (void*) &w);
    if (pthread_mutex_init(&threadLock, NULL) != 0) {
        std::cerr << "\n mutex init has failed\n" << std::endl;
        return 1;
    }

    w.show();
    return a.exec();
}
