/*
 *	Symbol table node.  Contains string, string table index,
 *	left and right pointers, and instance list.
 */
struct symbol
{
	char	*sy_name;
	long	sy_index;
	symbol	*sy_left;
	symbol	*sy_right;
	inst	*sy_inst;
};
