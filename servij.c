#include <sys/types.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <string.h>

#include <regex.h>

#define HIRDER_KEMENNADENN 1024

#define RESPONT_KELOU_RESEVET "[SERVER INFO] keloù resevet\n"

#define KEMENNADENN_DEGEMER "goulennoù an implijer : "

#define PING "PING"
#define PONG "PONG"

#define DEMAT "DEMAT"
#define DEMAT_RESPONT "DEMAT implijer\n"

#define FINN_KOJEADENN "exit"

typedef char t_message;

int main(){
	printf("[INFO] Krouidigezh an dalc'h.\n");
	int sock;
	int ret;
	struct sockaddr_in addr;
	int size;
	int cnx;
	struct sockaddr_in conn_addr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	printf("[INFO] Krouidigezh ar soket : %d\n", sock);

	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	

	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);

	printf("[INFO] kevreadenn graet\n");
    int valid = false;
    pid_t pid = -1;

    while(valid == false){
        ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
        ret = listen(sock, 1);

        if(ret == -1){
            return EXIT_FAILURE;
        }

        pid_t pid = fork();
        if (pid == 0){
            valid = true;
        }
    }

    if(pid == 0){
        size = sizeof(conn_addr);
        cnx = accept(sock, (struct sockaddr *)&conn_addr, (socklen_t *)&size);
        
        
        char buffer[HIRDER_KEMENNADENN] = {'\0'};

        while(strncmp(buffer, FINN_KOJEADENN, strlen(FINN_KOJEADENN) - 1) != 0){
            char message[HIRDER_KEMENNADENN];
            int taille = 0;

            write(cnx, KEMENNADENN_DEGEMER, strlen(KEMENNADENN_DEGEMER));
            
            /*lecture*/
            read(cnx, &buffer, HIRDER_KEMENNADENN);			
            taille = strlen(buffer);

            printf("[INFO] lennidigezh ar keloù\n");
            ret = printf("[KEMENNADER AN IMPLIJER] %*.*s", taille, taille, buffer);

            if(ret == -1){
                return EXIT_FAILURE;
            }

            printf("[INFO] digas keloioù\n");
            write(cnx, RESPONT_KELOU_RESEVET, sizeof(RESPONT_KELOU_RESEVET));

            if(strncmp(buffer, PING, sizeof(PING) - 1) == 0){
                int i;
                for(i = 0; i < 4; i ++){
                    sprintf(message, "PONG N°%d\n", i);
                    write(cnx, message, strlen(message));

                }
            }

            if(strncmp(buffer, DEMAT, sizeof(DEMAT) - 1) == 0){
                write(cnx, DEMAT_RESPONT, strlen(DEMAT_RESPONT));
            }
            
        }
    }

	printf("[INFO] finn ar kojeadenn\n");
	return EXIT_SUCCESS;
}

