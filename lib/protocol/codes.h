#ifndef PROTOCOL_CODES_H
#define PROTOCOL_CODES_H

/// @brief Enum raprresentante il codice dell'operazioni di richiesta
typedef enum
{
    /// @brief Operazione di verifica di un green pass
    VERIFY = 0,

    /// @brief Operazione di invalidazione di un green pass
    INVALIDATE = 1,

    /// @brief Operazione di validazione di un green pass
    VALIDATE = 2,

    /// @brief Operazione di creazione base (no dati addizionali) di un green pass
    BASIC_CREATION = 3,

    /// @brief Operazione di creazione con dati addizionali di un green pass
    EXTENDED_CREATION = 4

} RequestCode;

/// @brief Enum raprresentante il codice di risposta
typedef enum
{
    /// @brief Errore del server
    SERVER_ERROR = -3,

    /// @brief Dati invalidi
    INVALID_DATA = -2,

    /// @brief Operazione non implementata
    NOT_IMPLEMENTED = -1,

    /// @brief Operazione avvenuta con successo con risultato di non validità
    INVALID = 0,

    /// @brief Operazione avvenuta con successo con risultato di validità
    VALID = 1,

    /// @brief L'operazione richiede l'invio di dati addizionali
    SEND_AD = 2
    
} ResponseCode;

#endif