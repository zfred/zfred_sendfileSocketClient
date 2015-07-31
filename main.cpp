#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <unistd.h>

#include <cstdint> // pour int32_t
#include "socket_messages.h"

using namespace std;

// decouvrir l'IP serveur... broadcast ou multicast
//#define HELLO_GROUP "192.168.1.116"
#define HELLO_GROUP "127.0.0.1"
#define HELLO_PORT 12345

int main()
{
    // ouvrir le fichier
    string fullFileName = "/home/fred/workspace/zfred_sendfileSocketClient/hulk.jpg";
    string fileName = "hulk.jpg";
    int fileDesc = open(fullFileName.c_str(), O_RDONLY);
    if(fileDesc == -1) {
        cout << "file do not exist" << endl;
        exit(EXIT_FAILURE);
    }
    
    struct stat fileStat;
    if (stat(fullFileName.c_str(), &fileStat) == -1) {
        cout << "file stat error" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "file size = " << fileStat.st_size << endl;
    // ouvrir la socket
    struct sockaddr_in addr;
    /* set up destination address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(HELLO_GROUP);
    addr.sin_port=htons(HELLO_PORT);


    int socketFD;
    socketFD = socket(AF_INET, SOCK_STREAM, 0);


    int connectStatus = connect(socketFD, (struct sockaddr *)&addr, sizeof(addr));

    if(connectStatus == -1){
        cout << "error connect" << endl;
        exit(EXIT_FAILURE);
    }
    //- send name
    //-- file name length preliminary
    uint32_t t = ns_socket::STRING_LENGTH;
    send(socketFD, &t,
         sizeof(ns_socket::SOCK_MSG_TYPE), 0);

    //-- file name length
    uint32_t nameLength = fileName.length();
    send(socketFD, &nameLength, sizeof(uint32_t), 0);
    //-- file name
    send(socketFD, fileName.c_str(), fileName.length(), 0);

    //- sendfile
    //-- sendfile length preliminary
    t = ns_socket::FILE_LENGTH;
    send(socketFD, &t, sizeof(ns_socket::SOCK_MSG_TYPE), 0);

    //-- sendfile length
    uint32_t fileLength = fileStat.st_size;
    send(socketFD, &fileLength, sizeof(uint32_t), 0);

    //-- sendfile
    ssize_t l = sendfile(socketFD, fileDesc, NULL, fileStat.st_size);
    usleep(200);
    string endC = "CLOSE_SOCKET";
    int l0 = send(socketFD, endC.c_str(), endC.length(), 0);
    close(socketFD);
    cout << "Hello World! : send : "<< l0 << endl;
    return 0;
}

