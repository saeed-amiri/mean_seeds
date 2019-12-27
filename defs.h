#define ERRORFILE       "JOBINFO"
typedef struct LATTICE
{
  // space dimension
  int DIM;
  // size of system edges;
  double BOX[3];
  // number of atoms
  int NATOMS;
  // number of degrees of freedon
  int DOF;
  // arrays for particle positions
  double *POS;

} LATTICE;

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <mkl.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include "head.h"
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
