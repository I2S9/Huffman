#ifndef HUFFMAN_H
# define HUFFMAN_H

// Importation des bibliothèques
# include <stdbool.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <ctype.h>
# include <sys/stat.h>
# include <time.h>
# include <sys/time.h>

// Hauteur maximale de l'arbre de Huffman
# define MAX_TREE_HEIGHT 256

// Nombre maximum de symboles uniques
# define MAX_SYMBOLS 256 

// Macro pour échanger deux valeurs
# define SWAP(a, b)    \
	{                 \
		int temp = a; \
		a = b;        \
		b = temp;     \
	}				  \

typedef struct
{
	uint32_t	*frequencies;  // Liste des fréquences pour chaque symbole
	uint32_t total_symbols;    // Nombre de symboles uniques
	uint32_t total_characters; // Nombre total de caractères dans le fichier
}				FrequencyTable;

typedef struct HuffmanNode
{
	unsigned char character;   // Caractère stocké dans le noeud
	uint32_t frequency;        // Fréquence d'apparition
	struct HuffmanNode *left;  // Fils gauche
	struct HuffmanNode *right; // Fils droit
	bool is_leaf;              // Indique si le noeud est une feuille
}			HuffmanNode;

typedef struct
{
	uint8_t *code;   // Code binaire pour le symbole
	uint32_t length; // Longueur du code en bits
}			HuffmanCode;

typedef struct
{
	HuffmanCode codes[MAX_SYMBOLS]; // Table des codes pour chaque symbole
	bool used[MAX_SYMBOLS];         // Indique quels symboles sont utilisés
}			HuffmanTable;

#endif