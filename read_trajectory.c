#include "defs.h"

void read_dirs(void){
  char (*dir_name)[512];
  int nf, ndir, time_step;
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
  int y = 0, z = 0;
  for ( k = 1; k < 21; k++)
  {
    for ( i = 0; i < ndir; i++)
      {
        chdir ( dir_name[i] );
          char kick[4]; snprintf(kick, 4, "%f", k * 0.1);
          if ( k == 1 && i == 0 ) 
          { 
            get_info(kick, &time_step);
            //time_step = get_time(); 
            printf("%i time steps\n\n", time_step);
            y +=1;
            printf("\n why %i\n",y);
          }
          chdir(kick);
            //int t = get_gz();
            //printf("t = %i, %i\n",t,z);
            z +=1;
          chdir("..");
          //if (t == 1){
          //  printf("\non %s/%s there is traj file\n", dir_name[i], kick);
          //}
        chdir ("..");
      }
  }
}
