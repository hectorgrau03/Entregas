#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>

int contador;
pthread_mutex_t mutex =PTHREAD_MUTEX_INITIALIZER;

int i;
int sockets[100];

void *AtenderCliente(void *socket)
{
	int socket_conn;
	int *s;
	s =(int *) socket;
	socket_conn = *s;
	char peticion[512];
	char buff2[512];
	int ret;
	
	
	int terminar = 0;
	
	//Entramos en un bucle para atender todas las peticiones de este cliente
	//hasta que se desconecte
	while(terminar == 0)
	{
		//Ahora recibimos la peticion
		ret = read(sock_conn, peticion, sizeof(peticion));
		printf("Recibido\n");
		
		//Tenemos que a�adirle la marca de fin de string
		//Para que no escriba lo que hay despues en el buffer
		peticion[ret] = '\0';
		
		printf ("Se ha conectado: %s\n", peticion);
		
		
		char *p = strtok(peticion, "/");
		int codigo =  atoi (p);

		if(codigo == 0)
		{
			terminar = 1;
		}
		else
		{
			p = strtok( NULL, "/");
			char nombre[20];
			strcpy (nombre, p);
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
			
			if (codigo ==1) //piden la longitd del nombre
				sprintf (buff2,"%d,",strlen (nombre));
			else if (codigo == 2)
			{
				// quieren saber si el nombre es bonito
				if((nombre[0]=='M') || (nombre[0]=='S'))
					strcpy (buff2,"SI,");
				else
					strcpy (buff2,"NO,");
			}
			
			printf("%s\n", buff2);
			//Enviamos
			write(socket_conn, buff2, strlen(buff2));

			if((codigo ==1)||(codigo ==2))){
				pthread_mutex_lock(&mutex); //No me interrumpas ahora
				contador =contador + 1;
				pthread_mutex_unlock(&mutex);//Ya puedes interrumpirme
				//Notificar a todos los clientes conectados
				char notificacion[20];
				sprintf(notificacion,"%d",contador);
				int j;
				for(j=0,j<i;j++)
					write(sockets[j],notificacion,strlen(notificacion));
		}
	}
	
	//Cerramos la conexion con el servidor
	close(sock_conn); 
}

int main(int argc, char *argv[])
{
	int sock_conn, sock_listen, ret;
	struct sockaddr_in serv_adr;
	
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	
	
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// escucharemos en el port 9050
	serv_adr.sin_port = htons(9050);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	//La cola de peticiones pendientes no podr? ser superior a 4
	if (listen(sock_listen, 3) < 0)
		printf("Error en el Listen");
	
	int i =0;
	int sockets[100];
	pthread_t thread;
	// Atenderemos solo 5 peticione
	for(;;){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexi?n\n");
		
		//sockets[i] es el socket que usaremos para este cliente
		sockets[i] = socket_conn;
		
		//Creatr thead y decirle lo que tiene que hacer
		pthread_create(&thread, NULL, AtenderCliente, &sockets[i]); //Le passas el socket por referencia. Para no passar le una copia 
		
		i++;
	}
}
