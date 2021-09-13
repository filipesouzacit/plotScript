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



#ifndef _CONSTRAINTS_H
#define _CONSTRAINTS_H 1

#include "definitions.h"

char machineSatisfyRequirementOfProcess(const usint m, const usint p ) ;

/* assuming that usage is always  up to date */
static __inline__
char capacityConstraint(usint m, usint r ){
 return (usage[m][r] <= machines[m].capacities[r]) ;
}

static __inline__
char transientUsageConstraint(usint m, usint r ){
 return (tusage[m][r] <= machines[m].capacities[r]) ;
}


/* assuming that the  machines assigned to the processes of the service are up to-date */
static __inline__
char conflictConstraint(const usint sind){
return (services[sind].no_of_available_machines >= services[sind].no_of_unassigned_processes);
/*&& (services[sind].no_of_unassigned_processes + services[sind].no_of_used_machines == services[sind].no_of_processes) ;*/
}


/* assuming that the locations used by  the processes of the service are up to-date */
static __inline__
char spreadConstraint(const usint sind){
 /* assert(services[sind].no_of_unassigned_processes == 0) ;*/
  return (services[sind].no_of_used_locations + services[sind].no_of_unassigned_processes >=  services[sind].spreadMin) ;
}


static __inline__
char serviceDependencyConstraints(const usint sind){
	SERVICE sr,s=services[sind];
	usint d;
	if(s.no_of_mandatory_neighborhoods > s.no_of_unassigned_processes)
		return FALSE;
	for (d = 0; d < s.no_dependson; d++) {
		sr = services[s.dependson[d]] ;
		if(sr.no_of_mandatory_neighborhoods > sr.no_of_unassigned_processes)
			return FALSE;
	}
	return TRUE;
}





#endif /* constraints.h */
