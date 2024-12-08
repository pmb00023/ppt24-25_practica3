/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. de Ingeníería de Telecomunicación
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 1
 * Fichero: protocol.c
 * Versión: 4.0
 * Curso: 2024/2025
 * Descripción: Fichero de encabezado para práctica 1
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * Alumno 1:
 * Alumno 2:
 *
 ******************************************************/
#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define HELO "HELO"
#define MAIL "MAIL FROM:"
#define RCPT "RCPT TO:"
#define DATA "DATA"
#define SC "USER"
#define PW "PASS"
#define SD  "QUIT"
#define RESET "RESET"
#define MESSAGE "MESSGAE"
#define ECHO "ECHO"


// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define ER  "ER"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//PMB HELO, MAIL, RCPT, DATA, QUIT y RSET.
//ESTADOS
#define S_WELCOME 0
#define S_HELO 1
#define S_MAIL 2
#define S_RCPT 3
#define S_DATA 4
#define S_QUIT 5
#define S_RESET 6
#define S_MESSAGE 7


//PUERTO DEL SERVICIO
#define TCP_SERVICE_PORT	25

// NOMBRE Y CLAVE AUTORIZADOS
// Nota: esto no se debe hacer nunca, es solamente para simplificar el funcionamiento del
// servidor, el cual debería almacenar estos datos en una base de datos de forma segura.
#define USER		"user" 
#define PASSWORD	"1234"