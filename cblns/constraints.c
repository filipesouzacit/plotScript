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
#include<stdlib.h>
#include "constraints.h"

long long int *_tusage, *_usage;
unsigned int *capacities, *_R;
char machineSatisfyRequirementOfProcess( const usint m, const usint p ) {
	usint r;
	_tusage = tusage[m], _usage=usage[m];
	capacities=machines[m].capacities, _R=R[p];

	if (m!= OM[p]){
		for (r=0;r<no_resources;r++) {
			if(transient[r]) {
				if(_tusage[r] + _R[r] > capacities[r])
					return FALSE;
			} else {
				if(_usage[r] + _R[r] > capacities[r])
					return FALSE;
			}
		}
	} else {
		for (r=0;r<no_resources;r++) {
			if(!transient[r]) {
				if(_usage[r] + _R[r] > capacities[r])
					return FALSE;
			}
		}
	}

/*	for (r=0;r<no_resources;r++) {
		if(transient[r]) {
			if (m != OM[p]) {
				if(tusage[m][r] + R[p][r] > machines[m].capacities[r])
					return FALSE;
			}
		} else {
			if(usage[m][r] + R[p][r] > machines[m].capacities[r])
				return FALSE;
		}
	}*/
	return TRUE;
}

