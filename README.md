# Huffman Compression

> A file compression algorithm implementation using Huffman coding. This project provides lossless compression and decompression of files by constructing optimal prefix codes based on character frequency analysis. The implementation uses a greedy approach to build a binary tree where frequent characters receive shorter codes and rare characters receive longer codes, achieving compression through variable-length encoding.

## Algorithmic Approach

### Compression

**Frequency Analysis**
The algorithm performs a single linear pass through the input file to count character frequencies. This step has O(n) time complexity where n is the number of characters in the file.

**Huffman Tree Construction**
A greedy iterative algorithm builds the Huffman tree:
- Create a leaf node for each unique symbol with its frequency
- Iteratively select the two nodes with minimum frequencies
- Merge them into a parent node with combined frequency
- Repeat until a single root node remains

The minimum selection uses linear search, resulting in O(k²) complexity where k is the number of unique symbols. A priority queue would reduce this to O(k log k).

**Code Generation**
A recursive depth-first traversal assigns binary codes:
- Left child path represents bit 0, right child path represents bit 1
- The path from root to leaf forms the character's code
- Frequent characters naturally receive shorter codes

**Encoding**
Each character is replaced by its variable-length binary code. A bit buffer manages writing codes of different lengths into complete bytes.

### Decompression

**Tree Reconstruction**
The frequency table stored in the compressed file header is read, and the Huffman tree is reconstructed using the same greedy method.

**Decoding**
Tree traversal guided by input bits:
- Bit 0 → traverse left, bit 1 → traverse right
- Upon reaching a leaf, output the character and return to root
- Repeat until all characters are decoded

## Complexity Analysis

- **Time**: O(n + k²) for compression, O(n) for decompression, where n is file size and k is the number of unique symbols
- **Space**: O(k) for the tree and code tables

## Compilation

Build both executables:
```bash
make
```

Build only the compression tool:
```bash
make compress
```

Build only the decompression tool:
```bash
make decompress
```

Clean generated executables:
```bash
make clean
```

## Usage

Compress a file:
```bash
./compress <input_file> <output_file>
```

Decompress a file:
```bash
./decompress <compressed_file> <output_file>
```