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



#ifndef _UTILS_H
#define _UTILS_H 1
#include "definitions.h"

usint current_resource ;
void print_resource_availablities_of_machines(usint m) ;
void print_solution_in_file(char* name);
char checkSolution(void);
int cmp_unassigned_processes_based_on_resource_requirements( const void *a, const void *b) ;
int cmp_machines_based_on_resource_availabilities(  usint a,  usint b) ;
char* extractFilename(char *path) ;
char is_process_requirement_complaint_with_the_safety_capacity(const usint m, const usint p) ;

static __inline__
unsigned int min(unsigned int a, unsigned int b){
	return a < b? a : b ;
}

static __inline__
unsigned int max(unsigned int a, unsigned int b){
	return a > b? a : b ;
}

#endif /* utils.h */
