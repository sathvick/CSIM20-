#include "csim.h"
#include<stdio.h>

#define SIMTIME 1000.0
#define NUM_NODES 5L
#define TIME_OUT 2.0
#define T_DELAY 0.2
#define TRANS_TIME 0.1
#define REQUEST 1L
#define REPLY 2L

typedef struct msg *msg_t;

struct msg{
        long type;
        long from;
        long to;
        TIME start_time;
        msg_t link;
        };

msg_t msg_queue;

struct nde{
        FACILITY cpu;
        MBOX input;
        };

struct nde node[NUM_NODES];
FACILITY network[NUM_NODES][NUM_NODES];
TABLE resp_tm;
FILE *fp;

void init();
//void my_report();
void proc();
void send_msg();
void form_reply();
void decode_msg();
void return_msg();
msg_t new_msg();

double successful, failed, rtt, total, failed_trans;

void sim()
{
        create("sim");
		init();
		hold(SIMTIME);
		//my_report();		
		printf("\n\nAverage success %lf \n",successful/total);
		printf("Average Fail %lf \n",failed_trans/total);
		printf("Average RTT %lf \n",rtt/total);
}

void init()
{
	long i, j;
	char str[24];
	
	fp = fopen("xxx.out", "w");
    set_output_file(fp);
	max_facilities(NUM_NODES*NUM_NODES+NUM_NODES);
	max_servers(NUM_NODES*NUM_NODES+NUM_NODES);
	max_mailboxes(NUM_NODES);
	max_events(2*NUM_NODES*NUM_NODES);
	resp_tm = table("msg rsp tm");
	msg_queue = NIL;
	
	for(i=0;i<NUM_NODES;i++){
		sprintf(str, "cpu.%d", i);
		node[i].cpu = facility(str);
		sprintf(str, "input.%d", i);
		node[i].input = mailbox(str);
		}
		
	for(i=0;i<NUM_NODES;i++)
		for(j=0; j<NUM_NODES; j++){
			sprintf(str, "nt%d.%d", i, j);
			network[i][j] = facility(str);
			}
	
	
	for(i=0;i<NUM_NODES;i++){
		proc(i);
		hold(expntl(5));
	}
}

void proc(n)
long n;
{
	msg_t m;
	long s, t;
	long from, to;
	
	create("proc");
	while(clock < SIMTIME){
		s = timed_receive(node[n].input,(long*) &m, TIME_OUT);
		switch(s){
			case TIMED_OUT:
			m=new_msg(n);
			
			from = m->from;
			to = m->to;
			
	#ifdef TRACE
				decode_msg("timed out, send new msg", m, n);
	#endif
				rtt = rtt +2;
				failed++; 
				total++;
				if(failed > 1){
					failed_trans++;
				}
	
				send_msg(m);
				printf("node.%ld re-sends a HELLO to node.%ld at %.1f seconds.\n",from , to, clock);
				break;
			
			case EVENT_OCCURRED:
			
			printf("node.%ld sends a HELLO to node.%ld at %.1f seconds.\n",from, to, clock);
	#ifdef TRACE
				decode_msg("received msg", m, n);
	#endif
				t = m->type;
				switch(t) {
					case REQUEST:
						if(uniform(0.1,0.5)>0.4){
						form_reply(m);
		#ifdef TRACE
						decode_msg("return request", m, n);
		#endif
						send_msg(m);
						}
						printf("node.%ld replies a HELLO_ACK to node.%ld at %.1f seconds.\n", m->from, m->to, m->start_time);
						break;
						
					case REPLY:
		#ifdef TRACE
						decode_msg("receive reply", m, n);
		#endif
						record(clock - m -> start_time, resp_tm);
						return_msg(m);
						successful++;
						total++;
						rtt = rtt + 0.4;
						printf("node.%ld receives a HELLO_ACK from node.%ld at %.1f seconds.\n", m->to, m->from, clock);	
						break;
					default:
						decode_msg("***unexpected type", m, n);
						break;
				}
			break;
		}
	}
}

void send_msg(m)
msg_t m;
{
	long from, to;
	
	from = m->from;
	to = m->to;
	use(node[from].cpu, T_DELAY);
	reserve(network[from][to]);
	hold(TRANS_TIME);
	send(node[to].input,(long)m);
	release(network[from][to]);
}

msg_t new_msg(from)
long from;
{
	msg_t m;
	long i;
	
	if(msg_queue == NIL){
		m = (msg_t)do_malloc(sizeof(struct msg));
		}
	else{
		m = msg_queue;
		msg_queue = msg_queue->link;
		}
	do{
		i = random(01, NUM_NODES - 1);
	} while(i==from);
	m->to=i;
	m->from=from;
	m->type=REQUEST;
	m->start_time=clock;
	return(m);
}

void return_msg(m)
msg_t m;
{
	m->link = msg_queue;;
	msg_queue = m;
}

void form_reply(m)
msg_t m;
{
	long from, to;
	
	from=m->from;
	to=m->to;
	m->from=to;
	m->to=from;
	m->type=REPLY;

}

void decode_msg(str, m, n)
char *str; msg_t m; long n;
{
	printf ("%6.3f node %2ld: %s - msg: type = %s, from = %ld, to = %ld\n",
		clock, n, str, (m->type == REQUEST) ? "req" : "rep",
		m->from, m->to);
}