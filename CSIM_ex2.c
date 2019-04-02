/* simulate an M/M/1 queue
  (an open queue with exponential service times and interarrival intervals)
*/

#include "csim.h"
#include <stdio.h>

#define	SVTM	1.0	/*mean of service time distribution */
#define IATM	2.0	/*mean of inter-arrival time distribution */
#define NARS	5000	/*number of arrivals to be simulated*/

FACILITY f;		/*pointer for facility */
EVENT done;		/*pointer for counter */
TABLE tbl;		/*pointer for table */
QTABLE qtbl;	/*pointer for qhistogram */
int cnt;		/*number of active tasks*/
FILE *fp;

void cust();
void theory();

void sim()				/*1st process - named sim */
{
	int i;

	fp = fopen("csim.out", "w");
	set_output_file(fp);
	set_trace_file(fp);
	set_error_file(fp);
	set_model_name("M/M/1 Queue");
	create("sim");				/*required create statement*/

	f = facility("facility");		/*initialize facility*/
	done = event("done");			/*initialize event*/
	tbl = table("resp tms");		/*initialize table */
	qtbl = qhistogram("num in sys", 10l);	/*initialize qhistogram*/

	cnt = NARS;				/*initialize cnt*/
	for(i = 1; i <= NARS; i++) {
		hold(expntl(IATM));		/*hold interarrival*/
		cust();				/*initiate process cust*/
		}
	wait(done);				/*wait until all done*/
	report();				/*print report*/
	theory();				/*print theoretical res*/ 
	mdlstat();
}

void cust()				/*process customer*/
{
	TIME t1;
 
	create("cust");				/*required create statement*/

	t1 = clock;				/*time of request */
	note_entry(qtbl);			/*note arrival */
	reserve(f);				/*reserve facility f*/
		hold(expntl(SVTM));		/*hold service time*/
	release(f);				/*release facility f*/
	record(clock-t1, tbl);			/*record response time*/
	note_exit(qtbl);			/*note departure */
	cnt--;					/*decrement cnt*/
	if(cnt == 0)
		set(done);			/*if last arrival, signal*/
	terminate();
}

void theory()			/*print theoretical results*/
{
	double rho, nbar, rtime, tput;

	printf("\n\n\n\t\t\tM/M/1 Theoretical Results\n");

	tput = 1.0/IATM;
	rho = tput*SVTM;
	nbar = rho/(1.0 - rho);
	rtime = SVTM/(1.0 - rho);

	printf("\n\n");
	printf("\t\tInter-arrival time = %10.3f\n",IATM);
	printf("\t\tService time       = %10.3f\n",SVTM);
	printf("\t\tUtilization        = %10.3f\n",rho);
	printf("\t\tThroughput rate    = %10.3f\n",tput);
	printf("\t\tMn nbr at queue    = %10.3f\n",nbar);
	printf("\t\tMn queue length    = %10.3f\n",nbar-rho);
	printf("\t\tResponse time      = %10.3f\n",rtime);
	printf("\t\tTime in queue      = %10.3f\n",rtime - SVTM);
}
