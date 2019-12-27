EXEC  = ../mean_seeds

OBJ   = main.o read_trajectory.o tools.c read_INIT.c

# options for GWDG, intel cluster
INCLUDE = /cm/shared/apps/intel/compilers_and_libraries/2017.2.174/mkl/include/fftw
LIBS  = -lrt -mkl
FLAGS = -O3 -w3 -wd981,2259,1572 -diag-error167,144 -I$(INCLUDE)
CC    = mpiicc

# simple options, for deskop PC
# Be sure to append an underscore '_' to the lapack function names (!)
#CC = mpicc
#LIBS = -lm -lblas -llapack

$(EXEC):$(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(EXEC) $(LIBS)

# autodetect 32/64 bit systems
BIT = $(shell getconf LONG_BIT)

%.o : %.c Makefile defs.h head.h 
	$(CC) -c $(FLAGS) -DBIT$(BIT) $<

head.h:
	make head

head:
	rm -f head.h
	makeheaders -h *.c | grep -v mersenne > head.h

clean:
	rm -f *.o head.h makeheaders $(EXEC)

