#include <getopt.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "common.h"

void atender_cliente(int connfd);

char **delim_comandos(char *texto,char *delimitador){
	char *token;
	char *linea_copiar;
	int i,ntokens=0;
	char **argv=NULL;
	linea_copiar= (char *) malloc(strlen(texto)+1);
	strcpy(linea_copiar,texto);
	token=strtok(linea_copiar,delimitador);
	while(token!=NULL){
		token=strtok(NULL,delimitador);
		ntokens++;
	}
	free(linea_copiar);
	if(ntokens>0){
		argv=(char **) malloc((ntokens+1)* sizeof(char **));
		token=strtok(texto,delimitador);
		for(i=0;i<ntokens;i++){
			argv[i]=(char *) malloc(strlen(token)+1);
			strcpy(argv[i],token);
			token=strtok(NULL,delimitador);
		}
		argv[i]=NULL;
	}
	return argv;
}
void print_help(char *command)
{
	printf("Servidor simple de descarga de archivos.\n");
	printf("uso:\n %s <puerto>\n", command);
	printf(" %s -h\n", command);
	printf("Opciones:\n");
	printf(" -h\t\t\tAyuda, muestra este mensaje\n");
}

/**
 * Función ejemplo para separar una cadena de caracteres en
 * "tokens" delimitados por la cadena de caracteres delim.
 *
 * @param linea Cadena de caracteres a separar en tokens.
 * @param delim Cadena de caracteres a usar como delimitador.
 */
void separar_tokens(char *linea, char *delim)
{
	char *token;
	int i = 0;

	printf("Sepración en tokens:\n");

	/* obtiene el primer token */
	token = strtok(linea, delim);

	/* recorre todos los tokens */
	while( token != NULL ) {
		printf( "%d. %s\n", ++i, token);
		token = strtok(NULL, delim);
	}
}

bool seguir = true;
void recoger_hijos(int senal){
	while(waitpid(-1,0,WNOHANG)>0);
	return;
}

int main(int argc, char **argv)
{
	int opt;

	//Sockets
	int listenfd, connfd;
	unsigned int clientlen;
	//Direcciones y puertos
	struct sockaddr_in clientaddr;
	char *port;

	while ((opt = getopt (argc, argv, "h")) != -1){
		switch(opt)
		{
			case 'h':
				print_help(argv[0]);
				return 0;
			default:
				fprintf(stderr, "uso: %s <puerto>\n", argv[0]);
				fprintf(stderr, "     %s -h\n", argv[0]);
				return 1;
		}
	}

	if(argc != 2){
		fprintf(stderr, "uso: %s <puerto>\n", argv[0]);
		fprintf(stderr, "     %s -h\n", argv[0]);
		return 1;
	}else
		port = argv[1];

	//Valida el puerto
	int port_n = atoi(port);
	if(port_n <= 0 || port_n > USHRT_MAX){
		fprintf(stderr, "Puerto: %s invalido. Ingrese un número entre 1 y %d.\n", port, USHRT_MAX);
		return 1;
	}

	//Abre un socket de escucha en port
	listenfd = open_listenfd(port);


	if(listenfd < 0)
		connection_error(listenfd);

	printf("server escuchando en puerto %s...\n", port);
	signal(SIGCHLD,recoger_hijos);

	while (seguir) {
		clientlen = sizeof(clientaddr);
		connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

		/*para hcer ejecucion recurrente
		if(fork()==0){
			close(listenfd);
			atender_cliente(connfd);
			close(connfd);
			exit(0);
		}*/
		atender_cliente(connfd);
		close(connfd);
	}
}

void atender_cliente(int connfd)
{
	int n;
	char buf[MAXLINE] = {0};
	int status;
	char **comandos;
	pid_t pid;

	while(1){
		n = read(connfd, buf, MAXLINE);
		if(n <= 0)
			return;

		printf("Recibido: %s", buf);

		//Detecta "CHAO" y se desconecta del cliente
		if(strcmp(buf, "CHAO\n") == 0){
			write(connfd, "BYE\n", 4);
			return;
		}

		//Detecta "KILL", se desconecta del cliente y termina
		if(strcmp(buf, "KILL\n") == 0){
			write(connfd, "BYE\n", 4);
			seguir = false;
			return;
		}

		//Reenvía el mensaje al cliente
		// n = write(connfd, buf, n);
		// if(n <= 0)
		// 	return;
		

		//Remueve el salto de linea antes de extraer los tokens
		buf[n - 1] = '\0';

		//Recoge los argumentos, separados por espacio
		comandos= delim_comandos(buf," ");
		

		 if(comandos){
			if((pid=fork())==0){
				if(execvp(comandos[0],comandos)<0){
					fprintf(stderr,"Comando desconocido \n");
					exit(1);
				}
			}
			waitpid(pid,&status,0);
			if(WEXITSTATUS(status)==0){
				write(connfd,"OK\n",n);
			}else{
				write(connfd,"ERROR\n",n);
			}
			

			for(int i=0;comandos[i];i++){
				free(comandos[i]);
			}
			free(comandos);
		 }else{
			fprintf(stderr,"Comando vacio \n");
			exit(1);
		 }

		//Separa el buffer recibido en tokens usando el espacio
		//separar_tokens(buf, " ");

		memset(buf, 0, MAXLINE); //Encera el buffer
	}
}
