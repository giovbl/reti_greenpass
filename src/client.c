#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include "../lib/global/definitions.h"
#include "../lib/global/funs.h"
#include "../lib/protocol/codes.h"
#include "../lib/protocol/socket_data.h"


int main(int argc,char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;
    RequestData req;
    ResponseCode res;

    if((argc < 3 || argc > 4) || (atoi(argv[1]) == 3 && argc != 4))
    {
        fprintf(stderr,"Numero di argomenti non valido\n");
        printf("Utilizzo:\n client <operazione> <codice tessra sanitaria> <numero porta>\n\n");
        printf("Codici validi per <operazione>:\n");
        printf("  0 | Verifica Green Pass\n");
        printf("  1 | Invalidazione Green Pass\n");
        printf("  2 | Rivalidazione Green Pass\n");
        printf("  3 | Creazione Green Pass\n");
        printf("\n <numero porta> richiesto solo se si sceglie l'operazione 3\n");
        exit(1);
    }
    
    //Ottenimento operazione da effettuare
    int scelta = atoi(argv[1]);

    //Impostazione porta del server
    if(scelta >= 0 && scelta <= 2)
    {
        servaddr.sin_port=htons(SERVER_G);
    }
    else
    {
        /*
            Il client si deve collegare ad un centro vaccinale.
            Bisogna impostare la porta utilizzata dal centro vaccinale
        */

        servaddr.sin_port=htons(atoi(argv[3]));
    }

    //Verifica correttezza sintattica della tessera sanitaria
    if(!check_ts(argv[2]))
    {
        fprintf(stderr,"Codice della tessera sanitaria non valido\n");
        exit(1);
    }

    //Creazione socket
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
        fprintf(stderr,"Errore di creazione della socket\n");
        exit(1);
    }
 
    //Impostazione famiglia socket
    servaddr.sin_family=AF_INET;

    //Settaggio indirizzo di connessione per la socket
    if(inet_pton(AF_INET,IP_ADDRESS,&servaddr.sin_addr) < 0)
    {
        fprintf(stderr,"Errore di configurazione della socket\n");
        exit(1);
    }

    //Connessione al server
    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
    {
        fprintf(stderr,"Errore di connessione\n");
        exit(1);
    }

    //Creazione della richiesta
    req.req_code = scelta;
    memcpy(req.tessera_sanitaria,argv[2],TS_SIZE);

    //Invio richiesta
    if(write(sockfd,&req,sizeof(RequestData)) != sizeof(RequestData))
    {
        fprintf(stderr,"Errore nell'invio della richiesta\n");
        close(sockfd);
        exit(1);
    }

    //Ottenimento risposta
    if(read(sockfd,&res,sizeof(ResponseCode)) < 0)
    {
        fprintf(stderr,"Errore nella ricezione della risposta\n");
        close(sockfd);
        exit(1);
    }

    /*
        Decisione dell'azione da intraprendere
        in base alla risposta
    */
    switch(res)
    {
        case SERVER_ERROR:
            printf("Errore di elaborazione della richiesta da parte del server\n");
            break;

        case INVALID_DATA:
            printf("Errore nell'invio della richiesta: dati non validi\n");
            break;

        case NOT_IMPLEMENTED:
            printf("Errore nell'invio della richiesta: funzione non implementata\n");
            break;

        case INVALID:
            switch(req.req_code)
            {
                case BASIC_CREATION:
                    printf("La tessera sanitaria è già associata ad un green pass\n");
                    break;

                case VALIDATE:
                    printf("Impossibile validare il green pass: non esistente o già valido\n");
                    break;

                case INVALIDATE:
                    printf("Impossibile invalidare il green pass: non esistente o già invalido\n");
                    break;
            }
            break;

        case VALID:
            switch(req.req_code)
            {
                case BASIC_CREATION:
                    printf("La tessera sanitaria è stata creata con successo\n");
                    break;

                case VALIDATE:
                    printf("La validazione del green pass è stata effettuata con successo\n");
                    break;

                case INVALIDATE:
                    printf("L'invalidazione del green pass è stata effettuata con successo\n");
                    break;
            }
            break;

        case SEND_AD:
            printf("Errore nella ricezione della risposta: nessun dato addizionale da inviare\n");
            break;

        default:
            printf("Errore nella ricezione della risposta: valore di risposta non atteso\n");
            break;
    }


    exit(1);
    
}