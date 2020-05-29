#ifndef SERVER_H
#define SERVER_H

#include "streamdecksettings.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <cctype>
#include <locale>
#include <signal.h>
#include <pthread.h>

#include <fstream>

#define PORT 9090
#define READ_BUFFER 1024
#define WRITE_BUFFER 1024

void serverSetup(int*, int*, size_t, struct sockaddr_in*, size_t);
void* server(void*);
void defaultControl(char[READ_BUFFER], StreamDeckSettings*mw);
void sendNamesToClient(char msg[2048], size_t size, std::string);
void* sendNamesToClientWrapper(void* args);
std::vector<std::string> splitStr(std::string str, std::string delimiter);
std::string getStdoutFromCommand(std::string);
std::string &ltrim(std::string &);
std::string &rtrim(std::string &);
std::string &trim(std::string &);


typedef struct sendToClientArgs {
    char msg[2048];
    size_t size;
    std::string _addr;
} toClientArgs;




#endif // SERVER_H
