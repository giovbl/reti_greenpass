#include "funs.h"

int check_ts(char* tessera_sanitaria)
{
    /*
        Si controlla se il codice della tessera sanitaria corrisponde al
        numero giusto di caratteri
    */
    if(strlen(tessera_sanitaria) != TS_SIZE)
        return 0;

    for(int i=0; i < TS_SIZE; i++)
    {
        if(tessera_sanitaria[i] < '0' || tessera_sanitaria[i] > '9')
        {
            /*
                Nel codice della tessera sanitaria sono presenti caratteri non validi. 
                Il codice da verificare è quindi da considerarsi come non valido
            */
            return 0;
        }
    }

    return 1;
}