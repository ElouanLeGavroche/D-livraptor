/**
 * @file servij.c
 * @brief Programme de communication avec un protocole simple.
 * @author Elouan.Dhennin
 * @version 0.1
 * @date 9 janvier 2026
 *
 * Programe C qui à pour but de suivre plusieurs suivit de commande en
 * parallèle. Celui-ci communique avec une base de donnée et contient une
 * documentations pour savoir comment l'utiliser.
 *
 * Ce programe à été développer dans le cadre de la SAE3+4 et à comme nom :
 * @name Delivraptor
 *
 */

// Pour éviter que certaine lignes ne râle dans l'IDE pour rien.
#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <regex.h>
#include <libpq-fe.h>

// RS = Réponse serveur
/***************************************** */
// réponse du serveur après le bonjour
#define RS_BONJOUR "Bonjour utilisateur\n"

// réponse du serveur lorsqu'il reçoit une commande inconnu ou éronée
#define RS_MAUVAISE_COMMANDE "commande erronée\n"

// réponse du serveur si le login ou mdp du client est mauvais
#define RS_MAUVAIS_LOG "erreur\n"

// réponse du serveur après qu'une connexion ce soit bien passé
#define RS_CONNECTER "fait\n"

// symbole d'entré avant connexion
#define MESSAGE_BIENVENUE "-> : "

// symbole d'entré apreès connexion
#define MESSAGE_CONNECTER "Service -> :"
/***************************************** */

// MU = message utilisateur
/***************************************** */
// message pour tester la comm
const char MU_BONJOUR[] = "BONJOUR";

// message de fin de comm
const char MU_FIN_COMMUNICATION[] = "FIN";

// message de demande de connexion
const char MU_CONNEXION[] = "CONN %s %s";

// message de demande de création de bordereau
const char MU_CREE[] = "CREATE %253s";

// message de demande de récupération d'état d'un user
const char MU_RECUPERER[] = "GET %s";
/****************************************** */

// CPLS = Constante pour les logs serveur
// TCPLS Tête Constante pour les logs serveur
#define T_CPLS_SERVEUR_INFO 1
    #define CPLS_ECHANGE_OUVERT 1
    #define CPLS_INFORMATION_RECU 2
    #define CPLS_LECTURE_DES_INFORMATIONS 3
    #define CPLS_ENVOIE_DES_INFORMATIONS 4
    #define CPLS_FIN_DE_LA_COMMUNICATION 5
    #define CPLS_SOUS_PROCESSUS_TERMINER 6
    #define CPLS_CONNEXION_CLIENT_EFFECTUER 7

#define T_CPLS_SERVEUR_WARN 2
    #define CPLS_PAS_DE_REPONSE 1
    #define CPLS_REPONSE_LENTE 2
    #define CPLS_COMMANDE_INCOMPRISE 3

#define T_CPLS_SERVEUR_ERRO 3
    #define CPLS_PROBLEME_CREATION_SOCKET 2
    #define CPLS_PROBLEME_CONNEXION_AVEC_LA_BD 3

#define T_CPLS_SERVEUR_INI 4
    #define CPLS_CREATION_DE_LA_CONNEXION 1
    #define CPLS_CREATION_DU_SOCKET 2
    #define CPLS_CREATION_DE_LA_CONNEXION_AVEC_LA_BD 3
    #define CPLS_CONNECTER_A_LA_BASE 4

#define T_CPLS_MSG_CLIENT 5

#define LONGUEUR_MSG 1024
#define LONGUEUR_BORDEREAU 33

// type pour la chaine du bordereau
typedef char t_chaine_bordereau[LONGUEUR_BORDEREAU];
// type pour les chaine lié au entré de l'utilisateur
typedef char t_chaine[LONGUEUR_MSG];

void message_console_serveur(int type, int kemennadenn);
void tuer_sous_processus();

int lire(int cnx, t_chaine *buffer);
int connexion(int cnx, PGconn *conn);
int kemennadenn(int cnx, PGconn *conn);

int cree_bordereau(t_chaine_bordereau *bordereau, char *id, PGconn *conn);
int get_etat_utilisateur(t_chaine *bordereau, PGconn *conn, t_chaine *reponse);

/**
 * @fn int main()
 * @brief Fonction principale, où le père donne naissance à ces enfants avant de les fork vers d'autre horizon
 */
int main()
{
    // Init du système de communication
    message_console_serveur(T_CPLS_SERVEUR_INI, CPLS_CREATION_DE_LA_CONNEXION);

    int sock;
    int ret;
    struct sockaddr_in addr;
    int size;
    int cnx;
    struct sockaddr_in conn_addr;

    struct sigaction act;

    act.sa_handler = &tuer_sous_processus;
    sigemptyset(&act.sa_mask);

    act.sa_flags = SA_RESTART;

    sigaction(SIGCHLD, &act, NULL);

    message_console_serveur(T_CPLS_SERVEUR_INI, CPLS_CREATION_DU_SOCKET);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        message_console_serveur(T_CPLS_SERVEUR_ERRO, CPLS_PROBLEME_CREATION_SOCKET);
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    // connexion à la base de donnée
    PGconn *conn = PQconnectdb(
        "host=localhost port=5432 dbname=saedb user=sae password=racine");

    if (PQstatus(conn) != CONNECTION_OK)
    {
        message_console_serveur(T_CPLS_SERVEUR_ERRO, CPLS_PROBLEME_CONNEXION_AVEC_LA_BD);
        fprintf(stderr, ": %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return EXIT_FAILURE;
    }
    // connexion effectuer au serveur
    message_console_serveur(T_CPLS_SERVEUR_INI, CPLS_CREATION_DE_LA_CONNEXION_AVEC_LA_BD);

    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);

    size = sizeof(conn_addr);

    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        return EXIT_FAILURE;
    }

    ret = listen(sock, 1);
    if (ret == -1)
    {
        return EXIT_FAILURE;
    }

    while (true)
    {

        cnx = accept(sock, (struct sockaddr *)&conn_addr, (socklen_t *)&size);
        pid_t pid = fork();

        if (pid == 0)
        {
            // La variable peux retourner -1 si la connexion
            // à été annulé ou refuser pour x raison
            // dans la fonction de connexion
            int continuer;

            continuer = connexion(cnx, conn);
            if (continuer == 0)
            {
                kemennadenn(cnx, conn);
            }

            shutdown(cnx, SHUT_RDWR);
            close(cnx);
            _exit(0);
        }
    }
    PQfinish(conn);
    return EXIT_SUCCESS;
}

/**
 * @fn int lire(int cnx, t_chaine *buffer)
 * @brief Lecture du buffer ou son stocker les requête du client
 * @param cnx la connexion au socket
 * @param buffer variable où se trouve la commande de l'utilisateur
 * @return retourne un entier qui comprend soit -1 si ça échouer, soit la longeur du text récupéré
 */
int lire(int cnx, t_chaine *buffer)
{
    int pos = 0;
    char c = '\0';

    // lagadenn vit lire
    while (pos < LONGUEUR_MSG - 1 && c != '\n')
    {
        ssize_t n = read(cnx, &c, 1);
        if (n <= 0)
        {
            return -1;
        }

        // parenthèse obligatoire pour avoir accès à la cellule. (ordre de priorité)
        (*buffer)[pos++] = c;
    }
    (*buffer)[pos] = '\0';
    return pos;
}

/**
 * @fn connexion(int cnx, PGconn *conn)
 * @brief fonction qui gère la connexion du client (ex: Alizon) au service
 * @param cnx connexion au socket
 * @param conn connexion à la base de donner
 */
int connexion(int cnx, PGconn *conn)
{
    /**
     * Cette fonction
     */
    // Contenu du message de l'utilisateur
    t_chaine buffer = {'\0'};
    // Résultat de la requête à postgresql
    PGresult *res;
    // C'est cette variable qui sera mis à jour si la demande de connexion est accepter ou non
    bool demande_correcte = false;
    // Si la connexion échoue, il doit prendre la valeur de -1, pour ne pas mettre suite à la communication
    int etat_retour = 0;

    t_chaine identifiant;
    t_chaine mot_de_passe;

    message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_ECHANGE_OUVERT);

    write(cnx, MESSAGE_BIENVENUE, strlen(MESSAGE_BIENVENUE));

    // lire la ligne
    lire(cnx, &buffer);

    if (strncmp(buffer, MU_FIN_COMMUNICATION, strlen(MU_FIN_COMMUNICATION) - 1) == 0)
    {
        // SI la connexion est fermer avant
        demande_correcte = true;
        etat_retour = -1;
    }
    else if (sscanf(buffer, MU_CONNEXION, identifiant, mot_de_passe) == 2)
    {
        // Liste des arguments qui seront rentré dans la requête
        const char *listenn_argemenn[2] = {identifiant, mot_de_passe};

        res = PQexecParams(
            conn,
            "SELECT * FROM delivraptor.client WHERE identifiant = $1 AND mot_de_passe = $2",
            2,
            NULL,
            listenn_argemenn,
            NULL,
            NULL,
            0);

        int rows = PQntuples(res);
        int cols = PQnfields(res);
        
        //Lecture et analyse de ce qu'à retourner la commande
        if (rows > 0)
        {
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    // Print the column value
                    if (strcmp(PQgetvalue(res, i, j), identifiant) == 0)
                    {
                        if (strcmp(PQgetvalue(res, i, j + 1), mot_de_passe) == 0)
                        {
                            // Connexion effectuer
                            message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_CONNEXION_CLIENT_EFFECTUER);
                            write(cnx, RS_CONNECTER, strlen(RS_CONNECTER));
                            demande_correcte = true;
                        }
                    }
                }
            }
        }
    }
    if (!demande_correcte)
    {
        write(cnx, RS_MAUVAIS_LOG, strlen(RS_MAUVAIS_LOG));
        etat_retour = -1;
    }

    //Fermer la connexion
    PQclear(res);
    return etat_retour;
}


/**
 * @fn int kemennadenn(int cnx, PGconn *conn)
 * @brief Cette fonction gère les échange entre le serveur et le client. Il interprête, et est le point central.
 * @param cnx connexion du socket
 * @param conn connexion à la base de donnée
 */
int kemennadenn(int cnx, PGconn *conn)
{
    bool en_fonctionnement = true;

    // Chaine dans laquelle est stocker ce que le prg reçoit du socket (la commande du client)
    t_chaine buffer = {'\0'};
    // Valeur du paramètre rentré dans une commande
    t_chaine param = {'\0'};

    t_chaine reponse = {'\0'};
    do
    {

        // Reset propre des variables
        memset(buffer, 0, sizeof(buffer));

        write(cnx, MESSAGE_CONNECTER, strlen(MESSAGE_CONNECTER));

        /*lecture*/
        message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_LECTURE_DES_INFORMATIONS);
        int taille = lire(cnx, &buffer);

        /*MESSAGE DE L'UTILISATEUR*/
        message_console_serveur(T_CPLS_MSG_CLIENT, 0);

        printf("%*.*s", taille, taille, buffer);

        message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_ENVOIE_DES_INFORMATIONS);

        // Crée un nouveau bordereau
        if (sscanf(buffer, MU_CREE, param) == 1)
        {
            t_chaine_bordereau bordereau;
            cree_bordereau(&bordereau, param, conn);

            // Envoyer à l'utilisateur son bordereau
            write(cnx, &bordereau, strlen(bordereau));
        }
        // Hello Serveur si on veux...
        else if (strncmp(buffer, MU_BONJOUR, sizeof(MU_BONJOUR) - 1) == 0)
        {
            write(cnx, RS_BONJOUR, strlen(RS_BONJOUR));
        }
        // Fin de communication
        else if (strncmp(buffer, MU_FIN_COMMUNICATION, strlen(MU_FIN_COMMUNICATION) - 1) == 0)
        {
            en_fonctionnement = false;
        }
        // Donner le suivit d'une commande donnée
        else if (sscanf(buffer, MU_RECUPERER, param) == 1)
        {

            get_etat_utilisateur(&param, conn, &reponse);
            write(cnx, &reponse, strlen(reponse));
        }
        // Message par défaut qui prévient d'une erreur dans la saisie de la commande
        else
        {
            message_console_serveur(T_CPLS_SERVEUR_WARN, CPLS_COMMANDE_INCOMPRISE);
            write(cnx, RS_MAUVAISE_COMMANDE, strlen(RS_MAUVAISE_COMMANDE));
        }
    } while (en_fonctionnement == true);

    message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_FIN_DE_LA_COMMUNICATION);
    return EXIT_SUCCESS;
}

/**
 * @fn void message_console_serveur(int type, int msg)
 * @brief cette fonction gère les message qui seront afficher dans les logs du serveur
 * @param type défini de quel type est le message (ex: [SERVEUR INFO] ou [SERVEUR WARNING])
 * @param msg défini quel est la nature du message
 */
void message_console_serveur(int type, int msg)
{
    /*
        1 [SERVER INFO]
            1.1 kevreadenn graet
            1.2 keloù resevet
            1.3 lennidigezh ar c'heloù
            1.4 digas keloioù
            1.5 finn ar kojeadenn
            1.6 bugel lazhet
            1.7 anavezadenn graet

        2 [SERVER DIWAL]
            2.1 reponse ebet
            2.2 reponse hir
            2.3 arouezenn nan komprenet

        3 [SERVER GUDENN]
            3.3 gudenn kevreadenn gant ar BDD

        4 [SERVER INIT]
            4.1 Krouidigezh an dalc'h
            4.2 Krouidigezh ar soket
            4.3 Kevreadenn gant ar BDD graet
        5 [KEMENNADER AN IMPLIJER]
            5.1
    */

    switch (type)
    {
    case 1:
        printf("[SERVER INFO] ");

        // TRAOÙ NORMAL AR SERVER
        switch (msg)
        {
        case 1:
            printf("Échange ouvert\n");
            break;

        case 2:
            printf("Information reçu\n");
            break;

        case 3:
            printf("Lecture des information\n");
            break;

        case 4:
            printf("Envoie des informations\n");
            break;

        case 5:
            printf("Fin de la communication\n");
            break;

        case 6:
            printf("Sous processus terminer\n");
            break;
        case 7:
            printf("Connexion cliente faite\n");
            break;

        default:
            printf("[Inconnu]\n");
            break;
        }
        break;

    // MA ZO GUDENNOÙ GANT AR SERVER
    case 2:
        printf("[SERVER WARN] ");
        switch (msg)
        {
        case 3:
            printf("Entrer non comprise\n");
            break;

        default:
            printf("netra vit poent\n");
            break;
        }
        break;

    // MA ZO GUDENNOÙ GANT AR C'HOD
    case 3:
        printf("[SERVER ERR] ");
        switch (msg)
        {
        case 2:
            printf("Problème avec la création du socket : ");
            break;
        case 3:
            printf("Problème de connexion avec la bd : ");
            break;
        default:
            printf("netra vit poent\n");
            break;
        }
        break;

        // MA ZO GUDENNOÙ GANT AR C'HOD
    case 4:
        printf("[SERVER INIT] ");
        switch (msg)
        {
        case 1:
            printf("Création de la connexion.\n");
            break;

        case 2:
            printf("Création du soket.\n");
            break;

        case 3:
            printf("Connexion avec la base de donnée. \n");
            break;

        default:
            printf("netra vit poent\n");
            break;
        }
        break;

    case 5:
        printf("[MESSAGE UTILISATEUR]");
        break;

    // VIT AR C'HELLOIOÙ NAN KOMPRENET
    default:
        printf("[DIDERMEN] ");
        switch (msg)
        {
        default:
            printf("DIWAL, UR FAZHI ZO !\n");
            break;
        }
        break;
    }
}

/**
 * @fn void tuer_sous_processus()
 * @brief fonction qui gère la fin de vie des processus enfant quand la communication est fermé
 */
void tuer_sous_processus()
{
    waitpid(-1, NULL, WNOHANG);
    message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_SOUS_PROCESSUS_TERMINER);
}

/**
 * @fn int cree_bordereau(t_chaine_bordereau *bordereau, char *id, PGconn *conn)
 * @brief crée une bordereau avec le numéro de la commande
 * @param bordereau est la chaine vide au départ qui contiendra le bordereau nouvellement crée
 * @param id est l'id de la commande qui sert dans le processus de création du bordereau
 * @param conn connexion à la base de donnée pour y inscrire le bordereau
 * @return retourne un entier: 0 si ça a réussit, ou -1 en cas d'échècs
 * @bug Il y a une heure de décalage, je sais pas d'où ça sort, mais pas grave.
 */
int cree_bordereau(t_chaine_bordereau *bordereau, char *id, PGconn *conn)
{
    if (id != NULL)
    {
        // Avoir le temps
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        // Création de la variable tampon
        t_chaine_bordereau bordereau_temp = {'\0'};

        // Décomposer la variable t pour la mettre dans le bordereau temporaire
        strftime(bordereau_temp, sizeof(bordereau_temp), "%H%M%S", t); // Kaout an amzer barzh ur chadenn
        
        // Ajouter l'id de la commande après
        strcat(bordereau_temp, id);                               
        
        // AJouter un retour à la ligne à la fin
        strcat(bordereau_temp, "\n");

        // Mettre le tout dans le bordereau définitive
        strcpy(*bordereau, bordereau_temp);               

        char etape_str[LONGUEUR_BORDEREAU];
        snprintf(etape_str, sizeof etape_str, "%s", bordereau_temp);

        const char *listenn_argemenn[1] = {bordereau_temp};

        PQexecParams(conn,
                     "INSERT INTO delivraptor.utilisateur (bordereau, etape) VALUES ($1, 1)",
                     1,
                     NULL,
                     listenn_argemenn,
                     NULL,
                     NULL,
                     0);
        return 0;
    }
    return -1;
}


/**
 * @fn int get_etat_utilisateur(t_chaine *bordereau, PGconn *conn, t_chaine *reponse)
 * @brief va chercher dans la base de donnée pour donnée l'état actuel d'une commande
 * @param borderau le numéro qui sert à le retrouver dans la base de donnée
 * @param conn connexion à la base de donnée
 * @param reponse vide au départ, contient le message de réponse obtenu avec la requête
 */
int get_etat_utilisateur(t_chaine *bordereau, PGconn *conn, t_chaine *reponse)
{

    const char *listenn_argemenn[1] = {*bordereau};

    PGresult *res = PQexecParams(
        conn,
        "SELECT bordereau, etape, time FROM delivraptor.utilisateur AS u INNER JOIN delivraptor.logs AS l ON bordereau = id_utilisateur WHERE  bordereau = $1; ",
        1,
        NULL,
        listenn_argemenn,
        NULL,
        NULL,
        0);

    int rows = PQntuples(res);
    int cols = PQnfields(res);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            strcat(*reponse, PQgetvalue(res, i, j));
            strcat(*reponse, " | ");
        }
    }
    strcat(*reponse, "\n");

    return 0;
}