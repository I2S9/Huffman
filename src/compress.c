#include "../includes/huffman.h"

// Libère la mémoire allouée pour l'arbre de Huffman
void free_huffman_tree(HuffmanNode *root) 
{
	if (!root)
		return;
	free_huffman_tree(root->left);
	free_huffman_tree(root->right);
	free(root);
}

// Nettoie la table de Huffman pour libérer la mémoire
void free_huffman_table(HuffmanTable *table) 
{
	if (!table)
		return;
	for (int i = 0; i < MAX_SYMBOLS; i++) 
	{
		if (table->used[i])
			free(table->codes[i].code);
	}
	free(table);
}

FrequencyTable	*count_frequencies(FILE *input)
{
	FrequencyTable	*table;
	int				c;

	table = malloc(sizeof(FrequencyTable));
	if (!table)
		return (NULL);
	table->frequencies = calloc(MAX_SYMBOLS, sizeof(uint32_t));
	if (!table->frequencies)
	{
		free(table);
		return (NULL);
	}
	table->total_symbols = 0;
	table->total_characters = 0;

	// Compte les fréquences

	while ((c = fgetc(input)) != EOF)
	{
		if (table->frequencies[c] == 0) 
			table->total_symbols++; // Le nombre de symboles uniques
		table->frequencies[c]++;
		table->total_characters++;
	}
	rewind(input); // Remet le curseur au début du fichier

	return (table);
}

HuffmanNode	*create_node(unsigned char c, uint32_t freq)
{
	HuffmanNode	*node;

	node = malloc(sizeof(HuffmanNode));
	if (!node)
		return (NULL);

	node->character = c;
	node->frequency = freq;
	node->left = NULL;
	node->right = NULL;
	node->is_leaf = true; // Feuille par défaut (ça peut changer)

	return (node);
}

HuffmanNode	*build_huffman_tree(FrequencyTable *freq_table)
{
	HuffmanNode	**nodes;
	int			node_count;
	int			min1_idx, min2_idx;
	HuffmanNode	*parent;

	if (!freq_table || !freq_table->frequencies || freq_table->total_symbols == 0)
		return (NULL);

	nodes = malloc(freq_table->total_symbols * sizeof(HuffmanNode *));

	if (!nodes)
		return (NULL);

	// Créer les noeuds feuilles initiaux

	node_count = 0;

	for (int i = 0; i < MAX_SYMBOLS; i++)
	{
		if (freq_table->frequencies[i] > 0)
		{
			nodes[node_count] = create_node(i, freq_table->frequencies[i]);
			if (!nodes[node_count])
			{
				// Nettoyage et sortie

				for (int j = 0; j < node_count; j++)
					free(nodes[j]);

				free(nodes);

				return (NULL);
			}
			node_count++;
		}
	}
	// Construire l'arbre
	while (node_count > 1)
	{
		// Trouver les deux noeuds minimaux
		min1_idx = 0, min2_idx = 1;

		if (nodes[min1_idx]->frequency > nodes[min2_idx]->frequency)
			SWAP(min1_idx, min2_idx);

		for (int i = 2; i < node_count; i++)
		{
			if (nodes[i]->frequency < nodes[min1_idx]->frequency)
			{
				min2_idx = min1_idx;
				min1_idx = i;
			}

			else if (nodes[i]->frequency < nodes[min2_idx]->frequency)
				min2_idx = i;
		}

		// Créer le noeudd parent
		parent = create_node(0, nodes[min1_idx]->frequency + nodes[min2_idx]->frequency);
		if (!parent)
		{
			// Nettoyage et sortie
			for (int i = 0; i < node_count; i++)
				free(nodes[i]);
			free(nodes);
			return (NULL);
		}
		parent->is_leaf = false;
		parent->left = nodes[min1_idx];
		parent->right = nodes[min2_idx];

		// Remplacer min1 par le parent et min2 par le dernier nœud
		nodes[min1_idx] = parent;
		nodes[min2_idx] = nodes[node_count - 1];
		node_count--;
	}
	HuffmanNode *root = nodes[0];
	free(nodes);
	return (root);
}

void generate_codes_recursive(HuffmanNode *node, uint8_t *current_code, size_t depth, HuffmanTable *table) 
{
	if (!node)
		return;

	// Si nous sommes à un noeud feuille, stocker le code
	if (node->is_leaf) 
	{
		// Allouer l'espace pour le code (arrondi à l'octet supérieur)
		size_t bytes = (depth + 7) / 8;

		table->codes[node->character].code = malloc(bytes);
		if (!table->codes[node->character].code)
			return;
		// Copier le chemin actuel comme code
		memcpy(table->codes[node->character].code, current_code, bytes);
		table->codes[node->character].length = depth;
		table->used[node->character] = true;
		return;
	}

	// Parcourir à gauche (ajouter 0)
	if (node->left) 
	{
		current_code[depth / 8] &= ~(1 << (7 - (depth % 8)));  // Mettre le bit à 0
		generate_codes_recursive(node->left, current_code, depth + 1, table);
	}

	// Parcourir à droite (ajouter 1)
	if (node->right) 
	{
		current_code[depth / 8] |= (1 << (7 - (depth % 8)));   // Mettre le bit à 1
		generate_codes_recursive(node->right, current_code, depth + 1, table);
	}
}

HuffmanTable	*generate_huffman_codes(HuffmanNode *root)
{
	HuffmanTable	*table;
	uint8_t			current_code[MAX_TREE_HEIGHT / 8] = {0};

	table = calloc(1, sizeof(HuffmanTable));
	if (!table)
		return (NULL);

	// Tableau temporaire pour construire les codes
	generate_codes_recursive(root, current_code, 0, table);
	return (table);
}

char	*get_compressed_filename(char *input_file)
{
	char	*output;
	size_t	len;

	if (!input_file)
		return (NULL);

	len = strlen(input_file);

	output = malloc(len + 5 + 1); // +5 pour l'extension ".huff"
	if (!output)
		return (NULL);

	strcpy(output, input_file);
	strcat(output, ".huff");
	return (output);
}

bool write_compressed_file(FILE *input, FILE *output, FrequencyTable *freq_table, HuffmanTable *codes) 
{
	// 1. D'abord écrire la table de fréquences comme en-tête
	// Écrire le nombre de symboles uniques
	fwrite(&freq_table->total_symbols, sizeof(uint32_t), 1, output);
	
	// Écrire les fréquences pour les symboles utilisés
	for (int i = 0; i < MAX_SYMBOLS; i++) 
	{
		if (freq_table->frequencies[i] > 0) 
		{
			// Écrire le symbole et sa fréquence
			fwrite(&i, sizeof(unsigned char), 1, output);
			fwrite(&freq_table->frequencies[i], sizeof(uint32_t), 1, output);
		}
	}
	
	// 2. Écrire les données compressées
	unsigned char bit_buffer = 0;  // Buffer pour stocker les bits avant l'écriture
	int bits_in_buffer = 0;       // Nombre de bits actuellement dans le buffer
	int c;
	
	// Relire le fichier d'entrée et écrire les données compressées
	rewind(input);
	while ((c = fgetc(input)) != EOF) 
	{
		// Obtenir le code pour ce caractère
		uint8_t *code = codes->codes[c].code;
		uint32_t code_length = codes->codes[c].length;
		
		// Écrire les bits du code
		for (uint32_t i = 0; i < code_length; i++) 
		{
			// Obtenir le bit actuel du code
			unsigned char current_bit = (code[i / 8] >> (7 - (i % 8))) & 1;
			// Ajouter le bit au buffer
			bit_buffer = (bit_buffer << 1) | current_bit;
			bits_in_buffer++;
			// Si le buffer est plein, l'écrire dans le fichier
			if (bits_in_buffer == 8) 
			{
				fwrite(&bit_buffer, 1, 1, output);
				bit_buffer = 0;
				bits_in_buffer = 0;
			}
		}
	}
	// Écrire les bits restants dans le buffer
	if (bits_in_buffer > 0) 
	{
		// Compléter avec des zéros pour terminer l'octet
		bit_buffer <<= (8 - bits_in_buffer);
		fwrite(&bit_buffer, 1, 1, output);
	}
	return (true);
}

/* Écrit la définition d'un noeud dans le fichier DOT
Gère différents cas pour l'affichage des caractères spéciaux */
void	write_node_definition(FILE *dot_file, HuffmanNode *node, int node_id)
{
    // Si c'est pas un noeud feuille, afficher la fréquence
    if (!node->is_leaf)
        fprintf(dot_file, "    node%d [label=\"%d\", shape=circle];\n", node_id,
            node->frequency);
    else
    {
        // Pour les caractères spéciaux
        if (node->character == ' ')
            fprintf(dot_file, "    node%d [label=\"espace\\n%d\", shape=box];\n", 
            node_id, node->frequency);
        else if (node->character < 32 || node->character > 126)
            fprintf(dot_file, "    node%d [label=\"byte %d\\n%d\", shape=box];\n", 
            node_id, node->character, node->frequency);
        else if (node->character == '"' || node->character == '\\')
            fprintf(dot_file, "    node%d [label=\"\\%c\\n%d\", shape=box];\n", 
            node_id, node->character, node->frequency);
        else
            fprintf(dot_file, "    node%d [label=\"%c\\n%d\", shape=box];\n", 
            node_id, node->character, node->frequency);
    }
}

/* Parcourt l'arbre de façon récursive et écrit les connections entre les noeuds
Retourne l'ID du dernier noeud traité */
int	write_tree_recursive(FILE *dot_file, HuffmanNode *node, int current_id)
{
    int	my_id;

    my_id = current_id;
    if (!node)
        return (current_id);
    write_node_definition(dot_file, node, my_id);
    if (node->left)
    {
        fprintf(dot_file, "    node%d -> node%d [label=\"0\"];\n", my_id,
            current_id + 1);
        current_id = write_tree_recursive(dot_file, node->left, current_id + 1);
    }
    if (node->right)
    {
        fprintf(dot_file, "    node%d -> node%d [label=\"1\"];\n", my_id,
            current_id + 1);
        current_id = write_tree_recursive(dot_file, node->right, current_id + 1);
    }
    return (current_id);
}

void print_compression_stats(FILE *input_file, FILE *compressed_file, double compression_time) 
{
	struct stat file_info;
	size_t original_size = 0, compressed_size = 0;
	double compression_ratio = 0.0;

	// Obtenir la taille du fichier original
	if (fstat(fileno(input_file), &file_info) == 0)
		original_size = file_info.st_size;

	// Obtenir la taille du fichier compressé
	if (fstat(fileno(compressed_file), &file_info) == 0)
		compressed_size = file_info.st_size;

	// Calculer le taux de compression
	if (compressed_size > 0)
		compression_ratio = (double)original_size / compressed_size;

	printf("\n=== Statistiques de Compression ===\n");
	printf("Temps de compression: %.4f secondes\n", compression_time);
	printf("Taille originale: %zu octets\n", original_size);
	printf("Taille compressée: %zu octets\n", compressed_size);
	printf("Taux de compression: %.2fx\n", compression_ratio);
	printf("Espace économisé: %.2f%%\n", (1.0 - (double)compressed_size / original_size) * 100.0);
}

// Libère toutes les ressources allouées pendant la compression
void cleanup(char *filename, FrequencyTable *freq_table, HuffmanTable *codes, FILE *input, FILE *output, HuffmanNode *root)
{
	if (filename)
		free(filename);
	if (freq_table)
	{
		free(freq_table->frequencies);
		free(freq_table);
	}
	if (codes)
		free_huffman_table(codes);
	if (input)
		fclose(input);
	if (output)
		fclose(output);
	if (root)
		free_huffman_tree(root);
}

int	main(int argc, char **argv)
{
	FrequencyTable	*freq_table;
	char			*output_filename;
	HuffmanNode		*root;
	HuffmanTable	*codes;
	FILE *input, *output;
	struct timeval start, end;
	double compression_time;

	// Donne le temps au début de la compression
	gettimeofday(&start, NULL);

	// Génération du nom du fichier de sortie
	output_filename = get_compressed_filename(argv[1]);

	// Initialisation des fichiers d'entrée et de sortie 
	input = fopen(argv[1], "rb");
	output = fopen(output_filename, "wb");

	// Phase 1: Analyse des fréquences des caractères
	freq_table = count_frequencies(input);
	printf("Trouvé %u symboles uniques sur %u caractères au total\n",
		freq_table->total_symbols, freq_table->total_characters);

	// Phase 2: Construction de l'arbre de Huffman et génération des codes
	root = build_huffman_tree(freq_table);
	codes = generate_huffman_codes(root);

	// Phase 3: Compression et écriture du fichier
	write_compressed_file(input, output, freq_table, codes);

	// Donne le temps à la fin de la compression
	gettimeofday(&end, NULL);

	// Calculer le temps de compression
	compression_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

	printf("Fichier compressé avec succès!\n");

	// Afficher les statistiques de compression
	print_compression_stats(input, output, compression_time);
	cleanup(output_filename, freq_table, codes, input, output, root);
	return (0);
}