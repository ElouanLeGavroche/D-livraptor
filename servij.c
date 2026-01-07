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

#include <regex.h>


#define KEMENNADENN_DEGEMER "-> : "


// RESPONT AR SERVER 
#define DEMAT_RESPONT "demat implijer\n"
#define GUDENN_RESPONT "eur fazhi zo, barzh o komand\n"
#define GUDENN_GER_KUZH "ur fazhi zo, adklaskit marc'hplij\n"
#define NIVEREN_RESPONT "niveren : "

// KOMMANDE AN IMPLIJER

//Test
#define DEMAT "DEMAT"

//Lak find'ar kemenadenn
#define FINN_KOJEADENN "KUIT"

// KIR = kemenadenn Implijer Resevet
// s1
#define KIR_ENROLAN "CREATE"

//s2
#define KIR_HED_KUIT "s2"


#define HIRDER_KEMENNADENN 1024
#define HIRDER_CHADENN_BORDEREAU 33
#define MAX_PARAMETROU 5

typedef char t_chadenn_bordereau[HIRDER_CHADENN_BORDEREAU];
typedef char t_chadenn[HIRDER_KEMENNADENN];
typedef char *t_listen[MAX_PARAMETROU];

void skriv_kemennadenn(int type, int kemennadenn);
void lazhan_ur_bugel();


int lenn(int cnx, t_chadenn *buffer);
int enskrivadur(int cnx);
int kemennadenn(int cnx);


/* stadoù 
    = 0 peptra zo mat
    = -1 fazi
*/
int stad_1();
int stad_2();
int stad_3();
int stad_4();
int stad_5();
int stad_6();
int stad_7();
int stad_8();
int stad_9();

int main(){
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

    sigaction(SIGCHLD, &act, NULL);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	skriv_kemennadenn(4, 2);

	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);

    size = sizeof(conn_addr);



    while(true){

        ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
        ret = listen(sock, 1);

        if(ret == -1){
            return EXIT_FAILURE;
        }

        cnx = accept(sock, (struct sockaddr *)&conn_addr, (socklen_t *)&size);        
        pid_t pid = fork();

        if(pid == 0){
            // Ma ez eo ur bugel, mont barzh a ac'hwel vit ober war dro ar kom
            //kemennadenn(cnx);
            enskrivadur(cnx);
            shutdown(cnx, SHUT_RDWR);
            close(cnx);
            _exit(0);
        }
        
    }
    return EXIT_SUCCESS;
}

int lenn(int cnx, t_chadenn *buffer){
    /*lecture*/
    ssize_t n = read(cnx, *buffer, HIRDER_KEMENNADENN - 1);
    if (n <= 0) 
        return -1;
    *buffer[n] = '\0';
    int taille = strlen(*buffer);
    
    return taille;
}

int enskrivadur(int cnx){
    t_chadenn anavezer;
    t_chadenn ger_kuzh;

    skriv_kemennadenn(1, 1);
    int i;
    for(i = 0; i < 3; i ++){
        if(i != 0){
            write(cnx, GUDENN_GER_KUZH, strlen(GUDENN_GER_KUZH));    
        }
        write(cnx, KEMENNADENN_DEGEMER, strlen(KEMENNADENN_DEGEMER));
        t_chadenn buffer = {'\0'};
        lenn(cnx, buffer);
    }
    return 0;
}

int kemennadenn(int cnx){
    bool o_trein = true;

    t_chadenn buffer = {'\0'};

    t_listen argv;
    int argc;

    do{
        
        //Reset propre des variables
        memset(buffer,0,sizeof(buffer));
        memset(argv,0,sizeof(argv));

        write(cnx, KEMENNADENN_DEGEMER, strlen(KEMENNADENN_DEGEMER));
        
        /*lecture*/
        int taille = lenn(cnx, &buffer);

        /*MESSAGE DE L'UTILISATEUR*/
        skriv_kemennadenn(1, 3);    
        skriv_kemennadenn(5, 1);
        printf("%*.*s", taille, taille, buffer);

        argc = 0;

        char *token = strtok(buffer, " ");

        while (token != NULL && argc < MAX_PARAMETROU) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        
        skriv_kemennadenn(1, 4);

        //KROUIN UN BEAUREDAU NEVEZ
        if(strncmp(argv[0], KIR_ENROLAN, sizeof(KIR_ENROLAN) - 1) == 0){
            
            t_chadenn_bordereau chadenn_bordereau;
            stad_1(&chadenn_bordereau, argv[1]);

            //Digas ar Beauredeau d'an implijer
            write(cnx, chadenn_bordereau, strlen(chadenn_bordereau));

        }
        //TEST RESPONT
        else if(strncmp(argv[0], DEMAT, sizeof(DEMAT) - 1) == 0){
            write(cnx, DEMAT_RESPONT, strlen(DEMAT_RESPONT));
        }           
        //VIT LAK FINN
        else if(strncmp(argv[0], FINN_KOJEADENN, strlen(FINN_KOJEADENN) - 1) == 0){
            o_trein = false;
        }
        else{
            write(cnx, GUDENN_RESPONT, strlen(GUDENN_RESPONT));
        }
        
    }while(o_trein == true);

    skriv_kemennadenn(1, 5);  
    return EXIT_SUCCESS;

}

void skriv_kemennadenn(int seurt, int kemennadenn){
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

        3 [SERVER GUDEN]
            3.1 kevreadenn kolet
            3.2 guden barzh kod
        4 [SERVER INIT]
            4.1 Krouidigezh an dalc'h
            4.2 Krouidigezh ar soket
        5 [KEMENNADER AN IMPLIJER]
            5.1
    */

    switch(seurt){
        case 1:
            printf("[SERVER INFO] ");
            
            // TRAOÙ NORMAL AR SERVER
            switch(kemennadenn){
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
            switch(kemennadenn){
                default:
                    printf("netra vit poent\n");
                    break;
            }
            break;
        
        // MA ZO GUDENNOÙ GANT AR C'HOD
        case 3:
            printf("[SERVER GUDEN] ");
            switch(kemennadenn){
                default:
                    printf("netra vit poent\n");
                    break;
            }
            break;
        
            // MA ZO GUDENNOÙ GANT AR C'HOD
        case 4:
            printf("[SERVER INIT] ");
            switch(kemennadenn){
                case 1:
                    printf("Krouidigezh an dalc'h.\n");
                    break;
                
                case 2: 
                    printf("Krouidigezh ar soket.\n");
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
            switch(kemennadenn){
                default:
                    printf("DIWAL, UR FAZHI ZO !\n");
                    break;
            }
            break;
        
    }
}


void lazhan_ur_bugel(){
    skriv_kemennadenn(1, 6);

}

int stad_1(t_chadenn_bordereau *chadenn_bordereau, t_chadenn id){
    if(id != NULL){
        //Kaout an amzer
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        //Krouiñ ur kemmus vit ar bordereau
        t_chadenn_bordereau bordereau;

        //strcat(bordereau, "CMD");
        strftime(bordereau, sizeof(bordereau), "%H%M%S", t); //Kaout an amzer barzh ur chadenn 
        strcat(bordereau, id); //Lakaat id ar produce warlec'h
        strcpy(*chadenn_bordereau, bordereau); //Lakaat tout ze barzh an argemmenn

        return 0;
    }
    return -1;
}

int stad_2(){
    return 0;
}