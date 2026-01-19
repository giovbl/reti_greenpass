#include "green_pass_storage.h"

///@brief Puntatore al file in cui sono memorizzati i green pass
FILE* _gpstorage;

int gps_open(char* path)
{
    //Creazione del file se non esistente
    if(access(path, F_OK) != 0)
    {
        FILE* file = fopen(path, "wb");

        if(file == NULL)
            return 0;

        fclose(file);
    }

    //Apertura del file
    return ((_gpstorage = fopen(path,"rb+")) != NULL);
}

int gps_addData(GreenPassData* gpd,int *res)
{
    
    //Verifica dell'esistenza del green pass da inserire
    if(!gps_internal_point_at(gpd->tessera_sanitaria,res))
        return 0;

    if(res)
    {
        /*
            Il green pass che si vuole inserire è stato già
            precedentemente memorizzato.
            Non bisogna quindi crearne uno nuovo.
        */

       return 1;
    }

    //Posizionamento del puntatore alla fine del file
    if(fseek(_gpstorage,0,SEEK_END) != 0)
        return 0;

    //Aggiunta del green pass al file
    if(fwrite(gpd,sizeof(GreenPassData),1,_gpstorage) != 1)
    {
        if(!feof(_gpstorage))
            return 0;
    }
    
    return 1;
}

int gps_internal_point_at(char* tessera_sanitaria,int* res)
{
    GreenPassData buffer;

    //Posizionamento del puntatore all'inizio del file
    if(fseek(_gpstorage,0,SEEK_SET) != 0)
        return 0;

    //Posizionamento del puntatore al green pass interessato
    while(1)
    {
        //Lettura del record
        if(fread(&buffer,sizeof(GreenPassData),1,_gpstorage) != 1)
        {
            if(!feof(_gpstorage))
            {
                /*
                    Si è verificato un problema durante la lettura.
                */
                return 0;
            }
            else
            {
                /*
                    Si è arrivati alla fine del file
                    L'ultimo green pass non corrisponde a quello cercato
                    Si dichiara quindi che la ricerca non ha riscontrato risultati
                */
                res = 0;
                break;
            }
        }

        //Comparazione dei codice della T.S. del record con quello passato come parametro
        if(strcmp(buffer.tessera_sanitaria,tessera_sanitaria) == 0)
        {
            /*
                Il record è stato trovato.
                Si procede a posizionare correttamente il puntatore.
            */

            //Posizionamento del puntatore all'inizio del green pass interessato
            if(fseek(_gpstorage,-sizeof(GreenPassData),SEEK_CUR) != 0)
                return 0;

            *res = 1;
            break;
        }

    }

    return 1;
}

int gps_isValid(char* tessera_sanitaria,int* res)
{
    GreenPassData gp;

    //Posizionamento al green pass da verificare
    if(!gps_internal_point_at(tessera_sanitaria,res))
        return 0;
    
    if(!*res)
    {
        /*
            Il green pass non è stato trovato.
        */
        return 1;
    }

    //Lettura dei dati del green pass da verificare
    if(fread(&gp,sizeof(GreenPassData),1,_gpstorage) != 1)
    {
        if(!feof(_gpstorage))
            return 0;
    }

    //Verifica della validità del green pass
    *res = (/*gp.scadenza <= time(NULL) &&*/ gp.validity == GP_VALID);

    return 1;
}

int gps_changeValidity(char* tessera_sanitaria,GP_Validity vld,int* res)
{
    GreenPassData gp;

    //Posizionamento al green pass da modificare
    if(!gps_internal_point_at(tessera_sanitaria,res))
        return 0;

    if(!*res)
    {
        /*
            Il green pass non è stato trovato.
        */
        return 1;
    }

    //Lettura dei dati del green pass da modificare
    if(fread(&gp,sizeof(GreenPassData),1,_gpstorage) != 1)
    {
        if(!feof(_gpstorage))
            return 0;
    }


    /*
        Si verifica se il nuovo valore di validità non corrisponde 
        a quello già memorizzato
    */
    if(gp.validity == vld)
    {
        res = 0;
        return 1;
    }

    //Cambio della validità con il valore specificato come parametro
    gp.validity = vld;

    //Riposizionamento del puntatore per la scrittura delle modifiche effettuate
    if(fseek(_gpstorage,-sizeof(GreenPassData),SEEK_CUR) != 0)
        return 0;
    
    //Sovrascrittura del green pass con le modifiche apportate
    if(fwrite(&gp,sizeof(GreenPassData),1,_gpstorage) != 1)
    {
        if(!feof(_gpstorage))
            return 0;
    }

    return 1;
}

int gps_close()
{
    return (fclose(_gpstorage) == 0);
}