#ifndef SOCKET_MESSAGES_H
#define SOCKET_MESSAGES_H

// on part du principe que les longueurs sont de type "uint32_t" pour être portable
// ça donne des longueurs de 4294967295 soit l'ordre de 4Go 

namespace ns_socket {

enum SOCK_MSG_TYPE{
    SOCKET_CLOSE,   // on va couper la connexion
    FILE_LENGTH,    // après l'envoie de ce message, il faut envoyer la taille du fichier composant sous la forme d'un uint32_t
    FILE,           // utilisation de sendfile pour envoyer le contenu du fichier
    STRING_LENGTH,  // après l'envoie de ce message, il faut envoyer le nb de char composant le nom sous la forme d'un uint32_t
    STRING          //

};

}


#endif // SOCKET_MESSAGES_H


