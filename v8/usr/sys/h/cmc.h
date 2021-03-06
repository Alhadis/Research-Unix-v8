/*
 *	datakit common-control interface definitions
 */



/*
 *  protocol codes generated by driver
 */
#define T_CHG		3	/* status change to/from cmc */
#define   D_CLOSE	  1	/* close a channel */
#define   D_ISCLOSED	  2	/* channel is closed */
#define   D_CLOSEALL	  3	/* close all channels */

#define T_LSTNR		4	/* keep-alive message */

#define T_REPLY		2	/* reply to channel setup */
#define   D_OK		  1	/* setup OK */
#define   D_OPEN	  2	/* channel now open */
#define   D_FAIL	  3	/* setup failed */

#define T_RESTART	8	/* cmc crashed, we should init circuit */

struct	lmsg	{
	char	type ;		/* message type T_??? */
	char	srv ;		/* message code D_??? */
	short	param0 ;		/* various aditional info */
	short	param1 ;
	short	param2 ;
	short	param3 ;
	short	param4 ;
} ;
