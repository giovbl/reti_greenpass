#ifndef GREEN_PASS_DATA_H
#define GREEN_PASS_DATA_H

#include <time.h>

/// @brief Enum rappresentante la validità aggiuntiva di un green pass
typedef enum
{
    /// @brief Green pass valido
    GP_VALID,

    /// @brief Green pass non valido
    GP_INVALID
    
} GP_Validity;

/// @brief Struct rappresentate un green pass
typedef struct
{
    /// @brief Codice della tessera sanitaria associata al green pass
    char tessera_sanitaria[20];

    /// @brief Scadenza del green pass
    time_t scadenza;

    /// @brief Validità aggointiva del green pass
    GP_Validity validity;

} GreenPassData;

#endif