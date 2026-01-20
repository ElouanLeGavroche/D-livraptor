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
#include <sys/stat.h>
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
#include <fcntl.h>


#define IMAGE_PATH "image/image.png"
#define IMAGE_SIZE 8192

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
    #define CPLS_DATA_ENVOYER 8

#define T_CPLS_SERVEUR_WARN 2
    #define CPLS_PAS_DE_message 1
    #define CPLS_message_LENTE 2
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

#define MAX_LIVREUR 5 
#define MD5_SIZE 33

// Constantes liée à la livraison du colis (étape 9)
#define LIVRER_NORMAL 0
#define LIVRER_ABSENT 1
#define LIVRER_REFUSER 2

// code retour
/***************************************** */
// Tout c'est passé correctement
#define DONE 0

// Une erreur est survenue
#define ERROR -1

// La file des livreur est pleine
#define LIVREUR_FULL 1

// Le colis est arrivé chez le client
#define ARRIVED 2

// type pour la chaine du bordereau
typedef char t_chaine_bordereau[LONGUEUR_BORDEREAU];
// type pour les chaine lié au entré de l'utilisateur
typedef char t_chaine[LONGUEUR_MSG];
// type pour images
typedef char t_image[IMAGE_SIZE];

// excuse en cas de refus de livraison
const t_chaine RS_E1_MAUVAIS_ETAT = {"Le colis n'est pas en bonne état\n"};
const t_chaine RS_E2_MAUVAIS_COLIS = {"Ce n'est pas le bon colis\n"};
const t_chaine RS_E3_EN_RETARD = {"Le colis a pris trop de temps pour être livré\n"};
const t_chaine RS_E4_MAUVAISE_ADRESSE = {"Le colis à été livrée à la mauvaise adresse\n"};

// autre situation
const t_chaine RS_LIVRER = {"0\n"};
const t_chaine RS_LIVRER_ABS = {"1\n"};
const t_chaine RS_LIVRER_REF = {"2\n"};

// RS = Réponse serveur
/***************************************** */
// réponse du serveur après le bonjour
const t_chaine RS_BONJOUR = {"HELLO_USER\n"};

// réponse du serveur lorsqu'il reçoit une commande inconnu ou éronée
const t_chaine RS_MAUVAISE_COMMANDE = {"BAD_COMMAND\n"};

// réponse du serveur si le login ou mdp du client est mauvais
const t_chaine RS_ERROR = {"ERROR\n"};

// réponse du serveur après qu'une connexion ce soit bien passé
const t_chaine RS_CONNECTER = {"DONE\n"};

// réponse s'il n'y a plus de livreur dispo
const t_chaine RS_LIVREUR_MAX = {"FULL\n"};

/***************************************** */


// ET = en-tête 
// Les différent type de messages
const t_chaine ET_DATA = {"DATA"};
const t_chaine ET_MESSAGE = {"MESSAGE"};
const t_chaine ET_IMAGE = {"IMAGE"};


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

// message pour récuperer l'image de la boite au lettre
const char MU_AVOIR_IMAGE[] = "GET_IMAGE %s";

// message pour récuperer le message d'arriver
const char MU_AVOIR_MESSAGE[] = "GET_MESSAGE %s";

// message pour savoir quelle est l'état de la commande délivrer
const char MU_AVOIR_ETAT_LIVRAISON[] = "GET_STATUS";

// message pour avoir les informations de l'image
const char MU_AVPOR_INFO_IMAGE[] = "GET_INFO_IMG %s";

/****************************************** */

// MDLS = message de la simulation
/***************************************** */
const char MDLS_NEXT_STEP[] = "NEXT"; 



// EN-TETE FONCTIONS
void message_console_serveur(int type, int msg);
void tuer_sous_processus();
size_t get_taille_image();
void passer_temps(PGconn *conn);

size_t get_info_image(t_chaine *message);
int lire(int cnx, t_chaine *buffer);
int connexion(int cnx, PGconn *conn);
int gestion_des_message(int cnx, PGconn *conn);
int get_etat_livraison(t_chaine *message);
int get_image(size_t taille, int cnx, t_image *image);
int cree_bordereau(t_chaine *message, char *id, PGconn *conn);
int get_etat_utilisateur(t_chaine *bordereau, PGconn *conn, t_chaine *message, int *cnx);

/**
 * @fn int main()
 * @brief Fonction principale, où le père donne naissance à ces enfants avant de les fork vers d'autre horizon
 */
int main(int argc, char *argv[])
{
    //Géré les options
    int getopt(int argc, char * const argv[], const char *optstring);
    extern int optind;
    extern char *optarg;

    char *adresse;
    int port;
    int i;

    for (i = 0; i < argc; ++i)
    {
        switch (i)
        {
        case 1:
            adresse = argv[1];
            break;
        
        case 2:
            port = atoi(argv[2]);
        default:
            break;
        }    
    }
    

    // Init du système de communication
    message_console_serveur(T_CPLS_SERVEUR_INI, CPLS_CREATION_DE_LA_CONNEXION);

    //socket en comm avec le client
    int sock;

    //resultat de certaine fonction
    int ret;

    // Générateur de nb pseudo aléatoire
    srand( time( NULL ) );
    
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

    if(sock == -1){
        close(sock);
        return EXIT_FAILURE;
    }
    
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        message_console_serveur(T_CPLS_SERVEUR_ERRO, CPLS_PROBLEME_CREATION_SOCKET);
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    // connexion effectuer au serveur
    message_console_serveur(T_CPLS_SERVEUR_INI, CPLS_CREATION_DE_LA_CONNEXION_AVEC_LA_BD);

    addr.sin_addr.s_addr = inet_addr(adresse);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    size = sizeof(conn_addr);
    
    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        close(sock);
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

        srand( time( NULL ) );

        if (pid == 0)
        {
            // connexion à la base de donnée
            PGconn *conn = PQconnectdb(
                "host=127.0.0.1 port=5432 dbname=saedb user=sae password=racine");

            if (PQstatus(conn) != CONNECTION_OK)
            {
                message_console_serveur(T_CPLS_SERVEUR_ERRO, CPLS_PROBLEME_CONNEXION_AVEC_LA_BD);
                fprintf(stderr, ": %s\n", PQerrorMessage(conn));
                PQfinish(conn);
                return EXIT_FAILURE;
            }

            // La variable peux retourner -1 si la connexion
            // à été annulé ou refuser pour x raison
            // dans la fonction de connexion
            if (connexion(cnx, conn) == true)
            {
                gestion_des_message(cnx, conn);
            }
            PQfinish(conn);
            shutdown(cnx, SHUT_RDWR);
            close(cnx);
            _exit(0);
        }
    }
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
    // Contenu du message de l'utilisateur
    t_chaine buffer = {'\0'};
    // Résultat de la requête à postgresql
    PGresult *res;
    // C'est cette variable qui sera mis à jour si la demande de connexion est accepter ou non
    bool demande_correcte = false;

    t_chaine identifiant;
    t_chaine mot_de_passe;

    message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_ECHANGE_OUVERT);

    // lire la ligne
    lire(cnx, &buffer);

    if (sscanf(buffer, MU_CONNEXION, identifiant, mot_de_passe) == 2)
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

        
        if(PQntuples(res) == 1){
            demande_correcte = true;
        }
        PQclear(res);
    }

    if (!demande_correcte)
    {
        write(cnx, RS_ERROR, strlen(RS_ERROR));
    }
    else
    {
        write(cnx, RS_CONNECTER, strlen(RS_CONNECTER));
    }
    //Fermer la connexion
    return demande_correcte;
}


/**
 * @fn int gestion_des_message(int cnx, PGconn *conn)
 * @brief Cette fonction gère les échange entre le serveur et le client. Il interprête, et est le point central.
 * @param cnx connexion du socket
 * @param conn connexion à la base de donnée
 */
int gestion_des_message(int cnx, PGconn *conn)
{
    bool en_fonctionnement = true;

    // Chaine dans laquelle est stocker ce que le prg reçoit du socket (la commande du client)
    t_chaine buffer = {'\0'};
    // Valeur du paramètre rentré dans une commande
    t_chaine param = {'\0'};
    // Réponse du serveur à l'utilisateur
    t_chaine message = {'\0'};
    // Resultat des fonctions
    int res;
    
    message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_CONNEXION_CLIENT_EFFECTUER);
    
    do
    {

        // Reset propre des variables
        memset(buffer, 0, sizeof(buffer));

        /*lecture*/
        int taille = lire(cnx, &buffer);
        message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_LECTURE_DES_INFORMATIONS);

        /*MESSAGE DE L'UTILISATEUR*/
        message_console_serveur(T_CPLS_MSG_CLIENT, 0);

        printf("%*.*s", taille, taille, buffer);

        message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_ENVOIE_DES_INFORMATIONS);

        

        // Crée un nouveau bordereau
        if (sscanf(buffer, MU_CREE, param) == 1)
        {
            t_chaine message;
            res = cree_bordereau(&message, param, conn);

            if(res == ERROR){
                write(cnx, RS_ERROR, strlen(RS_ERROR));
            }
            else if(res == LIVREUR_FULL){
                write(cnx, RS_LIVREUR_MAX, strlen(RS_LIVREUR_MAX));
            }
            else if(res == DONE){
                // Envoyer à l'utilisateur son bordereau
                write(cnx, &message, strlen(message));
            }
        }           
        // Pour récuperer l'image 
        else if (sscanf(buffer, MU_AVOIR_IMAGE, param) == 1){
            t_image image;

            int res = get_image(get_taille_image(), cnx, &image);

            if(res == ERROR){
                write(cnx, RS_ERROR, strlen(RS_ERROR));
            }
            else{
                write(cnx, message, strlen(message));
            }
        }

        // Pour avoir les information de l'image
        else if (sscanf(buffer, MU_AVPOR_INFO_IMAGE, param) == 1){
            size_t res = get_info_image(&message);

            if(res == ERROR){
                write(cnx, RS_ERROR, strlen(RS_ERROR));
            }
            else{
                write(cnx, message, strlen(message));
            }

        }
        // Pour avoir l'état de la livraison arrivé à l'étape 9
        else if (strncmp(buffer, MU_AVOIR_ETAT_LIVRAISON, strlen(MU_AVOIR_ETAT_LIVRAISON)) == 0){
            int res = get_etat_livraison(&message);

            if(res == ERROR){
                write(cnx, RS_ERROR, strlen(RS_ERROR));
            }
            else{
                write(cnx, message, strlen(message));
            }

        }
        // Donner le suivit d'une commande donnée
        else if (sscanf(buffer, MU_RECUPERER, param) == 1)
        {
            res = get_etat_utilisateur(&param, conn, &message, &cnx);
            if(res == ERROR){
                write(cnx, RS_ERROR, strlen(RS_ERROR));    
            }
            else{
                write(cnx, message, strlen(message));
            }
            
        }
        // Hello Serveur si on veux...
        else if (strncmp(buffer, MU_BONJOUR, strlen(MU_BONJOUR)) == 0)
        {
            write(cnx, RS_BONJOUR, strlen(RS_BONJOUR));
        }
        // Fin de communication
        else if (strncmp(buffer, MU_FIN_COMMUNICATION, strlen(MU_FIN_COMMUNICATION) - 1) == 0)
        {
            en_fonctionnement = false;
        }
        // FAIRE PASSER LE TEMPS,  CMD RESERVER AU SIMULATEUR
        else if(strncmp(buffer, MDLS_NEXT_STEP, strlen(MDLS_NEXT_STEP) -1) == 0){
            passer_temps(conn);
        }
        // Message par défaut qui prévient d'une erreur dans la saisie de la commande
        else
        {
            message_console_serveur(T_CPLS_SERVEUR_WARN, CPLS_COMMANDE_INCOMPRISE);
            write(cnx, RS_MAUVAISE_COMMANDE, strlen(RS_MAUVAISE_COMMANDE));
        }

        message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_DATA_ENVOYER);

    } while (en_fonctionnement == true);

    message_console_serveur(T_CPLS_SERVEUR_INFO, CPLS_FIN_DE_LA_COMMUNICATION);
    return DONE;
}

/**
 * @fn void message_console_serveur(int type, int msg)
 * @brief cette fonction gère les message qui seront afficher dans les logs du serveur
 * @param type défini de quel type est le message (ex: [SERVEUR INFO] ou [SERVEUR WARNING])
 * @param msg défini quel est la nature du message
 */
void message_console_serveur(int type, int msg)
{
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
        case 8:
            printf("Donnée Envoyer\n");
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
 * @fn int cree_bordereau(t_chiane *message, char *id, PGconn *conn)
 * @brief crée une bordereau avec le numéro de la commande
 * @param bordereau est la chaine vide au départ qui contiendra le bordereau nouvellement crée
 * @param id est l'id de la commande qui sert dans le processus de création du bordereau
 * @param conn connexion à la base de donnée pour y inscrire le bordereau
 * @return retourne un entier: 0 si ça a réussit, ou -1 en cas d'échècs
 * @bug Il y a une heure de décalage, je sais pas d'où ça sort, mais pas grave.
 */
int cree_bordereau(t_chaine *message, char *id, PGconn *conn)
{
    int return_status = DONE;

    PGresult *res = PQexec(
        conn,
        "SELECT count(*) FROM delivraptor.utilisateur WHERE etape <= 3"
    );

    t_chaine nb_element_etape = {'\0'};
    strcpy(nb_element_etape, PQgetvalue(res, 0, 0));
    int nb_elt = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    if (nb_elt >= MAX_LIVREUR){
        return_status = LIVREUR_FULL;
    }

    else if (id != NULL)
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

        // Mettre le tout dans le bordereau définitive
        snprintf(*message, sizeof(*message), "%s | %s\n", ET_DATA, bordereau_temp);

        char etape_str[LONGUEUR_BORDEREAU];
        snprintf(etape_str, sizeof etape_str, "%s", bordereau_temp);

        const char *listenn_argemenn[1] = {bordereau_temp};

        PQexecParams(
            conn,
            "INSERT INTO delivraptor.utilisateur (bordereau, etape) VALUES ($1, 1)",
            1,
            NULL,
            listenn_argemenn,
            NULL,
            NULL,
            0);
    }
    else{
        return_status = ERROR;
    }
    return return_status;
}


/**
 * @fn int get_etat_utilisateur(t_chaine *bordereau, PGconn *conn, t_chaine *message, int *cnx)
 * @brief va chercher dans la base de donnée pour donnée l'état actuel d'une commande
 * @param borderau le numéro qui sert à le retrouver dans la base de donnée
 * @param conn connexion à la base de donnée
 * @param message vide au départ, contient le message de réponse obtenu avec la requête
 */
int get_etat_utilisateur(t_chaine *bordereau, PGconn *conn, t_chaine *message, int *cnx)
{
    int return_status = DONE;
    const char *listenn_argemenn[1] = {*bordereau};

    memset(*message, 0, sizeof(*message));

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
    
    
    if (rows == 0){
        return_status = ERROR;
    }
    else{
        snprintf(*message + strlen(*message), LONGUEUR_MSG + strlen(*message),  "%s | ", ET_DATA);

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                size_t len = strlen(*message);
                if(j + 1 == cols)
                {
                    snprintf
                        (*message + len, LONGUEUR_MSG - len,
                            "%s\n",
                            PQgetvalue(res, i, j)
                        );
                }
                else
                {
                    snprintf
                        (*message + len, LONGUEUR_MSG - len,
                            "%s | ",
                            PQgetvalue(res, i, j)
                        );
                }
            }
        }
        if (atoi(PQgetvalue(res, 0, 1)) == 9){
            return_status = ARRIVED;
        }
    }
    
    PQclear(res);

    return return_status;
}

size_t get_info_image(t_chaine *message){
    size_t taille;

    // Récupérer la taille de l'image
    struct stat st;
    if(stat(IMAGE_PATH, &st) == 0){
        // Permet d'envoyer la taille de l'image au script php
        taille = st.st_size;
    }
    snprintf(*message, LONGUEUR_MSG, "%s | %ld\n", ET_DATA, taille);
    return taille;
}

size_t get_taille_image(){
    size_t taille;

    // Récupérer la taille de l'image
    struct stat st;
    if(stat(IMAGE_PATH, &st) == 0){
        // Permet d'envoyer la taille de l'image au script php
        taille = st.st_size;
    }
    return taille;
}

int get_image(size_t taille, int cnx, t_image *image){
    int return_value = DONE;

    // Ouvrir l'image :
    FILE *fd = fopen(IMAGE_PATH, "rb");

    
    if(return_value == 0){
        //Envoie de l'image par paquet de 8192 octets

        // Variable qui se remplit tout au long de l'envoie de l'image 
        size_t taille_deja_envoyer = 0;
        // Var qui va s'ajuster pour ne pas faire passer plus d'octet que nécéssaire
        size_t taille_paquet = IMAGE_SIZE;

        while(fread(image, sizeof(taille_paquet), 1, fd) != 0){
            write(cnx, image, sizeof(image)); 

            if(taille_deja_envoyer + IMAGE_SIZE < taille)
            {
                taille_deja_envoyer = taille_deja_envoyer + IMAGE_SIZE;
            }
            else{
                taille_paquet = (taille_deja_envoyer + IMAGE_SIZE) - taille;
            }
        }
    }
    if(fd != NULL){
        fclose(fd);
    }

    return return_value;
}

/**
 * @fn int get_etat_livraison(t_chaine *message)
 * @brief permet de savoir si le colis à été reçus, refuser ou mis en boite au lettre
 * @param message est le message qui sera retourner au client
 */
int get_etat_livraison(t_chaine *message){
    int etat_livrer = rand()%3;
    
    if(etat_livrer == 2){
        int excuse = rand()%5;
        
        snprintf(
            *message, LONGUEUR_MSG, "%s | %d | %d\n", 
            ET_DATA, etat_livrer, excuse         
        );
    }
    else{
        snprintf(
            *message, LONGUEUR_MSG, "%s | %d\n", 
            ET_DATA, etat_livrer         
        );
    }
    return DONE;
}


/**
 * @fn passer_temps(PGconn *conn)
 * @brief fait passer le temps
 * @param conn Pour mettre à jour la base de donnée
 */
void passer_temps(PGconn *conn){
    PGresult *res = PQexec(
        conn,
        "UPDATE delivraptor.utilisateur SET etape = etape + 1 WHERE etape < 9"
    );

    PQclear(res);
}