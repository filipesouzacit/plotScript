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
#include "definitions.h"
#include "machine.h"
#include "constraints.h"
#include "search.h"
#include "utils.h"
#include "cost.h"

#define PRINT_PROBLEM_PARAMETERS 0
void readln( FILE *input ) {
	char dummy;
	while (fscanf( input, "%c", &dummy ) != EOF && dummy != '\n');
}
static __inline__
void read_resources(FILE *input){
	int i, ntransient=0;
	unsigned int temp;
	fscanf( input, "%u", &temp) ;      /* number of resources */
	no_resources = temp;
#if PRINT_PROBLEM_PARAMETERS > 0
	printf("number of resources: %u\n", no_resources);
#endif
	assert(no_resources > 0);
	transient = (unsigned int*)CALLOC( no_resources,sizeof(unsigned int)) ;
	weight_lc = (unsigned int*)CALLOC( no_resources, sizeof(unsigned int)) ;
	demand = (unsigned long long int*)CALLOC( no_resources, sizeof(unsigned long long int));
	supply = (unsigned long long int*)CALLOC( no_resources, sizeof(unsigned long long int));
	for(i=0; i < no_resources; i++){
		fscanf(input, "%u", &temp);
		transient[i] = temp;
		if (temp==1)
			ntransient++;
		/*	printf("transient: %d\n",t);*/
		fscanf(input, "%u", &(weight_lc[i]));
			/*printf("weight_loadcost %u\n",weight_lc[i]);*/
		readln(input);
	}
#if PRINT_PROBLEM_PARAMETERS > 0
	printf("number of transient resources: %u\n", ntransient);
#endif

}

static __inline__
void read_machines(FILE *input) {
	int i,r,j;
	unsigned  int sint, mmcmax = 0 ;
	MACHINE *machine ;
	fscanf( input, "%u", &sint) ;      /* number of resources */
	no_machines = sint;
#if PRINT_PROBLEM_PARAMETERS > 0
	printf("number of machines: %u\n", no_machines);
#endif

	readln(input);
	machines = (MACHINE_PTR)CALLOC(no_machines, sizeof(MACHINE));

	machine_cost = (long long int *)CALLOC(no_machines,sizeof( long long int));
	for (i=0; i< no_machines; i++){
		machine = &(machines[i]);
		fscanf(input, "%u", &(sint));
		machine->neighborhood = sint;
		if (sint > no_neighborhoods)
			no_neighborhoods = sint;
		/*   printf("neighborhood : %u\n", machines[i].neighborhood);*/

		fscanf(input, "%u", &(sint) );
		machines[i].location = sint;
		if (sint > no_locations)
			no_locations = sint;
		/*   printf("location : %u\n", machines[i].location);*/
		machine->n = 0;
		machine->size =0;
		machine->capacities = (unsigned int*)CALLOC( no_resources, sizeof(unsigned int ) );
		for (r=0; r < no_resources; r++) {
			fscanf(input, "%u", &(machine->capacities[r]));
		}

		machine->scapacities = (unsigned int*)CALLOC( no_resources, sizeof(unsigned int ) );

		for (r=0; r < no_resources; r++) {
			fscanf(input, "%u", &(machine->scapacities[r]));
			supply[r] += machine->scapacities[r];
		}
		machine->mmc = (unsigned int*)CALLOC( no_machines, sizeof(unsigned int ) );
		for (j=0; j < no_machines; j++) {
			fscanf(input, "%u", &(machine->mmc[j]));
			if (mmcmax < machine->mmc[j])
				mmcmax = machine->mmc[j];
		}
		readln(input);
	}
	no_locations++;
	no_neighborhoods++;
#if PRINT_PROBLEM_PARAMETERS > 0
	printf("number of locations: %u\n",no_locations);
	printf("number of neighborhoods: %u\n",no_neighborhoods);
	printf("mmcmax %d\n",mmcmax);
#endif

}
static __inline__
void read_services(FILE *input) {
	int i,ndependencies=0;
	SERVICE *service ;
	unsigned int s, temp;
	fscanf( input, "%u", &s) ;      /* number of resources */
	no_services = s;
#if PRINT_PROBLEM_PARAMETERS > 0
	printf("number of services: %u\n",no_services);
#endif
	readln(input);

	if((services = (SERVICE_PTR)CALLOC(no_services,sizeof(SERVICE)) ) == NULL) {
		fprintf(stdout, "could not allocate memory for services\n");
		abort();
	}


	for (s = 0; s < no_services; s++) {
		service = &(services[s]);
		service->no_of_used_locations = 0;
		service->no_of_processes = 0;
		service->no_of_moved_processes = 0;
		service->no_of_unassigned_processes = 0;
		service->no_of_used_machines = 0;
		service->no_of_mandatory_neighborhoods = 0;
		service->no_of_available_machines = no_machines;

		if((service->used_machines = (char*)CALLOC(no_machines,sizeof(char )) ) == NULL) {
			fprintf(stdout,"could not allocate memory for used_machines\n");
			abort();
		}

		if((service->used_locations = (usint*)CALLOC(no_locations,sizeof(usint))) == NULL) {
			fprintf(stdout,"could not allocate memory for used_locations\n");
			abort();
		}

		if((service->used_neighborhoods= (usint*)CALLOC(no_neighborhoods,sizeof(usint)))==NULL){
			fprintf(stdout,"could not allocate memory for used_neighborhoods\n");
			abort();
		}
		if((service->mandatory_neighborhoods= (usint*)CALLOC(no_neighborhoods,sizeof(usint)))==NULL){
			fprintf(stdout,"could not allocate memory for mandatory_neighborhoods\n");
			abort();
		}
		fscanf(input, "%u", &temp);
		service->spreadMin = temp;
		/*   printf("spreadMin : %d\n", services[s].spreadMin); */

		fscanf(input, "%u", &(temp));
		service->no_dependson= temp;
		if ((service->dependson = (usint*)CALLOC(service->no_dependson,sizeof(usint )) ) == NULL) {
			fprintf(stdout,"could not allocate memory for service dependencies\n");
			abort();
		}
		/*if ((service->rdependson = (usint*)CALLOC(no_services,sizeof(usint )) ) == NULL)
			fprintf(stdout,"could not allocate memory for reverse service dependencies\n");
		 */
		for (i=0; i < service->no_dependson ;i++) {
			fscanf(input, "%u", &temp);
			service->dependson[i] = temp;
			ndependencies++;
		}
		readln(input);
	}
#if PRINT_PROBLEM_PARAMETERS > 0
	printf("number of dependencies: %d\n",ndependencies);
#endif

}

static __inline__
void read_processes(FILE *input) {
	int p,r,i;
	unsigned int temp;
	fscanf( input, "%u", &temp) ;      /* number of processes */
	no_processes = temp;
#if PRINT_PROBLEM_PARAMETERS > 0
	printf("number of processes: %u\n",no_processes);
#endif
	readln(input);

/*	if((process_failures = (unsigned  int*)CALLOC(no_processes,sizeof(unsigned  int)) ) == NULL) {
		fprintf(stdout, "could not allocate memory for process failures\n");
		abort();
	}
*/
	if((no_available_machines = (usint*)CALLOC(SEARCH_NPROCESSES,sizeof(usint)) ) == NULL) {
		fprintf(stdout, "could not allocate memory for no_machines_available\n");
		abort();
	}


	if((dsize = (usint*)CALLOC(SEARCH_NPROCESSES, sizeof(usint)) ) == NULL) {
		fprintf(stdout, "could not allocate memory for dsize\n");
		abort();
	} else {
		for(i=0; i < SEARCH_NPROCESSES; i++)
			dsize[i]=0;
	}

	if ((values = (usint**)CALLOC(SEARCH_NPROCESSES,sizeof(usint*)) ) == NULL ) {
		fprintf(stdout, "could not allocate memory for values pointer\n");
		abort();
	}
	for (temp=0; temp < SEARCH_NPROCESSES; temp++) {
		if ((values[temp] = (usint*)CALLOC(no_machines,sizeof(usint)))==NULL){
			fprintf(stdout, "could not allocate memory for values \n");
			abort();
		}
	}


	if((S = (usint*)CALLOC(no_processes , sizeof(usint )))==NULL)
		fprintf(stderr,"could not allocate memory for S\n");
	if ((process_requirement_cost =   (unsigned long long*)CALLOC(no_processes, sizeof(unsigned long long )))==NULL)
		fprintf(stderr,"could not allocate memory for process_requirement_cost\n");
	if((pmc = (unsigned int*)CALLOC(no_processes , sizeof(unsigned int )))==NULL)
		fprintf(stderr,"could not allocate memory for pmc\n");
	if ((R = (unsigned int**)CALLOC(no_processes , sizeof(unsigned int *)) )==NULL)
		fprintf(stderr,"could not allocate memory for R\n");
	for (p=0; p < no_processes; p++) {
		fscanf(input, "%u", &temp);
		S[p] = temp;
		services[temp].no_of_processes++;
		services[temp].no_of_unassigned_processes++;
		/*	printf("service of process %d is %u\n",p, S[p]); */
		if ((R[p] = (unsigned int*)CALLOC(no_resources , sizeof(unsigned int )) )==NULL)
			fprintf(stderr,"could not allocate memory for R for process %d\n",p);
		for (r=0; r < no_resources;r++) {
			fscanf(input, "%u", &(R[p][r]));
			process_requirement_cost[p] += R[p][r]*weight_lc[r];
			/*	  printf("resource %d required by process %d is %u\n",r, p, R[p][r]); */
		}
		fscanf(input, "%u", &(pmc[p]));
		/*	printf("process move cost of %d is %u\n",p, pmc[p]); */
		readln(input);
	}


}

static __inline__
void read_balance_objectives(FILE *input) {
	int b;
	BALANCE *balance ;
	unsigned int temp;
	fscanf( input, "%u", &temp) ;      /* number of processes */
	no_balances = temp;
#if PRINT_PROBLEM_PARAMETERS > 0
	printf("number of balance objectives: %u\n", no_balances);
#endif
	readln(input);
	balances = (BALANCE_PTR)CALLOC(no_balances, sizeof(BALANCE));
	for (b = 0; b < no_balances; b++) {
		balance = &(balances[b]);
		fscanf(input, "%u", &(balance->i));
		fscanf(input, "%u", &(balance->j));
		fscanf(input, "%u", &(balance->target));
		readln(input);
		/*printf ("balance objective %d is (%u,%u,%u)\n",b,balances[b].i,balances[b].j,balances[b].target);*/
		fscanf( input, "%u", &(balance->weight_bc)) ;      /* number of processes */
		  /* printf("weight for balance cost: %u\n", balances[b].weight_bc);*/
		readln(input);
	}


}
static __inline__
void read_weights(FILE *input) {
	fscanf( input, "%u", &weight_pmc) ;      /* number of processes */
/*	 printf("weight pmc: %u\n", weight_pmc);*/

	fscanf( input, "%u", &weight_smc) ;      /* number of processes */
/*	 printf("weight smc: %u\n", weight_smc);*/

	fscanf( input, "%u", &weight_mmc) ;      /* number of processes */
	 /*printf("weight mmc:: %u\n", weight_mmc);*/

}

void readProblem(char *filename) {
	FILE *input = fopen( filename, "r" );
	if (input == NULL) {
		printf("%s\n",filename);
		fprintf( stderr, "Fatal error. Could not open %s", filename );
		fprintf( stderr, "Bailing out1.\n" );
		fflush( NULL );
		exit( 1 );
	}
	read_resources(input);
	read_machines(input);
	read_services(input);
	read_processes(input);
	read_balance_objectives(input);
	read_weights(input);
	fclose(input);
}


void readOriginalSolution(char *filename){
	FILE *input;
	usint m,p, s;
	char *instancename;
	unsigned int sint;
	SERVICE_PTR service ;
	MACHINE *machine;
	maxno_processes_moved_in_service = 0;
	instancename = extractFilename(filename);
	default_sol_filename = (char *)CALLOC((strlen(instancename)+3),sizeof(char));
	strcpy(default_sol_filename,"re");
	strcat(default_sol_filename,instancename);

	if ((input = fopen( filename, "r" )) == NULL) {
		fprintf( stderr, "Fatal error. Could not open %s.\n", filename );
		fprintf( stderr, "Bailing out.\n" );
	}

	usage = (long long**)CALLOC(no_machines, sizeof(long long*) );

	tusage = (long long**)CALLOC(no_machines, sizeof(long long*) );

	for (m=0; m < no_machines ; m++) {

		usage[m] = (long long*)CALLOC(no_resources, sizeof(long long));
		/* in future try to avoid creating memory for tusage for non-transitive resources*/
		tusage[m] = (long long*)CALLOC(no_resources, sizeof(long long));
	}

	active_services = (unsigned short int*)CALLOC(no_services, sizeof(unsigned short int));
	no_of_active_services = 0;
	for (s = 0; s < no_services; s++) {
		service = &(services[s]);
		/*size = service->no_of_processes > no_neighborhoods ? service->no_of_processes: no_neighborhoods;
	service->cneighborhoods =   (usint*)CALLOC(size, sizeof(usint));      */
		service->unassigned_process = (usint*)CALLOC(service->no_of_processes, sizeof(usint));
	}

	OM = (unsigned short int*)CALLOC(no_processes, sizeof(unsigned short int));
	NM = (unsigned short int*)CALLOC(no_processes, sizeof(unsigned short int));
	BM = (unsigned short int*)CALLOC(no_processes, sizeof(unsigned short int));
	initObjectiveCost();
	/*printf("initial objective cost: %llu\n",objective_cost);*/
	for (p=0; p < no_processes; p++) {
		fscanf( input, "%u", &(sint)) ;
		assign_original_machine_to_process(sint, p);
		BM[p]= NM[p] = OM[p];
		machine = &machines[sint];
		machine->size++;

	}
	best_objective_cost = objective_cost ;
	fclose(input);

}

void readNewSolution(char *filename){
	FILE *input;
	usint p;

	unsigned int sint;
	if ((input = fopen( filename, "r" )) == NULL) {
		fprintf( stderr, "Fatal error. Could not open %s.\n", filename );
		fprintf( stderr, "Bailing out.\n" );
	}

	for (p=0; p < no_processes; p++) {
		fscanf( input, "%u", &(sint)) ;
		if (NM[p] != sint) {
			unassign_machine_to_process(NM[p],p);
			/*        service = &(services[S[p]]);
        machine = machines[NM[p]];
        service->no_of_unassigned_processes++;
	remove_machine_from_service(service, sint);
        remove_location_from_service(service, machine.location);
        remove_neighborhood_from_service(service, machine.neighborhood);
			 */

		}
	}
	fclose(input);
	printf("after removing objective cost:         %llu\n", objective_cost);
	checkSolution();

	if ((input = fopen( filename, "r" )) == NULL) {
		fprintf( stderr, "Fatal error. Could not open %s.\n", filename );
		fprintf( stderr, "Bailing out.\n" );
	}

	for (p=0; p < no_processes; p++) {
		fscanf( input, "%u", &(sint)) ;
		if (OM[p] != sint) {
			assign_machine_to_process(sint,p);
			/*service = &(services[S[p]]);
        machine = machines[sint];
        service->no_of_unassigned_processes--;
        add_neighborhood_to_service(service, machine.neighborhood);
        add_location_to_service(service, machine.location);
        add_machine_to_service(service, sint);*/
		}
	}
	fclose(input);


}
