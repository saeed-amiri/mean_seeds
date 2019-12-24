#include "defs.h"

int RUNID;
int main(int argc,char **argv){

  // get RUNID of data to analyze
  sscanf(argv[1],"%i",&RUNID);
  // analyze the lammps trajectory file
  read_dirs();

  return 0;
}
