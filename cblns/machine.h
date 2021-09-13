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



#ifndef _MACHINE_H
#define _MACHINE_H 1
#include <assert.h>

char PROCESS_FAILED;
void initialize_usage_non_incremental(void);
void assign_original_machine_to_process(const usint m, const usint p);
void assign_machine_to_process(const usint m, const usint p);
void assign_machine_to_process_after_search(const usint m, const usint p);
void unassign_machine_to_process(const usint m, const usint p);
void unassign_machine_to_process_before_search(const usint m, const usint p);

void remove_machines_from_services_before_search(void );

void add_machines_to_services_after_search(void );
void add_machine_to_services_during_search(const usint m);


void remove_machine_from_the_processes_of_current_service(const usint m, const usint p);
void remove_machine_from_the_processes_of_noncurrent_service(const usint s, const usint m, const usint p, usint *counter, usint *mchanges);
void remove_machine_from_the_processes_of_noncurrent_services(const usint m, const usint p);
void remove_machines_from_the_domain_of_process(usint p);
void add_machine_to_the_processes_of_current_service(usint m, usint p) ;
void add_machine_to_the_processes_of_noncurrent_service(usint s, usint m, usint p) ;
void add_machine_to_the_processes_of_noncurrent_services(const usint m, const usint p) ;

static __inline__
void add_location_to_service(SERVICE *service, const usint l) {
	if (service->used_locations[l] == 0)
		service->no_of_used_locations++;
	service->used_locations[l]++;
}

static __inline__
void remove_process_from_machine(const usint m, const usint p) {
	usint i;
	MACHINE *machine = &machines[m];
	for (i=0; i < machine->n; i++)
		if (machine->processes[i] == p) {
			machine->processes[i]=machine->processes[machine->n-1];
			machine->n--;
			break;
		}
}

static __inline__
void add_process_to_machine(const usint m, const usint p) {
	MACHINE *machine = &machines[m];
	if (machine->n == machine->size) {
		machine->processes = (usint *)realloc(machine->processes, (machine->n+1)*sizeof(usint));		
		machine->size = machine->n+1;
	}
	machine->processes[machine->n]=p;
	machine->n++;
}


static __inline__
void remove_location_from_service(SERVICE *service, const usint l) {
	/*service->used_locations[l]--;*/
	if (--service->used_locations[l] == 0)
		service->no_of_used_locations--;
}


static __inline__
void add_machine_to_service(SERVICE *service, const usint m) {
#if assertion == 1
	assert(service->used_machines[m] == AVAILABLE);
#endif
	service->used_machines[m] = USED;
	service->no_of_used_machines++;
	service->no_of_available_machines--;
}

static __inline__
void remove_machine_from_service(SERVICE *service, const usint m) {
#if assertion == 1
	assert(service->used_machines[m] == USED);
#endif
	service->used_machines[m] = AVAILABLE;
	service->no_of_used_machines--;
	service->no_of_available_machines++;
}

static __inline__
void add_neighborhood_to_service(SERVICE *s, const usint n){
	SERVICE *sr;
	usint d;
	if (s->used_neighborhoods[n] == 0) {
		if (s->mandatory_neighborhoods[n]) {
			/* assert(s->no_of_mandatory_neighborhoods != 0 );*/
			s->no_of_mandatory_neighborhoods--;
		}
		for (d = 0; d < s->no_dependson; d++) {
			sr = &(services[s->dependson[d]]) ;
			if (sr->mandatory_neighborhoods[n]++ == 0 && sr->used_neighborhoods[n] == 0)
				sr->no_of_mandatory_neighborhoods++;
			/*sr->mandatory_neighborhoods[n]++;*/
		}
	}
	s->used_neighborhoods[n]++;
}

static __inline__
void remove_neighborhood_from_service(SERVICE *s, usint n){
	SERVICE *sr;
	usint d;
	/*s->used_neighborhoods[n]--;*/
	if (--s->used_neighborhoods[n] == 0) {
		if (s->mandatory_neighborhoods[n])
			s->no_of_mandatory_neighborhoods++;
		for (d = 0; d < s->no_dependson; d++) {
			sr = &(services[s->dependson[d]]) ;
			/*sr->mandatory_neighborhoods[n]--;*/
			if (--sr->mandatory_neighborhoods[n] == 0 && sr->used_neighborhoods[n] == 0)
				sr->no_of_mandatory_neighborhoods--;
		}
	}
}

static __inline__
void increment_usage_of_original_machine(const usint m, const usint p){
	usint r;
	for (r=0; r<no_resources; r++) {
		usage[m][r] += R[p][r] ;
		if (transient[r])
			tusage[m][r] += R[p][r] ;
	}
}


static __inline__
void increment_usage_of_machine(const usint m, const usint p){
	usint r;
	for (r=0; r<no_resources; r++) {
		usage[m][r] += R[p][r] ;
		if (transient[r]  && m!= OM[p])
			tusage[m][r] += R[p][r] ;
	}
}

static __inline__
void decrement_usage_of_machine(const usint m, const usint p){
	usint r;
	for (r=0; r<no_resources; r++) {
#if assertion == 1
		assert(usage[m][r] >= R[p][r]);
#endif
		usage[m][r] -= R[p][r] ;
		if (transient[r]  && m!= OM[p])
			tusage[m][r] -= R[p][r] ;
	}
}


#endif /* machine.h */
