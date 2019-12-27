#include "defs.h"
#include "shortcuts.h"

/* read lammps "write_data" file,
 * get NATOMS, BOX, ...
 */

void rl_2D_pbc(void)
{
  double min,max;
  FILE *fp;
  char fname[1024],buffer[1024];

  // space dimension
  DIM=2; 

  // open the lammps "write_data" file for reading
  sprintf(fname,"INIT.%i",RUNID); fp=fopen(fname,"r"); 
  if(fp==NULL) printerror("E: file=%s not found\n",fname);

  // for sanity check
  NATOMS=-1;

  // read the file
  while( fgets(buffer,sizeof buffer,fp)!=NULL )
  {

    if(EndsWith(buffer,"atoms\n")) 
    {

      sscanf(buffer,"%i",&NATOMS);
      if(NATOMS==0) printerror("E: no atom data\n");
      //printm("NATOMS=%i\n",NATOMS);
      // allocate space
      DOF=NATOMS*DIM;
      POS=(double*)malloc(DOF*sizeof(double));

    } 
    else if(EndsWith(buffer,"xlo xhi\n")) 
    {

      sscanf(buffer,"%lf %lf",&min,&max);
      BOX[0]=max-min; //printm("LX=%lf\n",BOX[0]);

    } 
    else if(EndsWith(buffer,"ylo yhi\n")) 
    {
      sscanf(buffer,"%lf %lf",&min,&max);
      BOX[1]=max-min; //printm("LY=%lf\n",BOX[1]);
    }
    else if(EndsWith(buffer,"zlo zhi\n")) 
    {
      sscanf(buffer,"%lf %lf",&min,&max);
      BOX[2]=max-min; //printm("LZ=%lf\n",BOX[2]);
    } 
  }

  fflush(fp); fclose(fp);
}
