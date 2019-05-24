
main() {
	register int i,j,k,acc;

	printf("\nstatic char bytwo[] = {");
	for (i = 0; i<256; i++) {
		if ((i%4)==0)
			printf("\n");
		for (j = 1; j >= 0; j--) {
			acc = 0;
			for (k = 0; k < 8; k++)
				if (i & (1<<((k+8*j)/2)))
					acc |= (1<<k);
			printf("\t0x%x,",acc);
		}
	}
	printf("\n};\n");
	printf("\nstatic char bythree[] = {");
	for (i = 0; i<256; i++) {
		if ((i%3)==0)
			printf("\n");
		for (j = 2; j >= 0; j--) {
			acc = 0;
			for (k = 0; k < 8; k++)
				if (i & (1<<((k+8*j)/3)))
					acc |= (1<<k);
			printf("\t0x%x,",acc);
		}
	}
	printf("\n};\n\n");
	exit(0);
}
