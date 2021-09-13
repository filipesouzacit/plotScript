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

#include<stdio.h>
#include "definitions.h"
#define SEARCH_NPROCESSES 40

int no_of_unassigned_processes,  no_of_rem_unassigned_processes ;
boolean stop_search, solution_found;
int remove_no_process;
boolean verification;

usint *unassigned_processes;
boolean *max_moved_store;
long long int *pmcost_up ;
usint *fp_up;
usint  *machineIndices ;
usint *pind;
unsigned long long int threshold, input_threshold, thresholdm;
unsigned int no_of_services_selected ;
unsigned long long int best_objective_cost;


unsigned long long int total_failures;
unsigned long long int no_of_failures ;

unsigned long long int iterations, siterations ;
int current_service_index ;
usint current_process;
usint process_selection(void) ;


void default_lns( void );
void roadef_lns( void );
void lns_machine_selection(const int local_time_limit );
void (*lns_type_selection)(int);
void (*lns)(void);
unsigned short int  *service_machines_stack, *pid_ds;
unsigned int service_machines_counter, service_machines_limit  ;

