#include "qcm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void enleverFinDeLigne(char *chaine)
{
    size_t longueur = strlen(chaine);
    if (longueur > 0 && chaine[longueur - 1] == '\n')
    {
        chaine[longueur - 1] = '\0';
    }
}

static int lireLigneFichier(FILE *fichier, char *chaine, int taille)
{
    if (fgets(chaine, taille, fichier) == NULL)
    {
        return 0;
    }
    enleverFinDeLigne(chaine);
    return 1;
}

void initialiserQCM(QCM *qcm)
{
    qcm->nom[0] = '\0';
    qcm->categorie[0] = '\0';
    qcm->pointsNegatifs = 0;
    qcm->plusieursReponses = 0;
    qcm->modeSequentiel = 0;
    qcm->nombreQuestions = 0;
    qcm->questions = NULL;
}

void libererQCM(QCM *qcm)
{
    free(qcm->questions);
    initialiserQCM(qcm);
}

int nomValide(const char *nom)
{
    int i;

    if (nom[0] == '\0')
    {
        return 0;
    }

    for (i = 0; nom[i] != '\0'; i++)
    {
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

void construireCheminQCM(const char *nom, char chemin[MAX_TEXTE])
{
    snprintf(chemin, MAX_TEXTE, "%s/%s.qcm", DOSSIER_QCM, nom);
}

int sauvegarderQCM(const QCM *qcm)
{
    FILE *fichier;
    char chemin[MAX_TEXTE];
    int i, j;

#ifdef _WIN32
    mkdir(DOSSIER_QCM);
#else
    mkdir(DOSSIER_QCM, 0755);
#endif

    construireCheminQCM(qcm->nom, chemin);
    fichier = fopen(chemin, "w");
    if (fichier == NULL)
    {
        return 0;
    }

    fprintf(fichier, "QCM_V1\n");
    fprintf(fichier, "%s\n", qcm->nom);
    fprintf(fichier, "%s\n", qcm->categorie);
    fprintf(fichier, "%d %d %d\n", qcm->pointsNegatifs, qcm->plusieursReponses, qcm->modeSequentiel);
    fprintf(fichier, "%d\n", qcm->nombreQuestions);

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

    fclose(fichier);
    return ajouterQCMDansIndex(qcm->nom);
}

int chargerQCM(const char *nomFichier, QCM *qcm)
{
    FILE *fichier;
    char chemin[MAX_TEXTE];
    char ligne[MAX_TEXTE];
    int i, j;

    initialiserQCM(qcm);
    construireCheminQCM(nomFichier, chemin);
    fichier = fopen(chemin, "r");
    if (fichier == NULL)
    {
        return 0;
    }

    if (!lireLigneFichier(fichier, ligne, MAX_TEXTE) || strcmp(ligne, "QCM_V1") != 0)
    {
        fclose(fichier);
        return 0;
    }

    if (!lireLigneFichier(fichier, qcm->nom, MAX_NOM) ||
        !lireLigneFichier(fichier, qcm->categorie, MAX_NOM) ||
        fscanf(fichier, "%d %d %d\n", &qcm->pointsNegatifs, &qcm->plusieursReponses, &qcm->modeSequentiel) != 3 ||
        fscanf(fichier, "%d\n", &qcm->nombreQuestions) != 1)
    {
        fclose(fichier);
        libererQCM(qcm);
        return 0;
    }

    if (qcm->nombreQuestions <= 0)
    {
        fclose(fichier);
        libererQCM(qcm);
        return 0;
    }

    qcm->questions = malloc(qcm->nombreQuestions * sizeof(Question));
    if (qcm->questions == NULL)
    {
        fclose(fichier);
        libererQCM(qcm);
        return 0;
    }

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

        if (qcm->questions[i].nombrePropositions < 2 || qcm->questions[i].nombrePropositions > MAX_PROPOSITIONS)
        {
            fclose(fichier);
            libererQCM(qcm);
            return 0;
        }

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

int ajouterQCMDansIndex(const char *nom)
{
    FILE *fichier;
    char ligne[MAX_NOM];

#ifdef _WIN32
    mkdir(DOSSIER_QCM);
#else
    mkdir(DOSSIER_QCM, 0755);
#endif

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

    fichier = fopen(FICHIER_INDEX, "a");
    if (fichier == NULL)
    {
        return 0;
    }
    fprintf(fichier, "%s\n", nom);
    fclose(fichier);
    return 1;
}

int listerQCM(char noms[][MAX_NOM], int tailleMax)
{
    FILE *fichier = fopen(FICHIER_INDEX, "r");
    int compteur = 0;

    if (fichier == NULL)
    {
        return 0;
    }

    while (compteur < tailleMax && fgets(noms[compteur], MAX_NOM, fichier) != NULL)
    {
        enleverFinDeLigne(noms[compteur]);
        if (noms[compteur][0] != '\0')
        {
            compteur++;
        }
    }

    fclose(fichier);
    return compteur;
}
