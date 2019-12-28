#include "defs.h"

int get_ndirs(void)
{
  char cmd[1024]; int n; FILE *fp;
  sprintf( cmd,"ls -d */ | sed 's#/##'|wc -l" );
  fp = popen( cmd,"r" ); fscanf( fp,"%i",&n ); pclose( fp );
  return n;
}

int get_nfiles(void)
{
  char cmd[1024]; int n; FILE *fp;
  sprintf( cmd,"ls | sed 's#/##'|wc -l" );
  fp = popen( cmd,"r" ); fscanf( fp,"%i",&n ); pclose( fp );
  return n;
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
  char cmd[1024], traj[256], snap[10], temp[10]; FILE *fp; int jj =0;
  sprintf( traj, "traj.%i.gz", RUNID ); sprintf( snap, "snap.%i.gz", RUNID );
  int i = file_exists(traj);
  if ( i == 1 ) { return 1; }
  else 
  {
    sprintf( cmd, "ls *gz | grep snap.%i.gz | sed 's#/##'",RUNID );
    fp = popen( cmd, "r" ); fscanf( fp, "%s", temp ); pclose( fp );
    printf("jj is %i\n",jj);
    jj +=1;
    if ( strcmp( snap, temp ) == 0 )
    {
      rename(temp, traj);
      printf( "get_gz: snap->traj" );
      return 1;
    }
  }
  return 0;
}

int get_time( void )
{
  char cmd[1024]; int n; FILE *fp;
  sprintf( cmd,"zgrep TIMESTEP traj.%i | wc -l", RUNID );
  fp = popen( cmd,"r" ); fscanf( fp,"%i",&n ); pclose( fp );
  return n;
}

char* get_pwd( void ){
  char cmd[1024]; FILE *fp;
  static char pwd[1024];
  sprintf( cmd,"pwd" );
  fp = popen( cmd,"r" ); fscanf( fp, "%s", pwd ); pclose( fp );
  return pwd;
}
//! [RV] HELPER ROUTINES, for processing of strings

int StartsWith(const char *a, const char *b)
{
 if(strncmp(a, b, strlen(b)) == 0) return 1;
 return 0;
}

int EndsWith(const char *a, const char *b)
{ int i=strlen(a)-strlen(b);
  if(i>=0) if(strcmp(&a[i],b)==0) return 1;
  return 0;
}
// //

void get_info( char dir[4], int *step )
{
  char kick[4]="0.1";
  char dirk[512], cwd[1024];
  strcpy( cwd, get_pwd() );
  sprintf(dirk,"%s/%s/%s",cwd,dir,kick);
  chdir( dirk );
      *step = get_time();
      rl_2D_pbc();
  chdir( cwd );
}
/* printwarning and printerror use variable argument lists :: this is tricky
   C-stuff which you don't need to understand */

void printwarning(char *msg, ...)
{ FILE *fp;
  char dd[1024];
  va_list argp;

  /* print error to screen... */

  va_start(argp,msg);
  vfprintf(stderr,msg,argp);
  va_end(argp);

  /* ...and to logfile... */

  fp=fopen(ERRORFILE,"a");
  va_start(argp,msg);
  fprintf(fp,"%s: RUNID=%i: ",dd,RUNID);
  vfprintf(fp,msg,argp);
  va_end(argp); fflush(fp); fclose(fp);
}

void printerror(char *msg, ...)
{ FILE *fp;
  char dd[1024];
  va_list argp;

  /* print error to screen... */

  va_start(argp,msg);
  vfprintf(stderr,msg,argp);
  va_end(argp);

  /* ...and to logfile... */

  fp=fopen(ERRORFILE,"a");
  va_start(argp,msg);
  fprintf(fp,"%s: RUNID=%i: ",dd,RUNID);
  vfprintf(fp,msg,argp);
  va_end(argp); fflush(fp); fclose(fp);

  exit(1);
}

int split_string(char *in,STRING *out)
{ int i,c,w=0; char buffer[1024];
  // add leading space to input string
  sprintf(buffer," %s",in);
  // scan the string, looking for words
  for(i=1;i<(int)strlen(buffer);i++) {
    // start of new word
    if(isspace(buffer[i-1]) && !isspace(buffer[i])) {
      c=0; w++;
    }
    // write single word characters to array out; be sure to null-terminate (!)
    if(!isspace(buffer[i])) {
      out[w-1].w[c]=buffer[i];
      c++;
      out[w-1].w[c]='\0';
    }
  }
  // also return how many words were counted
  return w;
}
