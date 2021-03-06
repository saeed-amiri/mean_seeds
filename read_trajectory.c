#include "defs.h"
#include "shortcuts.h"
// file pointers
static FILE *TR;
static double *xp,*yp,*vx,*vy,*CR1,*CR2,*fx,*fy;
static int *time_list;

void read_dirs(void){
  int nf,ndir,nStep,natoms,ind,j;
  int i,n,c,nc,nreq;
  double tmp,*data;
  long timestep;
  char (*dir_name)[5];
  char cmd[1024],pmd[1024],dir_list[512],buffer[1024]; 
  STRING KEYS[20],FIELDS[20],*PTR;
  FILE *fp;

  nf = get_ndirs();
  dir_name =  malloc( nf * sizeof(dir_name) );
  sprintf( pmd,"ls -d */ | sed 's#/##'" ); fp = popen( pmd,"r" ); 

  ndir = 0;
  while ( fscanf(fp,"%s",dir_list) == 1 )
  {
    chdir( dir_list );
      //! check if PARAM.base file exist in the directory
      int jfile = file_exists( "PARAM.base" );
      if (jfile == 1) {
        strcpy( dir_name[ndir], dir_list ); ndir += 1; 
      }
    chdir( ".." );
  }
  printf( "\nmain: number of run dirs: %i:\n", ndir ); pclose(fp);
  int ilen;
  for (ilen = 0; ilen < ndir; ilen++)
  {
    printf("%s, strlen: %lu\n",dir_name[ilen],strlen(dir_name[ilen]));
  }

  dir_name = realloc(dir_name, ndir * sizeof(dir_name));
  
  /* trying to get number of time step (as tStep),
   * number of atoms (as nAtoms),
   * also the size of the box (bxSize) 
  */
  get_info( dir_name[0], &nStep );
  printf( "NSTEP: %i, NATOMS: %i\n", nStep, NATOMS );
  int idir,ikick,iloop,NC;
  char kick[4];
  for ( ikick = 1; ikick < 21; ikick++)
  {
    xp = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    yp = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    vx = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    vy = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    vy = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    fx = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    fy = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    CR1 = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    CR2 = (double *)malloc( NATOMS * nStep * sizeof( double ) );
    time_list = (int *)malloc( nStep * sizeof( int ) );
    
    for (iloop = 0; iloop < NATOMS * nStep ; iloop++)
    {
      xp[iloop] = 0.0; yp[iloop] = 0.0; 
      vx[iloop] = 0.0; vy[iloop] = 0.0; 
      fx[iloop] = 0.0; fy[iloop] = 0.0; 
      CR2[iloop] = 0.0; CR2[iloop] = 0.0; 
    }
    for ( iloop = 0; iloop<nStep; iloop++ ) time_list[iloop] = 0;
    
    for ( idir = 0; idir < ndir; idir++ )
    {
      NC = -1;
      snprintf( kick, 4, "%f", ikick * 0.1 );
      char cwd[1024]; strcpy( cwd, get_pwd() );
      char here[2056];
      strcpy( here, strcat(strcat(strcat(cwd,"/"),dir_name[idir]),"/") );
      strcat( here, kick );
      char traj[10]; sprintf(traj, "traj.%i.gz",RUNID);
      strcat( strcat( here, "/" ), traj );
      //! prepare a list of the required lammps atom quantities, 
      //! store in data structure = KEYS
      //! set TR = file pointer for reading the trajectory
      sprintf(cmd,"zcat %s",here); TR=popen(cmd,"r");

      nreq=split_string("x y vx vy c_R[1] c_R[2] fx fy",KEYS);

      while( fgets(buffer,sizeof buffer,TR)!=NULL ) 
      {
        //! beginning of new snapshot
        if( StartsWith(buffer,"ITEM: TIMESTEP") )
        {
          fgets(buffer,sizeof buffer,TR);
          sscanf(buffer,"%li",&timestep);
          NC++; time_list[NC] = timestep; goto nextline;
        }

        if( StartsWith(buffer,"ITEM: NUMBER OF ATOMS") ) 
        {
          fgets(buffer,sizeof buffer,TR);
          sscanf(buffer,"%i",&natoms);
          goto nextline;
        }

        //! HERE THE ATOM DATA IS BEING READ, THEN ANALYZED 

        if( StartsWith(buffer,"ITEM: ATOMS") ) 
        {
          //! make a list structure of the atom properties stored in the data
          nc=split_string(buffer,FIELDS)-2; PTR=&FIELDS[2];

          //! needed for sanity check
          for( i=0; i<=nreq; i++ ) KEYS[i].ok=0;

          //! for every field in PTR mark the order in which it appears in KEYS; use -1 for fields that are __not__ members of KEYS
          for( c=0; c<nc; c++ ) 
          {
            PTR[c].c=-1;
            //! atom ids are always required, we put these last = nreq (!)
            if( strcmp(PTR[c].w,"id")==0 ) 
            {
              PTR[c].c=nreq; KEYS[nreq].ok++;
            } else 
            {
              for( i=0; i<nreq; i++) if( strcmp(PTR[c].w,KEYS[i].w)==0 ) 
              {
                PTR[c].c=i; KEYS[i].ok++; break;
              }
            }
          }
          //! SANITY CHECK (!)
          if( KEYS[nreq].ok!=1 ) printerror("E: missing/duplicate atom ids\n");
          for(i=0;i<nreq;i++) if( KEYS[i].ok!=1 ) printerror("E: missing/duplicate atom property: %s\n",KEYS[i].w);

          //! read and store the data
          data=(double*)malloc(natoms*nreq*sizeof(double));
          for( i=c=n=0; i<natoms*nc; i++) 
          {
            fscanf(TR,"%lf",&tmp);
            ind=PTR[c].c; if( ind>=0 ) KEYS[ind].v=tmp;
            c++; 
            if( c==nc ) 
            {
              //! convert lammps id to C-style, store data
              ind=(int)KEYS[nreq].v; ind--;
              for( j=0; j<nreq; j++ ) data[j*natoms+ind]=KEYS[j].v;
              c=0; n++;
              //! save to 1D-array
              xp[ind + NC * NATOMS] += data[ind];
              yp[ind + NC * NATOMS] += data[ind + NATOMS];
              vx[ind + NC * NATOMS] += data[ind + DOF];
              vy[ind + NC * NATOMS] += data[ind + DOF + NATOMS];
              CR1[ind + NC * NATOMS] += data[ind + 2 * DOF];
              CR2[ind + NC * NATOMS] += data[ind + 2 * DOF + NATOMS];
              fx[ind + NC * NATOMS] += data[ind + 3 * DOF ];
              fy[ind + NC * NATOMS] += data[ind + 3 * DOF + NATOMS];
            }
          }
          //! clean-up (!)
          free(data);
        }
        nextline:;
      }
    }
    char outer[1024];
    sprintf(outer,"traj.%i_%s", RUNID, kick);
    FILE *fout,*fzip;
    fout =fopen(outer,"w");
    printf("%s\n",outer);
    int iout, jout;
    for ( iout = 0; iout<nStep; iout++ )
    {
      fprintf(fout,"ITEM: TIMESTEP\n%i\n",time_list[iout]);
      fprintf(fout,"ITEM: NUMBER OF ATOMS\n%i\n",NATOMS);
      fprintf(fout,"ITEM: BOX BOUNDS pp ss pp\n");
      fprintf(fout,"0 %e\n",BOX[0]);
      fprintf(fout,"0 %e\n",BOX[1]);
      fprintf(fout,"-0.05 0.05\n");
      fprintf(fout,"ITEM: ATOMS id x y vx vy c_R[1] c_R[2] fx fy\n");
      for ( jout=0; jout<NATOMS; jout++ )
      {
        fprintf(fout,"%i %e %e",jout+1, xp[jout+iout*NATOMS]/ndir,yp[jout+iout*NATOMS]/ndir);
        fprintf(fout," %e %e",vx[jout+iout*NATOMS]/ndir,vy[jout+iout*NATOMS]/ndir);
        fprintf(fout," %e %e",CR1[jout+iout*NATOMS]/ndir,CR2[jout+iout*NATOMS]/ndir);
        fprintf(fout," %e %e\n",fx[jout+iout*NATOMS]/ndir,fy[jout+iout*NATOMS]/ndir);
      }
    }
    fclose(fout);
    sprintf(cmd, "gzip %s",outer);
    fzip = popen(cmd,"r");pclose(fzip);
    free(xp); free(yp); free(vx); free(vy); free(fx); free(fy); free(CR1); free(CR2);
  }
  FILE *fmv;
  sprintf(cmd, "mkdir mean & mv *.gz ./mean/" );
  fmv = popen(cmd, "r"); pclose(fmv);
}
