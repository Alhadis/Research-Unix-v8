typedef struct Trie{
	char byte;		/* byte to match */
	struct Trie *link;	/* trie node if byte matches */
	struct Trie *next;	/* next in list (if byte doesn't match) */
}Trie;
extern Trie	*tcreate();
extern Trie	*tlookup();
