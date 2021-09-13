MAKE = gnumake
OPT = -O3 

#  PROF = -pg
#  DEBUG = -g
# DEFS = -DCC=gcc -DDEBUG=1 $(OPT)
DEFS = -DCC=gcc $(OPT)
WOPTS = -Wall -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes \
        -Wno-long-long -Winline 
FLAGS = -static -ansi -pedantic-errors  $(WOPTS) $(OPT) $(DEBUG) $(PROF) 
CC=gcc $(FLAGS)

OBJS  =  parser.o  solver.o machine.o cost.o utils.o constraints.o search.o  selection.o
SRCS  =  parser.c  solver.c  cost.c utils.c constraints.c   selection.c search.c machine.c
HEADERS = definitions.h parser.h  machine.h cost.h utils.h constraints.h search.h  selection.h

depend:
	makedepend $(DEFS) -- ${FLAGS} -- $(SRCS)

roadef: $(OBJS)
	gcc -o machineReassignment ${OBJS} ${LIBS} ${PROF} -lm  
	

solver: $(OBJS)
	gcc -o solver ${OBJS} ${LIBS} ${PROF}  -lm
	

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)


# compile and generate dependency info
%.o: %.c
	gcc -c $(FLAGS) $*.c -o $*.o
	gcc -MM $(FLAGS) $*.c > $*.d


cblns:	
	mkdir cblns
	cp $(SRCS) cblns/
	cp $(HEADERS) cblns/
	cp COPYING cblns/
	cp README cblns/
	head -70  Makefile > cblns/Makefile
	#cp data/instances/*.txt  cblns/
	tar -cvf roadef.tar cblns/
	ls cblns
	rm cblns/*
	rmdir cblns  

binary:
	mkdir rg_solver
	make clean
	make roadef
	cp   machineReassignment rg_solver/   
	cp data/instances/*.txt rg_solver/
	tar -cvf roadef_binary.tar rg_solver/
	rm rg_solver/*
	rmdir rg_solver

clean:
	rm -f roadef solver *.o *.d

profile:
	@echo uncomment PROF entry
	@echo run make depend
	@echo run make
	@echo rm gmon.out
	@echo run program followed by gprof solver gmon.out
# DO NOT DELETE
