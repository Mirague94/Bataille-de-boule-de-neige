#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libClient.h"

void calculeEnergieLancer(int positionMoi, int positionAdversaire, int ventX, int ventY, int hauteurMur,  char *etatAdversaire, int forceAngleLancer[], int bonnet);
int analyseLancer(int bouleVX, int bouleVY, int BouleX, int BouleY, int positionMoi, int etat, int situation, int ventX);

int main (int argc, char **argv)
{
    Jeu jeu;
    Moi moi;
    Adversaire adversaire;
    int nbBoules;
    Boule boule[BOULES_NB_MAX];

    int etat=0;
    int forceAngleLancer[2];
    int forceLancer, angleLancer;

     // adresse IP du serveur sous forme de chaine de caracteres
    char adresse[255] = "127.0.0.1";//adresse fixe pour le pc interne
    //char adresse[255] = "192.168.43.37";
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

    serveurCaracRobot(4, 4, 1, 1);

    serveurDemarrerMatch();

    while (1)
    {
        serveurRecevoirSituation(&jeu, &moi, &adversaire, &nbBoules, boule);
        if (!(jeu.chrono % 100))
        {
            printf("etat = %d\n",etat);
        }

        if (adversaire.etat == ROBOT_LANCE)
        {
            etat = analyseLancer(boule[0].vx, boule[0].vy, boule[0].x, boule[0].y, moi.x, etat, moi.etat, jeu.ventX);
        }

        switch (etat)
        {
        //etat pivot
        case 0 :
            serveurStopperAction();
            if(moi.etat==ROBOT_IMMOBILE) etat=1;
            break;
        //fabrication boule
        case 1 :
            if (moi.neigeDispo > 20) etat=2;
            else etat=5;
            break;
        case 2 :
            serveurSAccroupir();
            if(moi.etat == ROBOT_ACCROUPI) etat=3;
            break;
        case 3 :
            if (moi.neigeDispo < 20) etat =4;
            if (moi.neigeRassemblee==1)
            {
                serveurCompacterNeige(20);
            }
            else
            {
                serveurRassemblerNeige();
            }
            if(moi.nbBoule != 0) etat=4;
            break;
        case 4 :
            serveurSeRelever();
            if(moi.etat == ROBOT_IMMOBILE)
            {
                if (moi.nbBoule == 0) etat =1;
                etat=20;
            }
            break;

        //déplacement en cas de manque de neige
        case 5 :
            serveurAvancer();
            if(moi.neigeDispo > 20) etat=0;
            if(moi.x >= 240) etat=6;
            break;
        case 6 :
            serveurStopperAction();
            if(moi.etat == ROBOT_IMMOBILE) etat=7;
            break;
        case 7 :
            serveurReculer();
            if (moi.neigeDispo > 20) etat = 1;
            if (moi.x <= 80) etat=5;
            break;

        //déplacement standard
        case 8 :
            serveurAvancer();
            if(moi.x >= 240) etat=12;
            break;
        case 9 :
            serveurStopperAction();
            if(moi.etat == ROBOT_IMMOBILE) etat=13;
            break;
        case 10 :
            serveurReculer();
            if (moi.x <= 80)
            {
                etat=0;
            }
            break;

        //Lancement boule
        case 20 :
            serveurNeRienChanger();
            if (!((adversaire.etat == ROBOT_AVANCE) || (adversaire.etat == ROBOT_IMMOBILE) || (adversaire.etat == ROBOT_RECULE) || (adversaire.etat == ROBOT_SE_RELEVE))) etat = 21;
        case 21 :
            calculeEnergieLancer(moi.x, adversaire.x, jeu.ventX, jeu.ventY, jeu.hauteurMur, adversaire.etat, forceAngleLancer, adversaire.bonnet);
            forceLancer=forceAngleLancer[0];
            angleLancer=forceAngleLancer[1];
            serveurLancer(forceLancer,angleLancer);
            etat=0;
            break;

        // esquive
        case 31:
            serveurStopperAction();
            if (moi.etat == ROBOT_IMMOBILE) etat=32;
            break;
        case 32:
            serveurSAccroupir();
            if (nbBoules==0) etat = 3;
            break;

       default :
            etat = 0;
            break;
        }
    }
    serveurFermer();
    printf("Serveur deconnecte\n");

    return 0;
}

void calculeEnergieLancer(int positionMoi, int positionAdversaire, int ventX, int ventY, int hauteurMur, char *etatAdversaire, int forceAngleLancer[],int bonnet)
{
    int distance, angle=30, energiePourcent, estimationX=-50, estimationY=-100;
    double vitesseInitial2, energie, angleRad, masse=0.1;

    if (positionAdversaire <= 800) angle = 45;
    if (positionAdversaire <= 700) angle = 60;

    if (bonnet==1) estimationY += 75;

    distance = positionAdversaire - positionMoi + estimationX;
    if (distance < 200) distance -= 75;
    angleRad = angle*2*M_PI/360;

    vitesseInitial2 = -10*distance*distance/(2*cos(angleRad)*(cos(angleRad)*estimationY-sin(angleRad)*distance)) - ventX;

    energie = 0.5*masse*vitesseInitial2;
    energiePourcent = energie/10;
    forceAngleLancer[0] = energiePourcent;
    forceAngleLancer[1] = angle;
    printf("Vitesse du lancer = %.2f\n", vitesseInitial2);
}


int analyseLancer(int bouleVX, int bouleVY, int BouleX, int BouleY, int positionMoi, int etat, int situation, int ventX)
{
    int positionImpact;
    double hauteurImpact;

    positionImpact = positionMoi - BouleX;

    hauteurImpact = BouleY-(5*positionImpact*positionImpact)/(bouleVY*bouleVY)+(bouleVX*positionImpact)/bouleVY;

    if ((etat==32) && (hauteurImpact <=175))
    {
        etat=0;
    }

    if (hauteurImpact < 270)
    {
        if (hauteurImpact > 150)
        {
            if ((situation == ROBOT_COMPACTE_BOULE) || (situation == ROBOT_ACCROUPI) || (situation == ROBOT_S_ACCROUPI) || (situation == ROBOT_RASSEMBLE_NEIGE));
            else etat = 31;
        }
        else
        {
            if ((situation == ROBOT_COMPACTE_BOULE) || (situation == ROBOT_ACCROUPI) || (situation == ROBOT_S_ACCROUPI) || (situation == ROBOT_RASSEMBLE_NEIGE))
            {
                serveurStopperAction();
                serveurSeRelever();
                etat = 20;
            }
        }
    }
    printf("Hauteur Impact = %.0f\n", hauteurImpact);
    return etat;
}
