#ifndef PROTOCOL_DATA_H
#define PROTOCOL_DATA_H

#include <time.h>

#include "codes.h"

/// @brief Struct rappresentante la struttura dei dati di una richiesta
typedef struct
{
    /// @brief Codice di richiesta
    RequestCode req_code;

    /// @brief Codice della tessera sanitaria utilizzata per la richiesta
    char tessera_sanitaria[20];

} RequestData;


#endif