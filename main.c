#include "defs.h"
#include "shortcuts.h"
int RUNID; LATTICE LAT;
int main(int argc,char **argv){

  // get RUNID of data to analyze
  sscanf(argv[1],"%i",&RUNID);
  // analyze the lammps trajectory file
  read_dirs();

  return 0;
}
