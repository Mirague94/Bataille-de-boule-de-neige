#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libClient.h"
int repositionnement (int position, int objectif, int sousEtat);

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
                //etat pivot
            case 0:

                //repositionnement du robot
            case 1:
                sousEtat = repositionnement (moi.x, positionvise, sousEtat);
                if (sousEtat==4) etat=0;
                break;
            default:
                etat=1;
                break;

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

    printf("%d",sousEtat);
    return sousEtat;
}
