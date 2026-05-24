#include "interface.h"
#include "qcm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Mot de passe demande pour acceder au mode enseignant. */
#define MOT_DE_PASSE "CYTECH2026"

/* Nombre maximum de QCM que l'on accepte d'afficher dans la liste. */
#define MAX_QCM_LISTE 50

/*
    Variable globale interne a ce fichier.
    Elle passe a 1 si l'entree standard est coupee. Elle permet d'eviter de rester bloque dans une boucle.
*/
static int entreeCoupee = 0;

/*
    Supprime le retour a la ligne laisse par fgets!!!
    Exemple : "bonjour\n" devient "bonjour".
*/
static void enleverFinDeLigne(char *chaine)
{
    /* longueur contient le nombre de caracteres de la chaine. */
    size_t longueur = strlen(chaine); /* size t est concu pour ne jamais etre negatif + s'adapte a la memoire de l'ordinateur (mieux que int)*/

    if (longueur > 0 && chaine[longueur - 1] == '\n')
    {
        chaine[longueur - 1] = '\0';
    }
}

/*
    Lit un texte saisi par l'utilisateur.
    La fonction redemande une saisie tant que l'utilisateur laisse la ligne vide.
*/
static void lireTexte(const char *message, char *destination, int taille)
{
    do
    {
        /* message est le texte affiche avant la saisie, par exemple "Nom du QCM : ". */
        printf("%s", message);

        /*
            destination est le tableau ou la reponse sera rangee.
            taille permet a fgets de ne pas depasser la taille du tableau.
        */
        if (fgets(destination, taille, stdin) == NULL)
        {
            /*
                Cas rare mais important : si la lecture echoue, on evite une
                boucle infinie et on place une valeur par defaut.
            */
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

/*
    Lit un nombre entier compris entre min et max.
    La fonction ne quitte pas tant que la saisie n'est pas correcte.
*/
static int lireEntier(const char *message, int min, int max)
{
    /* ligne contient la saisie brute de l'utilisateur. */
    char ligne[64];

    /* valeur recevra l'entier lu avec sscanf. */
    int valeur;

    /*
        reste sert a detecter les saisies du type "12abc".
        Si sscanf lit un caractere en plus, la saisie n'est pas un entier propre.
    */
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

        /* On accepte uniquement une valeur seule, sans texte supplementaire. */
        if (sscanf(ligne, "%d %c", &valeur, &reste) == 1 && valeur >= min && valeur <= max)
        {
            return valeur;
        }

        printf("Saisie invalide. Choisissez un nombre entre %d et %d.\n", min, max);
    }
}

/*
    Lit un nombre decimal compris entre min et max.
    Cette fonction sert pour le bareme personnalise des questions.
*/
static double lireDecimal(const char *message, double min, double max)
{
    /* ligne contient la saisie brute de l'utilisateur. */
    char ligne[64];

    /* valeur recevra le nombre decimal lu avec sscanf. */
    double valeur;

    /* reste permet de refuser une saisie qui contient du texte apres le nombre. */
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

/*
    Petite fonction pratique pour les questions oui/non.
    Dans ce programme, 1 signifie oui et 0 signifie non.
*/
static int demanderOuiNon(const char *message)
{
    return lireEntier(message, 0, 1);
}

/*
    Affiche un titre encadre par des lignes.
    Cela rend l'interface console plus lisible pour l'utilisateur.
*/
static void afficherTitre(const char *titre)
{
    printf("\n------------------------------\n");
    printf("%s\n", titre);
    printf("------------------------------\n");
}

/*
    Gere toute la partie enseignant :
    - verification du mot de passe ;
    - creation d'un QCM ;
    - saisie des questions et des reponses ;
    - sauvegarde dans un fichier.
*/
static void modeEnseignant(void)
{
    /* Tableau qui stocke le mot de passe saisi par l'utilisateur. */
    char motDePasse[64];

    /* Structure principale qui contiendra le QCM en cours de creation. */
    QCM qcm;

    /* i parcourt les questions et j parcourt les propositions. */
    int i, j;

    /* Vaut 1 si le professeur veut donner un nombre de points different par question. */
    int modePoints;

    afficherTitre("Mode enseignant");

    /* Le mode enseignant est protege par un mot de passe, comme demande. */
    lireTexte("Mot de passe : ", motDePasse, sizeof(motDePasse));
    if (strcmp(motDePasse, MOT_DE_PASSE) != 0)
    {
        printf("Mot de passe incorrect. Retour au menu principal.\n");
        return;
    }

    /* On initialise le QCM avant de remplir ses champs. */
    initialiserQCM(&qcm);

    /*
        Le nom du QCM sert aussi de nom de fichier.
        On impose donc un nom simple, sans espace ni caractere special.
    */
    do
    {
        lireTexte("Nom du QCM (sans espace ni accent) : ", qcm.nom, MAX_NOM);
        if (!nomValide(qcm.nom))
        {
            printf("Le nom doit contenir uniquement lettres, chiffres, '_' ou '-'.\n");
        }
    } while (!nomValide(qcm.nom));

    /* Saisie des parametres globaux du QCM. */
    lireTexte("Categorie du QCM : ", qcm.categorie, MAX_NOM);
    qcm.pointsNegatifs = demanderOuiNon("Points negatifs en cas de mauvaise reponse ? (1 oui / 0 non) : ");
    qcm.plusieursReponses = demanderOuiNon("Plusieurs bonnes reponses possibles ? (1 oui / 0 non) : ");
    qcm.modeSequentiel = demanderOuiNon("Reponse obligatoire avant de passer ? (1 oui / 0 non) : ");
    qcm.nombreQuestions = lireEntier("Nombre de questions : ", 1, 30);

    /*
        Allocation du tableau de questions.
        On utilise malloc car le nombre de questions depend du choix de l'enseignant.
    */
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
        /*
            auMoinsUneBonne permet de verifier qu'une question a au moins
            une bonne reponse quand les reponses multiples sont autorisees.
        */
        int auMoinsUneBonne = 0;

        printf("\nQuestion %d/%d\n", i + 1, qcm.nombreQuestions);

        /* Attribution des points de la question. */
        if (modePoints)
        {
            qcm.questions[i].points = lireDecimal("Nombre de points de cette question : ", 0.1, 100.0);
        }
        else
        {
            qcm.questions[i].points = 1.0;
        }

        /* Saisie de l'enonce et du nombre de propositions. */
        lireTexte("Texte de la question : ", qcm.questions[i].texte, MAX_TEXTE);
        qcm.questions[i].nombrePropositions = lireEntier("Nombre de propositions (2 a 6) : ", 2, MAX_PROPOSITIONS);

        /* Saisie des propositions une par une. */
        for (j = 0; j < qcm.questions[i].nombrePropositions; j++)
        {
            /* message permet d'afficher "Proposition 1", "Proposition 2", etc. */
            char message[MAX_TEXTE];
            snprintf(message, sizeof(message), "Proposition %d : ", j + 1);
            lireTexte(message, qcm.questions[i].propositions[j], MAX_TEXTE);

            /* Par defaut, une proposition est consideree comme fausse. */
            qcm.questions[i].bonnesReponses[j] = 0;
        }

        /* Cas ou plusieurs propositions peuvent etre correctes. */
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
                    /* On retient qu'au moins une bonne reponse a ete indiquee. */
                    auMoinsUneBonne = 1;
                }
            }

            /*
                Securite : une question sans aucune bonne reponse serait injuste
                pour l'etudiant, donc on corrige automatiquement.
            */
            if (!auMoinsUneBonne)
            {
                printf("Aucune bonne reponse choisie : la proposition 1 est mise correcte par securite.\n");
                qcm.questions[i].bonnesReponses[0] = 1;
            }
        }
        else
        {
            /* Cas simple : une seule bonne reponse est attendue. */
            int bonne = lireEntier("Numero de la bonne proposition : ", 1, qcm.questions[i].nombrePropositions);
            qcm.questions[i].bonnesReponses[bonne - 1] = 1;
        }
    }

    /* Sauvegarde finale du QCM dans un fichier. */
    if (sauvegarderQCM(&qcm))
    {
        printf("\nQCM sauvegarde avec succes.\n");
    }
    else
    {
        printf("\nErreur pendant la sauvegarde du QCM.\n");
    }

    /* On libere la memoire reservee avec malloc. */
    libererQCM(&qcm);
}

/*
    Compare les reponses de l'etudiant avec les bonnes reponses d'une question.
    La fonction renvoie 1 si tout correspond exactement, sinon 0.
*/
static int reponseCorrecte(const Question *question, const int reponses[])
{
    /* j parcourt toutes les propositions de la question. */
    int j;

    for (j = 0; j < question->nombrePropositions; j++)
    {
        /* Une seule difference suffit pour rendre la reponse incorrecte. */
        if (question->bonnesReponses[j] != reponses[j])
        {
            return 0;
        }
    }
    return 1;
}

/*
    Verifie si l'etudiant n'a coche aucune proposition pour une question.
    C'est utile quand le QCM autorise a passer une question.
*/
static int reponseVide(const Question *question, const int reponses[])
{
    /* j parcourt toutes les propositions de la question. */
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

/*
    Affiche une question et enregistre la reponse de l'etudiant.
    Les reponses sont stockees dans le tableau reponses.
*/
static void poserQuestion(const QCM *qcm, int numero, int reponses[][MAX_PROPOSITIONS])
{
    /* choix contient le numero de proposition saisi par l'etudiant. */
    int choix;

    /* j sert a afficher les propositions. */
    int j;

    /*
        question pointe vers la question actuelle.
        Cela evite de reecrire qcm->questions[numero] partout.
    */
    const Question *question = &qcm->questions[numero];

    printf("\nQuestion %d/%d", numero + 1, qcm->nombreQuestions);
    printf(" (%.1f point(s) avant conversion sur 20)\n", question->points);
    printf("%s\n", question->texte);

    /* Affichage des propositions et remise a zero des anciennes reponses. */
    for (j = 0; j < question->nombrePropositions; j++)
    {
        printf("  %d. %s\n", j + 1, question->propositions[j]);
        reponses[numero][j] = 0;
    }

    /* Cas ou l'etudiant peut choisir plusieurs propositions. */
    if (qcm->plusieursReponses)
    {
        /*
            reponseAcceptee vaut 1 si la reponse respecte les regles du QCM.
            En mode sequentiel, une reponse vide est refusee.
        */
        int reponseAcceptee;

        do
        {
            printf("Plusieurs choix possibles. Entrez les numeros un par un, puis 0 pour valider.\n");
            do
            {
                choix = lireEntier("Votre choix (0 pour terminer) : ", 0, question->nombrePropositions);
                if (choix > 0)
                {
                    /* -1 car les tableaux commencent a l'indice 0 en C. */
                    reponses[numero][choix - 1] = 1;
                }
            } while (choix != 0);

            /* Si le mode est obligatoire, on verifie que l'etudiant a coche au moins une case. */
            reponseAcceptee = !qcm->modeSequentiel || !reponseVide(question, reponses[numero]);
            if (!reponseAcceptee)
            {
                printf("Ce QCM demande une reponse avant de passer a la suite.\n");
            }
        } while (!reponseAcceptee);
    }
    else
    {
        /* Cas d'une seule reponse : l'etudiant donne directement le numero choisi. */
        choix = lireEntier("Votre choix : ", 1, question->nombrePropositions);
        reponses[numero][choix - 1] = 1;
    }
}

/*
    Calcule la note finale et affiche une correction rapide.
    La note est toujours ramenee sur 20, comme demande dans le sujet.
*/
static void afficherCorrection(const QCM *qcm, int reponses[][MAX_PROPOSITIONS])
{
    /* i parcourt les questions et j parcourt les propositions. */
    int i, j;

    /* Somme des points de toutes les questions. */
    double totalPossible = 0.0;

    /* Total obtenu par l'etudiant avant conversion sur 20. */
    double totalObtenu = 0.0;

    /* Nombre de questions entierement correctes. */
    int bonnes = 0;

    /* Nombre de questions laissees sans reponse. */
    int passees = 0;

    /* Parcours de toutes les questions pour calculer le score. */
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
            /* Penalite simple : une erreur retire la moitie des points de la question. */
            totalObtenu -= qcm->questions[i].points / 2.0;
        }
    }

    /* La note ne doit pas devenir negative. */
    if (totalObtenu < 0.0)
    {
        totalObtenu = 0.0;
    }

    afficherTitre("Resultat");

    /* Conversion du score vers une note sur 20. */
    printf("Note finale : %.2f / 20\n", (totalObtenu / totalPossible) * 20.0);
    printf("Bonnes reponses : %d / %d\n", bonnes, qcm->nombreQuestions);

    /* Des statistiques simples aident l'etudiant a comprendre son resultat. */
    printf("Questions passees : %d\n", passees);

    /* Correction rapide : on indique les numeros des bonnes propositions. */
    printf("\nCorrection rapide :\n");
    for (i = 0; i < qcm->nombreQuestions; i++)
    {
        printf("%d. %s\n", i + 1, reponseCorrecte(&qcm->questions[i], reponses[i]) ? "Correct" : "A revoir");
        printf("   Bonne(s) reponse(s) : ");
        for (j = 0; j < qcm->questions[i].nombrePropositions; j++)
        {
            /* Seules les propositions correctes sont affichees. */
            if (qcm->questions[i].bonnesReponses[j])
            {
                printf("%d ", j + 1);
            }
        }
        printf("\n");
    }
}

/*
    Gere toute la partie etudiant :
    - liste des QCM disponibles ;
    - choix d'un QCM ;
    - passage des questions ;
    - affichage de la note et de la correction.
*/
static void modeEtudiant(void)
{
    /* Tableau contenant les noms des QCM trouves dans l'index. */
    char noms[MAX_QCM_LISTE][MAX_NOM];

    /* Nombre reel de QCM disponibles. */
    int nombreQCM;

    /* Numero choisi par l'etudiant dans la liste. */
    int choix;

    /* i sert a parcourir les QCM puis les questions. */
    int i;

    /* QCM charge depuis le fichier choisi. */
    QCM qcm;

    /*
        Tableau dynamique des reponses de l'etudiant.
        Chaque ligne correspond a une question, chaque colonne a une proposition.
    */
    int (*reponses)[MAX_PROPOSITIONS];

    afficherTitre("Mode etudiant");

    /* Lecture de la liste des QCM disponibles. */
    nombreQCM = listerQCM(noms, MAX_QCM_LISTE);
    if (nombreQCM == 0)
    {
        printf("Aucun QCM disponible pour le moment.\n");
        return;
    }

    printf("QCM disponibles :\n");
    for (i = 0; i < nombreQCM; i++)
    {
        /*
            apercu sert seulement a charger rapidement le QCM pour afficher
            sa categorie dans la liste.
        */
        QCM apercu;
        printf("  %d. %s", i + 1, noms[i]);
        if (chargerQCM(noms[i], &apercu))
        {
            printf(" - categorie : %s", apercu.categorie);
            libererQCM(&apercu);
        }
        printf("\n");
    }

    /* L'etudiant choisit le QCM a passer. */
    choix = lireEntier("Numero du QCM choisi : ", 1, nombreQCM);
    if (!chargerQCM(noms[choix - 1], &qcm))
    {
        printf("Impossible de charger ce QCM.\n");
        return;
    }

    /*
        calloc initialise toutes les cases a 0.
        C'est pratique car 0 signifie "proposition non choisie".
    */
    reponses = calloc(qcm.nombreQuestions, sizeof(int[MAX_PROPOSITIONS]));
    if (reponses == NULL)
    {
        printf("Erreur memoire. QCM annule.\n");
        libererQCM(&qcm);
        return;
    }

    /* Petit resume des parametres avant de commencer le QCM. */
    afficherTitre(qcm.nom);
    printf("Categorie : %s\n", qcm.categorie);
    printf("Points negatifs : %s\n", qcm.pointsNegatifs ? "oui" : "non");
    printf("Plusieurs reponses : %s\n", qcm.plusieursReponses ? "oui" : "non");
    printf("Question obligatoire : %s\n", qcm.modeSequentiel ? "oui" : "non");

    for (i = 0; i < qcm.nombreQuestions; i++)
    {
        /*
            passer vaut 1 si l'etudiant decide de passer la question.
            Cela n'est propose que si le QCM n'est pas en mode obligatoire.
        */
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

    /* Fin du QCM : calcul et affichage du resultat. */
    afficherCorrection(&qcm, reponses);

    /* Liberation des deux zones dynamiques utilisees dans ce mode. */
    free(reponses);
    libererQCM(&qcm);
}

/*
    Fonction publique appelee par main.
    Elle affiche le menu principal jusqu'a ce que l'utilisateur choisisse de quitter.
*/
void lancerApplication(void)
{
    /* choix contient le choix du menu principal : enseignant, etudiant ou quitter. */
    int choix;

    do
    {
        afficherTitre("Gestionnaire de QCM - CY Tech");
        printf("1. Mode enseignant\n");
        printf("2. Mode etudiant\n");
        printf("0. Quitter\n");

        choix = lireEntier("Votre choix : ", 0, 2);

        /* Si l'entree a ete coupee, on quitte proprement au lieu de boucler. */
        if (entreeCoupee)
        {
            choix = 0;
        }

        /* Redirection vers le mode choisi. */
        if (choix == 1)
        {
            modeEnseignant();
        }
        else if (choix == 2)
        {
            modeEtudiant();
        }
    } while (choix != 0);

    printf("A bient\n");
}
