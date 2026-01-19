#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "../lib/global/definitions.h"
#include "../lib/global/funs.h"
#include "../lib/protocol/codes.h"
#include "../lib/protocol/socket_data.h"
#include "../lib/gp_storage/green_pass_storage.h"


/**
 * @brief 
 * Memoria condivisa tra il thread per la gestion della connessione e 
 * quello per la gestione del client
 */
struct
{
    /// @brief Mutex per la gestione dell'accesso al file memorizzante i green pass
    pthread_mutex_t gps_mutex;

} _shared_mem = {PTHREAD_MUTEX_INITIALIZER};

/**
 * @brief Gestisce le fasi del protocollo di una singola connessione al server
 * 
 * @param conn_sock Socket della connessione
 * 
 * @return void* Nessun valore è ritornato
 */
void* connection_handler(void* conn_sock);

int main(int argc,char **argv)
{
    int server_sock;
    struct sockaddr_in sa_server;

    //Creazione socket
    if((server_sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        fprintf(stderr,"Errore di creazione della socket\n");
        exit(1);
    }

    //Impostazione indirizzamento
    sa_server.sin_family = AF_INET;
    sa_server.sin_port = htons(SERVER_V);
    sa_server.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_sock,(struct sockaddr *)&sa_server,sizeof(sa_server)) < 0)
    {
        fprintf(stderr,"Errore nella bind del server\n");
        exit(1);
    }

    //Apertura del server all'ascolto
    if(listen(server_sock,QUEUE_NUMBER) < 0)
    {
        fprintf(stderr,"Errore durante la listen\n");
        close(server_sock);
        exit(1);
    }

    struct sockaddr_in cl_addr;
    socklen_t cl_addr_len = sizeof(cl_addr);
    int conn;

    //Gestione dell'accettazione delle connessioni
    while(1)
    {
        //Accettazione della connessione
        if((conn = accept(server_sock,(struct sockaddr *)&cl_addr,&cl_addr_len)) < 0)
        {
            fprintf(stderr,"Errore durante la accept\n");
            close(server_sock);
            exit(1);
        }

        //Logging della connessione accettata
        char buffer[21];
        inet_ntop(AF_INET,&cl_addr.sin_addr,buffer,sizeof(buffer));
        printf("Connessione da %s, porta %d\n",buffer,ntohs(cl_addr.sin_port));

        //Creazione ed esecuzione thread per gestione della socket
        pthread_t thread;
        if(pthread_create(&thread,NULL,connection_handler,(void*)(unsigned long)conn) < 0)
        {
            fprintf(stderr,"Errore durante la creazione del thread per la gestione della connessione\n");
            close(server_sock);
            exit(1);
        }

        /*
            Si distacca il thread per fare in modo che non si
            debba aspettare la terminazione dello stesso
        */
        if(pthread_detach(thread) < 0)
        {
            fprintf(stderr,"Errore durante il distaccamento del thread per la gestione della connessione\n");
            close(server_sock);
            exit(1);
        }
    }

    return 0;
}

void* connection_handler(void* conn_sock)
{
    RequestData request;
    ResponseCode response = SERVER_ERROR;
    int conn = (int)(unsigned long) conn_sock;

    //Lettura della richiesta
    if(read(conn,&request,sizeof(RequestData)) != sizeof(RequestData) || !check_ts(request.tessera_sanitaria))
    {
        /*
            Il motivo del falliemnto della read può
            essere una richiesta malformata.
            Invio di una risposta
        */
        response = INVALID_DATA;

        if(write(conn,(ResponseCode*)&response,sizeof(ResponseCode)) < 0)
        {
            /*
                Il fallimento è un problema non derivato da 
                una richiesta malformata.
                Si considera la connessione come fallita
            */
            fprintf(stderr,"Errore nell'invio di una risposta di errore'\n");
            close(conn);
            pthread_exit(NULL);
        }
    }

    /*
        La lettura della richiesta è andata a buon fine
        Si procede all'elaborazione della stessa
    */
    if(request.req_code == BASIC_CREATION)
    {
        response = NOT_IMPLEMENTED;
    }
    else
    {
        /*
            Apertura del file contenente i green pass
        */
        if(!gps_open("greenpass.data"))
        {
            fprintf(stderr,"Errore nell'apertura del file dei green pass\n");
            response = SERVER_ERROR;
            
            //Invio della risposta
            if(write(conn,(ResponseCode*)&response,sizeof(ResponseCode)) < 0)
            {
                /*
                    fallimento nell'invio della risposta
                    Si considera la connessione come fallita
                */
                fprintf(stderr,"Errore nell'invio della risposta'\n");
                close(conn);
            }

            pthread_exit(NULL);
        }

        int res;
        //Esecuzione dell'operazione richiesta
        switch(request.req_code)
        {
            case VERIFY:

                //Attesa del proprio turno per eseguire delle operazioni sul file
                if(pthread_mutex_lock(&_shared_mem.gps_mutex) != 0)
                {
                    fprintf(stderr,"Errore durante il lock del mutex\n");
                    response = SERVER_ERROR;
                    break;
                }

                if(!gps_isValid(request.tessera_sanitaria,&res))
                {
                    fprintf(stderr,"Errore nell'operazione su file'\n");
                    response = SERVER_ERROR;
                }
                else
                    response = (res)? VALID : INVALID;

                break;

            case INVALIDATE:

                //Attesa del proprio turno per eseguire delle operazioni sul file
                if(pthread_mutex_lock(&_shared_mem.gps_mutex) != 0)
                {
                    fprintf(stderr,"Errore durante il lock del mutex\n");
                    response = SERVER_ERROR;
                    break;
                }

                if(!gps_changeValidity(request.tessera_sanitaria,GP_INVALID,&res))
                {
                    fprintf(stderr,"Errore nell'operazione su file'\n");
                    response = SERVER_ERROR;
                }
                else
                    response = (res)? VALID : INVALID;

                break;

            case VALIDATE:

                //Attesa del proprio turno per eseguire delle operazioni sul file
                if(pthread_mutex_lock(&_shared_mem.gps_mutex) != 0)
                {
                    fprintf(stderr,"Errore durante il lock del mutex\n");
                    response = SERVER_ERROR;
                    break;
                }

                if(!gps_changeValidity(request.tessera_sanitaria,GP_VALID,&res))
                {
                    fprintf(stderr,"Errore nell'operazione su file'\n");
                    response = SERVER_ERROR;
                }
                else
                    response = (res)? VALID : INVALID;

                break;

            case EXTENDED_CREATION:

                time_t scadenza;

                response = SEND_AD;
                //Invio della risposta richiedente i dati addizionali
                if(write(conn,&response,sizeof(ResponseCode)) < 0)
                {
                    fprintf(stderr,"Errore nell'invio della richiesta al Centro Vaccinale\n");
                    response = SERVER_ERROR;
                    break;
                }

                //Ricezione dati addizionali per Green Pass
                if(read(conn,&scadenza,sizeof(time_t)) < 0)
                {
                    fprintf(stderr,"Errore nella ricezione della risposta dal Centro Vaccinale\n");
                    response = SERVER_ERROR;
                    break;
                }

                //Creazione green pass da inserire
                GreenPassData gps;
                memcpy(gps.tessera_sanitaria,request.tessera_sanitaria,TS_SIZE);
                gps.scadenza = scadenza;

                //Attesa del proprio turno per eseguire delle operazioni sul file
                if(pthread_mutex_lock(&_shared_mem.gps_mutex) != 0)
                {
                    fprintf(stderr,"Errore durante il lock del mutex\n");
                    response = SERVER_ERROR;
                    break;
                }

                //Creazione del green pass
                if(!gps_addData(&gps,&res))
                {
                    fprintf(stderr,"Errore nell'operazione su file'\n");
                    response = SERVER_ERROR;
                }
                else
                    response = (res)? INVALID : VALID;

                break;
        }

        //Operazioni sul file terminate: il mutex è sbloccato
        pthread_mutex_unlock(&_shared_mem.gps_mutex);

        //Chiusura del file
        gps_close();
    }

    //Invio della risposta
    if(write(conn,(ResponseCode*)&response,sizeof(ResponseCode)) < 0)
    {
        /*
            Fallimento nell'invio della risposta
            Si considera la connessione come fallita
        */
        fprintf(stderr,"Errore nell'invio della risposta'\n");
        close(conn);
        pthread_exit(NULL);
    }

    close(conn);
    pthread_exit(NULL);
}
