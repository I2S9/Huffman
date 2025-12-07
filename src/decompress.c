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

char *get_decompressed_filename(char *compressed_file, char *output_name)
{
    char *output;
    size_t len;
    
    // Si un nom de sortie est spécifié en deuxième arg on l'utilise
    if (output_name)
    {
        output = strdup(output_name);
        if (!output)
            return NULL;
        return output;
    }
    // Sinon on enlève .huff du nom d'entrée
    len = strlen(compressed_file);
    if (len < 5 || strcmp(compressed_file + len - 5, ".huff") != 0)
        return NULL;
    
    output = malloc(len - 4); // -4 pour enlever .huff et ajouter \0
    if (!output)
        return NULL;
    
    strncpy(output, compressed_file, len - 5);
    output[len - 5] = '\0';
    
    return output;
}

bool decode_file(FILE *input, FILE *output, HuffmanNode *root, uint32_t total_characters)
{
    HuffmanNode *current = root;
    unsigned char bit_buffer;
    uint32_t characters_written = 0;
    int bit_position;

    if (!root || !input || !output)
        return false;

    while (characters_written < total_characters)
    {
        // Lire un nouvel octet
        if (fread(&bit_buffer, sizeof(unsigned char), 1, input) != 1)
            return false;

        // Traiter chaque bit de l'octet
        for (bit_position = 7; bit_position >= 0 && characters_written < total_characters; bit_position--)
        {
            // Extraire le bit actuel
            bool bit = (bit_buffer >> bit_position) & 1;
            
            // Naviguer dans l'arbre
			if (bit)
				current = current->right;
			else
				current = current->left;
            
            // Si on atteint une feuille
            if (current && current->is_leaf)
            {
                if (fputc(current->character, output) == EOF)
                    return false;
                characters_written++;
                current = root; // Retour à la racine
            }
            
            // Vérification de sécurité
            if (!current)
                return false;
        }
    }
    
    return true;
}

void cleanup_decompress(char *filename, FrequencyTable *freq_table, HuffmanNode *root, FILE *input, FILE *output)
{
    if (filename)
        free(filename);
    if (freq_table)
    {
        if (freq_table->frequencies)
            free(freq_table->frequencies);
        free(freq_table);
    }
    if (root)
        free_huffman_tree(root);
    if (input)
        fclose(input);
    if (output)
        fclose(output);
}

FrequencyTable *read_frequency_table(FILE *input)
{
    FrequencyTable *table;
    uint32_t total_symbols;
    size_t read_size;

    // Lire d'abord le nombre de symboles
    read_size = fread(&total_symbols, sizeof(uint32_t), 1, input);
    if (read_size != 1)
        return NULL;

    table = malloc(sizeof(FrequencyTable));
    if (!table)
        return NULL;

    table->frequencies = calloc(MAX_SYMBOLS, sizeof(uint32_t));
    if (!table->frequencies)
    {
        free(table);
        return NULL;
    }

    table->total_symbols = total_symbols;
    table->total_characters = 0;

    // Lire chaque paire symbole-fréquence
    for (uint32_t i = 0; i < table->total_symbols; i++)
    {
        unsigned char symbol;
        uint32_t frequency;

        read_size = fread(&symbol, sizeof(unsigned char), 1, input);
        if (read_size != 1)
        {
            free(table->frequencies);
            free(table);
            return NULL;
        }

        read_size = fread(&frequency, sizeof(uint32_t), 1, input);
        if (read_size != 1)
        {
            free(table->frequencies);
            free(table);
            return NULL;
        }

        table->frequencies[symbol] = frequency;
        table->total_characters += frequency;
    }

    return table;
}

int main(int argc, char **argv)
{
    char *output_filename = NULL;
    FrequencyTable *freq_table = NULL;
    HuffmanNode *root = NULL;
    FILE *input = NULL, *output = NULL;
    
    // Vérifier et créer le nom du fichier de sortie
    if (argv[2])
        output_filename = get_decompressed_filename(argv[1], argv[2]);
    else
        output_filename = get_decompressed_filename(argv[1], NULL);

    // Ouvrir le fichier d'entrée
    input = fopen(argv[1], "rb");

    // Ouvrir le fichier de sortie
    output = fopen(output_filename, "wb");

    // Lire la table de fréquences
    freq_table = read_frequency_table(input);

    printf("Lecture de %u symboles uniques pour %u caractères totaux\n", 
           freq_table->total_symbols, freq_table->total_characters);

    // Reconstruire l'arbre de Huffman
    root = build_huffman_tree(freq_table);

    // Décoder le fichier
    decode_file(input, output, root, freq_table->total_characters);

    // Nettoyage
    cleanup_decompress(output_filename, freq_table, root, input, output);
    printf("Fichier décompressé avec succès!\n");
	
    return 0;
}