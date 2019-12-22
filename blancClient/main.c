#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libClient.h"

double forceDeLancer (int etatAdversaire, int posmoi, int posadversaire);

int main (int argc, char **argv)
{
    Jeu jeu;
    Moi moi;
    Adversaire adversaire;
    int nbBoules;
    Boule boule[BOULES_NB_MAX];

    int objectif=50;
    int F=0,alpha=0;
    int etat = 0;
    int energie;

    // adresse IP du serveur sous forme de chaine de caracteres
    char adresse[255] = "192.168.42.19";
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
            //ne rien afficher pour l'instant.
        }

        // MACHINES A ETATS
// MACHINES A ETATS
        switch (etat)
        {
        //etat pivot
        case 0 :
            serveurStopperAction();
            if((moi.etat == ROBOT_IMMOBILE) && (moi.nbBoule == 0))
            {
                etat=1;
                break;
            }
            if(moi.nbBoule == 1)
            {
                etat=20;
                break;
            }
            if(moi.etat == ROBOT_IMMOBILE)
            {
                etat=11;
                break;
            }

        //fabrication boule
        case 1 :
            serveurStopperAction();
            if (moi.neigeDispo > 20)
            {
                etat=2;
                break;
            }
            else
            {
                etat=5;
                break;
            }
        case 2 :
            serveurSAccroupir();
            if(moi.etat == ROBOT_ACCROUPI)
                etat=3;
            break;
        case 3 :
            if (moi.neigeRassemblee)
            {
                serveurCompacterNeige(20);
            }
            else
            {
                serveurRassemblerNeige();
            }
            if(moi.nbBoule != 0)
                etat=4;
            break;
        case 4 :
            serveurSeRelever();
            if(moi.etat == ROBOT_IMMOBILE)
                etat=0;
            break;

        //déplacement en cas de manque de neige
        case 5 :
            serveurAvancer();
            if(moi.neigeDispo > 20)
                etat = 1;
            if(moi.x >= 240)
                etat=6;
            break;
        case 6 :
            serveurStopperAction();
            if(moi.etat == ROBOT_IMMOBILE)
                etat=7;
            break;
        case 7 :
            serveurReculer();
            if (moi.neigeDispo > 20)
                etat = 1;
            if (moi.x <= 80)
                etat=1;
            break;

        //déplacement standard
        case 8 :
            serveurAvancer();
            if(moi.x >= 240)
            {
                etat=12;
            }
            break;
        case 9 :
            serveurStopperAction();
            if(moi.etat == ROBOT_IMMOBILE)
                etat=13;
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
            energie = forceDeLancer(adversaire.etat, moi.x, adversaire.x);
            serveurLancer(energie,45);
            if(moi.etat == ROBOT_LANCE)
                etat=0;
            break;

        default :
            serveurNeRienChanger();
            etat = 0;
        }
    }
    serveurFermer();
    printf("Serveur deconnecte\n");

    return 0;
}

double forceDeLancer (int etatAdversaire, int posmoi, int posadversaire)
{
    int distance, estimationX=0,estimationY=0;
    double vitesseInitial2, masseBoule, energie,energiePourcent;
    double angle,anglerad;

    if (etatAdversaire==ROBOT_AVANCE)
        estimationX = -40;
    if (etatAdversaire==ROBOT_RECULE)
        estimationX =  40;
    if (etatAdversaire==ROBOT_ACCROUPI)
        estimationY = -100;

    angle = 45;
    anglerad = angle*2.0*M_PI/360.0;
    distance = posadversaire - posmoi-80; //balle lancer plus loin que le joueur
    vitesseInitial2 = 5*distance/(cos(anglerad)*sin(anglerad));
    if (estimationY != 0)
        vitesseInitial2 -= (5*distance*distance)/(estimationY*sin(anglerad)*sin(anglerad));
    masseBoule = 0.1; //les boules ont une taille standard de 2
    energie = 0.5*masseBoule*vitesseInitial2;

    printf("%d   %d    %f\n",posadversaire,posmoi, energie);

    return energie;

}


//EXERCICE 1

/*
        // MACHINES A ETATS
        switch (etat)
        {
        case 0 :
            serveurStopperAction();
            if(moi.etat == ROBOT_IMMOBILE) etat=1;
            break;
        case 1 :
            serveurAvancer();
            if(moi.x >= 240)
            {
                etat=2;
            }
            break;
        case 2 :
            serveurStopperAction();
            if(moi.etat == ROBOT_IMMOBILE) etat=3;
            break;
        case 3 :
            serveurReculer();
            if (moi.x <= 80)
            {
                etat=0;
            }
            break;
        default :
            serveurNeRienChanger();
            etat = 0;
        }
*/


//EXERCICE 2

/*

        // MACHINES A ETATS
        switch (etat)
        {
        case 0 :
            serveurStopperAction();
            if(moi.etat == ROBOT_IMMOBILE) etat=1;
            break;
        case 1 :
            serveurSAccroupir();
            if(moi.etat == ROBOT_ACCROUPI) etat=2;
            break;
        case 2 :
            if (moi.neigeRassemblee)
            {
                serveurCompacterNeige(20);
            }
            else
            {
                serveurRassemblerNeige();
            }
            if(moi.nbBoule != 0) etat=3;
            break;
        case 3 :
            serveurSeRelever();
            if(moi.etat == ROBOT_IMMOBILE) etat=0;
            break;

        default :
            serveurNeRienChanger();
            etat = 0;
        }

*/
