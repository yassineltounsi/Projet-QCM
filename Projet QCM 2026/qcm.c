#include "qcm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
    Enleve le caractere '\n' ajoute par fgets a la fin d'une chaine.
    C'est utile car les comparaisons et les sauvegardes seraient faussees
    si le retour a la ligne restait dans la chaine.
*/
static void enleverFinDeLigne(char *chaine)
{
    /* strlen donne la longueur actuelle de la chaine. */
    size_t longueur = strlen(chaine);

    /* Si le dernier caractere est un retour a la ligne, on le remplace par '\0'. */
    if (longueur > 0 && chaine[longueur - 1] == '\n')
    {
        chaine[longueur - 1] = '\0';
    }
}

/*
    Lit une ligne dans un fichier et retire le retour a la ligne final.
    La fonction renvoie 1 si la lecture a reussi et 0 sinon.
*/
static int lireLigneFichier(FILE *fichier, char *chaine, int taille)
{
    /* fgets protege contre les depassements car on lui donne la taille du tableau. */
    if (fgets(chaine, taille, fichier) == NULL)
    {
        return 0;
    }

    enleverFinDeLigne(chaine);
    return 1;
}

/*
    Met toutes les informations d'un QCM a des valeurs de depart.
    Cela evite d'utiliser des valeurs inconnues en memoire.
*/
void initialiserQCM(QCM *qcm)
{
    /* Une chaine vide commence par le caractere de fin '\0'. */
    qcm->nom[0] = '\0';
    qcm->categorie[0] = '\0';

    /* Les parametres booleens sont initialises a 0, donc "non". */
    qcm->pointsNegatifs = 0;
    qcm->plusieursReponses = 0;
    qcm->modeSequentiel = 0;

    /* Au depart, le QCM ne contient aucune question. */
    qcm->nombreQuestions = 0;

    /* NULL indique qu'aucune zone memoire dynamique n'est encore reservee. */
    qcm->questions = NULL;
}

/*
    Libere la memoire utilisee par le tableau dynamique de questions.
    Ensuite, le QCM est remis a zero pour eviter de garder un pointeur invalide.
*/
void libererQCM(QCM *qcm)
{
    /* free(NULL) est autorise en C, donc cette ligne reste sure meme si rien n'est alloue. */
    free(qcm->questions);
    initialiserQCM(qcm);
}

/*
    Verifie qu'un nom de QCM peut etre utilise comme nom de fichier.
    On accepte seulement les lettres, chiffres, '_' et '-'.
    Cela evite les espaces, accents ou caracteres speciaux dans les noms de fichiers.
*/
int nomValide(const char *nom)
{
    /* i sert a parcourir la chaine caractere par caractere. */
    int i;

    /* Un nom vide n'est pas utilisable. */
    if (nom[0] == '\0')
    {
        return 0;
    }

    for (i = 0; nom[i] != '\0'; i++)
    {
        /*
            caractereValide vaut 1 si le caractere courant fait partie
            des caracteres autorises, sinon il vaut 0.
        */
        int caractereValide = (nom[i] >= 'A' && nom[i] <= 'Z') ||
                              (nom[i] >= 'a' && nom[i] <= 'z') ||
                              (nom[i] >= '0' && nom[i] <= '9') ||
                              nom[i] == '_' || nom[i] == '-';

        if (!caractereValide)
        {
            return 0;
        }
    }
    return 1;
}

/*
    Construit le chemin du fichier correspondant a un QCM.
    Exemple : le QCM "structures_c" devient "qcm/structures_c.qcm".
*/
void construireCheminQCM(const char *nom, char chemin[MAX_TEXTE])
{
    /* snprintf evite d'ecrire plus de caracteres que la taille du tableau chemin. */
    snprintf(chemin, MAX_TEXTE, "%s/%s.qcm", DOSSIER_QCM, nom);
}

/*
    Sauvegarde un QCM complet dans un fichier texte.
    Le fichier contient d'abord les informations generales, puis toutes les questions.
*/
int sauvegarderQCM(const QCM *qcm)
{
    /* Pointeur vers le fichier ouvert en ecriture. */
    FILE *fichier;

    /* Chemin complet du fichier a creer. */
    char chemin[MAX_TEXTE];

    /* i parcourt les questions et j parcourt les propositions. */
    int i, j;

/*
    Creation du dossier qcm tout en restant compatible avec tout systeme d'exploitation
*/
#ifdef _WIN32
    mkdir(DOSSIER_QCM);
#else
    mkdir(DOSSIER_QCM, 0755);
#endif

    /* On prepare le chemin puis on ouvre le fichier en mode ecriture ("w"). */
    construireCheminQCM(qcm->nom, chemin);
    fichier = fopen(chemin, "w");
    if (fichier == NULL)
    {
        /* Si fopen echoue, on renvoie 0 pour signaler l'erreur au programme. */
        return 0;
    }

    /*
        La premiere ligne est une petite signature de format.
        Elle permet de verifier au chargement que le fichier ressemble bien
        a un fichier cree par ce programme.
    */
    fprintf(fichier, "QCM_V1\n");

    /* Informations generales du QCM. */
    fprintf(fichier, "%s\n", qcm->nom);
    fprintf(fichier, "%s\n", qcm->categorie);
    fprintf(fichier, "%d %d %d\n", qcm->pointsNegatifs, qcm->plusieursReponses, qcm->modeSequentiel);
    fprintf(fichier, "%d\n", qcm->nombreQuestions);

    /* Sauvegarde de chaque question, puis de chacune de ses propositions. */
    for (i = 0; i < qcm->nombreQuestions; i++)
    {
        fprintf(fichier, "%.2f\n", qcm->questions[i].points);
        fprintf(fichier, "%s\n", qcm->questions[i].texte);
        fprintf(fichier, "%d\n", qcm->questions[i].nombrePropositions);

        for (j = 0; j < qcm->questions[i].nombrePropositions; j++)
        {
            fprintf(fichier, "%s\n", qcm->questions[i].propositions[j]);
            fprintf(fichier, "%d\n", qcm->questions[i].bonnesReponses[j]);
        }
    }

    /* Toujours fermer un fichier quand on a fini de l'utiliser. */
    fclose(fichier);

    /* Le QCM est ajoute dans l'index pour apparaitre dans le mode etudiant. */
    return ajouterQCMDansIndex(qcm->nom);
}

/*
    Charge un QCM depuis son fichier texte.
    La fonction remplit la structure qcm passee en parametre.
*/
int chargerQCM(const char *nomFichier, QCM *qcm)
{
    /* Pointeur vers le fichier ouvert en lecture. */
    FILE *fichier;

    /* Chemin complet du fichier a lire. */
    char chemin[MAX_TEXTE];

    /* Ligne temporaire utilisee pour lire la signature du fichier. */
    char ligne[MAX_TEXTE];

    /* i parcourt les questions et j parcourt les propositions. */
    int i, j;

    /* On part toujours d'un QCM vide avant de le remplir. */
    initialiserQCM(qcm);

    construireCheminQCM(nomFichier, chemin);
    fichier = fopen(chemin, "r");
    if (fichier == NULL)
    {
        return 0;
    }

    /* Verification de la signature pour eviter de lire un mauvais fichier. */
    if (!lireLigneFichier(fichier, ligne, MAX_TEXTE) || strcmp(ligne, "QCM_V1") != 0)
    {
        fclose(fichier);
        return 0;
    }

    /* Lecture des informations generales du QCM. */
    if (!lireLigneFichier(fichier, qcm->nom, MAX_NOM) ||
        !lireLigneFichier(fichier, qcm->categorie, MAX_NOM) ||
        fscanf(fichier, "%d %d %d\n", &qcm->pointsNegatifs, &qcm->plusieursReponses, &qcm->modeSequentiel) != 3 ||
        fscanf(fichier, "%d\n", &qcm->nombreQuestions) != 1)
    {
        fclose(fichier);
        libererQCM(qcm);
        return 0;
    }

    /* Un QCM sans question n'a pas de sens et pourrait provoquer une division par zero. */
    if (qcm->nombreQuestions <= 0)
    {
        fclose(fichier);
        libererQCM(qcm);
        return 0;
    }

    /*
        Allocation dynamique du tableau de questions.
        On connait seulement maintenant le nombre de questions, car il vient du fichier.
    */
    qcm->questions = malloc(qcm->nombreQuestions * sizeof(Question));
    if (qcm->questions == NULL)
    {
        fclose(fichier);
        libererQCM(qcm);
        return 0;
    }

    /* Lecture detaillee de chaque question. */
    for (i = 0; i < qcm->nombreQuestions; i++)
    {
        if (fscanf(fichier, "%lf\n", &qcm->questions[i].points) != 1 ||
            !lireLigneFichier(fichier, qcm->questions[i].texte, MAX_TEXTE) ||
            fscanf(fichier, "%d\n", &qcm->questions[i].nombrePropositions) != 1)
        {
            fclose(fichier);
            libererQCM(qcm);
            return 0;
        }

        /* Controle de securite : le nombre de propositions doit rester dans les limites. */
        if (qcm->questions[i].nombrePropositions < 2 || qcm->questions[i].nombrePropositions > MAX_PROPOSITIONS)
        {
            fclose(fichier);
            libererQCM(qcm);
            return 0;
        }

        /* Lecture des propositions et des indicateurs bonne/fausse. */
        for (j = 0; j < qcm->questions[i].nombrePropositions; j++)
        {
            if (!lireLigneFichier(fichier, qcm->questions[i].propositions[j], MAX_TEXTE) ||
                fscanf(fichier, "%d\n", &qcm->questions[i].bonnesReponses[j]) != 1)
            {
                fclose(fichier);
                libererQCM(qcm);
                return 0;
            }
        }
    }

    fclose(fichier);
    return 1;
}

/*
    Ajoute le nom d'un QCM dans le fichier index.txt.
    Cet index sert a afficher la liste des QCM disponibles dans le mode etudiant.
*/
int ajouterQCMDansIndex(const char *nom)
{
    /* Pointeur vers le fichier index, ouvert d'abord en lecture puis en ajout. */
    FILE *fichier;

    /* Ligne temporaire pour verifier si le QCM est deja dans l'index. */
    char ligne[MAX_NOM];

#ifdef _WIN32
    mkdir(DOSSIER_QCM);
#else
    mkdir(DOSSIER_QCM, 0755);
#endif

    /* Premiere etape : lire l'index pour eviter d'ajouter deux fois le meme nom. */
    fichier = fopen(FICHIER_INDEX, "r");
    if (fichier != NULL)
    {
        while (fgets(ligne, MAX_NOM, fichier) != NULL)
        {
            enleverFinDeLigne(ligne);
            if (strcmp(ligne, nom) == 0)
            {
                fclose(fichier);
                return 1;
            }
        }
        fclose(fichier);
    }

    /* Deuxieme etape : ajouter le nouveau nom a la fin du fichier. */
    fichier = fopen(FICHIER_INDEX, "a");
    if (fichier == NULL)
    {
        return 0;
    }
    fprintf(fichier, "%s\n", nom);
    fclose(fichier);
    return 1;
}

/*
    Lit le fichier index.txt et recopie les noms des QCM dans le tableau noms.
    La fonction renvoie le nombre de QCM trouves.
*/
int listerQCM(char noms[][MAX_NOM], int tailleMax)
{
    /* Ouverture de l'index en lecture. */
    FILE *fichier = fopen(FICHIER_INDEX, "r");

    /* compteur indique combien de noms ont deja ete lus. */
    int compteur = 0;

    if (fichier == NULL)
    {
        return 0;
    }

    /* On lit au maximum tailleMax noms pour ne pas depasser la taille du tableau. */
    while (compteur < tailleMax && fgets(noms[compteur], MAX_NOM, fichier) != NULL)
    {
        enleverFinDeLigne(noms[compteur]);

        /* Les lignes vides sont ignorees. */
        if (noms[compteur][0] != '\0')
        {
            compteur++;
        }
    }

    fclose(fichier);
    return compteur;
}
