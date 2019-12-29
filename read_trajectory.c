#include "defs.h"
#include "shortcuts.h"
// index counting the current snapshot while reading the lammps trajectory file
static int NC=-1;
// these are used to set the snapshot range each process has to analyze
static int NCMIN,NCMAX;
// file pointers, explained below
static FILE *TR,*PR;


void read_dirs(void){
  char (*dir_name)[2056];
  int nf, ndir, nStep;
  char cmd[1024], dir_list[512];
  double *xp, *yp, *vx, *vy;
  FILE *fp;
  int natoms,ind,j;
  long timestep;
  double tmp,*data;
  STRING KEYS[20],FIELDS[20],*PTR;
  char buffer[1024]; int i,n,c,nc,nreq;

  char pmd[1024];
  nf = get_ndirs();
  dir_name =  malloc(nf * sizeof(dir_name));
  sprintf(pmd,"ls -d */ | sed 's#/##'"); fp=popen(pmd,"r"); 

  ndir = 0;
  while (fscanf(fp,"%s",dir_list) == 1)
  {
    chdir(dir_list);
    //! checl=k if PARAM.base file exist in the directory
    int jfile = file_exists("PARAM.base");
    if (jfile == 1) {
      strcpy(dir_name[ndir], dir_list); ndir += 1; 
    }
    chdir("..");
  }
  printf("\nmain: number of run dirs: %i:\n\n", ndir); pclose(fp);

  dir_name = realloc(dir_name, ndir * sizeof(dir_name));
  
  /* trying to get number of time step (as tStep),
   * number of atoms (as nAtoms),
   * also the size of the box (bxSize) 
  */

  get_info( dir_name[0], &nStep );
  printf( "%i, %i\n", nStep, DOF );
  printf( "number %i\n", NATOMS );
  xp = (double *)malloc( NATOMS * nStep * sizeof( double ) );
  yp = (double *)malloc( NATOMS * nStep * sizeof( double ) );
  vx = (double *)malloc( NATOMS * nStep * sizeof( double ) );
  vy = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    
  int idir, ikick, iloop;
  for ( ikick = 1; ikick < 2; ikick++)
  {
    for (iloop = 0; iloop < NATOMS * nStep; iloop++)
    {
      xp[iloop] = 0; yp[iloop] = 0; 
      vx[iloop] = 0; vy[iloop] = 0; 
    }
    for ( idir = 0; idir < ndir; idir++)
    {
      char kick[4]; snprintf( kick, 4, "%f", ikick * 0.1 );
      char cwd[1024]; strcpy( cwd, get_pwd() );
      char here[2056]; int icntrl = 0;
      strcpy( here, strcat(strcat(strcat(cwd,"/"),dir_name[idir]),"/") );
      strcat( here, kick );
      char traj[10]; sprintf(traj, "traj.%i.gz",RUNID);
      strcat( strcat( here, "/" ), traj );
      //! prepare a list of the required lammps atom quantities, 
      //! store in data structure = KEYS
      //! set TR = file pointer for reading the trajectory
      sprintf(cmd,"zcat %s",here); TR=popen(cmd,"r");

      nreq=split_string("c_R[1] c_R[2] vx vy fx fy",KEYS);

      while( fgets(buffer,sizeof buffer,TR)!=NULL ) 
      {
        //! beginning of new snapshot
        if(StartsWith(buffer,"ITEM: TIMESTEP"))
        {
          fgets(buffer,sizeof buffer,TR);
          sscanf(buffer,"%li",&timestep);
          NC++; goto nextline;
        }
        
        //! if snapshot index NC is not in the range of the current processor, skip it
        if(NC<NCMIN || NC>=NCMAX) goto nextline;

        if(StartsWith(buffer,"ITEM: NUMBER OF ATOMS")) 
        {
          fgets(buffer,sizeof buffer,TR);
          sscanf(buffer,"%i",&natoms);
          goto nextline;
        }

        //! HERE THE ATOM DATA IS BEING READ, THEN ANALYZED 

        if(StartsWith(buffer,"ITEM: ATOMS")) 
        {
          //! make a list structure of the atom properties stored in the data
          nc=split_string(buffer,FIELDS)-2; PTR=&FIELDS[2];

          //! needed for sanity check
          for(i=0;i<=nreq;i++) KEYS[i].ok=0;

          //! for every field in PTR mark the order in which it appears in KEYS; use -1 for fields that are __not__ members of KEYS
          for(c=0;c<nc;c++) 
          {
            PTR[c].c=-1;
            //! atom ids are always required, we put these last = nreq (!)
            if(strcmp(PTR[c].w,"id")==0) 
            {
              PTR[c].c=nreq; KEYS[nreq].ok++;
            } else 
            {
              for(i=0;i<nreq;i++) if(strcmp(PTR[c].w,KEYS[i].w)==0) 
              {
                PTR[c].c=i; KEYS[i].ok++; break;
              }
            }
          }
          //! SANITY CHECK (!)
          if(KEYS[nreq].ok!=1) printerror("E: missing/duplicate atom ids\n");
          for(i=0;i<nreq;i++) if(KEYS[i].ok!=1) printerror("E: missing/duplicate atom property: %s\n",KEYS[i].w);

          //! read and store the data
          data=(double*)malloc(natoms*nreq*sizeof(double));
          for(i=c=n=0;i<natoms*nc;i++) 
          {
            fscanf(TR,"%lf",&tmp);
            ind=PTR[c].c; if(ind>=0) KEYS[ind].v=tmp;
            c++; 
            if(c==nc) 
            {
              //! convert lammps id to C-style, store data
              ind=(int)KEYS[nreq].v; ind--;
              for(j=0;j<nreq;j++) data[j*natoms+ind]=KEYS[j].v;
              c=0; n++;
            }
          }
          //! pointers to displacements, velocities, forces, DOF = 2 * NATOMS
          int icpy; printf("here data %lf\n", data[0]);
          for ( icpy = 0; icpy < NATOMS; icpy++)
          {
            xp[icpy + icntrl * NATOMS] += data[icpy];
            yp[icpy + icntrl * NATOMS] += data[icpy + NATOMS];
            vx[icpy + icntrl * NATOMS] += data[DOF];
            vy[icpy + icntrl * NATOMS] += data[DOF + NATOMS];
          }
          icntrl += 1;
          //xp=&data[0]; yp=&data[NATOMS]; 
          //vx=&data[DOF]; vx=&data[DOF+NATOMS];
          //! clean-up (!)
          free(data);
        }
        nextline:;
        //printf("%s\n%i\n", here, icntrl);
      }
    }
  }
}
