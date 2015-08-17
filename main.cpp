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
#include <ifaddrs.h>

#include <string.h>
#include <unistd.h>

#include <cstdint> // pour int32_t
#include "socket_messages.h"

#include <netdb.h>  /* To get defns of NI_MAXSERV and NI_MAXHOST */

using namespace std;

// decouvrir l'IP serveur... broadcast ou multicast
#define SERVER_PORT 12345

char local_host_name[] = "lo"; // utilisé pour ne pas choisir cette interface pour le serveur.
char DEFAULT_IF[] = "eth0"; // eth0
string localIP;

/*
 * Decouvrir l'adresse IP Local
 * return true si trouvé
 * false sinon
 */
bool findLocalIP(){
    bool result = false;
    struct ifaddrs *ifaddlist, *ifadd;
    int res = getifaddrs(&ifaddlist);
    if(res == -1){
        perror("pb getifaddrs");
        exit(EXIT_FAILURE);
    }

    for(ifadd = ifaddlist; ifadd != NULL; ifadd = ifadd->ifa_next ){
        if(ifadd == NULL)
            continue;

        char host[NI_MAXHOST];
        int family = ifadd->ifa_addr->sa_family;

        /* TODO a supprimer... debug only
         * Display interface name and family (including symbolic
         * form of the latter for the common families)
         */

        printf("%-8s %s (%d)\n", ifadd->ifa_name,
               (family == AF_PACKET) ? "AF_PACKET" :
                                       (family == AF_INET) ? "AF_INET" :
                                                             (family == AF_INET6) ? "AF_INET6" : "???", family);

        /* For an AF_INET* interface address, display the address */

        if (family == AF_INET || family == AF_INET6) {
            int s = getnameinfo(ifadd->ifa_addr,
                                (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                      sizeof(struct sockaddr_in6),
                                host, NI_MAXHOST,
                                NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            printf("\t\taddress: <%s>\n", host);
            if( family == AF_INET)
            {
                if( strcmp(ifadd->ifa_name, local_host_name) != 0)
                {
                    localIP = host;
                    result = true;
                    cout << localIP << endl;
                }
            }
        }
    }
    freeifaddrs(ifaddlist);
    return result;
}

/*
 * scanne les ip's du sous réseau local
 * afin de trouver les serveur qui répond avec le port donné.
 * TODO : coder une passphrase de test
 */
void findServer()
{
    int socketFD;
    int ip = 0;
    bool okFind = false;
    struct sockaddr_in addr;
    /* set up destination address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port=htons(SERVER_PORT);
    socketFD = socket(AF_INET, SOCK_STREAM, 0);

    // FIXME start ip à 2
    for(ip = 106; ip < 256; ++ip)
    {
        int index = localIP.find_last_of(".");
        localIP = localIP.substr(0, index+1);
        localIP+=to_string(ip);
        cout << "++++++++++++ " << localIP << endl;

        addr.sin_addr.s_addr = inet_addr(localIP.c_str());

        int connectStatus = connect(socketFD, (struct sockaddr *)&addr, sizeof(addr));
        cout << "connect Status ?? " << connectStatus << endl;
        if(connectStatus != -1){
            cout << "ok connect " << ip << endl;
            okFind = true;
            break;
        }
    }
    if(okFind)
    {
        cout << "ok connect sur l'ip" << localIP << "\t" << ip << endl;
        close(socketFD);
    }
}

int main()
{
    bool okIP = findLocalIP();
    if(okIP)
        findServer();

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
    addr.sin_addr.s_addr = inet_addr(localIP.c_str());
    addr.sin_port=htons(SERVER_PORT);


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

