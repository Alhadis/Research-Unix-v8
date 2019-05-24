/* step wedge for displaying color map or grey scale */
main()
{
  short w[50][254];
  int i, j;
  for(i=0;i<254;i++){
    for(j=0;j<50;j++){
      w[j][i] = i;
    }
  }
  view2d(1,254,50,0.,0,0,1,0,253,w);
  exit(0);
}
