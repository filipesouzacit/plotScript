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
#include <limits.h>
#include <string.h>
#include <math.h>
#include "definitions.h"
#include "machine.h"
#include "cost.h"
#include "constraints.h"
#include "utils.h"
#include "search.h"

void initialize_usage_non_incremental(void) {
	usint p,r;
	for (p=0; p < no_processes ; p++) {
		for (r=0; r < no_resources ; r++) {
			usage[OM[p]][r] += R[p][r] ;
			if(transient[r])
				tusage[OM[p]][r] += R[p][r] ;
		}
	}
}

void assign_original_machine_to_process(const usint m, const usint p){
	usint r,b;
	SERVICE *service = &(services[S[p]]);
	MACHINE machine = machines[m];
	long long int tcost = 0;
	/* assign machine to process */
	OM[p] = NM[p] = m;

	for (r=0; r < no_resources; r++) {
		tcost = loadCost(m,r)*weight_lc[r];
		if (tcost > 0) {
			objective_cost -= tcost;
			machine_cost[m] -= tcost;
		} else
			supply[r] -= machines[m].scapacities[r] - usage[m][r];
	}
	for (b=0; b < no_balances; b++) {
		tcost = balanceCost(m,b)*balances[b].weight_bc;
		objective_cost  -= tcost;
		machine_cost[m] -=  tcost;
	}
	increment_usage_of_original_machine(m,p);

/*	for (r = 0; r <no_resources; r++)
		supply[r] += machines[m].scapacities[r] > usage[m][r] ? machines[m].scapacities[r] - usage[m][r]: 0;
*/
	/* in future optimise the way machine_cost and objective_cost are updated */
	for (r=0; r < no_resources; r++)  {
		tcost = loadCost(m,r)*weight_lc[r];
		if (tcost > 0) {
			objective_cost   += tcost;
			machine_cost[m] += tcost;
		} else
			supply[r] += machines[m].scapacities[r] - usage[m][r];
	}
	for (b=0; b < no_balances; b++) {
		tcost = balanceCost(m,b)*balances[b].weight_bc;
		objective_cost   += tcost;
		machine_cost[m] +=  tcost;
	}

	add_neighborhood_to_service(service, machine.neighborhood);
	add_location_to_service(service, machine.location);

	add_machine_to_service(service, m);
	service->no_of_unassigned_processes--;
}


void assign_machine_to_process(const usint m, const usint p){
	SERVICE *service = &(services[S[p]]);
	usint r,b;
	long long int temp, tcost = 0;
	/* assign machine to process */
#if assertion == 1
	assert(NM[p]==UNASSIGNED);
	assert(m!=UNASSIGNED);
#endif

	NM[p] = m;
	if (m != OM[p]) {
		/*service->no_of_moved_processes++;*/
		if (maxno_processes_moved_in_service < ++service->no_of_moved_processes)  {
			maxno_processes_moved_in_service++; /* = service->no_of_moved_processes */;
			objective_cost += weight_smc;
			*max_moved_store = 1;
		} else
			*max_moved_store = 0;
		max_moved_store++;
	}
	for (r = 0; r <no_resources; r++) {
		demand[r] -= R[p][r];
		supply[r] -= machines[m].scapacities[r] > usage[m][r] ? machines[m].scapacities[r] -usage[m][r]: 0;
	}

	increment_usage_of_machine(m,p);


	for (r=0; r < no_resources; r++) {
		temp = loadCost(m,r)*weight_lc[r] ;
		if (temp > 0)
			tcost   += temp;
		else
			supply[r] += machines[m].scapacities[r] - usage[m][r];
	}
	for (b=0; b < no_balances; b++)
		tcost += balanceCost(m,b)*balances[b].weight_bc;

	tcost -= machine_cost[m];
	objective_cost += tcost;
	machine_cost[m] += tcost;

#if assertion == 1
	assert(machine_cost[m] == machineCost(m));
	assert(machine_cost[m] >= 0);
#endif
	/*objective_cost += processMoveCost(p)*weight_pmc;
	objective_cost += machineMoveCost(p)*weight_mmc;*/
	objective_cost += movecost[pid_ds[p]][m];

/*	assert(pmcost[pid_ds[p]][m]==tcost+movecost[pid_ds[p]][m]);*/
	add_neighborhood_to_service(service, machines[m].neighborhood);

	add_location_to_service(service, machines[m].location);
	add_machine_to_service(service, m);

	/*service->no_of_unassigned_processes--;*/
	if (--service->no_of_unassigned_processes == 0)
		no_of_active_services--;

}



void unassign_machine_to_process(const usint m, const usint p){
	SERVICE *service;
	MACHINE machine;
	usint r,b;
	boolean temp;
	long long int lcost, tcost = 0;
	service = &(services[S[p]]);
	machine = machines[NM[p]];

	if (service->no_of_unassigned_processes++ == 0)
		no_of_active_services++;
	/*service->no_of_unassigned_processes++;*/
	remove_machine_from_service(service, m);
	remove_location_from_service(service, machine.location);
	remove_neighborhood_from_service(service, machine.neighborhood);


	for (r=0; r < no_resources; r++) {
		demand[r] += R[p][r];
		supply[r] -= machines[m].scapacities[r] > usage[m][r] ? machines[m].scapacities[r] - usage[m][r]: 0;
	}
	decrement_usage_of_machine(m,p);

	tcost -= machine_cost[m];
	for (r=0; r < no_resources; r++)  {
		lcost = loadCost(m,r)*weight_lc[r];
		if (lcost > 0)
			tcost   += lcost;
		else
			supply[r] += machines[m].scapacities[r] - usage[m][r];
	}
	for (b=0; b < no_balances; b++)
		tcost += balanceCost(m,b)*balances[b].weight_bc;
	objective_cost += tcost;
	machine_cost[m] += tcost;

	objective_cost -= movecost[pid_ds[p]][m];
	if (m != OM[p]) {
		temp = *(--max_moved_store);
		maxno_processes_moved_in_service -= temp;
		objective_cost -= temp*weight_smc;
		service->no_of_moved_processes--;
	} 
	NM[p] = UNASSIGNED;
}

void assign_machine_to_process_after_search(const usint m, const usint p){
	SERVICE *service = &(services[S[p]]);
	usint r,b;
	long long int lcost, tcost = 0;

	add_process_to_machine(m,p);

	/* assign machine to process */
#if assertion == 1
	assert(NM[p]==UNASSIGNED);
	assert(m!=UNASSIGNED);
#endif

	NM[p] = m;
	if (m != OM[p]) {
		if (maxno_processes_moved_in_service < ++service->no_of_moved_processes)  {
			maxno_processes_moved_in_service = service->no_of_moved_processes ;
			objective_cost += weight_smc;
		}
	}

	/* save current costs */
	for (r=0; r < no_resources; r++)
		supply[r] -= machines[m].scapacities[r] > usage[m][r] ? machines[m].scapacities[r] - usage[m][r]: 0;

	increment_usage_of_machine(m,p);

		/* compute new costs*/
	for (r=0; r < no_resources; r++)  {
		lcost = (loadCost(m,r))*weight_lc[r];
		if (lcost > 0)
			tcost += lcost;
		else
			supply[r] += machines[m].scapacities[r] - usage[m][r];
	}

	for (b=0; b < no_balances; b++)
		tcost  += (balanceCost(m,b))*balances[b].weight_bc;

	tcost -= machine_cost[m];
	machine_cost[m] += tcost;
	objective_cost +=tcost;

	objective_cost += movecost[pid_ds[p]][m];
	/*objective_cost += processMoveCost(p)*weight_pmc;
	objective_cost += machineMoveCost(p)*weight_mmc;*/
#if assertion == 1
	assert(machine_cost[m]==machineCost(m));
	assert(machine_cost[m] >= 0);
#endif
	add_neighborhood_to_service(service, machines[m].neighborhood);
	add_location_to_service(service, machines[m].location);

	add_machine_to_service(service, m);

	/* the service is not active  */
	if (--service->no_of_unassigned_processes==0)
		no_of_active_services--;


}


void unassign_machine_to_process_before_search(const usint m, const usint p){
	SERVICE *service;
	MACHINE machine;
	usint r,b,s;
	long long int lcost, tcost = 0;


	service = &(services[S[p]]);
	machine = machines[NM[p]];

	/*remove_process_from_machine(m,p);*/

	/* update the list of unassigned processes of a service*/
	service->unassigned_process[service->no_of_unassigned_processes] = p;
	/* set the service of the process active */
	if (service->no_of_unassigned_processes==0) {
		active_services[no_of_active_services] = S[p];
		no_of_active_services++;
	}

	service->no_of_unassigned_processes++;

	/*remove_process_from_machine(m,p);*/
	remove_machine_from_service(service, m);
	remove_location_from_service(service, machine.location);
	remove_neighborhood_from_service(service, machine.neighborhood);


	for (r=0; r < no_resources; r++)
		supply[r] -= machines[m].scapacities[r] > usage[m][r] ? machines[m].scapacities[r] - usage[m][r]: 0;

	decrement_usage_of_machine(m,p);

	for (r=0; r < no_resources; r++)  {
		lcost = (loadCost(m,r))*weight_lc[r];
		if (lcost > 0)
			tcost   += lcost;
		else
			supply[r] +=  machines[m].scapacities[r] - usage[m][r];
		demand[r] += R[p][r];
	}
	for (b=0; b < no_balances; b++){
		tcost   += (balanceCost(m,b))*balances[b].weight_bc;
	}
	tcost -= machine_cost[m];
	objective_cost += tcost;
	machine_cost[m] += tcost;

	objective_cost -= processMoveCost(p)*weight_pmc;
	objective_cost -= machineMoveCost(p)*weight_mmc;

#if assertion == 1
	assert(machine_cost[m] == machineCost(m));
	assert(machine_cost[m] >= 0);
#endif
	/* update service cost*/
	if (m != OM[p]) {
		if (service->no_of_moved_processes-- == maxno_processes_moved_in_service) {
			maxno_processes_moved_in_service = 0;
			for (s=0;s<no_services;s++)
				maxno_processes_moved_in_service = maxno_processes_moved_in_service > services[s].no_of_moved_processes? maxno_processes_moved_in_service : services[s].no_of_moved_processes ;
			if (service->no_of_moved_processes+1 != maxno_processes_moved_in_service)
				objective_cost -= weight_smc;
		}
	} 

	NM[p] = UNASSIGNED;

}








/* This should be invoked after  assigning m to p. The assumption is therefore that m is USED by S[p] (and therefore not available to any unassigned
 * process of service S[p]) */
/* remove m from each unassigned process of the service of p.
 * also update the lb of the cost that will be incurred for each unassigned process of the same service
 *
 */
void remove_machine_from_the_processes_of_current_service(const usint m, const usint p) {
	usint i,fpid;
	SERVICE *service = &(services[S[p]]);
	for (i = 0; i < service->no_of_unassigned_processes; i++) {
		fpid = pid_ds[service->unassigned_process[i]];
		if (msrop[fpid][m]) {
			/*--no_available_machines[fpid] ;*/
			if(--no_available_machines[fpid])
				updateCurrentCostOfProcess_down(service->unassigned_process[i],m);
			else
				PROCESS_FAILED = TRUE;
			/*if (PROCESS_FAILED);
				return ;*/
		}
	}
}


/* This should be invoked before unassigning machine m from p. The assumption is therefore that m is USED by S[p] (and therefore not available to any unassigned process of service S[p]) */
void add_machine_to_the_processes_of_current_service(usint m, usint p) {
	usint fp,i,fpid;
	SERVICE *service = &(services[S[p]]);
	#if defined(assertion)
	assert(service->used_machines[m] == USED);
	#endif
	for (i = 0; i < service->no_of_unassigned_processes; i++) {
		fp = service->unassigned_process[i];
		fpid = pid_ds[fp];
		#if assertion == 1
		assert(fp != p);
		#endif
		/*assert(machineSatisfyRequirementOfFutureProcessBeforeAdding(m,p,fp)==msrop[fp][m]);*/
		if (msrop[fpid][m]){
			if(no_available_machines[fpid]++)
				updateCurrentCostOfProcess_up(fp,m);
			/*no_available_machines[fpid]++;*/
		}
	}
}


void add_machine_to_the_processes_of_noncurrent_services(const usint m, const usint p) {
	usint fpid,fp, counter =	*(--fp_up);
	while (counter-- > 0){
		fp = *(--fp_up);
		fpid = pid_ds[fp];
		if (msrop[fpid][m]==FALSE){
			msrop[fpid][m]=TRUE;
			if (no_available_machines[fpid]++)
				updateCurrentCostOfProcess_up(fp,m);
			/*no_available_machines[fpid]++;*/
		} else {
			/*printf("correct%10.0f current %lld\n",costOfAssigningProcessToMachine_ar(fp,p,m),pmcost[fpid][m]);*/
			pmcost[fpid][m] = *(--pmcost_up);
			updateCurrentCostOfProcess_up(fp,m);
		}
	}
}

/* This should be invoked before removing m from for non current service s. Therefore the assumption is  m is available for service s and s is not equal to the current service S[p]*/
void remove_machine_from_the_processes_of_noncurrent_service(const usint s, const usint m, const usint p, usint *counter, usint *mchanges) {
	usint i,fpid,fp;
	SERVICE *service  = &(services[s]);
	boolean supported = FALSE;

	for (i = 0; i < service->no_of_unassigned_processes; i++) {
		fp = service->unassigned_process[i];
		fpid = pid_ds[fp];
		/*		assert(fp != p && NM[fp] == UNASSIGNED);
		assert(machineSatisfyRequirementOfFutureProcessBeforeAdding(m,p,fp)==msrop[fp][m]);*/
		if (msrop[fpid][m]){
			*fp_up = fp; fp_up++;
			(*mchanges)++;
			if (!machineSatisfyRequirementOfProcess(m,fp)) {
				msrop[fpid][m] = FALSE;
				/*--no_available_machines[fpid];*/
				if(--no_available_machines[fpid])
					updateCurrentCostOfProcess_down(fp,m);
				else
					PROCESS_FAILED = TRUE;
			} else {
				supported = TRUE;
				*pmcost_up = pmcost[fpid][m]; pmcost_up++;
				costOfAssigningProcessToMachine_ds(fp,m);
				updateCurrentCostOfProcess_down(fp,m);
				/*assert(pmcost[fpid][m] >= costOfProcess[fpid]);*/
				/*if(costOfProcess[fpid]!=currentCostOfProcess(fp))
						printf("a p %5d m %5d fp %5d bestm %5d current %lld correct %10.0f\n",p,m,fp,bestM[fpid],costOfProcess[fpid],currentCostOfProcess(fp));
				assert(costOfProcess[fpid]==currentCostOfProcess(fp));*/
			}
			if (PROCESS_FAILED)
				return;
		}
	}
	if (!supported) {
		service->used_machines[m] = CANNOTBEUSED;
		service->no_of_available_machines--;
		service_machines_stack[service_machines_counter++] = s;
		(*counter)++;
	}

}

void remove_machine_from_the_processes_of_noncurrent_services(const usint m, const usint p) {
	usint i,s,counter=0, mchanges = 0;
	for (i=current_service_index;i<no_of_services_selected;i++)  {
		s = active_services[i];
		if (S[p] != s && services[s].used_machines[m]==AVAILABLE && services[s].no_of_unassigned_processes > 0)
			remove_machine_from_the_processes_of_noncurrent_service(s, m, p, &counter, &mchanges);
	}
	*fp_up = mchanges; fp_up++;
	service_machines_stack[service_machines_counter++] = counter;
}	





void add_machine_to_services_during_search(const usint m){
	usint s;
	SERVICE *service ;
	int counter = service_machines_stack[--service_machines_counter] ;
	while (counter-- > 0) {
		s =   service_machines_stack[--service_machines_counter] ;
		/*        assert (m ==   service_machines_stack[--service_machines_counter]) ;*/
		service = &(services[s]);
		service->used_machines[m] = AVAILABLE;
		service->no_of_available_machines++;
	}
}




const usint *ia;
const usint *ib;
int machine_cmp(const void *a, const void *b)
{
	ib = (const usint *)b;
	ia = (const usint *)a;

	if (pmcost[pid_ds[current_process]][*ia]  < pmcost[pid_ds[current_process]][*ib])
		return 1;
	else if (pmcost[pid_ds[current_process]][*ia]  > pmcost[pid_ds[current_process]][*ib])
		return -1;
	else {
		if (rand() < rand())
			return -1;
		else
			return  1;
	}
	/* integer comparison: returns negative if a > b
	and positive if a > b */
}




void remove_machines_from_service_before_search(usint s ){
	usint m,p,i,pid;
	boolean remove_m;
	SERVICE *service  = &(services[s]);

	for (m = 0; m < no_machines; m++) {
		if (service->used_machines[m] !=  AVAILABLE)
			continue;
		remove_m =TRUE;
		for (i = 0; i < service->no_of_unassigned_processes; i++){
			p = service->unassigned_process[i];
			pid = pid_ds[p];
			if (machineSatisfyRequirementOfProcess(m,p)) {
				msrop[pid][m] = TRUE;
				values[pid][dsize[pid]++] = m;
				remove_m = FALSE;
			} else
				msrop[pid][m] = FALSE;
		}
		if (remove_m) {
			service->used_machines[m] = CANNOTBEUSED;
			service->no_of_available_machines--;
			service_machines_stack[service_machines_counter++] = m;
			service_machines_stack[service_machines_counter++] = s;
#if assertion == 1
			assert(service->no_of_available_machines > 0);
			assert(service_machines_counter <= service_machines_limit );
#endif
		}
	}
}

void remove_machines_from_services_before_search(void ){
	usint i;
	for (i=0; i < no_of_unassigned_processes; i++) 
		dsize[pid_ds[unassigned_processes[i]]] = 0;
	for (i=0; i<no_of_active_services;i++) 
		remove_machines_from_service_before_search(active_services[i]);
	for (i=0; i < no_of_unassigned_processes; i++) 
		no_available_machines[pid_ds[unassigned_processes[i]]] = dsize[pid_ds[unassigned_processes[i]]] ;
}


void add_machines_to_services_after_search( ){
	usint s,m;
	SERVICE *service ;
	while (service_machines_counter > 0) {
		#if assertion == 1
		assert(no_of_unassigned_processes > 0);
		#endif
		s =   service_machines_stack[--service_machines_counter] ;
		m =   service_machines_stack[--service_machines_counter] ;
		service = &(services[s]);
		service->used_machines[m] = AVAILABLE;
		service->no_of_available_machines++;
	}
}


void remove_machines_from_the_domain_of_process(usint p){
	usint  m,tmachine, pid=pid_ds[p];
	int index,counter=0;
	usint *mvalues = values[pid];
	SERVICE *service = &(services[S[p]]);
    double cost = 1.0*best_objective_cost - 1.0*objective_cost;
	futureCost -=  1.0*costOfProcess[pid];
	cost -=   futureCost;
	for( index=dsize[pid]-1; index >= 0; --index) {
		m = mvalues[index];
		if (!(service->used_machines[m] == AVAILABLE && msrop[pid][m] && 1.0*pmcost[pid][m] + serviceCost(p,m)< cost))   {
			tmachine = mvalues[dsize[pid]-1];
			mvalues[dsize[pid]-1] = mvalues[index];
			mvalues[index] = tmachine;
			dsize[pid]--;
			counter++;
		}
	}
	/*assert(dsize[pid]>=0);*/
 	qsort(mvalues, dsize[pid], sizeof(usint), machine_cmp);
}


