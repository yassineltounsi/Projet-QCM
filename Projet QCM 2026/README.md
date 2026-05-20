# Projet QCM 2025-2026

Ce projet est un gestionnaire de QCM en langage C. Il propose un mode enseignant pour creer des questionnaires et un mode etudiant pour les passer avec une correction automatique.

## Compilation

```bash
make
```

## Execution

```bash
./qcm_app
```

## Mot de passe enseignant

```text
CYTECH2026
```

## Fonctionnalites principales

- deux modes : enseignant et etudiant ;
- creation d'un QCM sauvegarde dans un fichier ;
- liste des QCM disponibles ;
- choix des parametres globaux : points negatifs, plusieurs bonnes reponses, question obligatoire ou non ;
- correction automatique et note finale sur 20 ;
- trois QCM deja fournis dans le dossier `qcm`.

## Initiatives bonus

- `[INITIATIVE BONUS]` categorie affichee pour chaque QCM ;
- `[INITIATIVE BONUS]` bareme personnalisable par question ;
- `[INITIATIVE BONUS]` statistiques de fin simples : nombre de bonnes reponses et questions passees.

Le code reste volontairement simple pour correspondre a un niveau de premiere annee : structures, tableaux, allocation dynamique, fichiers et fonctions separees.
