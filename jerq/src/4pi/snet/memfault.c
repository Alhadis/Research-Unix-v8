#define ROM_BASE	0x200000
#define ROM_SIZE	(32*1024)
#define ROM_END		(ROM_BASE+ROM_SIZE)
#define	SNET_ROM(a)	((a)>=ROM_BASE && (a)<ROM_END)

#define RAM_BASE	0
#define RAM_SIZE	(500*1024)				/* bogus maxmem */
#define RAM_END		(RAM_BASE+RAM_SIZE)
#define	SNET_RAM(a)	((a)>=RAM_BASE && (a)<RAM_END)

char *AlignErr(addr,size)
long addr;
{
	if( !addr ) return "zero address";
	return addr&(size-1) ? "alignment error" : (char*)0;
}

char *ReadErr(addr,size)
register long addr;
{
	return (SNET_RAM(addr) || SNET_ROM(addr))
		? AlignErr(addr,size)
		: "memory read error";
}

char *WriteErr(addr,size)
register long addr;
{
	return SNET_RAM(addr) ? AlignErr(addr,size) : "memory write error";
}
