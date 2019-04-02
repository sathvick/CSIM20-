/* simulate an M/M/1 queue
  (an open queue with exponential service times and interarrival intervals)
*/

#include "csim.h"
#include <stdio.h>

#define	SVTM	1.0	/*mean of service time distribution */
#define IATM	2.0	/*mean of inter-arrival time distribution */
#define NARS	5	/*number of arrivals to be simulated*/

FACILITY f;		/*pointer for facility */
EVENT done;		/*pointer for counter */
int cnt;		/*number of active tasks*/
FILE *fp;

void cust();

void sim()				/*1st process - named sim */
{
	int i;

	fp = fopen("csim.out", "w");
	set_output_file(fp);
	set_trace_file(fp);
	set_error_file(fp);
	trace_on();
	create("sim");				/*required create statement*/

	f = facility("facility");		/*initialize facility*/
	done = event("done");			/*initialize event*/

	cnt = NARS;				/*initialize cnt*/
	for(i = 1; i <= NARS; i++) {
		hold(expntl(IATM));		/*hold interarrival*/
		cust();				/*initiate process cust*/
		}
	wait(done);				/*wait until all done*/
	report();				/*print report*/
}

void cust()				/*process customer*/
{
	create("cust");				/*required create statement*/

	reserve(f);				/*reserve facility f*/
		hold(expntl(SVTM));		/*hold service time*/
	release(f);				/*release facility f*/
	cnt--;					/*decrement cnt*/
	if(cnt == 0)
		set(done);			/*if last arrival, signal*/
	terminate();
}
