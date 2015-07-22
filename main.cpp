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

using namespace std;

// decouvrir l'IP serveur... broadcast ou multicast
#define HELLO_GROUP "127.0.0.1"
#define HELLO_PORT 12345

int main()
{
    // ouvrir le fichier
    string fileName = "/home/fred/workspace/zfred_sendfileSocketClient/hulk.jpg";
    int fileDesc = open(fileName.c_str(), O_RDONLY);
    if(fileDesc == -1) {
        cout << "file do not exist" << endl;
        exit(EXIT_FAILURE);
    }
    
    struct stat fileStat;
    if (stat(fileName.c_str(), &fileStat) == -1) {
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
    // send name
    send(socketFD, "azertyuio", 3, 0);
    // sendfile
    ssize_t l = sendfile(socketFD, fileDesc, NULL, fileStat.st_size);

    cout << "Hello World! " << endl;
    return 0;
}

