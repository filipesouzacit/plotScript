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
#include <time.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include "definitions.h"
#include "search.h"
#include "constraints.h"
#include "machine.h"
#include "utils.h"
#include "selection.h"
#include "cost.h"
#define MAX_FILENAME_SIZE 1000	
#define MAX_NO_MACHINES_SELECTION 11


int int_cmp(const void *a, const void *b)
{ 
	const usint *ia = (const usint *)a;
	const usint *ib = (const usint *)b;
	if (machine_cost[*ia]   < machine_cost[*ib])
		return 1;
	return -1;
}

boolean consistent(const usint m, const usint p, int *reason) {
	PROCESS_FAILED=FALSE;

	/* update domains */
	assign_machine_to_process(m,p) ;
	*reason=1;

	if (objective_cost  + futureCost  >= best_objective_cost) {
		dsize[pid_ds[p]]=0;
		return FALSE;
	}else if (objective_cost  + lcBound() + bcBound()  >= best_objective_cost) {
		dsize[pid_ds[p]]=0;
		return FALSE;
	}
	else if( !(conflictConstraint(S[p]) && spreadConstraint(S[p]) && serviceDependencyConstraints(S[p])))
		return FALSE;

	remove_machine_from_the_processes_of_current_service(m, p);
	*reason=2;
	if (PROCESS_FAILED)
		return FALSE;

	remove_machine_from_the_processes_of_noncurrent_services(m, p);
	*reason=3;
	if (PROCESS_FAILED)
		return FALSE;

	/*remove_machines_from_services_becauseof_spread_constraint( ) ;
	remove_machines_from_services_becauseof_neighborhood_constraint(S[p]);
	*reason=4;
	if (PROCESS_FAILED) {
		return FALSE;
	}else if( !(conflictConstraint(S[p]) && spreadConstraint(S[p]) && serviceDependencyConstraints(S[p]))){
		return FALSE;
	}
*/
	/*for (s = current_service_index; s < no_of_services_selected; s++) {
		if(active_services[s] != S[p] && services[active_services[s]].no_of_unassigned_processes > 0 && !( conflictConstraint(active_services[s]) && spreadConstraint(active_services[s]) && serviceDependencyConstraints(active_services[s]) )) {
			return FALSE;
		}
	}*/
	return TRUE;
}


void undo(const usint m, const usint p, const int reason) {
	/*			add_machines_becauseof_neighborhood_constraint(S[p]);

	add_machines_to_services_becauseof_spread_constraint( ) ;*/


	if (reason == 3) {
		add_machine_to_services_during_search(m);
		add_machine_to_the_processes_of_noncurrent_services(m, p);
		add_machine_to_the_processes_of_current_service(m, p);
		unassign_machine_to_process(m,p) ;
	}
	else if (reason == 2) {
		add_machine_to_the_processes_of_current_service(m, p);
		unassign_machine_to_process(m,p) ;
	}
	else
		unassign_machine_to_process(m,p) ;
}

static __inline__
boolean stopping_condition(void ){
	if (difftime(time(NULL), start_time) > (time_limit-print_time))
		stop_search= TRUE;
	else
		stop_search = FALSE;
	return stop_search;
} 

static __inline__
void save_solution(void) {
	usint i;
	for (i=0;i < no_of_unassigned_processes; i++) 
		BM[unassigned_processes[i]] = NM[unassigned_processes[i]];
}

void search( void ) {
	usint pid,p,m=NONE, service_index, size;
	int reason;
	if (no_of_rem_unassigned_processes == 0) {
		if (best_objective_cost > objective_cost)  {
			best_objective_cost = objective_cost ;
			save_solution( ) ;
			/*print_solution_in_file(solutionfilename);            */
			/*if (verification)
				checkSolution( );*/
		}
		stop_search = stopping_condition();
	} else {
		/* select process */
		service_index = current_service_index;
		current_process =  p = select_and_remove_process();
		pid = pid_ds[p];
		size = dsize[pid];

		remove_machines_from_the_domain_of_process(p);
		/* propagate */
		if (dsize[pid] == 0)
			no_of_failures++;
		else {
			/* select,remove and assign machine to process */
			/* filter domains */
			do {
				m=select_and_remove_machine(p);
				if (consistent(m,p,&reason))
					search( );
				else
					no_of_failures++;
				undo(m,p,reason);
			}while(dsize[pid] > 0 && !stop_search   &&  no_of_failures < threshold)  ;
		}
		dsize[pid] = size;
		restore_process(p);
		current_service_index = service_index;
	}
}


usint get_machine_based_on_machine_costs(void) {
	static int mindex = 0, total_rem = 0;
	usint m;
	if (mindex == 0 || machine_cost[machineIndices[mindex]] <= 0) {
		qsort(machineIndices, no_machines, sizeof(usint), int_cmp);
		mindex = 0;
	}
	m = machineIndices[mindex];
	total_rem += remove_no_process;
	if (total_rem + (no_processes/no_machines)/2 >= machines[m].n ) {
		mindex = (mindex+1)%no_machines;
		total_rem = 0;
	}
	return m;
}

/* maintain a list of machines with cost greater 0 */
void create_random_subproblem_machines_restricted(const usint k) {
	int i,j,m,p,trial=0;
	int n;
	int avgp = no_processes/no_machines ;
	MACHINE *machine ;
	no_of_unassigned_processes = 0;

	if (k <= 4 )
		remove_no_process =  avgp/2 < 10 ? avgp/2: 10;
	else
		remove_no_process =  avgp/2 < 40/k ? avgp/2 : 40/k;

	for (i = 0 ; i < k; i++) {
		while(no_of_unassigned_processes < remove_no_process*(i+1)) {
			if (i == 0 && trial < 2  ) {
				m=get_machine_based_on_machine_costs();
				trial++;
			}
			else
				m=rand()%no_machines;
			machine = &(machines[m]);
			/*n = machine->n < real_remove? machine->n : real_remove;*/
			n = machine->n < remove_no_process*(i+1)-no_of_unassigned_processes? machine->n : remove_no_process*(i+1)-no_of_unassigned_processes;
			/*assert(n >= 0);*/
			if (n < machine->n ) {  /* requires randomization */
				while ( n ) {
					j = rand()%machine->n;
					p = machine->processes[j];
					#if assertion == 1
					assert(NM[p]==m);
					#endif
					machine->processes[j]=machine->processes[machine->n-1];
					machine->n--;
					unassigned_processes[no_of_unassigned_processes++] = p;
					unassign_machine_to_process_before_search(m,p) ;
					n--;
				}
			} else {		/* no randomization since all processes are removed*/
				for (j=0; j < n; j++) {
					p = machine->processes[j];
					#if assertion == 1
					assert(NM[p]==m);
					#endif
					unassigned_processes[no_of_unassigned_processes++] = p;
					unassign_machine_to_process_before_search(m,p) ;
					machine->n--;
				}
			}
		}
	}
}







void init_lns(void ){

	iterations = 0;
	total_failures = 0;
	no_of_failures = 0;
	service_machines_counter = 0;

	if ((pid_ds = (usint*)CALLOC(no_processes,sizeof(usint)))==NULL){
		fprintf(stdout, "could not allocate memory for pid_ds\n");
		abort();
	}

	if ((machineIndices = (usint*)CALLOC(no_machines, sizeof(usint))) == NULL){
		fprintf(stdout, "could not allocate memory for machineIndices\n");
		abort();
	}

	if ((unassigned_processes = (usint*)CALLOC(SEARCH_NPROCESSES, sizeof(usint))) == NULL){
		fprintf(stdout, "could not allocate memory for unassigned_processes\n");
		abort();
	}

	if ((pmcost_up = (long long int*)CALLOC((SEARCH_NPROCESSES*(SEARCH_NPROCESSES+1))/2,sizeof(long long int)))==NULL){
			fprintf(stdout, "could not allocate memory for pmcost_up\n");
			abort();
	}

	if ((fp_up = (usint*)CALLOC((SEARCH_NPROCESSES*(SEARCH_NPROCESSES+1))/2,sizeof(usint)))==NULL){
				fprintf(stdout, "could not allocate memory for fp_up\n");
				abort();
	}

	service_machines_limit =  SEARCH_NPROCESSES + no_machines*SEARCH_NPROCESSES*2;
	if ((service_machines_stack = (usint*)CALLOC(service_machines_limit,sizeof(usint)))==NULL){
		fprintf(stdout, "could not allocate memory for service_machines_stack\n");
		abort();
	}

	if(printinfile) {
		char buf[20];
		logfilename = (char*)CALLOC(MAX_FILENAME_SIZE,sizeof(char));
		strcpy(logfilename,"log_mh-");
		if (mh_id == MH_RANDOM)
			strcat(logfilename,"random_");
		else if (mh_id == MH_COST)
			strcat(logfilename, "cost_");

		strcat(logfilename,"tf-");
		sprintf(buf, "%llu", input_threshold);
		strcat(logfilename, buf);

		strcat(logfilename, "_time-");
		sprintf(buf,"%d",time_limit);
		strcat(logfilename,buf);
		strcat(logfilename, "_");

		strcat(logfilename,solutionfilename);
		if ( (logfile=fopen(logfilename, "w"))== NULL) 
			printf("Error in opening a file..%s",logfilename);

	}

	current_service_index =-1 ;
}

void exit_lns(void ) {
	free(service_machines_stack);
	free(machineIndices);
	free(unassigned_processes);
	free(pmcost_up);
	free(fp_up);

	if(printinfile) {
		fclose(logfile);
		free(logfilename);
	}
}


static __inline__
void update_counters(void ) {
	iterations++;
	/*total_failures+=no_of_failures;*/
	no_of_failures = 0;
}


void roadef_lns(void) {
	usint i;
	init_lns();
	if(printinfile)
		fprintf(logfile, "%5llu %10llu  %10llu %5.0f  0\n", iterations, total_failures,  best_objective_cost,difftime(time(NULL), start_time));
	for(i=0;i<no_machines;i++)
		machineIndices[i] = i;
	threshold = input_threshold;
	machine_heuristic_weight = machine_heuristic_weight_cost ;
	lns_machine_selection(time_limit-print_time);
	print_solution_in_file(solutionfilename);
	exit_lns();
}


void default_lns(void) {
	usint i;
	init_lns();
	if(printinfile)
		fprintf(logfile, "%5llu %10llu  %10llu %5.0f  0\n", iterations, total_failures,  best_objective_cost,difftime(time(NULL), start_time));
	for(i=0;i<no_machines;i++)
		machineIndices[i] = i;
	threshold = input_threshold;
	machine_heuristic_weight = input_machine_heuristic_weight ;
	lns_machine_selection(time_limit-print_time);
	print_solution_in_file(solutionfilename);
	exit_lns();
}

static __inline__
void map_pids_for_lns(void){
	usint i;
	for (i=0; i < no_of_unassigned_processes; i++)
		pid_ds[unassigned_processes[i]]  = i;
}

static __inline__
int propagate(void){
	/* usage based filtering */
	remove_machines_from_services_before_search( );
	update_costs_before_search( );
	/* cost based filtering */
	return TRUE; /*costBasedFiltering();*/
}

static __inline__
int findMax(unsigned long long int *s, unsigned long long int *t, int c,unsigned long long int *ts){
	double bratio=-LONG_MAX;
	int i,n=c;
	for (i=0;i<MAX_NO_MACHINES_SELECTION-1;i++){
			if  (ts[i] + 3  < iterations && s[i]/(t[i]*1.0) > bratio) {
				n = i+1;
				bratio = s[i]/(t[i]*1.0);
			}
	}
	return n;
}


void lns_machine_selection(int const local_time_limit ) {
	int km=1;
	unsigned long long int old_objective_cost = 0;
	int no_improvement_m = 0;
	unsigned long long int success[MAX_NO_MACHINES_SELECTION-1];
	unsigned long long int total[MAX_NO_MACHINES_SELECTION-1];
	unsigned long long int ts[MAX_NO_MACHINES_SELECTION-1];
	double factor ;
	int no_improvement_threshold =    100 < no_processes/no_machines ? 100: no_processes/no_machines ;
	usint r,i;
	if (no_processes > 10000)
		factor = 0.5;
	else if (no_processes > 1000)
		factor = 1;
	else
		factor = 2;

	for (i=0;i < MAX_NO_MACHINES_SELECTION-1;i++) {
		success[i]=1;
		total[i]=1;
		ts[i]=0;
	}

	do {
		no_of_failures = 0;
		for(r=0; r < no_resources;r++)
			demand[r]=0;
		create_random_subproblem_machines_restricted(km);
		map_pids_for_lns();
		threshold = ceil(no_of_unassigned_processes*factor);

		no_of_rem_unassigned_processes = no_of_unassigned_processes;
		no_of_services_selected = no_of_active_services;
		old_objective_cost = best_objective_cost;
		current_service_index = -1;
		if (propagate())
			search( );
		iterations++;
		/*total_failures+=no_of_failures;*/
		current_service_index = 0;
		total[km-1]++;
		if (old_objective_cost > best_objective_cost){
			if (printonscreen)
				printf( "iterations: %llu   failures:%llu cost:%llu  time %5.0f  m %d p %d ineffective_iterations: %d\n",iterations, no_of_failures, best_objective_cost, difftime(time(NULL), start_time), km, remove_no_process, no_improvement_m);
			/*printf( "M: iterations: %llu total_failures: %llu  failures:%llu cost:%llu  time %5.0f  m %d p %d ineffective_iterations: %d\n",iterations, total_failures, no_of_failures, best_objective_cost, difftime(time(NULL), start_time), km, remove_no_process, no_improvement_m);*/

			/*else if (printinfile)
				fprintf(logfile, "%5llu %10llu  %10llu %5.0f  %2d\n", iterations, total_failures, best_objective_cost, difftime(time(NULL), start_time), *km);*/
			no_improvement_m = 0;
			success[km-1]++;


		}
		else {
			no_improvement_m++;
			ts[km-1]=iterations;
		}
		if (no_improvement_m >  no_improvement_threshold)
				km=findMax(success,total,km,ts);
		add_machines_to_services_after_search( );
		for (i=0;i < no_of_unassigned_processes; i++)
			assign_machine_to_process_after_search(BM[unassigned_processes[i]],unassigned_processes[i]) ;
		#if assertion == 1
		assert(no_of_active_services == 0);
		#endif
	} while(difftime(time(NULL),start_time) <= local_time_limit);
}

