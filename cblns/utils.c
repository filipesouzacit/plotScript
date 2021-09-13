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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "constraints.h"
#include "definitions.h"

/* in future use different ordering on resources (based on learnt during search or based on the weights of the resources)*/
/* all resource requirements can be aggregated by multiplying them with their weights */
int cmp_unassigned_processes_based_on_resource_requirements( const void *a, const void *b) {
	 usint r;
         const usint *pa = (const usint *)a ;
         const usint *pb = (const usint *)b ;
         for (r=0; r < no_resources; r++) {
		if (R[*pa][r] != R[*pb][r])
			return	R[*pa][r] - R[*pb][r] ;
	 }
	return (*pa - *pb) ;
}

/*
int cmp_machines_based_on_resource_availabilities(const usint ma, const usint mb) {
	 usint r;
         unsigned int ca, cb; 
         for (r=0; r < no_resources; r++) {
		if(transient[r]) {
	            ca = machines[ma].capacities[r] - tusage[ma][r] ;
		    cb = machines[mb].capacities[r] - tusage[mb][r] ;
		    if ( ca != cb)
			return	ca - cb ;
                } else {
	            ca = machines[ma].capacities[r] - usage[ma][r] ;
		    cb = machines[mb].capacities[r] - usage[mb][r] ;
		    if ( ca != cb)
			return	ca - cb ;
                }   
	 }
	return (ma - mb) ;
}
*/

int cmp_machines_based_on_resource_availabilities(const usint ma, const usint mb) {

         unsigned int ca, cb; 
	 usint r = current_resource;
		
		if(transient[r]) {
	            ca = machines[ma].capacities[r] - tusage[ma][r] ;
		    cb = machines[mb].capacities[r] - tusage[mb][r] ;
		    if ( ca != cb)
			return	(ca - cb) ;
                } else {
	            ca = machines[ma].capacities[r] - usage[ma][r] ;
		    cb = machines[mb].capacities[r] - usage[mb][r] ;
		    if ( ca != cb)
			return	(ca - cb) ;
                }   
	return (ma - mb) ;
}

char is_process_requirement_complaint_with_the_safety_capacity(const usint m, const usint p) {
	usint r;
	for (r = 0 ; r < no_resources ; r++) {
		if (machines[m].scapacities[r] <  usage[m][r]+R[p][r])
			return FALSE;
	}
	return TRUE;	
}

void print_resource_availablities_of_machines(usint m) {
	usint r;
	for (r = 0 ; r < no_resources ; r++) {
		if (transient[r])
			printf("%10llu ",(machines[m].capacities[r] - tusage[m][r])   );
		else
			printf("%10llu ",(machines[m].capacities[r] - usage[m][r]) );
	}
	printf("\n");
}	

char checkSolution(void){

 usint m,r, sa;
 for (m=0;m<no_machines;m++) {
   for (r=0;r<no_resources;r++) {
     if(capacityConstraint(m,r)<1)     {
       fprintf(stderr,"capacity constraint is violated for machine %d and resource %d\n",m,r);
       fprintf(stderr,"bailing out\n");
       exit(1);
     }
    if(transientUsageConstraint(m,r)<1)     {
       fprintf(stderr,"transient usage constraint is violated for machine %d and resource %d\n",m,r);
       fprintf(stderr,"bailing out\n");
       exit(1);
     }
    
 }
}
for (sa=0; sa<no_services;sa++) {
    if(conflictConstraint(sa)==FALSE ) {
       fprintf(stderr,"conflict constraint is violated for service %u\n",sa);
       fprintf(stderr,"bailing out\n");
       exit(1);
     }

  if(spreadConstraint(sa)==FALSE ) {
       fprintf(stderr,"spread constraint is violated for service %u\n",sa);
       fprintf(stderr,"bailing out\n");
       exit(1);
     }
  if(serviceDependencyConstraints(sa)==FALSE ) {
       fprintf(stderr,"dependency constraint is violated for service %u\n",sa);
       fprintf(stderr,"bailing out\n");
       exit(1);
     }
  
 /*printf("no of processes in the service %u is %u\n",s,services[s].no_of_processes);*/
/*  for (d=0; d < services[sa].no_dependson ; d++) {
      sb = services[sa].dependson[d];
      
      if(dependencyConstraint(sa, sb)==FALSE) {
       fprintf(stderr,"dependency constraint is violated for service %u\n",sa);
       fprintf(stderr,"bailing out\n");
       exit(1);
     }
  }*/

}

return TRUE;
}


void print_solution_in_file(char* name){
  usint p;
  FILE *file = fopen(name, "w");
  if (file== NULL) 
  	printf("Error in opening a file..");
  for (p=0; p < no_processes; p++) 
 	 fprintf(file,"%u\t",BM[p]);
  fclose(file);
}

char* extractFilename(char *path) {
     
     char *p,*filename=NULL;
     filename = p = path;
     while(( p = strchr( p, '/' )) != NULL )
     {
       ++p ;
       filename = p;	
     }
     return filename;
}
