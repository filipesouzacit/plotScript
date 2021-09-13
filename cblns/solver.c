/**************************************************************************************************
	Copyright 2011 Deepak Mehta, Barry O'Sullivan, Helmut Simonis, University College Cork, Ireland

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU  General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU  General Public License for more details.

    You should have received a copy of the GNU  General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "parser.h"
#include "cost.h"
#include "search.h"
#include "utils.h"
#include "selection.h"
#include "definitions.h"

void shift( int start, int number, int *pargc, char *argv[] ) {
	int i;
	*pargc -= number;
	for (i = start; i < *pargc; i++)
		argv[ i ] = argv[ i+number ];
}


void correct_usage( char *argv[] ) {
	fprintf( stderr, "Usage: %s [-help] \\\n", argv[ 0 ] );
	fprintf( stderr, "                [-model filename]  \\\n" );
	fprintf( stderr, "                [-initsol filename]  \\\n" );
}


void usage_error( char *argv[] ) {
	correct_usage( argv );
	fprintf( stderr, "Bailing out.\n" );
	/* abort( ); */
	exit( 1 );
}

void log_selection( int *pargc, char *argv[] ) {
	if (*pargc < 2)
		usage_error(argv);
	if (strcmp(argv[2],"infile")==0)
		printinfile=TRUE;
	else if (strcmp(argv[2],"onscreen")==0)
		printonscreen=TRUE;
	else
		usage_error(argv);
	shift( 1, 2, pargc, argv );
}


void time_setting( int *pargc, char *argv[] ) {
	if (*pargc < 2)
		usage_error(argv);
	time_limit = atoi(argv[2]);
	print_time = 2;
	shift( 1, 2, pargc, argv );
}

void readSeed( int *pargc, char *argv[] ) {
	if (*pargc < 2)
		usage_error(argv);
	/*seed = atoi(argv[2]);*/
	shift( 1, 2, pargc, argv );
}


void model_selection( int *pargc, char *argv[] ) {
	if (*pargc < 3)
		usage_error(argv);
	modelfilename = (char *)CALLOC(strlen(argv[2])+1,sizeof(char));
	strcpy(modelfilename,argv[2]);
	/*printf("%d %s\n",strlen(modelfilename),modelfilename);*/
	shift( 1, 2, pargc, argv );
}

void initsol_selection( int *pargc, char *argv[] ) {
	if (*pargc < 3)
		usage_error(argv);
	iassignmentfilename = (char *)CALLOC(strlen(argv[2])+1,sizeof(char));
	strcpy(iassignmentfilename,argv[2]);

	shift( 1, 2, pargc, argv );
}


void newsol_selection( int *pargc, char *argv[] ) {
	if (*pargc < 3)
		usage_error(argv);
	readNewSolution(argv[2]);
	shift( 1, 2, pargc, argv );
}

void read_solution_filename( int *pargc, char *argv[]) {
	if (*pargc < 2)
		usage_error(argv);
	solutionfilename = (char *)CALLOC(strlen(argv[2])+1,sizeof(char));
	strcpy(solutionfilename,argv[2]);
	shift( 1, 2, pargc, argv );
}

void machine_heuristic_selection(int *pargc, char *argv[] ) {
	if (*pargc < 1)
		usage_error(argv);

	shift( 1, 1, pargc, argv );

	if(strcmp(argv[1], "random")==0) {
		input_machine_heuristic_weight = machine_heuristic_weight_random;
		mh_id=MH_RANDOM;
		shift( 1, 1, pargc, argv );
	}
	else if(strcmp(argv[1], "cost")==0) {
		input_machine_heuristic_weight = machine_heuristic_weight_cost;
		mh_id=MH_COST;
		shift( 1, 1, pargc, argv );
	}
	else
		usage_error(argv);
}

void dynamic_threshold_selection(int *pargc, char *argv[]){
	shift( 1, 1, pargc, argv );
	if (*pargc < 1)
		usage_error(argv);
	input_threshold=atoi(argv[1]);
	shift( 1, 1, pargc, argv );
}

/*void static_threshold_selection(int *pargc, char *argv[]){
   shift( 1, 1, pargc, argv );
   if (*pargc < 1)
	usage_error(argv);
   input_dynamic_threshold=FALSE;
   input_threshold=atoi(argv[1]);
   shift( 1, 1, pargc, argv );
}*/

void lns_with_selection(int *pargc, char *argv[]){
	shift( 1, 1, pargc, argv );
	if (*pargc < 1)
		usage_error(argv);
	if(strcmp(argv[1], "machine")==0) {
		lns_type_selection =  lns_machine_selection;
		shift( 1, 1, pargc, argv );
	}
	else
		usage_error(argv);
}

void lns_selection(int *pargc, char *argv[]){
	shift( 1, 1, pargc, argv );
	if (*pargc < 1)
		usage_error(argv);
	else if(strcmp(argv[1], "roadef")==0) {
		lns =  roadef_lns;
		shift( 1, 1, pargc, argv );
	}
	else
		usage_error(argv);
}


void default_selection(void){
	input_machine_heuristic_weight = machine_heuristic_weight_cost;
	mh_id=MH_COST;
	input_threshold = 1;
	verification = FALSE;
	printinfile = FALSE;
	printonscreen = FALSE;
	lns_type_selection = lns_machine_selection;
	lns = default_lns;
	seed=1979;
	time_limit = 300;
	print_time = 3;
}



void create_structures_required_for_processes(void){
	int i;
	bestM = (usint*)CALLOC(SEARCH_NPROCESSES, sizeof(usint));
	costOfProcess = (long long*)CALLOC(SEARCH_NPROCESSES, sizeof(long long));
	if ((pmcost = (long long int**)CALLOC(SEARCH_NPROCESSES, sizeof(long long int*)))==NULL){
		fprintf(stderr, "could not allocate memory for pmcost\n");
		abort();
	}
	if ((movecost = (long long int**)CALLOC(SEARCH_NPROCESSES, sizeof(long long int*)))==NULL){
		fprintf(stderr, "could not allocate memory for pmcost\n");
		abort();
	}

	for (i=0; i < SEARCH_NPROCESSES; i++) {
		if ((pmcost[i] = (long long int*)CALLOC(no_machines, sizeof(long long int)))==NULL){
			fprintf(stderr, "could not allocate memory for pmcost[%d]\n",i);
			abort();
		}
		if ((movecost[i] = (long long int*)CALLOC(no_machines, sizeof(long long int)))==NULL){
					fprintf(stderr, "could not allocate memory for pmcost[%d]\n",i);
					abort();
				}

	}
	if ((max_moved_store = (boolean *)CALLOC(SEARCH_NPROCESSES, sizeof(boolean)))==NULL){
		fprintf(stderr, "could not allocate memory for max_moved_store\n");
		abort();
	}

}

void free_structures_required_for_processes(void){
	int i;

	free(costOfProcess);

	for (i=0; i < SEARCH_NPROCESSES; i++) {
		free(pmcost[i]);
		free(movecost[i]);
	}

	free(pmcost);
	free(movecost);
	free(max_moved_store);
}

void create_msrop( void ) {
	usint i;
	if ((msrop = (char**)calloc( SEARCH_NPROCESSES,sizeof(char*)))==NULL)
	 {
			fprintf(stdout, "could not allocate memory for msrop\n");
			abort();
	}

	/*loadcost = (unsigned long long*)CALLOC(no_machines, sizeof(unsigned long long));*/
	for (i=0; i < SEARCH_NPROCESSES; i++)
		msrop[i] = (char*)CALLOC( no_machines, sizeof(char));		



}

void free_msrop( void ) {
	usint i;
	for(i=0;i< SEARCH_NPROCESSES;i++)
		free(msrop[i]);
	free(msrop);
}

void create_lists_of_processes_for_machines(void){
	usint i;
	MACHINE *machine ;
	usint asize = (usint)ceil((1.0*no_processes)/no_machines);
	for(i=0;i<no_machines;i++){
		machine = &machines[i];
		machine->size = asize > machine->size ? asize : machine->size;
		machine->processes = (usint*)CALLOC( machine->size, sizeof(usint));
	}	
	for (i = 0; i < no_processes; i++) {
		machine = &machines[OM[i]];
		machine->processes[machine->n++] = i;
	}
}

void init_solver(void){
	start_time = time(NULL);
	default_selection();
	modelfilename = NULL;
	iassignmentfilename = NULL;
	solutionfilename = NULL;
	default_sol_filename = NULL;
}




void exit_solver(void){
	int i;



	free(weight_lc);

	free(S);

	free(OM);

	free(NM);

	free(BM);

	free(active_services);


	for(i=0; i < no_machines; i++){
		free(machines[i].capacities);
		free(machines[i].scapacities);
		free(machines[i].mmc);
		free(machines[i].processes);
	}
	free(machines);


	free(demand);
	free(supply);

	free(no_available_machines);
	free(dsize);
	for(i=0;i<SEARCH_NPROCESSES;i++)
		free(values[i]);

	free_msrop();

	free_structures_required_for_processes();

	for(i=0;i<no_processes;i++)
		free(R[i]);

	/*free(transient);
*/
}

int main(int argc, char *argv[]) {
	init_solver();
	while (argc != 1) {
		if (strcmp("-t", argv[1]) == 0)
			time_setting(&argc, argv);
		else if (strcmp( "-p", argv[ 1 ] ) == 0)
			model_selection(&argc, argv);
		else if (strcmp( "-i", argv[ 1 ] ) == 0) {
			initsol_selection(&argc, argv);
		}
		else if (strcmp( "-newsol", argv[1] ) == 0)  {
			newsol_selection(&argc, argv);
		}
		else if (strcmp( "-o",argv[1]) == 0) {
			read_solution_filename(&argc,argv);
		}
		else if (strcmp("-s", argv[1]) == 0) {
			readSeed(&argc,argv);
		}
		else if (strcmp("-mh", argv[1]) == 0){
			machine_heuristic_selection(&argc,argv);
		}
		else if (strcmp("-dt", argv[1]) == 0)
			dynamic_threshold_selection(&argc,argv);
		/*  else if (strcmp("-st", argv[1]) == 0)
	static_threshold_selection(&argc,argv);*/
		else if (strcmp("-lns_with", argv[1]) == 0)
			lns_with_selection(&argc,argv);
		else if (strcmp("-lns", argv[1]) == 0) {
			lns_selection(&argc,argv);
		}
		else if (strcmp("-log", argv[1]) == 0) {
			log_selection(&argc,argv);
		}
		else if (strcmp("-verification", argv[1]) == 0) {
			verification=TRUE;
			shift( 1, 1, &argc, argv );
		}
		else if (strcmp("-name", argv[1]) == 0) {
			fprintf(stdout,"S38\n");
			shift( 1, 1, &argc, argv );
		}
		else
			usage_error(argv);
	}


	if (modelfilename != NULL && iassignmentfilename != NULL) {
		readProblem(modelfilename);
		readOriginalSolution(iassignmentfilename);
		/*printf("original cost: %llu\n", objective_cost);*/


		if (solutionfilename == NULL)
			solutionfilename = default_sol_filename;

		create_lists_of_processes_for_machines();
		create_structures_required_for_processes();

		create_msrop();
		srand(seed);


		/*random_machine = TRUE;*/
		/*machine_heuristic_weight = machine_heuristic_weight_random;*/
		/*machine_heuristic_weight = machine_heuristic_weight_composite;*/
		/*machine_heuristic_weight = machine_heuristic_weight_current_machine_cost;*/
		/*machine_heuristic_weight = machine_heuristic_weight_cost;*/ /*previous_machine_cost;*/
		/*machine_heuristic_weight = machine_heuristic_weight_best_fit;*/


		/*process_heuristic_weight = process_heuristic_weight_no_machines*/
		/*process_heuristic_weight = process_heuristic_weight_no_failures;*/
		process_heuristic_weight = process_heuristic_weight_composite;
		/*process_heuristic_weight = process_heuristic_weight_current_cost;*/
		service_heuristic_weight = service_heuristic_weight_composite;

		/*service_heuristic_weight = service_heuristic_weight_current_cost;*/
		/*service_heuristic_weight = service_heuristic_weight_no_failures;*/
		/*service_heuristic_weight = service_heuristic_weight_total_requirement_cost;*/
		/*service_heuristic_weight = service_heuristic_weight_no_machines;*/
		restore_process = restore_process_and_service;
		select_and_remove_process = select_and_remove_service_and_then_process;

		/*computeInitialLowerBound();*/
		lns();
		exit_solver();

	}
	return EXIT_SUCCESS;
}



