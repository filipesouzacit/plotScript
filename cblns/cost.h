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



#ifndef _COST_H
#define _COST_H 1

#include "definitions.h"
double futureCost ;
/*double forwardCost_test(void) ;*/
double currentCostOfProcess(const usint p);
/*int costBasedFiltering(void);*/
void update_costs_before_search(void);
void updateCurrentCostOfProcess_down(const usint p, const usint m);
void updateCurrentCostOfProcess_up(const usint p, const usint m);
/*void updateCurrentCostOfProcess_down_search(const usint m);*/
unsigned long long int totalMachineCost(void) ;
unsigned long long int machineLoadCost(int m) ;
unsigned long long int machineCost(int);
unsigned long long int totalLoadCost(void);
unsigned long long int totalBalanceCost(void); 
unsigned long long int resourceLoadCost(int r); 
unsigned long long int balanceBalanceCost(int b);
long long int forwardCost(void);
long long int costOfAssigningProcessToMachine(usint p, usint m);
long long int costOfAssigningProcessToMachine_ds(const usint p, const usint m);
double costOfAssigningProcessToMachine_ar(usint fp, usint p, usint m);
double costOfAssigningProcessToMachine_ws(usint p, usint m);
double computeInitialLowerBound(void);

void initObjectiveCost(void);


static __inline__
unsigned int serviceCost(int p, int m){
	if (m != OM[p] && services[S[p]].no_of_moved_processes >= maxno_processes_moved_in_service ) {
		return weight_smc;
	}
	return 0;
}


static __inline__
long long int loadCost(int m, int r){
   return (usage[m][r] > machines[m].scapacities[r])  ? (usage[m][r] - machines[m].scapacities[r]) : 0 ;
}


static __inline__
long long int A(int m, int r) {
return (machines[m].capacities[r]  - usage[m][r]) ;
}

static __inline__
long long int balanceCost(int m, int b) {
   BALANCE bal = balances[b];
   long long int bcost = bal.target*A(m,bal.i)-A(m,bal.j);
   return bcost > 0 ? bcost : 0;
   /*return (bal.target*A(m,bal.i) > A(m,bal.j)) ? (bal.target*A(m,bal.i) - A(m,bal.j)) : 0 ;*/
}

static __inline__
unsigned  int processMoveCost(int p) {
   return (OM[p] == NM[p] ) ? 0: pmc[p] ;
}


static __inline__
unsigned  int machineMoveCost(int p) {
  return (OM[p] == NM[p] ) ? 0: machines[OM[p]].mmc[NM[p]] ;
}

static __inline__
usint serviceMoveCost(void ) {
    return maxno_processes_moved_in_service;
}

static __inline__
double lcBound(void) {
	double cost=0;
	usint r;
	for (r = 0; r < no_resources; r++)
		cost += demand[r] > supply[r] ? (demand[r]-supply[r])*1.0*weight_lc[r]: 0;
	return cost;
}

static __inline__
double bcBound(void){
	double cost  = 0;
	usint b;
	for (b=0; b < no_balances; b++) {
		BALANCE bal = balances[b];
		cost += (bal.target*(-1.0)*demand[bal.i] + 1.0*demand[bal.j])*bal.weight_bc;
	}

	return cost;
}


#endif /* cost.h */
