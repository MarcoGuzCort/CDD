
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

#define KILO 1024
#define BUFF_LENGTH 1000
#define PROTO_PORT 60000


int sd;
int served = 0;

void *manage_reading(){
	
	int n, i;
	char inbuf[BUFF_LENGTH];	

	while(served == 0){

		for(i = 0; i < BUFF_LENGTH; i++){
			inbuf[i] = 0;
		}

		n = read(sd, inbuf, sizeof(inbuf));

		if(!strcmp(inbuf, "QUIT"))
			served = 1;
		else
			printf("Received message!!: %s\n", inbuf);		
	}	
}

int main(int argc, char** argv){

	if(argc < 3){
		printf("\nUsage: ./clienttcp 'ingresa tu nombre de usuario y la ip donde esta el servido\n\n");
		return;
	}

	struct sockaddr_in sad;
	socklen_t alen;

	int i, n, port, oc = 0;
	char inbuf[BUFF_LENGTH], outbuf[BUFF_LENGTH], outchar;
	char *hostname;
	pthread_t tid;

	for(i = 0; i < BUFF_LENGTH; i++){
		inbuf[i] = 0;
		outbuf[i] = 0;
	}

	if(argc == 4){
		port = atoi(argv[3]);
		while(port < 0 || port > 64 * KILO){
			printf("Bad port number, buond limits are (0,%d)\n\nEnter a new port number: ", 64 * KILO);
			scanf("%d", &port);
		}
	}else{
		port = PROTO_PORT;
	}


	memset((char*)&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_port = htons((u_short)port);
	n = inet_aton(argv[2], &sad.sin_addr);
	printf("\nServidor a contactar: [%s]:[%d]\n", argv[2], port);
	printf("Servidor a contactar: [%d]:[%d]\n\n", sad.sin_addr.s_addr, sad.sin_port);

	hostname = argv[1];
	printf("El nombre del cliente: [%s]\n\n", hostname);

	sd = socket(PF_INET, SOCK_STREAM, 0);
	
	printf("Conectando a [%d]:[%d]...\n\n", sad.sin_addr.s_addr, sad.sin_port);
	sleep(5);
	connect(sd, (struct sockaddr*)&sad, sizeof(sad));
	printf("Conectado al servidor: [%d]:[%d]\n", sad.sin_addr.s_addr, sad.sin_port);
	
	sprintf(outbuf, "HELLO I AM <%s>", hostname);
	write(sd, outbuf, sizeof(outbuf));
	
	printf("Esperando por el estado del servidor...\n\n");
	sleep(5);
		
	n = read(sd, inbuf, sizeof(inbuf));
	if(strcmp(inbuf, "BUSY")){
		printf("Clientes en linea:\n");
		while(strcmp(inbuf, "END")){
			printf("\t\t - %s\n", inbuf);
			n = read(sd, inbuf, sizeof(inbuf));
			oc++;
		}

		if(oc == 0){
reask:			printf("No contacts online, do you wish to close connection? (Y/N)\n");
			scanf("%c", &outchar);
			switch(outchar){
				case 'Y': goto out;
					  	
				case 'y': goto out;
					  
				case 'N': 
					  break;
				case 'n': 
					  break;
				default: 
					printf("wrong character pressed\n");
					while(getchar() != '\n');					
					goto reask;
					break;
			}		
		}

		printf("\t\t- <[contactname]:[message] envia un mensaje privado>\n\t\t- <[message] envia a todos los clientes>\n\t\t- <[QUIT] para salir del chat>\n\n"); //controlla nuovi arrivi utenti
		
		if(pthread_create(&tid, NULL, manage_reading)!=0) {
			perror("Thread creation");
		}		

		while(served == 0){
			
			for(i = 0; i < BUFF_LENGTH; i++){
				outbuf[i] = 0;
			}

			while(getchar() != '\n');		//secondo client in poi prima stampa a vuoto...
			scanf("%[^\n]s", outbuf);
			write(sd, outbuf, sizeof(outbuf));			
			if(!strcmp(outbuf, "QUIT"))
				served = 1;						
		}
		

	}else{
		printf("\nServidor ocupado, cerrando coneccion\n");
		close(sd);	
	}

out:	printf("Cerrando coneccion...\n");

	sleep(1);

	printf("Bye!\n");
	sleep(5);
	
	close(sd);
	printf("\n\nCliente finalizado\n\n");
}
