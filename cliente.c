/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. de Ingeníería de Telecomunicación
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 1
 * Fichero: cliente.c
 * Versión: 3.1
 * Curso: 2024/2025
 * Descripción: Cliente sencillo TCP para IPv4 e IPv6
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * Alumno 1:Pablo Martínez Bruque
 * Alumno 2:
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <conio.h>		// Biblioteca de entrada salida básica
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.
#include "protocol.h"	// Declarar constantes y funciones de la práctica

#pragma comment(lib, "Ws2_32.lib")//Inserta en la vinculación (linking) la biblioteca Ws2_32.lib


int main(int* argc, char* argv[])
{
	SOCKET sockfd;
	struct sockaddr* server_in = NULL;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024], sender[1024], recipient[1024], subject[1024];
	int recibidos = 0, enviados = 0;
	int state;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char default_ip4[16] = "192.168.1.146";
	char default_ip6[64] = "::1";

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	//Inicialización de idioma
	setlocale(LC_ALL, "es_ES.UTF8");


	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);

	if (err != 0) {
		return(0);
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");

	do {
		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			//Si se introduce 6 se empleará IPv6
			ipversion = AF_INET6;

		}
		else { //Distinto de 6 se elige la versión IPv4
			ipversion = AF_INET;
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0);
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets_s(ipdest, sizeof(ipdest));

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*) & server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(TCP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*) & server_in6;
				address_size = sizeof(server_in6);
			}

			//BIENVENIDA->HELO(QUIT)->MAIL(QUIT)->RCPT(QUIT)->DATA(QUIT)->MENSAJE->FIN

			//Cada nueva conexión establece el estado incial en
			state = S_WELCOME;

			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
				//PMB HELO, MAIL, RCPT, DATA, QUIT y RSET.
				//Inicio de la máquina de estados
				do {
					//!!!!!!!!!!!!!!PMB REVISAR TEORIA Y RFC PARA  ENVIO MENSAJES CORRECTOS EN ABNF !!!!!!!!
					switch (state) {
						case S_WELCOME:
							// Se recibe el mensaje de bienvenida
							break;
						case S_HELO:
							// establece la conexion de aplicacion 
							printf("CLIENT> Enter the domain (press enter to exit): ");
							gets_s(input, sizeof(input));
							if (strlen(input) == 0) {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
								state = S_QUIT;
							}
							else {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", HELO, input, CRLF);
							}
							break;
						case S_MAIL:
							printf("CLIENT> Enter the sender (enter para salir): ");
							gets_s(input, sizeof(input));
							if (strlen(input) == 0) {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
								state = S_QUIT;
							}
							else
								sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", MAIL, input, CRLF);
							break;
						case S_RCPT:
							printf("CLIENTE> Enter the recipient (enter o QUIT para salir): ");
							gets_s(input, sizeof(input));
							if (strlen(input) == 0) {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
								state = S_QUIT;
							}
							else {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", RCPT, input, CRLF);
							}
							break;

					
						case S_DATA:
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", DATA, CRLF);
							break;

						case S_QUIT:
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							break;

						case S_RESET:
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", RESET, CRLF);
							break;

						case S_MESSAGE:
							
							printf("Enter the subject: ");
							gets_s(subject, sizeof(subject));
							if (strlen(input) == 0) {
								state = S_QUIT;
							}
							else {
								int p = 0;
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", SUBJECT, subject, CRLF);
								printf("Enter the body or use only a dot (.) to end the email \r\n");
								do {
									printf("Enter a new line or use only  a dot (.) to end the email \r\n");
									gets_s(input, sizeof(input));
									if (strcmp(input, ".") == 0) {
										p = 1; // O la acción que quieras realizar para terminar el proceso
									}
									else {
										// Procesar la línea del mensaje
										printf("You entered: %s\r\n", input);
									}
									sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
								} while (p == 0);
								
							}
							
							break;
					}

					if (state != S_WELCOME) {
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						if (enviados == SOCKET_ERROR) {
							state = S_QUIT;
							continue;// La sentencia continue hace que la ejecución dentro de un
									 // bucle salte hasta la comprobación del mismo.
						}
					}

					recibidos = recv(sockfd, buffer_in, 512, 0);

					if (recibidos <= 0) {
						DWORD error = GetLastError();
						if (recibidos < 0) {
							printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
							state = S_QUIT;
						}
						else {
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							state = S_QUIT;
						}
					}
					else {
						buffer_in[recibidos] = 0x00;
						printf(buffer_in);
						char code[1024];
						strncpy_s(code, sizeof(code), buffer_in, 3);//PMB SE ALMACENAN LOS 3 PRIMEROS VALORES DE LA RESPUESTA, SIENDO EN ESTE CASO LOS CODIGOS DE SMTP (220,250....)
						code[3] = 0;
						if (state == S_WELCOME) {
							if (strcmp(code, "220") == 0) {//CODIGO OK QUE DEVUELVE EL ARGOSOFT
								state = S_HELO;
							}
							else {
								printf("Respuesta no reconocida: %s\n", code);//PMB Si la respuesta es distinta será un comando no reconocido
								state = S_QUIT;
							}
						}
						else if (state == S_HELO) {
							if (strcmp(code, "250") == 0) {//cODIGO OK QUE DEVUELVE EL ARGOSOFT
								state = S_MAIL;
							}
							else if (code[0] == "4") { //PMB Si el sevidor SMTP devuelve un 4XX normalmente un 421 signficia que el servicio no esta disponible temporalmente
								printf("Error transitorio: %s\n", code);
								state = S_QUIT;
							}
							else if (code[0] == "5") {// PMB si el sevidor SMTP devuelve un 5XX puede ser un error de sintaxis o un error en los parametros de los comandos
								printf("Error permanente: %s\n", code);
								state = S_QUIT;
							}
							else {
								printf("Respuesta no reconocida: %s\n", code);//PMB Si la respuesta es distinta será un comando no reconocido
								state = S_QUIT;
							}
							
						}
						else if (state == S_MAIL) {
							if (strcmp(code, "250") == 0) {//cODIGO OK QUE DEVUELVE EL ARGOSOFT
								state = S_RCPT;
							}
							else if (code[0] == "4") { //PMB Si el sevidor SMTP devuelve un 4XX normalmente un 421 signficia que el servicio no esta disponible temporalmente
								printf("Error transitorio: %s\n", code);
								state = S_QUIT;
							}
							else if (code[0] == "5") {// PMB si el sevidor SMTP devuelve un 5XX puede ser un error de sintaxis o un error en los parametros de los comandos
								printf("Error permanente: %s\n", code);
								state = S_QUIT;
							}
							else {
								printf("Respuesta no reconocida: %s\n", code);//PMB Si la respuesta es distinta será un comando no reconocido
								state = S_QUIT;
							}

						}
						else if (state == S_RCPT) {
							if (strcmp(code, "250") == 0) {//cODIGO OK QUE DEVUELVE EL ARGOSOFT
								state = S_DATA;
							}
							else if (code[0] == "4") { //PMB Si el sevidor SMTP devuelve un 4XX normalmente un 421 signficia que el servicio no esta disponible temporalmente
								printf("Error transitorio: %s\n", code);
								state = S_QUIT;
							}
							else if (code[0] == "5") {// PMB si el sevidor SMTP devuelve un 5XX puede ser un error de sintaxis o un error en los parametros de los comandos
								printf("Error permanente: %s\n", code);
								state = S_QUIT;
							}
							else {
								printf("Respuesta no reconocida: %s\n", code);//PMB Si la respuesta es distinta será un comando no reconocido
								state = S_QUIT;
							}

						}
						else if (state == S_DATA) {
							if (strcmp(code, "354") == 0) {//cODIGO OK QUE DEVUELVE EL ARGOSOFT
								state = S_MESSAGE;
							}
						}
						else if (state == S_MESSAGE) {
							if (strcmp(code, "250") == 0) {//cODIGO OK QUE DEVUELVE EL ARGOSOFT
								state = S_MAIL;
							}
						}
						
						

						if (state != S_DATA && strncmp(buffer_in, OK, 2) == 0){
							state++;
						}
						//Si la autenticación no es correcta se vuelve al estado S_USER
						/*if (estado == S_PASS && strncmp(buffer_in, OK, 2) != 0) {
							estado = S_USER;
						}*/
					}

				} while (state != S_QUIT);
			}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			closesocket(sockfd);

		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	return(0);
}
