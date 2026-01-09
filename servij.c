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

// BDD
#include <libpq-fe.h>

#define KEMENNADENN_DEGEMER "-> : "
#define KEMENADDENN_ENSKIRVET " Servij -> :"

// RESPONT AR SERVER
#define DEMAT_RESPONT "demat implijer\n"
#define GUDENN_RESPONT "eur fazhi zo, barzh o komand\n"
#define GUDENN_LOG "fazi gant an identelezh, c'huited\n"
#define NIVEREN_RESPONT "niveren : "

#define HIRDER_KEMENNADENN 1024
#define HIRDER_CHADENN_BORDEREAU 33

typedef char t_chadenn_bordereau[HIRDER_CHADENN_BORDEREAU];
typedef char t_chadenn[HIRDER_KEMENNADENN];

void skriv_kemennadenn(int type, int kemennadenn);
void lazhan_ur_bugel();

int lenn(int cnx, t_chadenn *buffer);
int enskrivadur(int cnx, PGconn *conn);
int kemennadenn(int cnx, PGconn *conn);

/* stadoù
    = 0 peptra zo mat
    = -1 fazi
*/
int krouin_bordereau(t_chadenn_bordereau *chadenn_bordereau, char *id, PGconn *conn);
int kaout_implijer_stad(t_chadenn *bordereau, PGconn *conn, t_chadenn *respont);


// Test
const char KIR_DEMAT[] = "DEMAT";

// Lak find'ar kemenadenn
const char KIR_FINN_KOJEADENN[] = "KUIT";

// KIR = kemenadenn Implijer Resevet
// Konnektin
const char KIR_KONEKTIN[] = "CONN %s %s";
// Krouiñ ur bordereau
const char KIR_ENROLAN[] = "CREATE %253s";
// Kaout Bordereau unann benak
const char KIR_KAOUT[] = "GET %s";

// s2
const char KIR_HED_KUIT[] = "s2";

int main()
{
    // Init du système de communication
    skriv_kemennadenn(4, 1);

    int sock;
    int ret;
    struct sockaddr_in addr;
    int size;
    int cnx;
    struct sockaddr_in conn_addr;

    struct sigaction act;

    act.sa_handler = &lazhan_ur_bugel;
    sigemptyset(&act.sa_mask);

    act.sa_flags = SA_RESTART;

    sigaction(SIGCHLD, &act, NULL);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    skriv_kemennadenn(4, 2);

    // VIT AR bdd
    PGconn *conn = PQconnectdb(
        "host=localhost port=5432 dbname=saedb user=sae password=racine");

    if (PQstatus(conn) != CONNECTION_OK)
    {
        skriv_kemennadenn(3, 1);
        fprintf(stderr, ": %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return EXIT_FAILURE;
    }

    skriv_kemennadenn(4, 3);
    // Kon krouet

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);

    size = sizeof(conn_addr);

    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1){
        return EXIT_FAILURE;
    }

    ret = listen(sock, 1);
    if(ret == -1){
        return EXIT_FAILURE;
    }

    while (true)
    {

        cnx = accept(sock, (struct sockaddr *)&conn_addr, (socklen_t *)&size);
        pid_t pid = fork();

        if (pid == 0)
        {
            // Ma ez eo ur bugel, mont barzh ha ac'hwel vit ober war dro ar kom
            enskrivadur(cnx, conn);
            kemennadenn(cnx, conn);
            
            shutdown(cnx, SHUT_RDWR);
            close(cnx);
            _exit(0);
        }
    }
    PQfinish(conn);
    return EXIT_SUCCESS;
}

/*
int lenn(int cnx, t_chadenn *buffer)
{
    //Lennadenn
    ssize_t n = read(cnx, *buffer, HIRDER_KEMENNADENN - 1);
    if (n <= 0) {return -1;}

    *buffer[n] = '\0';
    int taille = strlen(*buffer);
    return taille;
}
*/

int lenn(int cnx, t_chadenn *buffer)
{
    int dalch = 0;
    char c = '\0';

    //lagadenn vit lenn
    while (dalch < HIRDER_KEMENNADENN - 1 && c != '\n') {
        ssize_t n = read(cnx, &c, 1);
        if (n <= 0) {return -1;}

        //Vit kaout petra zo barzh a cellulenn 1 ha ket ar pointeur
        (*buffer)[dalch++] = c;
    }

    //Vit kaout petra zo barzh a cellulenn 1 ha ket ar pointeur
    (*buffer)[dalch] = '\0';
    return dalch;
}


int enskrivadur(int cnx, PGconn *conn)
{
    //Buffer vit kemenadenn an implijer
    t_chadenn buffer = {'\0'};
    t_chadenn identelezh;
    t_chadenn ger_kuz;

    bool kenkas_reiz = false;

    do{
        skriv_kemennadenn(1, 1);
        
        write(cnx, KEMENNADENN_DEGEMER, strlen(KEMENNADENN_DEGEMER));
        
        //Lenn al linenn
        lenn(cnx, &buffer);
        
        if(sscanf(buffer, KIR_KONEKTIN, identelezh, ger_kuz) == 2)
        {
            //Klask en diaz roadennoù evit kavout ur c'hemm 
            //etre ar ger-tremen hag an anv implijer 
            //evit mont e-barzh ar c'hlient.
            const char *listenn_argemenn[1];
            listenn_argemenn[0] = "jean";
            listenn_argemenn[1] = "123!!";

            PGresult *res = PQexecParams(
                    conn, 
                    "SELECT * FROM delivraptor.client WHERE identifiant = $1 AND mot_de_passe = $2",
                    2,
                    NULL,
                    listenn_argemenn,
                    NULL,
                    NULL,
                    0
            );

            int rows = PQntuples(res);
            printf("%d\n", rows);
            int cols = PQnfields(res);
            for (int i = 0; i < cols; i++)
            {
                printf("%s\t", PQfname(res, i));
            }

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    // Print the column value
                    if (strcmp(PQgetvalue(res, i, j), "jean") == 0)
                    {
                        if (strcmp(PQgetvalue(res, i, j + 1), "123!!") == 0)
                        {
                            // Mettre authentification ici
                            printf("LOGIN MDP OK");
                            kenkas_reiz = true;
                        }
                    }
                    printf("%s\t", PQgetvalue(res, i, j));
                }
                printf("\n");
            }

            ExecStatusType resStatus = PQresultStatus(res);
            printf("Etat requete: %s\n", PQresStatus(resStatus));
            
            if(kenkas_reiz){
                PQclear(res);
            }

        }
        else{
            write(cnx, GUDENN_LOG, strlen(GUDENN_LOG));
        }
    }while(!kenkas_reiz);

    return 0;
}

int kemennadenn(int cnx, PGconn *conn)
{
    bool o_trein = true;

    t_chadenn buffer = {'\0'};
    t_chadenn param = {'\0'};
    t_chadenn respont = {'\0'};

    skriv_kemennadenn(1, 1);
    do
    {

        // Reset propre des variables
        memset(buffer, 0, sizeof(buffer));

        write(cnx, KEMENADDENN_ENSKIRVET, strlen(KEMENADDENN_ENSKIRVET));

        /*lecture*/
        int taille = lenn(cnx, &buffer);

        /*MESSAGE DE L'UTILISATEUR*/
        skriv_kemennadenn(1, 3);
        skriv_kemennadenn(5, 1);

        printf("%*.*s", taille, taille, buffer);

        skriv_kemennadenn(1, 4);

        // KROUIN UN BEAUREDAU NEVEZ
        if(sscanf(buffer, KIR_ENROLAN, param) == 1)
        {
            t_chadenn_bordereau chadenn_bordereau;
            krouin_bordereau(&chadenn_bordereau, param, conn);

            // Digas ar Beauredeau d'an implijer
            write(cnx, &chadenn_bordereau, strlen(chadenn_bordereau));
        }
        // TEST RESPONT
        else if (strncmp(buffer, KIR_DEMAT, sizeof(KIR_DEMAT) - 1) == 0)
        {
            write(cnx, DEMAT_RESPONT, strlen(DEMAT_RESPONT));
        }
        // VIT LAK FINN
        else if (strncmp(buffer, KIR_FINN_KOJEADENN, strlen(KIR_FINN_KOJEADENN) - 1) == 0)
        {
            o_trein = false;
        }
        // Gouzout pelec'h eo ur c'hommand
        else if(sscanf(buffer, KIR_KAOUT, param) == 1)
        {
            
            kaout_implijer_stad(&param, conn, &respont);
            write(cnx, &respont, strlen(respont));
        }
        else
        {
            write(cnx, GUDENN_RESPONT, strlen(GUDENN_RESPONT));
        }
    } while (o_trein == true);

    skriv_kemennadenn(1, 5);
    return EXIT_SUCCESS;
}

void skriv_kemennadenn(int seurt, int kemennadenn)
{
    /*
        1 [SERVER INFO]
            1.1 kevreadenn graet
            1.2 keloù resevet
            1.3 lennidigezh ar c'heloù
            1.4 digas keloioù
            1.5 finn ar kojeadenn
            1.6 bugel lazhet

        2 [SERVER DIWAL]
            2.1 respont ebet
            2.2 respont hir
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

    switch (seurt)
    {
    case 1:
        printf("[SERVER INFO] ");

        // TRAOÙ NORMAL AR SERVER
        switch (kemennadenn)
        {
        case 1:
            printf("kevreadenn graet\n");
            break;

        case 2:
            printf("keloù resevet\n");
            break;

        case 3:
            printf("lennidigezh ar c'heloù\n");
            break;

        case 4:
            printf("digas keloioù\n");
            break;

        case 5:
            printf("finn ar kojeadenn\n");
            break;

        case 6:
            printf("bugel lazhet\n");
            break;

        default:
            printf("[DIDERMEN]\n");
            break;
        }
        break;

    // MA ZO GUDENNOÙ GANT AR SERVER
    case 2:
        printf("[SERVER GUDEN] ");
        switch (kemennadenn)
        {
        default:
            printf("netra vit poent\n");
            break;
        }
        break;

    // MA ZO GUDENNOÙ GANT AR C'HOD
    case 3:
        printf("[SERVER FAZI] ");
        switch (kemennadenn)
        {
        case 3:
            printf("gudenn kevreadenn gant ar BDD : ");
            break;
        default:
            printf("netra vit poent\n");
            break;
        }
        break;

        // MA ZO GUDENNOÙ GANT AR C'HOD
    case 4:
        printf("[SERVER INIT] ");
        switch (kemennadenn)
        {
        case 1:
            printf("Krouidigezh an dalc'h.\n");
            break;

        case 2:
            printf("Krouidigezh ar soket.\n");
            break;

        case 3:
            printf("Kevreadenn gant ar BDD graet. \n");
            break;

        default:
            printf("netra vit poent\n");
            break;
        }
        break;

    case 5:
        printf("[KEMENNADER AN IMPLIJER]");
        break;

    // VIT AR C'HELLOIOÙ NAN KOMPRENET
    default:
        printf("[DIDERMEN] ");
        switch (kemennadenn)
        {
        default:
            printf("DIWAL, UR FAZHI ZO !\n");
            break;
        }
        break;
    }
}

void lazhan_ur_bugel()
{
    waitpid(-1, NULL, WNOHANG);
    skriv_kemennadenn(1, 6);
}
int krouin_bordereau(t_chadenn_bordereau *chadenn_bordereau, char *id, PGconn *conn)
{
    if (id != NULL)
    {
        // Kaout an amzer
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        // Krouiñ ur argemmenn vit ar bordereau
        t_chadenn_bordereau bordereau = {'\0'};

        // strcat(bordereau, "CMD");
        strftime(bordereau, sizeof(bordereau), "%H%M%S", t); // Kaout an amzer barzh ur chadenn
        strcat(bordereau, id);                               // Lakaat id ar produce warlec'h
        strcpy(*chadenn_bordereau, bordereau);               // Lakaat tout ze barzh an argemmenn
        
        char etape_str[HIRDER_CHADENN_BORDEREAU];
        snprintf(etape_str, sizeof etape_str, "%s", bordereau);

        const char *listenn_argemenn[1];
        listenn_argemenn[0] = bordereau;

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

int kaout_implijer_stad(t_chadenn *bordereau, PGconn *conn, t_chadenn *respont)
{

    const char *listenn_argemenn[1];
    listenn_argemenn[0] = *bordereau;

    PGresult *res = PQexecParams(conn, 
        "SELECT bordereau, etape, time FROM delivraptor.utilisateur AS u INNER JOIN delivraptor.logs AS l ON bordereau = id_utilisateur WHERE  bordereau = $1; ",
        1,
        NULL,
        listenn_argemenn,
        NULL,
        NULL,
        0
    );
    int rows = PQntuples(res);
    int cols = PQnfields(res);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            strcat(*respont, PQgetvalue(res, i, j));
            strcat(*respont, " | ");
        }
    }
    strcat(*respont, "\n");

    return 0;
}