#ifndef GLOBAL_FUNS_H
#define GLOBAL_FUNS_H

#include <string.h>

#include "definitions.h"

/**
 * @brief Verifica se il codice della tessera sanitaria è sintatticamente valido
 * 
 * @param tessera_sanitaria Codice da verificare
 * @return int Ritorna 1 se il codice è valido, altrimenti 0
 */
int check_ts(char* tessera_sanitaria);

#endif