/*
 *  Ethernet pup header
 */
struct puphead {
		unsigned char dest[6];
		unsigned char source[6];
		unsigned short type;
};

