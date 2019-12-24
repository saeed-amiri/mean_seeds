#include "defs.h"

void read_dirs(void){
  char (*dir_name)[512];
  int nf, ndir;
  char cmd[1024], dir_list[512];
  FILE *fp;

  nf = get_ndirs();
  dir_name = malloc(nf * sizeof(dir_name));
  //memset(dir_name, '.', nf*sizeof(char));
  printf("%lu\n",sizeof(dir_name));
  sprintf(cmd,"ls -d */ | sed 's#/##'");
  fp=popen(cmd,"r"); 

  ndir = 0;
  while (fscanf(fp,"%s",dir_list) == 1)
  {
    chdir(dir_list);
    //! checl=k if PARAM.base file exist in the directory
    int j = file_exists("PARAM.base");
    if (j == 1) {
      strcpy(dir_name[ndir], dir_list);
      //dir_name[ndir]= dir_list;
      printf("%s\n",dir_list);
      ndir += 1; 
    }
    chdir("..");
  }
  printf("main: number of run dirs is %i:\n", ndir);
  pclose(fp);


  dir_name = realloc(dir_name, ndir * sizeof(dir_name));
  printf("main: directories to get mean of %i:\n",ndir);
  int i;
  for ( i = 0; i < ndir; i++)
  {
    printf("%s\n", dir_name[i]);
  }
}
