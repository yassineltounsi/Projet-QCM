# Rapport court - Projet QCM

## Presentation

Le projet realise est une application console en C permettant de creer, sauvegarder et passer des QCM. Le programme respecte le cahier des charges avec un mode enseignant protege par mot de passe et un mode etudiant qui charge les QCM existants.

## Organisation du code

- `main.c` lance l'application.
- `interface.c` gere les menus, les saisies et le deroulement du QCM.
- `qcm.c` gere les fichiers, la sauvegarde et le chargement.
- `qcm.h` contient les structures principales.

Cette separation evite d'avoir tout le code dans un seul fichier et rend le projet plus lisible.

## Choix techniques

Les QCM sont sauvegardes dans des fichiers texte dans le dossier `qcm`. Un fichier `index.txt` contient la liste des QCM disponibles. Ce choix rend le fonctionnement facile a verifier pendant la soutenance.

La note est toujours convertie sur 20. Quand les points negatifs sont actives, une mauvaise reponse retire la moitie des points de la question. La note ne descend jamais sous 0.

## Difficultes et solutions

La principale difficulte est la gestion des saisies utilisateur. Pour eviter les erreurs et les boucles infinies, les entrees sont controlees avec des fonctions de lecture qui redemandent une valeur tant que la saisie n'est pas correcte.

## Initiatives bonus

- `[INITIATIVE BONUS]` Les QCM ont une categorie.
- `[INITIATIVE BONUS]` Le professeur peut choisir un bareme different par question.
- `[INITIATIVE BONUS]` L'etudiant obtient des statistiques simples a la fin.

Ces ajouts restent raisonnables et proches du niveau attendu en premiere annee.
