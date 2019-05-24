struct	dkmodule {
	char	*dkstate;		/* open/closed status of channels */
	struct	queue *listnrq;		/* channel to controller */
	short	dev;			/* major device */
	short	nchan;			/* number of channels */
};
