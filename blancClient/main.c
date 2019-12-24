#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libClient.h"

int repositionnement (int position, int objectif, int sousEtat);
int CompactageNeige(char *etatRobot, int neigerassemble, int boule, int sousEtat);
void calculeEnergieLancer(int positionMoi, int positionAdversaire, int ventX, int ventY, int hauteurMur, int forceAngleLancer[]);

int main (int argc, char **argv)
{
    Jeu jeu;
    Moi moi;
    Adversaire adversaire;
    int nbBoules;
    Boule boule[BOULES_NB_MAX];

    int position;
    int positionvise=170;
    int etat=1, sousEtat=0;
    int forceAngleLancer[2];
    int forceLancer, angleLancer;

     // adresse IP du serveur sous forme de chaine de caracteres
    char adresse[255] = "192.168.1.24";
    // numero du port du serveur
    int port = 1050;

    if (serveurConnecter(adresse, port) != SERVEUR_CONNECTE)
    {
        printf("Serveur introuvable %s:%d\n", adresse, port);
        exit(-1);
    }
    else
    {
        printf("Connexion au serveur %s:%d\n", adresse, port);
    }

    serveurUpload(IMAGE_DEBOUT, "../images/debout.png");
    serveurUpload(IMAGE_MARCHE, "../images/marche.png");
    serveurUpload(IMAGE_ACCROUPI, "../images/accroupi.png");
    serveurUpload(IMAGE_LANCE, "../images/lance.png");
    serveurUpload(IMAGE_BONNET_PROFIL, "../images/bonnet_profil.png");
    serveurUpload(IMAGE_VICTOIRE, "../images/victoire.png");
    serveurUpload(IMAGE_DEFAITE, "../images/defaite.png");
    serveurUpload(IMAGE_BONNET_FACE, "../images/bonnet_face.png");

    serveurNomRobot("MIRAGUE");

    serveurCaracRobot(3, 3, 2, 2);

    serveurDemarrerMatch();

    while (1)
    {
        serveurRecevoirSituation(&jeu, &moi, &adversaire, &nbBoules, boule);
        if (!(jeu.chrono % 100))
        {
            switch(etat)
            {
            case 0: //etat pivot
                sousEtat=0;
                if (moi.nbBoule==1) etat=2;
                else if (moi.neigeDispo>=20) etat=3;
                else
                {
                    if (moi.x<380) positionvise = moi.x + 5;
                    else positionvise=100;
                    etat=1;
                }
                break;
            case 1: //repositionnement du robot
                sousEtat = repositionnement (moi.x, positionvise, sousEtat);
                if (sousEtat==4) etat=0;
                break;
            case 2: //lancement de le boule de neige
                calculeEnergieLancer(moi.x, adversaire.x, jeu.ventX, jeu.ventY, jeu.hauteurMur, forceAngleLancer);
                forceLancer=forceAngleLancer[0];
                angleLancer=forceAngleLancer[1];
                serveurLancer(forceLancer,angleLancer);
                if (nbBoules==0) etat=0;
            break;
            case 3: // fabrication de la boule de neige
                sousEtat=CompactageNeige(moi.etat, moi.neigeRassemblee, moi.nbBoule, sousEtat);
                if (sousEtat==4) etat=0;
                break;
            default: //defaut
                etat=1;
                break;

            }
        }

    }
    serveurFermer();
    printf("Serveur deconnecte\n");

    return 0;
}

int repositionnement (int position, int objectif, int sousEtat)
{
    switch (sousEtat)
    {
    case 0:
        serveurStopperAction();
        if (position<objectif) sousEtat=1;
        else if (position>objectif) sousEtat=2;
        else if (position==objectif) sousEtat=3;
        break;
    case 1:
        serveurAvancer();
        if (position>=objectif) sousEtat=3;
        break;
    case 2:
        serveurReculer();
        if (position<=objectif) sousEtat=3;
        break;
    case 3:
        serveurStopperAction();
        sousEtat=4;
        break;
    }

    return sousEtat;
}

int CompactageNeige(char *etatRobot,int neigerassemble, int boule, int sousEtat)
{
    switch (sousEtat)
    {
    case 0:
        serveurSAccroupir();
        if (etatRobot==ROBOT_ACCROUPI) sousEtat=1;
        break;
    case 1:
        serveurRassemblerNeige();
        if (neigerassemble==1) sousEtat=2;
        break;
    case 2:
        serveurCompacterNeige(20);
        if (boule==1) sousEtat=3;
        break;
    case 3:
        serveurSeRelever();
        if (etatRobot==ROBOT_IMMOBILE) sousEtat=4;
    }

    return sousEtat;
}

void calculeEnergieLancer(int positionMoi, int positionAdversaire, int ventX, int ventY, int hauteurMur, int forceAngleLancer[])
{
    int distance, angle=45, energiePourcent;
    double vitesseInitial2, vitesseInitial, energie, angleRad, masse=0.1;

    distance = positionAdversaire-positionMoi - 10; //balle lancer plus loin que le joueur
    angleRad = angle*2*M_PI/360;
    vitesseInitial2 = 5*distance/(cos(angleRad)*sin(angleRad));
    energie = 0.5*masse*vitesseInitial2;
    energiePourcent = energie/10;
    forceAngleLancer[0] = energiePourcent;
    forceAngleLancer[1] = angle;
    printf("%f    %d \n", energie, energiePourcent);
}
