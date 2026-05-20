#include "interface.h"
#include "qcm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOT_DE_PASSE "CYTECH2026"
#define MAX_QCM_LISTE 50

static int entreeCoupee = 0;

static void enleverFinDeLigne(char *chaine)
{
    size_t longueur = strlen(chaine);
    if (longueur > 0 && chaine[longueur - 1] == '\n')
    {
        chaine[longueur - 1] = '\0';
    }
}

static void lireTexte(const char *message, char *destination, int taille)
{
    do
    {
        printf("%s", message);
        if (fgets(destination, taille, stdin) == NULL)
        {
            entreeCoupee = 1;
            strncpy(destination, "valeur_non_saisie", taille);
            destination[taille - 1] = '\0';
            printf("\nEntree interrompue. Valeur par defaut utilisee.\n");
            return;
        }
        enleverFinDeLigne(destination);
        if (destination[0] == '\0')
        {
            printf("Merci de saisir une valeur non vide.\n");
        }
    } while (destination[0] == '\0');
}

static int lireEntier(const char *message, int min, int max)
{
    char ligne[64];
    int valeur;
    char reste;

    while (1)
    {
        printf("%s", message);
        if (fgets(ligne, sizeof(ligne), stdin) == NULL)
        {
            entreeCoupee = 1;
            printf("\nEntree interrompue. Retour automatique au menu.\n");
            return min;
        }
        if (sscanf(ligne, "%d %c", &valeur, &reste) == 1 && valeur >= min && valeur <= max)
        {
            return valeur;
        }
        printf("Saisie invalide. Choisissez un nombre entre %d et %d.\n", min, max);
    }
}

static double lireDecimal(const char *message, double min, double max)
{
    char ligne[64];
    double valeur;
    char reste;

    while (1)
    {
        printf("%s", message);
        if (fgets(ligne, sizeof(ligne), stdin) == NULL)
        {
            entreeCoupee = 1;
            printf("\nEntree interrompue. Valeur minimale utilisee.\n");
            return min;
        }
        if (sscanf(ligne, "%lf %c", &valeur, &reste) == 1 && valeur >= min && valeur <= max)
        {
            return valeur;
        }
        printf("Saisie invalide. Choisissez un nombre entre %.1f et %.1f.\n", min, max);
    }
}

static int demanderOuiNon(const char *message)
{
    return lireEntier(message, 0, 1);
}

static void afficherTitre(const char *titre)
{
    printf("\n========================================\n");
    printf("%s\n", titre);
    printf("========================================\n");
}

static void modeEnseignant(void)
{
    char motDePasse[64];
    QCM qcm;
    int i, j;
    int modePoints;

    afficherTitre("Mode enseignant");
    lireTexte("Mot de passe : ", motDePasse, sizeof(motDePasse));
    if (strcmp(motDePasse, MOT_DE_PASSE) != 0)
    {
        printf("Mot de passe incorrect. Retour au menu principal.\n");
        return;
    }

    initialiserQCM(&qcm);

    do
    {
        lireTexte("Nom du QCM (sans espace ni accent) : ", qcm.nom, MAX_NOM);
        if (!nomValide(qcm.nom))
        {
            printf("Le nom doit contenir uniquement lettres, chiffres, '_' ou '-'.\n");
        }
    } while (!nomValide(qcm.nom));

    lireTexte("Categorie du QCM : ", qcm.categorie, MAX_NOM);
    qcm.pointsNegatifs = demanderOuiNon("Points negatifs en cas de mauvaise reponse ? (1 oui / 0 non) : ");
    qcm.plusieursReponses = demanderOuiNon("Plusieurs bonnes reponses possibles ? (1 oui / 0 non) : ");
    qcm.modeSequentiel = demanderOuiNon("Reponse obligatoire avant de passer ? (1 oui / 0 non) : ");
    qcm.nombreQuestions = lireEntier("Nombre de questions : ", 1, 30);

    qcm.questions = malloc(qcm.nombreQuestions * sizeof(Question));
    if (qcm.questions == NULL)
    {
        printf("Erreur memoire. Creation annulee.\n");
        return;
    }

    /* [INITIATIVE BONUS] Le professeur peut choisir un bareme different par question. */
    modePoints = demanderOuiNon("Bareme personnalise par question ? (1 oui / 0 non) : ");

    for (i = 0; i < qcm.nombreQuestions; i++)
    {
        int auMoinsUneBonne = 0;
        printf("\nQuestion %d/%d\n", i + 1, qcm.nombreQuestions);
        if (modePoints)
        {
            qcm.questions[i].points = lireDecimal("Nombre de points de cette question : ", 0.1, 100.0);
        }
        else
        {
            qcm.questions[i].points = 1.0;
        }

        lireTexte("Texte de la question : ", qcm.questions[i].texte, MAX_TEXTE);
        qcm.questions[i].nombrePropositions = lireEntier("Nombre de propositions (2 a 6) : ", 2, MAX_PROPOSITIONS);

        for (j = 0; j < qcm.questions[i].nombrePropositions; j++)
        {
            char message[MAX_TEXTE];
            snprintf(message, sizeof(message), "Proposition %d : ", j + 1);
            lireTexte(message, qcm.questions[i].propositions[j], MAX_TEXTE);
            qcm.questions[i].bonnesReponses[j] = 0;
        }

        if (qcm.plusieursReponses)
        {
            printf("Indiquez maintenant les bonnes reponses.\n");
            for (j = 0; j < qcm.questions[i].nombrePropositions; j++)
            {
                char message[MAX_TEXTE];
                snprintf(message, sizeof(message), "La proposition %d est correcte ? (1 oui / 0 non) : ", j + 1);
                qcm.questions[i].bonnesReponses[j] = demanderOuiNon(message);
                if (qcm.questions[i].bonnesReponses[j])
                {
                    auMoinsUneBonne = 1;
                }
            }
            if (!auMoinsUneBonne)
            {
                printf("Aucune bonne reponse choisie : la proposition 1 est mise correcte par securite.\n");
                qcm.questions[i].bonnesReponses[0] = 1;
            }
        }
        else
        {
            int bonne = lireEntier("Numero de la bonne proposition : ", 1, qcm.questions[i].nombrePropositions);
            qcm.questions[i].bonnesReponses[bonne - 1] = 1;
        }
    }

    if (sauvegarderQCM(&qcm))
    {
        printf("\nQCM sauvegarde avec succes.\n");
    }
    else
    {
        printf("\nErreur pendant la sauvegarde du QCM.\n");
    }

    libererQCM(&qcm);
}

static int reponseCorrecte(const Question *question, const int reponses[])
{
    int j;
    for (j = 0; j < question->nombrePropositions; j++)
    {
        if (question->bonnesReponses[j] != reponses[j])
        {
            return 0;
        }
    }
    return 1;
}

static int reponseVide(const Question *question, const int reponses[])
{
    int j;
    for (j = 0; j < question->nombrePropositions; j++)
    {
        if (reponses[j])
        {
            return 0;
        }
    }
    return 1;
}

static void poserQuestion(const QCM *qcm, int numero, int reponses[][MAX_PROPOSITIONS])
{
    int choix;
    int j;
    const Question *question = &qcm->questions[numero];

    printf("\nQuestion %d/%d", numero + 1, qcm->nombreQuestions);
    printf(" (%.1f point(s) avant conversion sur 20)\n", question->points);
    printf("%s\n", question->texte);
    for (j = 0; j < question->nombrePropositions; j++)
    {
        printf("  %d. %s\n", j + 1, question->propositions[j]);
        reponses[numero][j] = 0;
    }

    if (qcm->plusieursReponses)
    {
        int reponseAcceptee;

        do
        {
            printf("Plusieurs choix possibles. Entrez les numeros un par un, puis 0 pour valider.\n");
            do
            {
                choix = lireEntier("Votre choix (0 pour terminer) : ", 0, question->nombrePropositions);
                if (choix > 0)
                {
                    reponses[numero][choix - 1] = 1;
                }
            } while (choix != 0);

            reponseAcceptee = !qcm->modeSequentiel || !reponseVide(question, reponses[numero]);
            if (!reponseAcceptee)
            {
                printf("Ce QCM demande une reponse avant de passer a la suite.\n");
            }
        } while (!reponseAcceptee);
    }
    else
    {
        choix = lireEntier("Votre choix : ", 1, question->nombrePropositions);
        reponses[numero][choix - 1] = 1;
    }
}

static void afficherCorrection(const QCM *qcm, int reponses[][MAX_PROPOSITIONS])
{
    int i, j;
    double totalPossible = 0.0;
    double totalObtenu = 0.0;
    int bonnes = 0;
    int passees = 0;

    for (i = 0; i < qcm->nombreQuestions; i++)
    {
        totalPossible += qcm->questions[i].points;
        if (reponseVide(&qcm->questions[i], reponses[i]))
        {
            passees++;
        }
        else if (reponseCorrecte(&qcm->questions[i], reponses[i]))
        {
            totalObtenu += qcm->questions[i].points;
            bonnes++;
        }
        else if (qcm->pointsNegatifs)
        {
            totalObtenu -= qcm->questions[i].points / 2.0;
        }
    }

    if (totalObtenu < 0.0)
    {
        totalObtenu = 0.0;
    }

    afficherTitre("Resultat");
    printf("Note finale : %.2f / 20\n", (totalObtenu / totalPossible) * 20.0);
    printf("Bonnes reponses : %d / %d\n", bonnes, qcm->nombreQuestions);
    /* [INITIATIVE BONUS] Des statistiques simples aident l'etudiant a comprendre son resultat. */
    printf("Questions passees : %d\n", passees);

    printf("\nCorrection rapide :\n");
    for (i = 0; i < qcm->nombreQuestions; i++)
    {
        printf("%d. %s\n", i + 1, reponseCorrecte(&qcm->questions[i], reponses[i]) ? "Correct" : "A revoir");
        printf("   Bonne(s) reponse(s) : ");
        for (j = 0; j < qcm->questions[i].nombrePropositions; j++)
        {
            if (qcm->questions[i].bonnesReponses[j])
            {
                printf("%d ", j + 1);
            }
        }
        printf("\n");
    }
}

static void modeEtudiant(void)
{
    char noms[MAX_QCM_LISTE][MAX_NOM];
    int nombreQCM;
    int choix;
    int i;
    QCM qcm;
    int (*reponses)[MAX_PROPOSITIONS];

    afficherTitre("Mode etudiant");
    nombreQCM = listerQCM(noms, MAX_QCM_LISTE);
    if (nombreQCM == 0)
    {
        printf("Aucun QCM disponible pour le moment.\n");
        return;
    }

    printf("QCM disponibles :\n");
    for (i = 0; i < nombreQCM; i++)
    {
        QCM apercu;
        printf("  %d. %s", i + 1, noms[i]);
        if (chargerQCM(noms[i], &apercu))
        {
            printf(" - categorie : %s", apercu.categorie);
            libererQCM(&apercu);
        }
        printf("\n");
    }

    choix = lireEntier("Numero du QCM choisi : ", 1, nombreQCM);
    if (!chargerQCM(noms[choix - 1], &qcm))
    {
        printf("Impossible de charger ce QCM.\n");
        return;
    }

    reponses = calloc(qcm.nombreQuestions, sizeof(int[MAX_PROPOSITIONS]));
    if (reponses == NULL)
    {
        printf("Erreur memoire. QCM annule.\n");
        libererQCM(&qcm);
        return;
    }

    afficherTitre(qcm.nom);
    printf("Categorie : %s\n", qcm.categorie);
    printf("Points negatifs : %s\n", qcm.pointsNegatifs ? "oui" : "non");
    printf("Plusieurs reponses : %s\n", qcm.plusieursReponses ? "oui" : "non");
    printf("Question obligatoire : %s\n", qcm.modeSequentiel ? "oui" : "non");

    for (i = 0; i < qcm.nombreQuestions; i++)
    {
        int passer = 0;
        if (!qcm.modeSequentiel)
        {
            passer = demanderOuiNon("\nPasser cette question ? (1 oui / 0 non) : ");
        }
        if (!passer)
        {
            poserQuestion(&qcm, i, reponses);
        }
    }

    afficherCorrection(&qcm, reponses);
    free(reponses);
    libererQCM(&qcm);
}

void lancerApplication(void)
{
    int choix;

    do
    {
        afficherTitre("Gestionnaire de QCM - CY Tech");
        printf("1. Mode enseignant\n");
        printf("2. Mode etudiant\n");
        printf("0. Quitter\n");
        choix = lireEntier("Votre choix : ", 0, 2);
        if (entreeCoupee)
        {
            choix = 0;
        }

        if (choix == 1)
        {
            modeEnseignant();
        }
        else if (choix == 2)
        {
            modeEtudiant();
        }
    } while (choix != 0);

    printf("Au revoir !\n");
}
