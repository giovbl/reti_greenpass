#ifndef GREEN_PASS_STORAGE_H
#define GREEN_PASS_STORAGE_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "green_pass_data.h"

/**
 * @brief Apre (o crea se non esiste) un file dove memorizzare i green pass
 * 
 * @param path Il path del file in cui memorizzare i green pass
 * @return int 
 * Se l'operazione è eseguita con successo il valore restituito è 1, altrimenti 0
 */
int gps_open(char* path);

/**
 * @brief Aggiunge un green pass all'interno del file
 * 
 * @param gpd I dati del green pass da aggiungere
 * @param res 
 * Assume valore 1 se il green pass da inserire risulta già memorizzato nel file, altrimenti 0
 * @return int 
 * Se l'operazione è eseguita con successo il valore restituito è 1, altrimenti 0
 */
int gps_addData(GreenPassData* gpd,int* res);

/**
 * @brief Verifica se un green pass è valido
 * 
 * @param tessera_sanitaria Codice della tessera sanitaria associata al green pass
 * @param res Assume valore 1 se valido, 0 se invalido
 * @return int 
 * Se l'operazione è eseguita con successo il valore restituito è 1, altrimenti 0
 */
int gps_isValid(char* tessera_sanitaria,int* res);

/**
 * @brief Cambia la validità di un green pass con la validità addizionale specificata
 * 
 * @param tessera_sanitaria Codice della tessera sanitaria associata al green pass
 * @param vld Il valore da assegnare alla validità del green pass
 * @param res 
 * Assume valore 1 se il green pass è stato trovato e la validità addizionale impostata non corrisponde 
 * a quella già impostata, altrimenti 0
 * @return int
 * Se l'operazione è eseguita con successo il valore restituito è 1, altrimenti 0 
 */
int gps_changeValidity(char* tessera_sanitaria,GP_Validity vld,int*  res);

/**
 * @brief 
 * Posiziona il puntatore del file all'inizio del green pass
 * identificato dal codice della tessera sanitaria passato come parametro
 * 
 * @param tessera_sanitaria Codice della tessera sanitaria associata al green pass
 * @param res Assume valore 1 se il green pass è stato trovato, altrimenti 0
 * @return int 
 * Se l'operazione è eseguita con successo il valore restituito è 1, altrimenti 0
 */
int gps_internal_point_at(char* tessera_sanitaria,int* res);

/**
 * @brief Chiude il file in cui sono memorizzati i green pass
 * 
 * @return int 
 * Se l'operazione è eseguita con successo il valore restituito è 1, altrimenti 0
 */
int gps_close();

#endif