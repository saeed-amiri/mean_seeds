#include "defs.h"

void read_dirs(void){
  char (*dir_name)[512];
  int nf, ndir;
  char cmd[1024], dir_list[512];
  FILE *fp;

  nf = get_ndirs();
  dir_name =  malloc(nf * sizeof(dir_name));
  sprintf(cmd,"ls -d */ | sed 's#/##'"); fp=popen(cmd,"r"); 

  ndir = 0;
  while (fscanf(fp,"%s",dir_list) == 1)
  {
    chdir(dir_list);
    //! checl=k if PARAM.base file exist in the directory
    int j = file_exists("PARAM.base");
    if (j == 1) {
      strcpy(dir_name[ndir], dir_list); ndir += 1; 
    }
    chdir("..");
  }
  printf("\nmain: number of run dirs: %i:\n\n", ndir); pclose(fp);

  dir_name = realloc(dir_name, ndir * sizeof(dir_name));

  int i, k;
  for ( k = 1; k < 21; k++)
  {
    for ( i = 0; i < ndir; i++)
      {
        chdir (dir_name[i]);
          char kick[256]; snprintf(kick, 4, "%f", k * 0.1);
          chdir(kick);
            int t = get_gz();
          chdir("..");
          if (t == 1){
            printf("\non %s/%s there is traj file\n", dir_name[i], kick);
            
          }
        chdir ("..");
      }
  }
}
