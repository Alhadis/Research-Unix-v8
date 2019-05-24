struct ptmr {
	char	unused;
	char	filler1;	/* NOTHING */
	char	p_status;	/* status register */
	char	filler2;	/* NOTHING */
	char	p_tmrc1;	/* timer 1 counter */
	char	filler3;	/* NOTHING */
	char	p_lsb1;		/* LSB buffer register */
	char	filler4;	/* NOTHING */
	char	p_tmrc2;	/* timer 2 counter */
	char	filler5;	/* NOTHING */
	char	p_lsb2;		/* LSB buffer register */
	char	filler6;	/* NOTHING */
	char	p_tmrc3;	/* timer 3 counter */
	char	filler7;	/* NOTHING */
	char	p_lsb3;		/* LSB buffer register */
};

struct ptmw {
	char	p_cr13;		/* control register 1 or 3 */
	char	filler1;	/* NOTHING */
	char	p_cr2;		/* control register 2 */
	char	filler2;	/* NOTHING */
	char	p_msb1;		/* MSB buffer register */
	char	filler3;	/* NOTHING */
	char	p_tmrl1;	/* timer 1 latches */
	char	filler4;	/* NOTHING */
	char	p_msb2;		/* MSB buffer register */
	char	filler5;	/* NOTHING */
	char	p_tmrl2;	/* timer 1 latches */
	char	filler6;	/* NOTHING */
	char	p_msb3;		/* MSB buffer register */
	char	filler7;	/* NOTHING */
	char	p_tmrl3;	/* timer 1 latches */
};

/* control bits */
#define	P_PRESET	0x01	/* CR1 only: preset timers */
#define	P_CR1SELECT	0x01	/* CR2 only: select CR1 */
#define	P_PRESCALE	0x01	/* CR3 only: prescale T3 clock by 8 */
#define	P_ENABCLK	0x02	/* use enable clock, not external */
#define	P_COUNTMODE	0x04	/* use dual 8-bit mode, not 16-bit */
#define	P_IRQEN		0x40	/* enable IRQ */
#define	P_OUTEN		0x80	/* enable timer output */

#define	P_CONTINUOUS	0x00	/* continuous operation mode */
#define	P_SINGSHOT	0x08	/* single shot operation mode */
#define	P_CSNOWRIN	0x10	/* write does not initialize */

#define	P_FREQCOMP	0x20	/* frequency comparison operation mode */
#define	P_PULSEWCOMP	0x30	/* pulse width operation mode */
#define	P_FPNOWRIN	0x08	/* write does not initialize */

/* Status Register Bits */
#define	P_INT1		0x01	/* Timer 1 interrupt flag */
#define	P_INT2		0x02	/* Timer 2 interrupt flag */
#define	P_INT3		0x04	/* Timer 3 interrupt flag */
#define	P_INTCOMP	0x80	/* Composite interrupt flag */

/* PTM locations in Design Module */
#define	PTM	0x3ff61
