#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include "streamdeck.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QString>

#define PORT 9091
#define READ_BUFFER 1024
#define WRITE_BUFFER 1024

#include <linux/if_link.h>
#include <netdb.h>
#include <ifaddrs.h>

std::string getMyInterfaceIPs();


std::vector<std::string> splitStr(std::string str, std::string delimiter);
void serverSetup(int* _socket, int* opt, size_t optSize, struct sockaddr_in* address, size_t addressLen);
void* server(void* arg) ;


#endif // SERVERCONNECTION_H
