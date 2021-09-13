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
#include <assert.h>
#include <math.h>
#include "definitions.h"
#include "selection.h"
#include "search.h"
#include "cost.h"

double machine_heuristic_weight_cost(usint const p, usint const m) {
	return  pmcost[pid_ds[p]][m] ;
}


double machine_heuristic_weight_current_machine_cost(usint const p, usint const m) {
	double cost = machine_cost[m];	
	if (cost == 0)
		cost = -rand();
	return cost;
}

double machine_heuristic_weight_random(usint const p, usint const m){
	return rand();
}


double process_heuristic_weight_no_machines(usint const p) {
	return no_available_machines[pid_ds[p]];
}


double process_heuristic_weight_requirement_cost(usint const p){
	return process_requirement_cost[p] ;
}

double process_heuristic_weight_no_failures(usint const p){
	return 1 ; /*process_failures[p] ;*/
}

double process_heuristic_weight_current_cost(usint const p){
	return costOfProcess[pid_ds[p]];
}

double process_heuristic_weight_composite(usint const p){
	return (costOfProcess[pid_ds[p]]+process_requirement_cost[p])
	/(no_available_machines[pid_ds[p]]+0.1);
}


double service_heuristic_weight_no_machines(usint const i){
     return (-1.0*services[i].no_of_available_machines);
}


/* in future maintain this information during search */
double service_heuristic_weight_total_requirement_cost(usint const i){
     usint j;
     SERVICE *service = &(services[i]);
     double cost = 0.0 ;
     for (j=0; j < service->no_of_unassigned_processes; j++)
	cost += process_requirement_cost[service->unassigned_process[j]];
     return cost;
}


double service_heuristic_weight_current_cost(usint const i){
     usint j;
     SERVICE *service = &(services[i]);
     double cost = 0.0 ;
     for (j=0; j < service->no_of_unassigned_processes; j++)
	cost +=  costOfProcess[pid_ds[service->unassigned_process[j]]] ;
     return cost;
}

double service_heuristic_weight_composite(usint const i){
	usint j;
	SERVICE *service = &(services[i]);
	double cost = 0.0 ;
	for (j=0; j < service->no_of_unassigned_processes; j++)
		cost += (costOfProcess[pid_ds[service->unassigned_process[j]]]
		        +process_requirement_cost[service->unassigned_process[j]])
		        /(services[i].no_of_available_machines+1);
	return cost;
}



usint select_and_remove_service_and_then_process(){
	usint p;
	select_and_remove_service();
	p = select_and_remove_process_from_the_current_service();
	/*for (r = 0; r < no_resources; r++)
		demand[r] -= R[p][r];*/
	return p;
}



void select_and_remove_service( void ){
	unsigned int tservice,sindex,index;
	double sweight, weight;
	/* service selection */
	if (current_service_index == -1 || services[active_services[current_service_index]].no_of_unassigned_processes == 0) {
		/*current_service_index++;*/
		sindex = ++current_service_index;
		sweight = service_heuristic_weight(active_services[sindex]);
		for (index=current_service_index+1; index < current_service_index + no_of_active_services ; index++) {
			weight = service_heuristic_weight(active_services[index]);
			if (weight > sweight) {
				sindex  = index;
				sweight = weight;
			}
		}
		if (sindex != current_service_index) {
			tservice = active_services[current_service_index];
			active_services[current_service_index] = active_services[sindex];
			active_services[sindex] = tservice;
		}
	}
	/*  current_service_index = no_of_active_services - 1;*/
#if assertion == 1
	assert (no_of_rem_unassigned_processes > 0);
#endif
	/*return active_services[current_service_index];*/
}

    /* process selection (stack wise)*/
usint select_and_remove_process_from_the_current_service(void){
    SERVICE *service;
    int index,sindex,tprocess;
    double weight, sweight;
    unsigned int currentp;
    service = &services[active_services[current_service_index]];
    #if assertion == 1
    assert(service->no_of_unassigned_processes > 0 );
    #endif
    sindex = service->no_of_unassigned_processes-1;
    sweight = process_heuristic_weight(service->unassigned_process[sindex]);	
    for (index=service->no_of_unassigned_processes-2; index>=0; index--){
	    weight = process_heuristic_weight(service->unassigned_process[index]);
            if (weight > sweight) {
                sindex  = index;
                sweight = weight; 
            }
    }
    if (sindex != service->no_of_unassigned_processes-1) {
		tprocess = service->unassigned_process[service->no_of_unassigned_processes-1];
		service->unassigned_process[service->no_of_unassigned_processes-1] = service->unassigned_process[sindex];
		service->unassigned_process[sindex] = tprocess; 
    }
    currentp = service->unassigned_process[service->no_of_unassigned_processes-1];
    no_of_rem_unassigned_processes--;
    return currentp;
}

void restore_process_and_service( usint const p ){	     
    no_of_rem_unassigned_processes++;
    if (active_services[current_service_index] != S[p])
	current_service_index--;
    #if assertion == 1 
    assert(active_services[current_service_index] == S[p]);
    #endif
    futureCost += costOfProcess[pid_ds[p]];
}

usint select_and_remove_machine(usint const p) {
   int pid = pid_ds[p];
   return values[pid][--dsize[pid]];
}


