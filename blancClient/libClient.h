// version 1.0 07/12/2014
#ifndef _LIBCLIENT_H
#define _LIBCLIENT_H

#include <SDL2/SDL_net.h>

#define SERVEUR_ERREUR_ID         -1
#define SERVEUR_INTROUVABLE        0
#define SERVEUR_CONNECTE           1

#define JEU_ATTENTE                0
#define JEU_MATCH_EN_COURS         1
#define JEU_FIN_MATCH              2

#define IMAGE_DEBOUT               0
#define IMAGE_MARCHE               1
#define IMAGE_ACCROUPI             2
#define IMAGE_LANCE                3
#define IMAGE_BONNET_PROFIL        4
#define IMAGE_VICTOIRE             5
#define IMAGE_DEFAITE              6
#define IMAGE_BONNET_FACE          7

#define ROBOT_NEW                100
#define ROBOT_PARTIEL            101
#define ROBOT_COMPLET            102
#define ROBOT_FILE               103

#define ROBOT_IMMOBILE             0
#define ROBOT_AVANCE               1
#define ROBOT_RECULE               2
#define ROBOT_S_ACCROUPI           3
#define ROBOT_ACCROUPI             4
#define ROBOT_RASSEMBLE_NEIGE      5
#define ROBOT_COMPACTE_BOULE       6
#define ROBOT_SE_RELEVE            7
#define ROBOT_LANCE                8

#define ROBOT_VICTOIRE             9
#define ROBOT_DEFAITE             10

#define ROBOT_XMIN                40
#define ROBOT_XMAX               380
#define ROBOT_DX                  20

#define BOULES_NB_MAX             20

typedef struct {
    int x;
    int y;
    int vx;
    int vy;
} Boule;

typedef struct {
    int etat;
    int x;
    int bonnet;
    int nbBoule;
    int neigeDispo;
    int neigeRassemblee;
    int ptsDeVie;
    int score;
    int blackOut;
    int reponse;
} Moi;

typedef struct {
    int etat;
    int x;
    int bonnet;
    int nbBoule;
    int neigeDispo;
    int ptsDeVie;
    int score;
    int blackOut;
    int rapidite;
    int precision;
    int force;
    int endurance;
} Adversaire;

typedef struct {
    int etat;
    int nbJoueursCnx;
    int chrono;
    int ventX;
    int ventY;
    int hauteurMur;
    int ilNeige;
} Jeu;

int serveurConnecter(const char *adresse, unsigned short int port);
int serveurFermer(void);

int serveurDemarrerMatch(void);

int serveurUpload(int ROBOT_AVANCEnum, char *nom);
int serveurNomRobot(char *nom);
int serveurCaracRobot(int rapidite, int precision, int force, int endurance);

int serveurAvancer(void);
int serveurReculer(void);
int serveurSAccroupir(void);
int serveurLancer(double energie, double angle);
int serveurSeRelever(void);
int serveurRassemblerNeige(void);
int serveurCompacterNeige(int neige);
int serveurNeRienChanger(void);

int serveurStopperAction(void);

int serveurRecevoirSituation(Jeu *pJeu, Moi *pMoi, Adversaire *pAdversaire, int *pNbBoules, Boule *pBoules);

#endif
