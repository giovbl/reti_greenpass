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

    if(argc != 2)
    {
        fprintf(stderr,"Numero di argomenti non valido\n");
        fprintf(stderr,"Utilizzo:\n centro_vaccinale <numero porta>\n");
        exit(1);
    }

    //Creazione socket
    if((server_sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        fprintf(stderr,"Errore di creazione della socket\n");
        exit(1);
    }

    //Impostazione indirizzamento
    sa_server.sin_family = AF_INET;
    sa_server.sin_port = htons(atoi(argv[1]));
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
            return 0;
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
            return 0;
        }

        /*
            Si distacca il thread per fare in modo che non si
            debba aspettare la terminazione dello stesso
        */
        if(pthread_detach(thread) < 0)
        {
            fprintf(stderr,"Errore durante il distaccamento del thread per la gestione della connessione\n");
            close(server_sock);
            return 0;
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
        }

        pthread_exit(NULL);
    }

    /*
        La lettura della richiesta è andata a buon fine
        Si procede all'elaborazione della stessa
    */
    switch(request.req_code)
    {
        case VERIFY:
        case INVALIDATE:
        case VALIDATE:
        case EXTENDED_CREATION:
            //Funzionalità non implementata, si procede a rispondere con un codice d'errore
            response = NOT_IMPLEMENTED;
            break;

        case BASIC_CREATION:

            int _clientSock;
            struct sockaddr_in sa_client;

            //Creazione socket
            if((_clientSock=socket(AF_INET,SOCK_STREAM,0)) < 0)
            {
                fprintf(stderr,"Errore di creazione della socket\n");
                response = SERVER_ERROR;
                break;
            }

            //Impostazione indirizzamento
            sa_client.sin_family = AF_INET;
            sa_client.sin_port = htons(SERVER_V);

            //Settaggio indirizzo di connessione per la socket
            if(inet_pton(AF_INET,IP_ADDRESS,&sa_client.sin_addr) < 0)
            {
                fprintf(stderr,"Errore di configurazione della socket\n");
                response = SERVER_ERROR;
                break;
            }

            //Connessione al serverH
            if(connect(_clientSock,(struct sockaddr*)&sa_client,sizeof(sa_client)) < 0)
            {
                fprintf(stderr,"Errore di connessione al serverH\n");
                response = SERVER_ERROR;
                close(_clientSock);
                break;
            }
                
            /*
                Modifica del codice dell'operazione con quello opportuno 
                per il serverH (extended creation) prima dell'invio della richiesta
            */
            request.req_code = EXTENDED_CREATION;

            //Invio richiesta al serverH
            if(write(_clientSock,&request,sizeof(request)) < 0)
            {
                fprintf(stderr,"Errore nell'invio della richiesta al serverH\n");
                response = SERVER_ERROR;
                close(_clientSock);
                break;
            }

            //Ricezione risposta dal serverH
            if(read(_clientSock,&response,sizeof(ResponseCode)) < 0)
            {
                fprintf(stderr,"Errore nella ricezione della risposta dal serverH\n");
                response = SERVER_ERROR;
                close(_clientSock);
                break;
            }

            if(response == SEND_AD)
            {
                /*
                    Sono richiesti dati addizionali.
                    Si procede ad inviare la data di scadenza del green pass
                */
                time_t ct = time(NULL);
                struct tm* tm = localtime(&ct);
                tm->tm_mon += 6; //La scadenza è di 6 mesi dalla creazione
                time_t scadenza = mktime(tm);

                //Invio dati addizionali (scadenza del green pass) al serverH
                if(write(_clientSock,&scadenza,sizeof(time_t)) < 0)
                {
                    fprintf(stderr,"Errore nell'invio della richiesta al serverH\n");
                    response = SERVER_ERROR;
                    close(_clientSock);
                    break;
                }

                //Ricezione risposta dal serverH
                if(read(_clientSock,&response,sizeof(ResponseCode)) < 0)
                {
                    fprintf(stderr,"Errore nella ricezione della risposta dal serverH\n");
                    response = SERVER_ERROR;
                }

                //Chiusura della connessione con il serverH
                close(_clientSock);
            }
            else
            {
                /*
                    Non sono stati richiesti dati addizionali.
                    La richiesta non è stata gestita correttamente dal serverH.
                    Si procede a dichiarare un'errore lato server
                */
                if(response < 0)
                    fprintf(stderr,"Ricevuta una risposta di errore dal serverH\n");
                else
                    fprintf(stderr,"Errore nella ricezione della risposta dal serverH: risposta non attesa\n");

                response = SERVER_ERROR;
                close(_clientSock);
                break;
            }

            break;


        default:
            //Il codice non è valido, si procede a rispondere con un codice d'errore
            response = INVALID_DATA;
            break;
    }

    //Invio della risposta
    if(write(conn,(ResponseCode*)&response,sizeof(ResponseCode)) < 0)
    {
        /*
            fallimento nell'invio della risposta
            Si considera la connessione come fallita
        */
        fprintf(stderr,"Errore nell'invio della risposta'\n");
        close(conn);
        pthread_exit(NULL);
    }

    close(conn);
    pthread_exit(NULL);
}
