#define	OARMAG	0177545
struct	oar_hdr {
	char	oar_name[14];
	long	oar_date;
	char	oar_uid;
	char	oar_gid;
	int	oar_mode;
	long	oar_size;
};
