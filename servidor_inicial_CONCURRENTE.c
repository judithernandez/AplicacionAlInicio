#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>

void *AtenderCliente(void *socket)
{
	int sock_conn;
	int *s;
	s =(int *) socket;
	sock_conn = *s;
	
	char peticion[512];
	char respuesta[512];
	int ret;
	
	//Bucle de atencion al cliente 
	int terminar = 0;
	while (terminar ==0)
	{
		
		// Ahora recibimos su nombre, que dejamos en buff
		//2)RECOGE LA PETICION
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		
		// Tenemos que a?adirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		
		//Escribimos el nombre en la consola
		
		printf ("Se ha conectado: %s\n",peticion);
		
		//3)DETERMINA QUE ME PIDEN
		char *p = strtok( peticion, "/");
		int codigo =  atoi (p);
		char nombre[20];
		if (codigo !=0)
		{
			p = strtok( NULL, "/");
			strcpy (nombre, p);
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
		}
		
		//4)PREPARA LA RESPUESTA
		if (codigo == 0)
		{
			terminar = 1;
		}
		else if (codigo ==1)
		{//piden la longitd del nombre
			int lenght = (int) strlen (nombre);
			sprintf (respuesta,"%d",lenght);
		}
		else if (codigo ==2)
		{// quieren saber si el nombre es bonito
			if((nombre[0]=='M') || (nombre[0]=='S'))
			strcpy (respuesta,"SI");
			else
				strcpy (respuesta,"NO");
		}
		else if (codigo == 3)
		{//el cliente quiere saber si es alto
			p = strtok( NULL, "/");
			float altura =  atof (p);
			if (altura > 1.70)
				sprintf(respuesta, "%s: eres alto\n", nombre);
			else 
				sprintf(respuesta, "%s: eres bajo\n", nombre);
			
		}
		else if (codigo == 4)
		{//quiere saber si su nombre es palindromo
			int i = 0;
			int j = strlen(nombre) - 1;  // get the last valid index
			int is_palindrome = 1;
			while(i <= j && is_palindrome == 1)
			{
				if (nombre[i] != nombre[j])
				{
					is_palindrome = 0;
					//	break;
				}
				
				i++;  
				j--;
			}
			sprintf(respuesta,"%d",is_palindrome);
		}
		else if (codigo == 5)
		{//quiere recibir su nombre en mayusculas
			for (int i = 0; nombre[i]!='\0'; i++) {
				if(nombre[i] >= 'a' && nombre[i] <= 'z') {
					nombre[i] = nombre[i] -32;
				}
			}
			sprintf(respuesta,"Tu nombre en mayuscula es : %s",nombre);
		}
		
		//5)ENVIA LA RESPUESTA
		if (codigo !=0)
		{
			printf ("%s\n", respuesta);
			write (sock_conn,respuesta, strlen(respuesta));
		}
	}
	//6)CIERRA LA CONEXION
	// Se acabo el servicio para este cliente
	close(sock_conn); 
}

int main(int argc, char *argv[])
{
	int sock_conn, sock_listen;
	struct sockaddr_in serv_adr;
	
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	
	// FEM EL BIND AL PORT
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	// asocia el socket a cualquiera de las IP de la maquina. 
	//htons formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// escucharemos en el port 9050
	serv_adr.sin_port = htons(9080);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	//La cola de peticiones pendientes no podra ser superior a 4
	if (listen(sock_listen, 2) < 0)
		printf("Error en el Listen");
	
	int i;
	int sockets[100];
	pthread_t thread[100];
	// Atenderemos 5 PETICIONES
	for(i=0;i<5;i++){
		printf ("Escuchando\n");
		
		//1)ESPERANDO CONEXION
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		sockets[i] = sock_conn; //guardo el socket para comunicarme con el nuevo cliente
		//sock_conn es el socket que usaremos para este cliente
		
		//Crear un thread y decirle lo que tiene que hacer
		pthread_create(&thread[i],NULL, AtenderCliente, &sockets[i]);
		//thread[i] es un parametro de salida, la funcion mete dentro un id de thread
		//socket[i] es un parametro de entrada, la funcion necesita saber que socket se esta usando
		
	}
	return 0;
}
