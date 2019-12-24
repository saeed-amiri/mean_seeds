#include "defs.h"

int get_ndirs(void)
{
  char cmd[1024]; int nf; FILE *fp;
  sprintf(cmd,"ls -d */ | sed 's#/##'|wc -l");
  fp = popen(cmd,"r"); fscanf(fp,"%i",&nf); pclose(fp);
  printf("get_ndirs: total dirs: %i\n",nf);
  return nf;
}


int file_exists (char * fileName)
{
  struct stat buf;
  int i = stat ( fileName, &buf );
    /* File found */
    if ( i == 0 ) {
      return 1;
    }
    return 0;
}

