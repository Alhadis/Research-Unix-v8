#define u_short unsigned short
#define u_char unsigned char

#include "../chunix/chconf.h"
#include "../chunix/chsys.h"
#include "../chaos/chaos.h"
#include "../chaos/address-res.h"

/* NOTE!!! The size of this structure must not be an even multiple of 8 !!! */
struct chilpkt {
	struct il_rheader			ilp_rhdr;
	union {
		struct ar_packet		ilp_Arpkt;
		struct {
			struct pkt_header	ilp_Chhead;
			char			ilp_Chdata[CHMAXDATA];
		}				ilp_Chpkt;
	}					ilp_data;
	char					ilp_crc[4];
};
#define ilp_arpkt	ilp_data.ilp_Arpkt
#define ilp_chhead	ilp_data.ilp_Chpkt.ilp_Chhead
#define ilp_chdata	ilp_data.ilp_Chpkt.ilp_Chdata
#define ilp_chpkt	ilp_data.ilp_Chpkt

main()
{
	printf("Total size: %d\n", sizeof(struct chilpkt));
	printf("arpkt: %d\n", &0->ilp_arpkt);
	printf("chhead: %d\n", &0->ilp_chhead);
	printf("chhdata: %d\n", &0->ilp_chdata);
	printf("chpkt: %d\n", &0->ilp_chpkt);
	printf("crc: %d\n", &0->ilp_crc);
}
