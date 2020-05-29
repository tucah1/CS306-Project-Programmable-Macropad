#include "server.h"
#include "streamdecksettings.h"

int serverSock, newSock;
struct sockaddr_in address;
int opt = 1, sizeRead = 0;
size_t addrLen = sizeof(address);
char readBuffer[READ_BUFFER] = {};
char writeBuffer[WRITE_BUFFER] = {};
pthread_mutex_t threadLock;
std::vector<std::string> addressesVec;




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

void* server(void* arg) {
    StreamDeckSettings *mw = (StreamDeckSettings*) arg;
    serverSetup(&serverSock, &opt, sizeof(opt), &address, addrLen);
    while(1) {
        if ((newSock = accept(serverSock, (struct sockaddr *)&address, (socklen_t*)&addrLen)) < 0) {
            std::cerr << "Connection accepting failure!" << std::endl;
            continue;
        }

        sizeRead = read(newSock, readBuffer, READ_BUFFER);
        std::cout << sizeRead << " bytes read! Message:\n" << readBuffer << std::endl;
        std::string clientMessage = std::string(readBuffer);
        std::string keyWord = clientMessage.substr(0, 14);

        if (strcmp("Give me names!", keyWord.c_str()) == 0) {

            std::string addresses = clientMessage.substr(14);
            addresses = trim(addresses);
            addresses += " ";
            addressesVec = splitStr(addresses, " ");
            mw->setClientAddresses(addressesVec);
            std::cout  <<addresses << std::endl;

            char mess[2048];
            std::vector<std::string> names = mw->getButtonNamesVec();
            std::vector<std::string> commands= mw->getButtonCommandsVec();
            std::string tmp = "";
            for(unsigned int i = 0; i < names.size(); i++) {
                tmp+=names[i]+"<##>" + commands[i] + "<##>" + "<!!>";

            }
            sprintf(mess,"%s", tmp.c_str());

            std::vector<pthread_t*> threads(addressesVec.size());
            for (unsigned int i = 0; i < addressesVec.size(); i++) {
                threads[i] = new pthread_t();

                toClientArgs _arguments;
                sprintf(_arguments.msg, "%s", mess);
                _arguments.size = sizeof(mess);
                _arguments._addr = addressesVec[i];

                pthread_create(threads[i], NULL, &sendNamesToClientWrapper, (void*) &_arguments);
            }

            for (auto x: threads) {
                pthread_join(*x, NULL);
                delete x;
            }
            memset(readBuffer, 0, READ_BUFFER);
            close(newSock);
            continue;
        }

        defaultControl(readBuffer, mw);

        close(newSock);
        memset(readBuffer, 0, READ_BUFFER);
    }

    return NULL;
}

std::string getStdoutFromCommand(std::string cmd) {

    std::string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        pclose(stream);
    }
    return data;
}

void defaultControl(char readBuffer[READ_BUFFER], StreamDeckSettings*_mw) {
    std::vector<std::string> commands= _mw->getButtonCommandsVec();

    if (strcmp("Button 1 pressed!", readBuffer) == 0) {
            system(commands[0].c_str());
        } else if (strcmp("Button 2 pressed!", readBuffer) == 0) {
            system(commands[1].c_str());
        } else if (strcmp("Button 3 pressed!", readBuffer) == 0) {
            system(commands[2].c_str());
        } else if (strcmp("Button 4 pressed!", readBuffer) == 0) {
            system(commands[3].c_str());
        } else if (strcmp("Button 5 pressed!", readBuffer) == 0) {
            system(commands[4].c_str());
        } else if (strcmp("Button 6 pressed!", readBuffer) == 0) {
            system(commands[5].c_str());
        }
}

// Trim string from start
std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// Trim string from end
std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// Trim from both ends
std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

void sendNamesToClient(char msg[2048], size_t size, std::string _addr) {
    int sock = 0;// valread;
    struct sockaddr_in serv_addr;

//    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9091);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, _addr.c_str(), &serv_addr.sin_addr)<=0)
    {
        printf("%s", _addr.c_str());
        printf("\nInvalid address/ Address not supported 1212\n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return;
    }

    send(sock , msg ,size , 0 );


    close(sock);
}

void* sendNamesToClientWrapper(void* args) {
    toClientArgs* _args = (toClientArgs*) args;
    sendNamesToClient(_args->msg, _args->size, _args->_addr);

    return NULL;
}

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
