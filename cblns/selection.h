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



#ifndef _SELECTION_H
#define _SELECTION_H 1
#define MH_COMPOSITE 0
#define MH_RANDOM 1
#define MH_COST 2
#define MH_NFAILURES 3

char mh_id;


double service_heuristic_weight_composite(usint const i);
double service_heuristic_weight_current_cost(const usint i);
double service_heuristic_weight_total_requirement_cost(const usint i);
double service_heuristic_weight_no_machines(const usint i);
double service_heuristic_weight_no_failures(usint const i);

double process_heuristic_weight_no_machines(usint const p) ;
double process_heuristic_weight_requirement_cost(usint const p);
double process_heuristic_weight_no_failures(usint const p);
double process_heuristic_weight_current_cost(usint const p);
double process_heuristic_weight_composite(usint const p);


double machine_heuristic_weight_current_machine_cost(usint const p, usint const m);
double machine_heuristic_weight_cost(usint const p, usint const m);
double machine_heuristic_weight_random(usint const p, usint const m);

usint select_and_remove_service_and_then_process( void );
void select_and_remove_service( void ) ;
usint select_and_remove_process_from_the_current_service(void);
void restore_process_and_service(const usint p);
usint select_and_remove_machine(usint const p);
#endif /* selection.h */
