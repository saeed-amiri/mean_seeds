#include "defs.h"

int get_ndirs(void)
{
  char cmd[1024]; int nf; FILE *fp;
  sprintf( cmd,"ls -d */ | sed 's#/##'|wc -l" );
  fp = popen( cmd,"r" ); fscanf( fp,"%i",&nf ); pclose( fp );
  return nf;
}

int get_nfiles(void)
{
  char cmd[1024]; int nf; FILE *fp;
  sprintf( cmd,"ls | sed 's#/##'|wc -l" );
  fp = popen( cmd,"r" ); fscanf( fp,"%i",&nf ); pclose( fp );
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

int get_gz(void)
{
  char cmd[1024], traj[256], snap[10], temp[10]; FILE *fp; 
  sprintf( traj, "traj.%i.gz", RUNID ); sprintf( snap, "snap.%i.gz", RUNID );
  int i = file_exists(traj);
  if ( i == 1 ) { return 1; }
  else 
  {
    sprintf( cmd, "ls *gz | grep snap.%i.gz | sed 's#/##'",RUNID );
    fp = popen( cmd, "r" ); fscanf( fp, "%s", temp ); pclose( fp );
    if ( strcmp( snap, temp ) == 0 )
    {
      rename(temp, traj);
      printf( "get_gz: snap->traj" );
      return 1;
    }
  }
  return 0;
}