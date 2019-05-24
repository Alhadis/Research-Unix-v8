#define EKGC	(('e'<<8)|0)
#define EKDC	(('e'<<8)|1)
#define EKGS	(('e'<<8)|2)
#define EKDS	(('e'<<8)|3)
#define EKDEV	(('e'<<8)|4)
#define EKMOD	(('e'<<8)|5)
#define EKDMA	(('e'<<8)|6)
#define EK_8BITST	06
#define EK_12BITST	04
/* general control register bits */
#define EKGC_HOME	0200
#define EKGC_CONS	0100
#define EKGC_DARK	040
#define EKGC_RS		04
#define EKGC_RDY	02
#define EKGC_IE		01

/* device control register device codes */
#define DC_MASK		07777
#define EKDC_INT	0
#define EKDC_MAG	040000
#define EKDC_ND		050000
#define EKDC_SOL	0100000
#define EKDC_CON	0110000
#define EKDC_STG	0140000
#define EKDC_STGPO	0140000

/* device control general bits */
#define EKDC_READ	04000
#define EKDC_CC		02000
#define EKDC_IE		01

/* device control stage position mask */
#define EKDC_STP_M	017777
/*device control solenoid control bits */
#define EKDC_SO_DK	040
#define EKDC_SO_ND	020
#define EKDC_SO_ON	02
#define EKDC_SO_OF	04

/* device control stage control bits */
#define EKDC_ST_POS	02000
#define EKDC_ST_UPA	0200
#define EKDC_ST_DNA	0100
#define EKDC_ST_HM	040
#define EKDC_ST_FL	020
#define EKDC_ST_UP	010
#define EKDC_ST_DN	04

/* device control console control bits */
#define EKDC_CN_SW	020
#define EKDC_CN_OL	010
#define EKDC_CN_RDY	02

/* general status bits */
#define EKGS_DIR	01
#define EKGS_DSR	02
#define EKGS_CON	04
#define EKGS_FIL	010
#define EKGS_STG	020
#define EKGS_ONL	0100
#define EKGS_RUN	0200
#define EKGS_BUSY	0100000

/* device status bits - general */
#define EKDS_ER		010
#define EKDS_IG		077
/* device status magnification bits */
#define EKDS_MG_MV	0400
#define EKDS_MG_AD	0377
/* device status - solonoid bits */
#define EKDS_SO_DK	040
#define EKDS_SO_ND	020
/* device status - console bits */
#define EKDS_CO_ON	040
#define EKDS_CO_RUN	020
/* device status - stage bits */
#define EKDS_ST_FU	040
#define EKDS_ST_HM	020
