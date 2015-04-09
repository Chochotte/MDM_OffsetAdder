/*************************************************************************************
*				Programme de gestion des offset en tension 							 *
*				sur les fichiers de mesures IC-CAP (.mdm) 			 				 *
*			ainsi que la supression des catégorie Vb=0V et Vs=0v. 					 *
**************************************************************************************
																by Laurent VARIZAT

	INFOS :
		1) Ce programme prend en arguments :
				- le type : NMOS ou PMOS
				- le fichier à modifier 

		2) On doit avoir : 
				- Vb un connecteur (CON) ou une liste (LIST) de 4 éléments
				- Vs un connecteur (CON) ou une liste (LIST) de 2 éléments
				- Vg et Vd linéaire (LIN).		
																					 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAILLE_MAX	1000
#define NB_PARAM 	10	
#define V_OFFSET	3.3		// Offset de décalage (en Volts)

/* un script shell modifie le nom du fichier (ajout de "_BAK" à la fin) avant d'appeler la fonction main. */
int main (int argc, char * argv[])
{
    FILE * 	fancien 	= NULL;
    FILE * 	fnouveau 	= NULL;
    char 	a, b, c;
    char * 	chaine 		= (char *) malloc(sizeof(char) * TAILLE_MAX);
	char * 	ptrChaine 	= NULL;
    int 	type 		= 0; 	// 0 : UNKNOWN, 1 : NMOS, 2 : PMOS
	int 	nb_modif 	= 0; 	// compte le nb de ligne modifiée : 4 pour l'en-tete...
	int 	cpt_ligne	= 0; 	// compte le nombre de ligne apres une ligne X
	float	params[NB_PARAM];
	int 	ligne 		= 0;	// numéro de la ligne traitée
	int 	stop		= 0;	// On ne recopie les lignes que si stop vaut 0
	float	v_offset	= V_OFFSET;		// On enleve l'offset sur les PMOS

    printf("Execution du programme de correction des mesures ICCAP ...\n\n");

    // Récupération du type et du nom de fichier
    if (argc != 3) {
        printf("Veuillez entrer 2 arguments : \
               \n - le type de transistor (\"NMOS\" ou \"PMOS\") \
               \n - le nom du fichier a transformer (avec son chemin)\n");
        return 0;
    }

    if (!strcmp("NMOS", argv[1])) type = 1;
    else if (!strcmp("PMOS", argv[1])) type = 2;
    if (type == 0) {
        printf("Veuillez entrer 2 arguments : \
               \n - le type de transistor (\"NMOS\" ou \"PMOS\") \
               \n - le nom du fichier à transformer (avec son chemin)\n");
        return 0;
    }
	if (type == 1) v_offset = 0;

 	// Ouverture des fichiers (l'ancien et le nouveau)
    strcpy(chaine, argv[2]);
    strcat(chaine, "_BAK");
    fancien = fopen(chaine, "r");
    if (!fancien) {
        printf("Erreur lors de l'ouverture du fichier %s", chaine);
        exit(1);
    }

    fnouveau = fopen(argv[2], "w");
    if (!fnouveau) {
        printf("Erreur lors de la création du fichier %s", argv[2]);
        exit(1);
    }

    // On parcours le fichier tant qu'on a pas d'erreur
    while (fgets(chaine, TAILLE_MAX, fancien)) {
		
		ligne++;
		sscanf(chaine, " %c%c ", &a, &b); // On récupère les 2 premières lettres de la chaine

		/* MODIFICATION DE L'EN-TETE */
		if (nb_modif < 4 && a == 'v') { 	// si je suis dans l'en-tete (moins de 4 lignes modifiée) et que la première lettre commence par v
			if (b == 'g' || b == 'd') {		// pour vg et vd (dans l'en-tete)
				ptrChaine = strstr(chaine, "LIN") + 3 * sizeof(char);	// On déplace le pointeur de la chaine
						// On lit les paramètres de VG ou VD afin de modifier leurs valeurs
				if (sscanf(ptrChaine, " %f %f %f %f %f ", &params[0], &params[1], &params[2], &params[3], &params[4]) != 5) 
					printf("WARNING : Les 5 parametres de %c%c n'ont pas été récupéré correctement\n", a, b); 
						// On ré-écrit la nouvelle chaine avec les nouvelles valeures
				sprintf(ptrChaine, "\t%d %.2f %.2f %d %.2f\n\0", (int) params[0], params[1] - v_offset, params[2] - v_offset, (int) params[3], params[4]);
						// On a changé une ligne, on incrémente le compteur
				nb_modif++;
			}

			else if (b == 'b') {			// pour vb dans l'en-tete
				ptrChaine = strstr(chaine, "LIST") + 4 * sizeof(char);	// On déplace le pointeur de la chaine
				if ((int)ptrChaine > 4) {	// On a une liste
							// On lit les paramètres de VB afin de supprimer le zéro final et de retirer l'offset
					if (sscanf(ptrChaine, " %f %f %f %f %f %f ", &params[0], &params[1], &params[2], &params[3], &params[4], & params[5]) != 6) 
						printf("WARNING : Les 6 parametres de %c%c n'ont pas été récupéré correctement\n", a, b); 
							// On ré-écrit la nouvelle chaine avec les nouvelles valeures
					sprintf(ptrChaine, "\t%d %d %.2f %.2f %.2f\n\0", (int) params[0], (int) params[1] - 1, params[2] - v_offset, params[3] - v_offset, params[4] - v_offset);
				}
				else {		// On a un connecteur (normalement)
					ptrChaine = strstr(chaine, "CON") + 3 * sizeof(char);
							// On lit le paramètre de VB afin de retirer l'offset
					if (sscanf(ptrChaine, " %f ", &params[0]) != 1) 
						printf("WARNING : Le parametre de %c%c n'a pas été récupéré correctement\n", a, b); 
							// On ré-écrit la nouvelle chaine avec la nouvelle valeure
					sprintf(ptrChaine, "\t%.2f\n\0", params[0] - v_offset);
				}
						// On a changé une ligne, on incrémente le compteur
				nb_modif++;
			}

			else if (b == 's') {			// pour vs dans l'en-tete
				ptrChaine = strstr(chaine, "LIST") + 4 * sizeof(char);	// On déplace le pointeur de la chaine
				if ((int)ptrChaine > 4) { 	// On a une liste
							// On lit les paramètres de VS afin de supprimer le zéro final et de retirer l'offset
					if (sscanf(ptrChaine, " %f %f %f %f ", &params[0], &params[1], &params[2], &params[3]) != 4) 
						printf("WARNING : Les 4 parametres de %c%c n'ont pas été récupéré correctement\n", a, b); 
							// On ré-écrit la nouvelle chaine avec les nouvelles valeures
					sprintf(ptrChaine, "\t%d %d %.0f\n\0", (int) params[0], (int) params[1] - 1, params[2] - v_offset);
				}
				else {		// On a un connecteur (normalement)
					ptrChaine = strstr(chaine, "CON") +3 * sizeof(char);
							// On lit le paramètre de VB afin de retirer l'offset
					if (sscanf(ptrChaine, " %f ", &params[0]) != 1) 
						printf("WARNING : Le parametre de %c%c n'a pas été récupéré correctement\n", a, b); 
							// On ré-écrit la nouvelle chaine avec la nouvelle valeure
					sprintf(ptrChaine, "\t%.2f\n\0", params[0] - v_offset);
				}
						// On a changé une ligne, on incrémente le compteur
				nb_modif++;
			}
		}

		/* ON MODIFIE LE CORP DU FICHIER */
		else if (nb_modif > 3) {
			// Si notre ligne courante fait partie de la description des stimulis vd, vb, vs ou/et vg
			if ((ptrChaine = strstr(chaine, "ICCAP_VAR")) != NULL) {
				ptrChaine += 9 * sizeof(char); // On se place apres le mot ICCAP_VAR
					// On récupère les infos importante de la ligne (variable + valeur)	
				if (sscanf(ptrChaine, " %c%c %f ", &a, &b, &params[0]) != 3) 
					printf("WARNING : Parametre manquant  !!\n");
				
				if ((b == 's' || b == 'b') && params[0] == 0) {	// si VS = 0V ou VB = 0V : on ne recopie plus
					 // ATTENTION : il serait mieux de supprimer les derniere lignes inutile
					stop = 1;
				}
					// On ré-écrit la nouvelle chaine avec les nouvelles valeures
				else sprintf(ptrChaine, " %c%c \t%.2f\t\n\0", a, b, params[0] - v_offset); 
				cpt_ligne++;
			}

			else if ((ptrChaine = strstr(chaine, "END_DB")) != NULL) {
				cpt_ligne	= 0;
				stop 		= 0;
			}
			else if (cpt_ligne > 0 && cpt_ligne < 5) cpt_ligne++;

			else if (cpt_ligne >= 5) {			// On modifie la première valeur de v_offset
				if (sscanf(chaine, " %f %f %f %f %f ", &params[0], &params[1], &params[2], &params[3], &params[4]) != 5) 
					printf("WARNING : Les 5 mesures v + 4i n'ont pas été récupéré correctement\n"); 
						// On ré-écrit la nouvelle chaine avec les nouvelles valeures
				sprintf(chaine, "  %f \t%f \t%f \t%f \t%f \n\0", params[0] - v_offset, params[1], params[2], params[3], params[4]);
			}	
		}

		/* ON ECRIT LA NOUVELLE CHAINE DANS LE FICHIER */
		if (!stop)
			fprintf(fnouveau, "%s", chaine);
    }

    fclose(fancien);
    fclose(fnouveau);

	printf("SUCCESS\n");

    return EXIT_SUCCESS;
}
