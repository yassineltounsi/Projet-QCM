#ifndef QCM_H
#define QCM_H

#define DOSSIER_QCM "qcm"
#define FICHIER_INDEX "qcm/index.txt"
#define MAX_NOM 64
#define MAX_TEXTE 256
#define MAX_PROPOSITIONS 6

typedef struct
{
    char texte[MAX_TEXTE];
    int nombrePropositions;
    char propositions[MAX_PROPOSITIONS][MAX_TEXTE];
    int bonnesReponses[MAX_PROPOSITIONS];
    double points;
} Question;

typedef struct
{
    char nom[MAX_NOM];
    char categorie[MAX_NOM]; /* [INITIATIVE BONUS] Les QCM sont ranges par categorie. */
    int pointsNegatifs;
    int plusieursReponses;
    int modeSequentiel;
    int nombreQuestions;
    Question *questions;
} QCM;

void initialiserQCM(QCM *qcm);
void libererQCM(QCM *qcm);
int sauvegarderQCM(const QCM *qcm);
int chargerQCM(const char *nomFichier, QCM *qcm);
int ajouterQCMDansIndex(const char *nom);
int listerQCM(char noms[][MAX_NOM], int tailleMax);
int nomValide(const char *nom);
void construireCheminQCM(const char *nom, char chemin[MAX_TEXTE]);

#endif
