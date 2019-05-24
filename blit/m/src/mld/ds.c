#include "stdio.h"
#include "../a.out.h"
#include "../ld.h"
#include "../sym.h"
#include "../optim.h"
#define UNK 0
#define b20(n)	((n) & 07)
#define b30(n)	((n) & 0xf)
#define b43(n)	(((n) & 030) >> 3)
#define b50(n)	((n) & 077)
#define b54(n)	(((n) & 060) >> 4)
#define b53(n)	(((n) & 070) >> 3)
#define b64(n)	(((n) & 0160) >> 4)
#define b70(n)	((n) & 0xff)
#define b76(n)	(((n) & 0xc0) >> 6)
#define b83(n)	(((n) & 0x1f8) >> 3)
#define b84(n)	(((n) & 0x1f0) >> 4)
#define b86(n)	(((n) & 0x1c0) >> 6)
#define b109(n)	(((n) & 0x600) >> 9)
#define b118(n) (((n) & 0xf00) >> 8)
#define b119(n)	(((n) & 0xe00) >> 9)
#define b1412(n) (((n) & 0x7000) >> 12)

#define foo(i)	(((i & 0700) >> 3) | ((i & 07000) >> 9))
struct exec hdr;
unsigned short *txt, *dta;
struct nlist *syms;
struct reloc *txtr, *dtar;
long addr;
short bit[16];
#define bwl(n)	str[b76(n)]
char *str[] = {"b", "w", "l", "?"};
#define len(n)	xlen[b76(n)]
int xlen[] = {1, 2, 4, 0};
char *cond[] = {"t", "f", "hi", "ls", "cc", "cs", "ne",
	"eq", "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le"};
char *bitop[] = {"btst", "bchg", "bclr", "bset"};

main()
{	int i;
	for(i = 0; i < 16; i++)
		bit[i] = 1 << i;
	read(0, (char *)&hdr, sizeof(hdr));
	if(hdr.a_magic != A_MAGIC1 && hdr.a_magic != 0406) {
		fprintf(stderr, "not header %o\n", hdr.a_magic);
		exit(1);
	}
	if(hdr.a_flag) {
		fprintf(stderr, "no reloc bits\n");
	}
	txt = (short *)malloc(hdr.a_text);
	read(0, (char *)txt, hdr.a_text);
	dta = (short *)malloc(hdr.a_data);
	read(0, (char *)dta, hdr.a_data);
	txtr = (struct reloc *)malloc(hdr.a_text);
	read(0, (char *)txtr, hdr.a_text);
	dtar = (struct reloc *)malloc(hdr.a_data);
	read(0, (char *)dtar, hdr.a_data);
	syms = (struct nlist *)malloc(hdr.a_syms);
	read(0, (char *)syms, hdr.a_syms);
	textout();
}

textout()
{	int oaddr;
	while(addr < hdr.a_text) {
		oaddr = addr;
		printf("%4x: ", addr + hdr.a_unused);
		addr += instr(addr / sizeof(short));
		reltxt(oaddr/2, addr/2);
		putchar('\n');
	}
}

instr(n)	/* fat chance */
{	int del = 0;
	unsigned short i = txt[n];
	long x;
	addr += 2;
	switch((i & 0xf000) >> 12) {
	case 0:	/* start with bit 8 */
		/* if bit 8 is on, either movp or dynamic bit */
		if(i & bit[8]) {
			if((i & 070) == 010) {
				printf("movp.");
				if(i & bit[6])
					putchar('l');
				else
					putchar('w');
				if(i & bit[7])
					printf(" %%d%d,0x%x(%%a%d)",
						b119(i), txt[n+1], b20(i));
				else
					printf(" 0x%x(%%a%d),%%d%d",
						txt[n+1], b20(i), b119(i));
				del += 2;
			}
			else {
				printf("%s %%d%d,", bitop[b76(i)], b119(i));
				del += prea(b50(i), 1);
			}
		}
		/* bits 11, 10, 9 distinguish */
		else if(b119(i) == 4) {
			printf("%s &%d(%%d%d),", bitop[b76(i)], txt[n+1],
				b119(i));
			addr += 2;
			del += prea(b50(i), 1);
		}
		else {
			if(b119(i) == 0)
				printf("ori");
			else if(b119(i) == 1)
				printf("andi");
			else if(b119(i) == 2)
				printf("subi");
			else if(b119(i) == 3)
				printf("addi");
			else if(b119(i) == 5)
				printf("eori");
			else if(b119(i) == 6)
				printf("cmpi");
			else
				printf("??immediate");
			printf(".%s ", bwl(i));
			addr += 2;
			if(len(i) == 1)
				printf("&0x%x,", txt[n+1]);
			else if(len(i) == 2)
				printf("&0x%x,", txt[n+1]);
			else {
				addr += 2;
				x = 0;
				x |= txt[n+2];
				x |= (txt[n+1] << 16);
				printf("&0x%x,", x);
			}
			del += prea(b50(i), len(i));
		}
		break;
	case 1:
		printf("mov.b ");
		addr += prea(b50(i), 1);
		del = 0;
		putchar(',');
		del += prea(foo(i), 1);
		break;
	case 2:
		printf("mov.l ");
		addr += prea(b50(i), 4);
		del = 0;
		putchar(',');
		del += prea(foo(i), 4);
		break;
	case 3:
		printf("mov.w ");
		addr += prea(b50(i), 2);
		del = 0;
		putchar(',');
		del += prea(foo(i), 2);
		break;
	case 4:	/* miscellany, 11, 10, 9, 8 mostly distinguish */
		switch(b118(i)) {
		case 0:
			if(b76(i) == 3) {
				printf("mov sr,");
				del += prea(b50(i), 2);
			}
			else {
				printf("negx.%s ", bwl(i));
				del += prea(b50(i), len(i));
			}
			break;
		case 2:
			printf("clr.%s ", bwl(i));
			del += prea(b50(i), len(i));
			break;
		case 4:
			if(b76(i) == 3) {
				printf("mov ");
				del += prea(b50(i), 2);
				printf(",ccr");
			}
			else {
				printf("neg.%s ", bwl(i));
				del += prea(b50(i), len(i));
			}
			break;
		case 6:
			if(b76(i) == 3) {
				printf("mov ");
				del += prea(b50(i), 2);
				printf(",sr");
			}
			else {
				printf("not.%s ", bwl(i));
				del += prea(b50(i), len(i));
			}
			break;
		case 8:
			if(b76(i) == 0) {
				printf("nbcd ");
				del += prea(b50(i), 1);
			}
			else if(b76(i) == 1) {
				if(b53(i) == 0)
					printf("swap %%d%d", b20(i));
				else {
					printf("pea ");
					del += prea(b50(i), 4);
				}
			}
			else if(b76(i) == 2) {
				if(b53(i) == 0)
					printf("ext.w %%d%d", b20(i));
				else {
					printf("movm.w &0x%x,", txt[n+1]);
					addr += 2;
					del += prea(b50(i), 2);
				}
			}
			else if(b76(i) == 3) {
				if(b53(i) == 0)
					printf("ext.l %%d%d", b20(i));
				else {
					printf("movm.l &0x%x,", txt[n+1]);
					addr += 2;
					del += prea(b50(i), 4);
				}
			}
			break;
		case 10:
			if(b76(i) == 3) {
				printf("tas ");
				del += prea(b50(i), 1);
			}
			else {
				printf("tst.%s ", bwl(i));
				del += prea(b50(i), len(i));
			}
			break;
		case 12:
			if(bit[6] & i)
				printf("movm.l ");
			else
				printf("movm.w ");
			addr += 2;
			del += prea(b50(i), bit[6] & i? 4: 2);
			printf(",&0x%x", txt[n+1]);
			break;
		case 14:
			if(i & bit[7]) {
				if(i & bit[6])
					printf("jmp ");
				else
					printf("jsr ");
				del += prea(b50(i), 0);
				break;
			}
			if(b64(i) == 4)
				printf("trap &0xx", b30(i));
			else if(b64(i) == 5) {
				if(bit[3] & i)
					printf("unlk %%a%d", b20(i));
				else {
					printf("link %%a%d", b20(i));
					printf(",&0x%x", txt[n+1]);
					del += 2;
				}
			}
			else if(b64(i) == 6) {
				if(bit[3] & i)
					printf("mov usp,%%a%d", b20(i));
				else
					printf("mov %%a%d,usp", b20(i));
			}
			else switch(b30(i)) {
			default:
				printf("??reset");
				break;
			case 0:
				printf("reset");
				break;
			case 1:
				printf("nop");
				break;
			case 2:
				printf("stop");
				printf(" &0x%x", txt[n+1]);
				del += 2;
				break;
			case 3:
				printf("rte");
				break;
			case 5:
				printf("rts");
				break;
			case 6:
				printf("trapv");
				break;
			case 7:
				printf("rtr");
				break;
			}
			break;
		default:
			if(bit[6] & i) {
				printf("lea ");
				del += prea(b50(i), 4);
				printf(",%%a%d", b119(i));
			}
			else {
				printf("chk ");
				del += prea(b50(i), 2);
				printf(",%%d%d", b119(i));
			}
			break;
		}
		break;
	case 5:
		if(b76(i) == 3) {
			if(b53(i) == 1) {
				printf("db%s %%d%d", cond[b118(i)], b20(i));
				printf(",&0x%x", addr + txt[n+1]);
				del += 2;
			}
			else {
				printf("s%s ", cond[b118(i)]);
				del += prea(b50(i), 1);
			}
		}
		else if(i & bit[8]) {
			printf("subq.%s %d,", bwl(i), b119(i));
			del += prea(b50(i), len(i));
		}
		else {
			printf("addq.%s %d,", bwl(i), b119(i));
			del += prea(b50(i), len(i));
		}
		break;
	case 6:
		printf("b%s ", cond[b118(i)]);
		if(b70(i)) {
			n = b70(i);
			if(n >= 128)
				n -= 256;
			printf("0x%x", addr + n);
		}
		else {
			printf("0x%x", addr + txt[n+1]);
			del += 2;
		}
		break;
	case 7:
		printf("movq ");
		if(b70(i) & bit[7])
			printf("-&0x%x", -b70(i));
		else
			printf("&0x%x", b70(i));
		printf(",%%d%d", b119(i));
		break;
	case 8:
		if(b84(i) == 020) {
			printf("sbcd ");
			if(i & bit[3])
				printf("-(%%a%d),-(%%a%d)", b20(i), b119(i));
			else
				printf("%%d%d,%%d%d", b20(i), b119(i));
		}
		else if(b86(i) == 7) {
			printf("divs ");
			del += prea(b50(i), 2);
			printf(",%%d%d", b119(i));
		}
		else if(b86(i) == 3) {
			printf("divu ");
			del += prea(b50(i), 2);
			printf(",%%d%d", b119(i));
		}
		else switch(b86(i)) {
		case 0:
			printf("or.b ");
			del += prea(b50(i), 1);
			printf(",%%d%d", b119(i));
			break;
		case 1:
			printf("or.w ");
			del += prea(b50(i), 2);
			printf(",%%d%d", b119(i));
			break;
		case 2:
			printf("or.l ");
			del += prea(b50(i), 4);
			printf(",%%d%d", b119(i));
			break;
		case 4:
			printf("or.b %%d%d,", b119(i));
			del += prea(b50(i), 1);
			break;
		case 5:
			printf("or.w %%d%d,", b119(i));
			del += prea(b50(i), 2);
			break;
		case 6:
			printf("or.l %%d%d,", b119(i));
			del += prea(b50(i), 4);
			break;
		default:
			printf("or??");
			break;
		}
		break;
	case 9:
		if((i & bit[8]) && b54(i) == 0 && b76(i) != 3) {
			printf("subx.%s ", bwl(i));
			if(i & bit[3])
				printf("-(%%a%d),-(%%a%d)", b20(i), b119(i));
			else
				printf("%%d%d,%%d%d", b20(i), b119(i));
		}
		else switch(b86(i)) {
		case 0:
			printf("sub.b ");
			del += prea(b50(i), 1);
			printf(",%%d%d", b119(i));
			break;
		case 1:
			printf("sub.w ");
			del += prea(b50(i), 2);
			printf(",%%d%d", b119(i));
			break;
		case 2:
			printf("sub.l ");
			del += prea(b50(i), 4);
			printf(",%%d%d", b119(i));
			break;
		case 3:
			printf("suba.w ");
			del += prea(b50(i), 2);
			printf(",%%a%d", b119(i));
			break;
		case 4:
			printf("sub.b %%d%d,", b119(i));
			del += prea(b50(i), 1);
			break;
		case 5:
			printf("sub.w %%d%d,", b119(i));
			del += prea(b50(i), 2);
			break;
		case 6:
			printf("sub.l %%d%d,", b119(i));
			del += prea(b50(i), 4);
			break;
		case 7:
			printf("suba.l ");
			del += prea(b50(i), 4);
			printf(",%%a%d", b119(i));
			break;
		default:
			printf("sub.??");
			break;
		}
		break;
	case 11:
		if(b76(i) == 3) {
			printf("cmpa.%c ", (bit[8] & i)? 'l': 'w');
			del += prea(b50(i), (bit[8] & i)? 4: 2);
			printf(",%%a%d", b119(0));
		}
		else if(bit[8] & i) {
			if(b53(i) == 1)
				printf("cmpm.%s (%%a%d)+,(%%a%d)+", bwl(i),
					b20(i), b119(i));
			else {
				printf("eor.%s %%d%d,", bwl(i), b119(i));
				del += prea(b50(i), len(i));
			}
		}
		else {
			printf("cmp.%s ", bwl(i));
			del += prea(b50(i), len(i));
			printf(",%%d%d", b119(i));
		}
		break;
	case 12:
		if(b83(i) == 050)
			printf("exgd %%d%d,%%d%d", b119(i), b20(i));
		else if(b83(i) == 051)
			printf("exga %%a%d,%%a%d", b119(i), b20(i));
		else if(b83(i) == 061)
			printf("exgm %%d%d,%%a%d", b119(i), b20(i));
		else if(b84(i) == 020)
			if(bit[3] & i)
				printf("abcd -(%%a%d),-(%%a%d)", b20(i),
					b119(i));
			else
				printf("abcd %%d%d,%%d%d", b20(i), b119(i));
		else if(b86(i) == 7) {
			printf("muls ");
			del += prea(b50(i), 2);
			printf(",%%d%d", b119(i));
		}
		else if(b86(i) == 3) {
			printf("mulu ");
			del += prea(b50(i), 2);
			printf(",%%d%d", b119(i));
		}
		else switch(b86(i)) {
		case 0:
			printf("and.b ");
			del += prea(b50(i), 1);
			printf(",%%d%d", b119(i));
			break;
		case 1:
			printf("and.w ");
			del += prea(b50(i), 2);
			printf(",%%d%d", b119(i));
			break;
		case 2:
			printf("and.l ");
			del += prea(b50(i), 4);
			printf(",%%d%d", b119(i));
			break;
		case 4:
			printf("and.b %%d%d,", b119(i));
			del += prea(b50(i), 1);
			break;
		case 5:
			printf("and.w %%d%d,", b119(i));
			del += prea(b50(i), 2);
			break;
		case 6:
			printf("and.l %%d%d,", b119(i));
			del += prea(b50(i), 4);
			break;
		default:
			printf("and??");
			break;
		}
		break;
	case 13:
		if((i & bit[8]) && b54(i) == 0) {
			if(i & bit[3])
				printf("addx.%s -(%%a%d),-(%%a%d)", bwl(i),
					b20(i), b119(i));
			else
				printf("addx.%s %%d%d,%%d%d", bwl(i),
					b20(i), b119(i));
		}
		else if(b76(i) == 3) {
			printf("adda.%c ", (i & bit[8])? 'l': 'w');
			del += prea(b50(i), (i & bit[8])? 4: 2);
			printf(",%%a%d", b119(i));
		}
		else if(bit[8] & i) {
			printf("add.%s ", bwl(i));
			del += prea(b50(i), len(i));
			printf(",%%d%d", b119(i));
		}
		else {
			printf("add.%s %%d%d,", bwl(i), b119(i));
			del += prea(b50(i), len(i));
		}
		break;
	case 14:
		if(b76(i) == 3) {
			switch(b109(i)) {
			case 0:
				printf("as");
				break;
			case 1:
				printf("ls");
				break;
			case 2:
				printf("rox");
				break;
			case 3:
				printf("ro");
				break;
			}
			if(i & bit[8])
				putchar('l');
			else
				putchar('r');
			putchar(' ');
			del += prea(b50(i), 2);
		}
		else {
			switch(b43(i)) {
			case 0:
				printf("as");
				break;
			case 1:
				printf("ls");
				break;
			case 2:
				printf("rotx");
				break;
			case 3:
				printf("ro");
				break;
			}
			if(i & bit[8])
				putchar('l');
			else
				putchar('r');
			printf(".%s ", bwl(i));
			if(i & bit[5])
				printf("%%d%d,%%d%d", b119(i), b20(i));
			else
				printf("%d,%%d%d", b119(i), b20(i));
		}
		break;
	default:
		printf("???");
	}
	return(del);
}

prea(n, l)
{	int reg, mode, del = 0, i;
	long x;
	reg = n & 7;
	mode = (n >> 3) & 7;
	switch(mode) {
	case 0:
		printf("%%d%d", reg);
		break;
	case 1:
		printf("%%a%d", reg);
		break;
	case 2:
		printf("(%%a%d)", reg);
		break;
	case 3:
		printf("(%%a%d)+", reg);
		break;
	case 4:
		printf("-(%%a%d)", reg);
		break;
	case 5:
		printf("%d(%%a%d)", txt[addr/sizeof(short)], reg);
		del += 2;
		break;
	case 6:
		del += 2;
		n = txt[addr/sizeof(short)];
		printf("%d", b70(n));
		printf("(%%a%d,%%", reg);
		if(n & bit[15])
			putchar('a');
		else
			putchar('d');
		printf("%d.", b1412(i));
		if(n & bit[11])
			putchar('l');
		else
			putchar('w');
		putchar(')');
		break;
	case 7:
		switch(reg) {
		default:
			printf("?7%d", reg);
			break;
		case 0:
			del += 2;
			printf("0x%x", txt[addr/sizeof(short)]);
			break;
		case 1:
			del += 4;
			n = addr/sizeof(short);
			x = 0;
			x |= txt[n+1];
			x |= (txt[n] << 16);
			printf("0x%x", x);
			break;
		case 2:
			printf("%d(%%pc)", txt[addr/sizeof(short)]);
			del += 2;
			break;
		case 3:
			n = txt[addr/sizeof(short)];
			printf("%d(%%pc,%%%c%d.%c)", b70(n),
				(n&bit[15])? 'a': 'd',
				b1412(n), (n&bit[11])? 'l': 'w');
			del += 2;
			break;
		case 4:
			n = addr/sizeof(short);
			x = 0;
			del += 2;
			if(l == 1)
				printf("&0x%x", txt[n] & 0xff);
			else if(l == 2)
				printf("&0x%x", txt[n]);
			else if(l == 4) {
				del += 2;
				x |= txt[n+1];
				x |= (txt[n] << 16);
				printf("&0x%x", x);
			}
			else
				printf("??immed.len.%x", l);
			break;
		}
	}
	return(del);
}

reltxt(lo, hi)
{	int i, n;
	if(hdr.a_flag)
		return;
	printf(" #");
	for(i = lo; i < hi; i++) {
		if(txtr[i].rext)
			printf(" e");
		else
			printf(" l");
		switch(txtr[i].rseg) {
		case UNK:
			printf(" unk");
			break;
		case ABS:
			printf(" abs");
			break;
		case TEXT:
			printf(" txt");
			break;
		case BSS:
			printf(" bss");
			break;
		case COMM:
			printf(" cmn");
			break;
		case DATA:
			printf(" dta");
			break;
		default:
			printf(" ?sg");
			break;
		}
		if(txtr[i].r2wds)
			printf(" 2");
		else
			printf(" 1");
		if(txtr[i].rpc)
			putchar('p');
		if(txtr[i].rseg != ABS)
			printf(" %d", txtr[i].rnum);
		else {
			n = txtr[i].rnum << 7;
			pritype(n & TMASK);
			if(n & TMASK1)
				pritype((n & TMASK1) >> TSHFT1);
		}
		putchar(';');
		if(txtr[i].r2wds)
			i++;
	}
}

pritype(n)
{
	switch(n) {
	case 0:
		break;
	case TBR0:
		printf(" br0");
		break;
	case TBR1:
		printf(" br1");
		break;
	case TEA0:
		printf(" ea0");
		break;
	case TEA1:
		printf(" ea1");
		break;
	case TEA2:
		printf(" ea2");
		break;
	case TIM0:
		printf(" im0");
		break;
	default:
		printf(" ???");
		break;
	}
}
