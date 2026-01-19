#ifndef GLOBAL_DEFINITIONS_H
#define GLOBAL_DEFINITIONS_H

/// @brief Indirizzo IP del server
#define IP_ADDRESS "127.0.0.1"

/// @brief Numero massimo di connessioni in coda
#define QUEUE_NUMBER 500

/// @brief Numero di caratteri della tessera sanitaria
#define TS_SIZE 20

/// @brief Enum rappresentate il numero di porte dei server
typedef enum
{
    SERVER_V = 1030,
    SERVER_G = 1031
} SERVER_PORTS;

#endif