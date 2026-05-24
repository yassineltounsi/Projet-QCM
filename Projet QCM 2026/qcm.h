#ifndef QCM_H
#define QCM_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
/*
    Ce fichier d'en-tete contient toutes les definitions communes du projet.
    Il est inclus par plusieurs fichiers .c pour qu'ils utilisent les memes
    constantes, les memes structures et les memes prototypes de fonctions.
*/

/* Nom du dossier dans lequel les fichiers de QCM sont sauvegardes. */
#define DOSSIER_QCM "qcm"

/* Fichier qui contient la liste des QCM disponibles pour les etudiants. */
#define FICHIER_INDEX "qcm/index.txt"

/* Tailles maximales utilisees pour eviter des tableaux trop grands ou des depassements. */
#define MAX_NOM 64
#define MAX_TEXTE 256
#define MAX_PROPOSITIONS 6

/*
    Structure Question
    Elle represente une seule question d'un QCM.
    Une question contient son texte, plusieurs propositions, les bonnes reponses
    et le nombre de points associe a cette question.
*/
typedef struct
{
    /* Enonce de la question affiche a l'etudiant. */
    char texte[MAX_TEXTE];

    /* Nombre reel de propositions utilisees dans le tableau propositions. */
    int nombrePropositions;

    /* Tableau contenant les textes des propositions de reponse. */
    char propositions[MAX_PROPOSITIONS][MAX_TEXTE];

    /*
        Tableau parallele a propositions :
        - 1 signifie que la proposition est correcte ;
        - 0 signifie que la proposition est fausse.
    */
    int bonnesReponses[MAX_PROPOSITIONS];

    /* Nombre de points de la question avant conversion de la note sur 20. */
    double points;
} Question;

/*
    Structure QCM
    Elle represente un questionnaire complet.
    Les parametres comme les points negatifs ou les reponses multiples sont
    valables pour tout le QCM, comme demande dans le cahier des charges.
*/
typedef struct
{
    /* Nom court du QCM, utilise aussi pour creer le nom du fichier. */
    char nom[MAX_NOM];

    /* [INITIATIVE BONUS] Les QCM sont ranges par categorie. */
    char categorie[MAX_NOM];

    /* Vaut 1 si une mauvaise reponse retire des points, sinon 0. */
    int pointsNegatifs;

    /* Vaut 1 si une question peut avoir plusieurs bonnes reponses, sinon 0. */
    int plusieursReponses;

    /*
        Vaut 1 si l'etudiant doit obligatoirement repondre avant de passer
        a la question suivante. Vaut 0 si les questions peuvent etre passees.
    */
    int modeSequentiel;

    /* Nombre total de questions du QCM. */
    int nombreQuestions;

    /*
        Tableau dynamique de questions.
        On utilise un pointeur car le nombre de questions est choisi par
        l'enseignant pendant l'execution.
    */
    Question *questions;
} QCM;

/* Initialise un QCM avec des valeurs neutres avant de l'utiliser. */
void initialiserQCM(QCM *qcm);

/* Libere le tableau dynamique de questions, puis remet le QCM a zero. */
void libererQCM(QCM *qcm);

/* Sauvegarde un QCM dans un fichier texte. Renvoie 1 si tout va bien, sinon 0. */
int sauvegarderQCM(const QCM *qcm);

/* Charge un QCM depuis un fichier texte. Renvoie 1 si le chargement reussit. */
int chargerQCM(const char *nomFichier, QCM *qcm);

/* Ajoute le nom d'un QCM dans l'index, sans le dupliquer. */
int ajouterQCMDansIndex(const char *nom);

/* Remplit le tableau noms avec les QCM disponibles et renvoie leur nombre. */
int listerQCM(char noms[][MAX_NOM], int tailleMax);

/* Verifie que le nom peut etre utilise comme nom de fichier simple. */
int nomValide(const char *nom);

/* Construit le chemin complet vers le fichier d'un QCM. */
void construireCheminQCM(const char *nom, char chemin[MAX_TEXTE]);

#endif
