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
	char buffer_in[1024], buffer_out[1024], input[1024], sender[1024], recipient[1024], subject[1024], n_recipient[10];
	int recibidos = 0, enviados = 0, pmb = 0;
	int state,  num_recipients;
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

	printf("**************\r\nSIMPLE TCP CLIENT OVER IPv4 or IPv6\r\n*************\r\n");

	do {
		printf("CLIENT> Which IP version do you want to use? 6 for IPv6, 4 for IPv4 [default] ");
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
			printf("CLIENT> ERROR\r\n");
			exit(-1);
		}
		else {
			printf("CLIENT> Enter the destination IP (press enter for default IP): ");
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
				printf("CLIENT> CONNECTION ESTABLISHED WITH %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
				//PMB HELO, MAIL, RCPT, DATA, QUIT y RSET.
				//Inicio de la máquina de estados
				do {
					switch (state) { //The SMTP state machine is initialised
						case S_WELCOME:
							// Se recibe el mensaje de bienvenida
							break;
						case S_HELO://HELO STATE  
							printf("CLIENT> Enter the domain (press enter to exit): ");
							gets_s(input, sizeof(input));
							if (strlen(input) == 0) {//Enter to exit 
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
								state = S_QUIT;//Command quit that ends the connection with the server
							}
							else {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", HELO, input, CRLF);//Helo command to send with the domain
							}
							break;
						case S_MAIL://MAIL STATE 
							printf("CLIENT> Enter the sender (press enter to exit): ");
							gets_s(input, sizeof(input));
							if (strlen(input) == 0) {//Enter to exit
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
								state = S_QUIT;
							}
							else
								sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", MAIL, input, CRLF); //Mail command to send with the destination
							break;
						case S_RCPT://RCPT STATE 
							do {
								int correct_input = 1;
								printf("Client> Indicates the number of recipients you wish to send the mail to : ");//You are asked for the number of recipients to whom you want to send the mail.
								gets_s(n_recipient, sizeof(n_recipient));
								for (int i = 0; n_recipient[i] != '\0'; i++) {// Loop through each character in the string until the of the string(the null terminator)
									if (!isdigit(n_recipient[i])) {
										correct_input = 0; 
										break;//IF there are any character or something different it exit the for 
									}
								}

								if (!correct_input) {//If the string hasnt only numbers
									printf("Invalid input: Please enter a valid number of recipients (only numbers allowed).\n");
								}
								else {
									num_recipients = atoi(n_recipient);

									//It has to be more than 1 and a natural number
									if (num_recipients <= 0) {
										printf("Invalid number of recipients. The number must be a positive integer.\n");
									}
									else {
										pmb = 1; 
									}
								}
							} while (pmb == 0);
							
							for (int i = 0; i < num_recipients; i++) {//For loop where the recipient are asked according to those indicated above.
								printf("CLIENT> Enter the recipient (enter to exit): ");
								gets_s(input, sizeof(input));
								if (strlen(input) == 0) {//Enter to exit 
									sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
									state = S_QUIT;
								}
								else {
									sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", RCPT, input, CRLF);//RCPT command to send with the recipient
									enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
									if (enviados == SOCKET_ERROR) {
										state = S_QUIT;
										continue;// La sentencia continue hace que la ejecución dentro de un
										// bucle salte hasta la comprobación del mismo.
									}
								}
							}
							printf("CLIENT> Press Enter to continue or type RESET if you want to reset the SMTP session: ");//If want to reset the session the state change to S_RESET
							gets_s(input, sizeof(input));

							if (strcmp(input, "RESET") == 0) {
								state = S_RESET;
							
							}
							else {
								printf("CLIENT> Continuing with the current session.\n");//If not continue to prepare the email
							}
							
							break;

					
						case S_DATA:// DATA STATE 
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", DATA, CRLF);//DATA command to send
							break;

						case S_QUIT://QUIT STATE
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);//QUIT command to close the connection with the server
							break;

						case S_RESET://RESET STATE
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", RESET, CRLF);//RESET command to send
							break;

						case S_MESSAGE:
							
							printf("CLIENT > Enter the subject: ");
							gets_s(subject, sizeof(subject));
							if (strlen(subject) == 0) {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
								state = S_QUIT;

							}
							else {
								int p = 0;
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", SUBJECT, subject, CRLF);//SUBJECT to send
								do {
									printf("CLIENT > Enter a new line or use only  a dot (.) to end the email \r\n");
									gets_s(input, sizeof(input));
									if (strcmp(input, ".") == 0) {
										p = 1; //If write only a dot the body is finish and is going to be send
									}
									else if (strlen(input) > 998) {

										printf("CLIENT > You have exceeded the maximum number of characters allowed per line in SMTP, please enter less than 1000.");
									}
									sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
								} while (p == 0);	
							}
							break;
					}

					if (state != S_WELCOME && state != S_RCPT) {
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
							printf("CLIENTE> Error %d in receiving data\r\n", error);
							state = S_QUIT;
						}
						else {
							printf("CLIENT> Connection to server closed\r\n");
							state = S_QUIT;
						}
					}
					else {
						buffer_in[recibidos] = 0x00;
						printf(buffer_in);
						char code[1024];
						strncpy_s(code, sizeof(code), buffer_in, 3); // Store the first 3 characters of the SMTP response codes (220, 250, ...)
						code[3] = 0;
						if (state == S_WELCOME) {
							if (strcmp(code, "220") == 0) { // OK response from the server
								state = S_HELO;
							}
							else {
								printf("Unrecognized response: %s\n", code); // Unrecognized command response
								state = S_QUIT;
							}
						}
						else if (state == S_HELO) {
							if (strcmp(code, "250") == 0) { // OK response from the server
								state = S_MAIL;
							}
							else if (code[0] == "4") { // 4XX response means temporary failure, e.g. 421 service unavailable
								printf("Transient error: %s\n", code);
								state = S_QUIT;
							}
							else if (code[0] == "5") { // 5XX response means permanent failure, syntax error or bad command parameters
								printf("Permanent error: %s\n", code);
								state = S_QUIT;
							}
							else {
								printf("Unrecognized response: %s\n", code); // Unrecognized command response
								state = S_QUIT;
							}
						}
						else if (state == S_MAIL) {
							if (strcmp(code, "250") == 0) { // OK response from the server
								state = S_RCPT;
							}
							else if (code[0] == "4") { // 4XX response means temporary failure, e.g. 421 service unavailable
								printf("Transient error: %s\n", code);
								state = S_QUIT;
							}
							else if (code[0] == "5") { // 5XX response means permanent failure, syntax error or bad command parameters
								printf("Permanent error: %s\n", code);
								state = S_QUIT;
							}
							else {
								printf("Unrecognized response: %s\n", code); // Unrecognized command response
								state = S_QUIT;
							}
						}
						else if (state == S_RCPT) {
							if (strcmp(code, "250") == 0) { // OK response from the server
								state = S_DATA;
							}
							else if (code[0] == "4") { // 4XX response means temporary failure, e.g. 421 service unavailable
								printf("Transient error: %s\n", code);
								state = S_QUIT;
							}
							else if (code[0] == "5") { // 5XX response means permanent failure, syntax error or bad command parameters
								printf("Permanent error: %s\n", code);
								state = S_QUIT;
							}
							else {
								printf("Unrecognized response: %s\n", code); // Unrecognized command response
								state = S_QUIT;
							}
						}
						else if (state == S_RESET) {
							if (strcmp(code, "250") == 0) { // OK response from the server
								state = S_HELO;
							}
							else {
								printf("Unrecognized response: %s\n", code); // Unrecognized command response
								state = S_QUIT;
							}
						}
						else if (state == S_DATA) {
							if (strcmp(code, "354") == 0) { // OK response from the server
								state = S_MESSAGE;
							}
							else if (code[0] == "4") { // 4XX response means temporary failure, e.g. 421 service unavailable
								printf("Transient error: %s\n", code);
								state = S_QUIT;
							}
							else if (code[0] == "5") { // 5XX response means permanent failure, syntax error or bad command parameters
								printf("Permanent error: %s\n", code);
								state = S_QUIT;
							}
							else {
								printf("Unrecognized response: %s\n", code); // Unrecognized command response
								state = S_QUIT;
							}

						}
						else if (state == S_MESSAGE) {
							if (strcmp(code, "250") == 0) { // OK response from the server
								state = S_MAIL;
							}
							else if (code[0] == "4") { // 4XX response means temporary failure, e.g. 421 service unavailable
								printf("Transient error: %s\n", code);
								state = S_QUIT;
							}
							else if (code[0] == "5") { // 5XX response means permanent failure, syntax error or bad command parameters
								printf("Permanent error: %s\n", code);
								state = S_QUIT;
							}
							else {
								printf("Unrecognized response: %s\n", code); // Unrecognized command response
								state = S_QUIT;
							}
						}

						
						
					}

				} while (state != S_QUIT);
			}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR TO CONNECT WITH %s:%d\r\n", ipdest, TCP_SERVICE_PORT);
			}
			closesocket(sockfd);

		}
		printf("-----------------------\r\n\r\nCLIENT> Reconnect? (Y/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	return(0);
}
