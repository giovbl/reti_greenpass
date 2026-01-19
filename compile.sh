#!/bin/bash
#Script per la compilazione del progetto

#Creazione cartella contente i file binari (se non esiste)
mkdir bin

#Compilazione client
gcc ./src/client.c ./lib/global/funs.c -o ./bin/client

#Compilazione centro vaccinale
gcc ./src/centro_vaccinale.c ./lib/global/funs.c -o ./bin/centro_vaccinale

#Compilazione server G
gcc ./src/serverG.c ./lib/global/funs.c -o ./bin/serverG

#Compilazione server V
gcc ./src/serverV.c ./lib/global/funs.c ./lib/gp_storage/green_pass_storage.c -o ./bin/serverV