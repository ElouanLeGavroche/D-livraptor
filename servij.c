#include <sys/types.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <regex.h>

#define HIRDER_KEMENNADENN 1024

#define RESPONT_KELOU_RESEVET "[SERVER INFO] keloù resevet\n"

#define KEMENNADENN_DEGEMER "goulennoù an implijer : "

#define PING "PING"
#define PONG "PONG"

#define DEMAT "DEMAT"
#define DEMAT_RESPONT "demat implijer\n"

#define FINN_KOJEADENN "kuit"

typedef char t_message;

void skriv_kemennadenn(int type, int kemennadenn);
void lazhan_ur_bugel();

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
            kemennadenn(cnx);
            
            shutdown(cnx, SHUT_RDWR);
            close(cnx);
            _exit(0);
        }
        
    }
    return EXIT_SUCCESS;
}

int kemennadenn(int cnx){
    int ret;

    skriv_kemennadenn(1, 1);
    
    
    char buffer[HIRDER_KEMENNADENN] = {'\0'};

    do{
         
        char message[HIRDER_KEMENNADENN];
        int taille = 0;

        write(cnx, KEMENNADENN_DEGEMER, strlen(KEMENNADENN_DEGEMER));
        
        /*lecture*/
        read(cnx, &buffer, HIRDER_KEMENNADENN);			
        taille = strlen(buffer);

        skriv_kemennadenn(1, 3);    

        /*MESSAGE DE L'UTILISATEUR*/
        skriv_kemennadenn(5, 1);
        ret = printf("%*.*s", taille, taille, buffer);
        if(ret == -1){
            return EXIT_FAILURE;
        }

        skriv_kemennadenn(1, 4);

        if(strncmp(buffer, PING, sizeof(PING) - 1) == 0){
            int i;
            for(i = 0; i < 4; i ++){
                sprintf(message, "PONG N°%d\n", i);
                write(cnx, RESPONT_KELOU_RESEVET, sizeof(RESPONT_KELOU_RESEVET));
                write(cnx, message, strlen(message));

            }
        }

        if(strncmp(buffer, DEMAT, sizeof(DEMAT) - 1) == 0){
            write(cnx, RESPONT_KELOU_RESEVET, sizeof(RESPONT_KELOU_RESEVET));
            write(cnx, DEMAT_RESPONT, strlen(DEMAT_RESPONT));
        }
        
    }while(strncmp(buffer, FINN_KOJEADENN, strlen(FINN_KOJEADENN) - 1) != 0);

    skriv_kemennadenn(1, 5);  
    return 0;

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