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



#include <limits.h>
#include "definitions.h"
#include "cost.h"
#include "search.h"
#include "machine.h"

long long int resourceSupply(const usint r){
	usint m;
	long long int rsupply = 0;
	for (m=0; m <no_machines; m++)
		 rsupply += (usage[m][r] < machines[m].scapacities[r])  ? (machines[m].scapacities[r] - usage[m][r]) : 0 ;
	return rsupply;
}

long long int resourceDemand(const usint r){
	usint i,s,j,p;
	long long int rdemand = 0;
	for (i=0; i < no_of_services_selected; i++) {
		s=active_services[i];
		for (j=0; j < services[s].no_of_unassigned_processes; j++) {
			p = services[s].unassigned_process[j];
			rdemand += R[p][r];
		}
	}
	return rdemand;
}



unsigned long long int machineLoadCost(int m) {
	usint r;
	unsigned long long int lcost = 0;
	for (r=0; r <no_resources; r++)
		lcost += loadCost(m,r)* (weight_lc[r]);
	return lcost ;
}

static __inline__
unsigned long long int machineBalanceCost(int m) {
	usint b;
	unsigned long long int  bcost = 0 ;
	for (b=0; b <no_balances; b++)
		bcost += balanceCost(m,b)*(balances[b].weight_bc);
	return bcost;
}


unsigned long long int machineCost(int m) {
	return machineLoadCost(m) + machineBalanceCost(m);
}

unsigned long long int totalMachineCost( ){
	usint m;
	unsigned long long int mcost = 0;
	for (m=0; m <no_machines; m++)
		mcost += machineCost(m);
	return mcost ;
}

unsigned long long int resourceLoadCost(int r){
	usint m;
	unsigned long long int cost = 0;
	for (m=0; m < no_machines; m++)
		cost += loadCost(m,r);
	return cost;
}


unsigned long long int totalLoadCost( ){
	usint r;
	unsigned long long int cost = 0;
	for (r=0; r < no_resources; r++)
		cost += resourceLoadCost(r)*weight_lc[r];
	return cost;
}

unsigned long long int balanceBalanceCost(int b ){
	usint m;
	unsigned long long int cost = 0;
	for (m=0; m < no_machines; m++) {
		cost += machine_cost[m];
	}
	return cost;
}

unsigned long long int totalBalanceCost( ){
	usint m;
	unsigned long long int cost = 0;
	for (m=0; m < no_machines; m++){
		machine_cost[m]= machineBalanceCost(m);
		cost += machine_cost[m];
	}
	/*for (b=0; b < no_balances; b++) {
		cost += balanceBalanceCost(b)*balances[b].weight_bc;
	}*/
	return cost;
}

void initObjectiveCost() {
	objective_cost = totalBalanceCost() ;
}



double costOfAssigningProcessToMachine_ws(usint p, usint m){
	double cost  = 0, old, new;
	usint r,b;

	/* load costs*/
	for (r=0; r < no_resources; r++) {
		old = 1.0*usage[m][r]  - 1.0*machines[m].scapacities[r];
		if (old > 0)
			cost += 1.0*R[p][r]*weight_lc[r];
		else {
			new = old + R[p][r];
			if (new > 0)
				cost += new*weight_lc[r];
		}
		/*new = (usage[m][r] + R[p][r] > machines[m].scapacities[r])  ? (usage[m][r] + R[p][r] - machines[m].scapacities[r]) : 0 ;
		old = (usage[m][r]  > machines[m].scapacities[r])  ? (usage[m][r] - machines[m].scapacities[r]) : 0 ;
		cost += (new - old)*weight_lc[r];*/
	}

	/* balance costs */
	for (b=0; b < no_balances; b++) {
		BALANCE bal = balances[b];
		old = bal.target*machines[m].capacities[bal.i]  - machines[m].capacities[bal.j] - bal.target*usage[m][bal.i] + usage[m][bal.j]  ;
		new = old - bal.target*R[p][bal.i] + R[p][bal.j] ;
		if (old < 0)
			old = 0;
		if (new < 0)
			new = 0;
		cost += (new - old)*bal.weight_bc;
	}

	if (m != OM[p]) {
		/* process move cost */
		cost += weight_pmc*pmc[p]  ;
		/* machine move cost */
		cost += weight_mmc*machines[OM[p]].mmc[m] ;

		/* service cost */
		/*if (services[S[p]].no_of_moved_processes >= maxno_processes_moved_in_service )
			cost += weight_smc;*/
	}
	return cost ;
}

double costOfAssigningProcessToMachine_ar(usint p, usint cp, usint m){
	double cost  = 0, old, new;
	usint r,b;

	/* load costs*/
	for (r=0; r < no_resources; r++) {
		old = 1.0*usage[m][r] - R[cp][r] - 1.0*machines[m].scapacities[r];
		if (old > 0)
			cost += 1.0*R[p][r]*weight_lc[r];
		else {
			new = old + R[p][r];
			if (new > 0)
				cost += new*weight_lc[r];
		}
		/*new = (usage[m][r] + R[p][r] > machines[m].scapacities[r])  ? (usage[m][r] + R[p][r] - machines[m].scapacities[r]) : 0 ;
		old = (usage[m][r]  > machines[m].scapacities[r])  ? (usage[m][r] - machines[m].scapacities[r]) : 0 ;
		cost += (new - old)*weight_lc[r];*/
	}

	/* balance costs */
	for (b=0; b < no_balances; b++) {
		BALANCE bal = balances[b];
		old = bal.target*machines[m].capacities[bal.i]  - machines[m].capacities[bal.j] - bal.target*usage[m][bal.i] + usage[m][bal.j]  ;
		new = old - bal.target*R[p][bal.i] + R[p][bal.j] ;
		if (old < 0)
			old = 0;
		if (new < 0)
			new = 0;
		cost += (new - old)*bal.weight_bc;
	}

	if (m != OM[p]) {
		/* process move cost */
		cost += weight_pmc*pmc[p]  ;
		/* machine move cost */
		cost += weight_mmc*machines[OM[p]].mmc[m] ;

		/* service cost */
		/*if (services[S[p]].no_of_moved_processes >= maxno_processes_moved_in_service )
			cost += weight_smc;*/
	}
	return cost ;
}

long long int costOfAssigningProcessToMachine_ds(const usint p, const usint m){
	long long int cost  = movecost[pid_ds[p]][m], old, new;
	usint r,b;

	/* load costs*/
	for (r=0; r < no_resources; r++) {
		old = usage[m][r] - machines[m].scapacities[r];
		if (old > 0)
			cost += R[p][r]*weight_lc[r];
		else {
			new = old + R[p][r];
			if (new > 0)
				cost += new*weight_lc[r];
		}
	}

	/* balance costs */
	for (b=0; b < no_balances; b++) {
		BALANCE bal = balances[b];
		old = bal.target*machines[m].capacities[bal.i]  - machines[m].capacities[bal.j] - bal.target*usage[m][bal.i] + usage[m][bal.j]  ;
		new = old - bal.target*R[p][bal.i] + R[p][bal.j] ;
		if (old < 0)
			old = 0;
		if (new < 0)
			new = 0;
		cost += (new - old)*bal.weight_bc;
	}
	pmcost[pid_ds[p]][m] = cost;
	return cost ;
}

long long int costOfAssigningProcessToMachine(usint p, usint m){
	long long int cost  = 0, old, new;
	usint r,b;

	if (m != OM[p]) {
			/* process move cost */
			cost += weight_pmc*pmc[p]  ;
			/* machine move cost */
			cost += weight_mmc*machines[OM[p]].mmc[m] ;
	}
	movecost[pid_ds[p]][m] = cost;

	/* load costs*/
	for (r=0; r < no_resources; r++) {
		old = usage[m][r] - machines[m].scapacities[r];
		if (old > 0)
			cost += R[p][r]*weight_lc[r];
		else {
			new = old + R[p][r];
			if (new > 0)
				cost += new*weight_lc[r];
		}
	}

	/* balance costs */
	for (b=0; b < no_balances; b++) {
		BALANCE bal = balances[b];
		old = bal.target*machines[m].capacities[bal.i]  - machines[m].capacities[bal.j] - bal.target*usage[m][bal.i] + usage[m][bal.j]  ;
		new = old - bal.target*R[p][bal.i] + R[p][bal.j] ;
		if (old < 0)
			old = 0;
		if (new < 0)
			new = 0;
		cost += (new - old)*bal.weight_bc;
	}
	pmcost[pid_ds[p]][m] = cost;
	return cost ;
}



void updateCurrentCostOfProcess_down(const usint p, const usint m){
	usint k,pid = pid_ds[p],_m,supportFound=FALSE;
	long long int pcost = LONG_MAX;
	double cost = 1.0*best_objective_cost ;
	if (bestM[pid] == m){
		futureCost -= costOfProcess[pid];
		cost -= (objective_cost + futureCost) ;
		for(k=0; k < dsize[pid]; k++) {
			_m = values[pid][k];
			if (services[S[p]].used_machines[_m] == AVAILABLE && msrop[pid][_m] && (pmcost[pid][_m] + serviceCost(p,_m)) < cost) {
				if (pmcost[pid][_m] < pcost) {
					pcost = pmcost[pid][_m];
					bestM[pid] = _m;
					supportFound=TRUE;
				}
			}
		}
		if(supportFound)
			costOfProcess[pid] = pmcost[pid][bestM[pid]];
		else
			PROCESS_FAILED = TRUE;
		futureCost += costOfProcess[pid];
	}
}

void updateCurrentCostOfProcess_up(const usint p, const usint m){
	usint pid = pid_ds[p];
	if (pmcost[pid][m] < costOfProcess[pid]) {
		futureCost -= costOfProcess[pid];
		costOfProcess[pid] = pmcost[pid][m];
		futureCost += costOfProcess[pid];
		bestM[pid] = m;
	}
}



double currentCostOfProcess(const usint p){
	usint m,i;
	double tcost, pcost = LONG_MAX;
	int pid = pid_ds[p];
	for(i=0; i < dsize[pid]; i++) {
		m = values[pid][i];
		if(services[S[p]].used_machines[m] == AVAILABLE && msrop[pid][m] ) {
			tcost = costOfAssigningProcessToMachine_ws(p, m);
			if (pmcost[pid][m]!=tcost)
				printf("current %lld correct %10.0f\n",pmcost[pid][m],tcost);
			assert(pmcost[pid][m]==tcost);
			if (pcost > tcost) {
				pcost = tcost ;
			}
		}
	}
/*	if(costOfProcess[pid]!=pcost)
		printf("current %lld correct %10.0f\n",costOfProcess[pid],pcost);
	assert(costOfProcess[pid]==pcost);*/
	return pcost;
}



int currentBestMachine(const usint p){
	usint m,i;
	long long int tcost, pcost = LONG_MAX;
	int bm=-1;
	int pid = pid_ds[p];
	for(i=0; i < dsize[pid]; i++) {
		m = values[pid][i];
		if(services[S[p]].used_machines[m] == AVAILABLE && msrop[pid][m]) {
			tcost = costOfAssigningProcessToMachine_ws(p, m);
			if (pcost > tcost) {
				pcost = tcost ;
				bm =m;
			}
		}
	}
	return bm;
}



double forwardCost_test(void) {
	usint i, p,j,s;
	double fcost = 0;
	for (i=0; i < no_of_services_selected; i++) {
		s=active_services[i];
		for (j=0; j < services[s].no_of_unassigned_processes; j++) {
			p = services[s].unassigned_process[j];
			fcost += currentCostOfProcess(p);
		}
	}
	return fcost;
}


long long int currentCostOfProcess_before_search(const usint p){
	usint m,i;
	long long int tcost, pcost = LONG_MAX;
	int pid = pid_ds[p];
	for(i=0; i < dsize[pid]; i++) {
		m = values[pid][i];
	/*assert(services[S[p]].used_machines[m] == AVAILABLE && msrop[pid][m]) ;*/
		tcost  =  costOfAssigningProcessToMachine(p,m);
		if (pcost > tcost) {
			pcost = tcost;
			bestM[pid] = m;
		}
	}
	return pcost;
}


void update_costs_before_search( ){
	usint i, p,j,s,pid;
	futureCost = 0;
	for (i=0; i < no_of_services_selected; i++) {
		s=active_services[i];
		for (j=0; j < services[s].no_of_unassigned_processes; j++) {
			p = services[s].unassigned_process[j];
			pid = pid_ds[p];
			costOfProcess[pid] = currentCostOfProcess_before_search(p);
			futureCost += 1.0*costOfProcess[pid];
		}
	}
}



int _costBasedFilteringOfDomains(const usint p){
	usint  tmachine, pid=pid_ds[p];
	int index;
	usint *mvalues = values[pid];
	double 	cost = 1.0*best_objective_cost - 1.0*objective_cost - futureCost   + 1.0*costOfProcess[pid] ;
	for( index=dsize[pid]-1; index >= 0; --index) {
		if (1.0*pmcost[pid][mvalues[index]] + 1.0*serviceCost(p,mvalues[index]) >= cost) {
			msrop[pid][mvalues[index]] = FALSE;
			tmachine = mvalues[dsize[pid]-1];
			mvalues[dsize[pid]-1] = mvalues[index];
			mvalues[index] = tmachine;
			dsize[pid]--;
		}
	}
	/*if (dsize[pid] > 0)
		assert(costOfProcess[pid]==currentCostOfProcess(p));*/
	no_available_machines[pid] = dsize[pid];
	return (dsize[pid] == 0);
}

int costBasedFiltering(void){
	usint i, s, j;
	if (objective_cost + futureCost >= best_objective_cost) {
		return FALSE;
	}
/*	if (objective_cost + lcBound() + bcBound() >= best_objective_cost)
		return FALSE;*/
	for (i=0; i < no_of_services_selected; i++) {
		s=active_services[i];
		for (j=0; j < services[s].no_of_unassigned_processes; j++)
			if(_costBasedFilteringOfDomains(services[s].unassigned_process[j]))
				return FALSE;
	}
	return TRUE;
}








double computeInitialLowerBound(void){
	int r,b,p,m;
	double lcost=0, bcost=0, costr=0, costb=0;
	for (r=0; r <no_resources; r++){
		costr=0;
		for(p=0; p < no_processes; p++)
			costr+=1.0*R[p][r];
		for(m=0; m < no_machines; m++)
			costr-=1.0*machines[m].scapacities[r] ;
		if (costr > 0)
			lcost+=costr*weight_lc[r];
	}

	printf("The initial  load cost is %15.2f\n",lcost);
	for (b=0;b<no_balances;b++){
		costb=0;
		for(p=0; p < no_processes; p++)
			costb = costb - balances[b].target*R[p][balances[b].i] + R[p][balances[b].j] ;
		for(m=0; m < no_machines; m++)
			costb += 1.0*balances[b].target*machines[m].capacities[balances[b].i] - machines[m].capacities[balances[b].j] ;
		if (costb > 0)
			bcost += costb*balances[b].weight_bc;
	}
	printf("The initial balance cost is %15.2f\n",bcost);
	printf("The initial  lower bound is %15.2f\n",lcost+bcost);
	return lcost+bcost;
}
