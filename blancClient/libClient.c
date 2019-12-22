// version 1.0 07/12/2014
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libClient.h"

#define CLIENT_DECONNECTE         -1
#define CLIENT_CONNECTE            0

typedef uint32_t Identificateur;

static int SDL_net_init = 0;
static TCPsocket sock = NULL;
static Identificateur id = 0;
static int status = CLIENT_DECONNECTE;

int serveurStatus(void) {
    return status;
}

static int octetHexLire(char *s) {
    int n;
    int c = s[0];

    if ((c>='0') && (c<='9')) n = c - '0';
    else if ((c>='A') && (c<='F')) n = c + 10 - 'A';
    else return -1;
    c = s[1];
    if ((c>='0') && (c<='9')) n = n * 16 + c - '0';
    else if ((c>='A') && (c<='F')) n = n * 16 + c + 10 - 'A';
    else return -1;
    return n;
}

static int16_t bits16HexLire(char *s) {
    int h = octetHexLire(s);
    if (h<0) return -1;
    int l = octetHexLire(s+2);
    if (l<0) return -1;
    int16_t v = h*256+l;
    return v;
}


static int lireId(Identificateur *pid, char *strId) {
    int i, k;
    char c;

    for (i=0; i<8; i++) {
        c = strId[i];
        if ((c>='0') && (c<='9')) k = c - '0';
        else if ((c>='A') && (c<='F')) k = c + 10 - 'A';
        else return 0;
        *pid = (*pid)*16 + k;
    }
    return 1;
}

static int serveurRecevoir(char *msg, int *pn, int attente) {

    SDLNet_SocketSet set;
    int n, max=*pn-1;

    set=SDLNet_AllocSocketSet(1);
    if (!set) {
        printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
        return 0;
    }
    if (SDLNet_TCP_AddSocket(set, sock) == -1) {
        printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
        SDLNet_FreeSocketSet(set);
        return 0;
    }

    n = SDLNet_CheckSockets(set, attente); // attente en ms
    if (n==-1) {
        printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
        SDLNet_FreeSocketSet(set);
        return 0;
    } else if (n) {
        if (SDLNet_SocketReady(sock)) {
            n = SDLNet_TCP_Recv(sock,msg,max);
            if (n<=0) {
                SDLNet_FreeSocketSet(set);
                return 0;
            }
            msg[n] = '\0';
            *pn = n;
            /*
            char buf[1024];
            int i;
            for (i=0; i<n; i++) buf[i] = (msg[i]=='\0') ? '_' : msg[i];
            buf[n] = '\0';
            printf("Reception %d/%d : %s\n", *pn, max+1, buf);
            */
//printf("Reception %d/%d : %s\n", *pn, max+1, msg);
        } else {
            SDLNet_FreeSocketSet(set);
            return 0;
        }
    } else {
        SDLNet_FreeSocketSet(set);
        return 0;
    }

    SDLNet_FreeSocketSet(set);
    return 1;
}

static int serveurEnvoyer(char *msg) {
    char *p;
    int n;
    int env;

    if (status == CLIENT_DECONNECTE) {
        fprintf(stderr, "Non connecte\n");
        return 0;
    }
    p = malloc(strlen(msg)+15);
    if (!p) return 0;
    n = strlen(msg) + 13;
    sprintf(p, "%08X%04X%s", id, n, msg);
    env = SDLNet_TCP_Send(sock, p, n);
// printf("Emission %d : %s\n", n, p);
    free(p);
    if (env < n) {
        fprintf(stderr, "Erreur d'envoi\n");
        return 0;
    }
    return 1;
}

int serveurNomRobot(char *nom) {
    char msg[17];
    int n;
    strncpy(&msg[1], nom, 15);
    msg[0] = 'N';
    msg[16] = '\0';
    if (serveurEnvoyer(msg)) {
        n = 255;
        if (serveurRecevoir(msg, &n, 1000)) {
            if (msg[0] == '1') {
                fprintf(stderr, "Modification du nom du robot\n");
                return 1;
            }
        }
    }
    fprintf(stderr, "Nom du robot non pris en compte\n");
    return 0;
}

int serveurCaracRobot(int rapidite, int precision, int force, int endurance) {
    char msg[6];
    int n;
    msg[0] = 'C';
    msg[1] = rapidite + '0';
    msg[2] = precision + '0';
    msg[3] = force + '0';
    msg[4] = endurance + '0';
    msg[5] = '\0';
    if (serveurEnvoyer(msg)) {
        n = 255;
        if (serveurRecevoir(msg, &n, 1000)) {
            if (msg[0] == '1') {
                fprintf(stderr, "Modification des caracteristiques du robot\n");
                return 1;
            }
        }
    }
    fprintf(stderr, "Modification des caracteristiques non prise en compte\n");
    return 0;
}

int serveurConnecter(const char *adresse, unsigned short int port) {
    int n;
    char buffer[255];
    IPaddress ip;

    if (!SDL_net_init) {
        if (SDLNet_Init()==-1) {
            printf("SDLNet_Init: %s\n", SDLNet_GetError());
            exit(2);
        }
        atexit(SDLNet_Quit);
        SDL_net_init = 1;
    }

    if (sock) serveurFermer();
    if (SDLNet_ResolveHost(&ip,adresse,port)==-1) {
//        printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return SERVEUR_INTROUVABLE;
    }

    sock=SDLNet_TCP_Open(&ip);
    if (!sock) {
//        printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
        return SERVEUR_INTROUVABLE;
    }

    n = 255;
    serveurRecevoir(buffer, &n, 100);
    if (n <= 0) {
        serveurFermer();
        return SERVEUR_ERREUR_ID;
    }
    buffer[n] = '\0';
//    printf("%d %s\n", (int)strlen(buffer), buffer);
    if (!lireId(&id, buffer)) {
        printf("id invalide\n");
        serveurFermer();
        return SERVEUR_ERREUR_ID;
    }
    status = CLIENT_CONNECTE;
    return SERVEUR_CONNECTE;
}

int serveurFermer(void) {
    if (sock) SDLNet_TCP_Close(sock);
    sock = NULL;
    status = CLIENT_DECONNECTE;
    return 1;
}

int serveurDemarrerMatch(void) {
    return serveurEnvoyer("S");
}

int serveurAvancer(void) {
    return serveurEnvoyer("A");
}

int serveurReculer(void) {
    return serveurEnvoyer("R");
}

int serveurSAccroupir(void) {
    return serveurEnvoyer("B");
}

int serveurLancer(double energie, double angle) {
    char msg[20];
    sprintf(msg, "L%04hX%04hX", (short)(energie*100), (short)((angle+90)*100));
    return serveurEnvoyer(msg);
}

int serveurSeRelever(void) {
    return serveurEnvoyer("E");
}

int serveurRassemblerNeige(void) {
    return serveurEnvoyer("N");
}

int serveurCompacterNeige(int neige) {
    char msg[10];
    if (neige>50) neige = 50; else if (neige<0) neige=0;
    sprintf(msg, "O%02X", neige & 0xFF);
    return serveurEnvoyer(msg);
}

int serveurNeRienChanger(void) {
    return serveurEnvoyer("P");
}

int serveurStopperAction(void) {
    return serveurEnvoyer("S");
}

int serveurRecevoirSituation(Jeu *pJeu, Moi *pMoi, Adversaire *pAdversaire, int *pNbBoules, Boule *pBoules) {
    char msg[1024];
    int n=1023, index=0;

    if (!serveurRecevoir(msg, &n, 1000)) return 0;
    pJeu->etat = msg[index++] - '0';
    pJeu->nbJoueursCnx = msg[index++] - '0';
    pJeu->ilNeige = msg[index++] - '0';
    pJeu->chrono = bits16HexLire(&msg[index])*10;
    index += 4;
    pJeu->ventX = bits16HexLire(&msg[index]);
    index += 4;
    pJeu->ventY = bits16HexLire(&msg[index]);
    index += 4;
    pJeu->hauteurMur = bits16HexLire(&msg[index]);
    index += 4;
    pMoi->reponse =  msg[index++] - '0';
    pMoi->bonnet =  msg[index++] - '0';
    pMoi->nbBoule =  msg[index++] - '0';
    pMoi->neigeRassemblee =  msg[index++] - '0';
    pMoi->x = bits16HexLire(&msg[index]);
    index += 4;
    pMoi->etat = octetHexLire(&msg[index]);
    index += 2;
    pMoi->neigeDispo = bits16HexLire(&msg[index]);
    index += 4;
    pMoi->ptsDeVie = octetHexLire(&msg[index]);
    index += 2;
    pMoi->score = bits16HexLire(&msg[index]);
    index += 4;
    pMoi->blackOut = octetHexLire(&msg[index])*20;
    index += 2;
    pAdversaire->rapidite = msg[index++] - '0';
    pAdversaire->precision = msg[index++] - '0';
    pAdversaire->force = msg[index++] - '0';
    pAdversaire->endurance = msg[index++] - '0';
    pAdversaire->bonnet = msg[index++] - '0';
    pAdversaire->nbBoule = msg[index++] - '0';
    pAdversaire->x = bits16HexLire(&msg[index]);
    index += 4;
    pAdversaire->etat = octetHexLire(&msg[index]);
    index += 2;
    pAdversaire->neigeDispo = bits16HexLire(&msg[index]);
    index += 4;
    pAdversaire->ptsDeVie = octetHexLire(&msg[index]);
    index += 2;
    pAdversaire->score = bits16HexLire(&msg[index]);
    index += 4;
    pAdversaire->blackOut = octetHexLire(&msg[index])*20;
    index += 2;

    *pNbBoules = 0;
    while (msg[index]) {
        pBoules[*pNbBoules].x = bits16HexLire(&msg[index]);
        index += 4;
        pBoules[*pNbBoules].y = bits16HexLire(&msg[index]);
        index += 4;
        pBoules[*pNbBoules].vx = bits16HexLire(&msg[index]);
        index += 4;
        pBoules[*pNbBoules].vy = bits16HexLire(&msg[index]);
        index += 4;
        (*pNbBoules)++;
    }
    return 1;
}

int serveurUpload(int num, char *nom) {
    FILE *f;
    char msg[1024];
    int env, c;

    f = fopen(nom, "rb");
    if (!f) {
        fprintf(stderr, "Impossible de charger %s\n", nom);
        return 0;
    }
    sprintf(msg, "I%d", num);
    serveurEnvoyer(msg);
    int n = 0;
    do {
        c = fgetc(f);
        if (c != EOF) {
            n++;
            msg[n+12] = c;
            if (n == 512) {
                sprintf(msg, "%08X%04X", id, n+13);
                env = SDLNet_TCP_Send(sock, msg, n+13);
                if (env < n+13) {
                    fprintf(stderr, "Erreur d'envoi\n");
                }
                n = 0;
            }
        }
    } while (c != EOF);
    if (n != 0) {
        sprintf(msg, "%08X%04X", id, n+13);
        env = SDLNet_TCP_Send(sock, msg, n+13);
        if (env < n+13) {
            fprintf(stderr, "Erreur d'envoi\n");
        }
    }
    sprintf(msg, "%08X000D", id);
    env = SDLNet_TCP_Send(sock, msg, 13);
    if (env < 13) {
        fprintf(stderr, "Erreur d'envoi\n");
    }
    fclose(f);
    n = 255;
    if (serveurRecevoir(msg, &n, 1000)) {
        if (msg[0] == '1') {
            fprintf(stderr, "Upload de \"%s\" reussi\n", nom);
            return 1;
        }
    }
    fprintf(stderr, "Echec de l'upload de \"%s\"\n", nom);
    return 0;
}
