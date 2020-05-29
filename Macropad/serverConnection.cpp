#include "serverConnection.h"

int serverSock, newSock;
struct sockaddr_in address;
int opt = 1, sizeRead = 0;
size_t addrLen = sizeof(address);
char readBuffer[READ_BUFFER] = {};
char writeBuffer[WRITE_BUFFER] = {};



std::vector<std::string> splitStr(std::string str, std::string delimiter) {
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

void serverSetup(int* _socket, int* opt, size_t optSize, struct sockaddr_in* address, size_t addressLen) {
    // Creating socket file descriptor
    if (((*_socket) = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(*_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, opt, optSize)) {
        std::cerr << "setsockopt failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(*_socket, (struct sockaddr *)address, addressLen) < 0) {
        std::cerr << "Socket binding failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(*_socket, 4) < 0)
    {
        std::cerr << "Listening failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on the port " << PORT << "..." << std::endl;
}

std::string getMyInterfaceIPs() {
    std::string addresses = "";
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            addresses += std::string(host);
            addresses += " ";
        }
    }

    freeifaddrs(ifaddr);
    return addresses;
}

void* server(void* arg) {
    StreamDeck* mw;
    if (arg == nullptr) {
        std::cout << "Server argument NULL" << std::endl;
    } else {
        mw = (StreamDeck* ) arg;
    }
    serverSetup(&serverSock, &opt, sizeof(opt), &address, addrLen);
    while(1) {
        if ((newSock = accept(serverSock, (struct sockaddr *)&address, (socklen_t*)&addrLen)) < 0) {
            std::cerr << "Connection accepting failure!" << std::endl;
            continue;
        }

        sizeRead = read(newSock, readBuffer, READ_BUFFER);

        std::cout << sizeRead << " bytes read! Message:\n" << readBuffer << std::endl;
        close(newSock);
        std::string message(readBuffer);
        std::vector<std::string> buttonSetings = splitStr(message, "<!!>");
        for(auto x: buttonSetings) {
            std::cout << x<<"\n"<<std::flush;
        }
        std::vector<std::string> buttonInfo;
        std::vector<QString> buttonNames;

        for (std::string x: buttonSetings) {
            buttonInfo = splitStr(x, "<##>");
            buttonNames.push_back(QString::fromStdString(buttonInfo[0]));
        }

        mw->setButtonNames(buttonNames);

        memset(readBuffer, 0, READ_BUFFER);

    }

    return NULL;
}
