#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "jobs.h"
#include "scheduler.h"
#include "banker.h"
#include "logger.h"
#include "ipc.h"
#include "worker.h"
#include "limits.h"

static Job jobs[MAX_JOBS];
static int count=0, quantum=2;

static int number(const char *label, int *out) {
    char buf[128], extra;
    long value;
    printf("%s",label);
    if (!fgets(buf,sizeof(buf),stdin)) return 0;
    if (sscanf(buf," %ld %c",&value,&extra)!=1 || value<INT_MIN || value>INT_MAX) {
        puts("Invalid integer input."); return 0;
    }
    *out=(int)value; return 1;
}
static void seed(void) {
    count=0;
    add_job(jobs,&count,1,0,5,2);
    add_job(jobs,&count,2,1,3,1);
    add_job(jobs,&count,3,2,8,3);
    add_job(jobs,&count,4,3,6,2);
}
static void banker_demo(int safe) {
    if (safe) {
        const int avail[3]={3,3,2};
        const int alloc[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES]={{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
        const int max[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES]={{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
        banker_check_safety(5,3,avail,alloc,max);
    } else {
        const int avail[1]={0};
        const int alloc[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES]={{1},{0}};
        const int max[BANKER_MAX_PROCESSES][BANKER_MAX_RESOURCES]={{2},{1}};
        banker_check_safety(2,1,avail,alloc,max);
    }
}
static int demo(int option) {
    switch(option) {
        case 3: if (!count) {puts("Add jobs first.");return 1;} run_fcfs(jobs,count);mark_jobs_completed(jobs,count);return 0;
        case 4: if (!count) {puts("Add jobs first.");return 1;} run_round_robin(jobs,count,quantum);mark_jobs_completed(jobs,count);return 0;
        case 5: if (!count) {puts("Add jobs first.");return 1;} run_priority_non_preemptive(jobs,count);mark_jobs_completed(jobs,count);return 0;
        case 6: if (!count) {puts("Add jobs first.");return 1;} compare_schedulers(jobs,count,quantum);mark_jobs_completed(jobs,count);return 0;
        case 7: banker_demo(1);return 0;
        case 8: banker_demo(0);return 0;
        case 9: return run_logger_demo();
        case 10:return run_ipc_demo();
        case 11:return run_worker_demo();
        case 12:return run_limits_demo();
        default:puts("Unknown demo.");return 1;
    }
}
static int integrated(void) {
    int result=0;
    if (!count) seed();
    puts("=== Integrated CampusCompute demonstration ===");
    show_jobs(jobs,count);
    for(int i=3;i<=12;i++) {
        printf("\n=== Module %d ===\n",i);
        if(demo(i)) result=1;
    }
    printf("\nIntegrated result: %s\n",result?"Some modules failed":"All module checks passed");
    return result;
}
static void help(void) {
    puts("Usage: ./campuscompute [menu|all|fcfs|rr|priority|compare|banker-safe|banker-unsafe|logger|ipc|worker|limits|help]");
    puts("Command demos use sample jobs; menu supports adding custom jobs.");
    puts("Priority: smaller number runs first. RR default quantum: 2.");
}
static void menu(void) {
    int choice;
    for (;;) {
        puts("\nCampusCompute\n1 Add job\n2 Show jobs\n3 FCFS\n4 Round Robin\n5 Priority\n6 Compare scheduling\n7 Banker safe\n8 Banker unsafe\n9 Producer-consumer logger\n10 Shared memory IPC\n11 Worker normal/failure\n12 Resource limits\n13 Set RR quantum\n14 Integrated demo\n15 Load sample jobs\n16 Clear jobs\n0 Exit");
        if(!number("Choice: ",&choice)) {if(feof(stdin))return;continue;}
        if(choice==0)return;
        if(choice==1) {
            int id,arrival,burst,priority;
            if(number("Job ID: ",&id)&&number("Arrival (>=0): ",&arrival)&&number("Burst (>0): ",&burst)&&number("Priority (>=0): ",&priority))
                add_job(jobs,&count,id,arrival,burst,priority);
        } else if(choice==2) show_jobs(jobs,count);
        else if(choice>=3&&choice<=12) {if(demo(choice))puts("Demo failed; see errors above.");}
        else if(choice==13) {int q;if(number("Quantum (>0): ",&q)) {if(q>0)quantum=q;else puts("Quantum must be positive.");}}
        else if(choice==14) integrated();
        else if(choice==15) seed();
        else if(choice==16) {count=0;puts("Job list cleared.");}
        else puts("Invalid command. Enter a menu number 0-16.");
    }
}
int main(int argc,char **argv) {
    if(argc==1 || (argc==2&&!strcmp(argv[1],"menu"))) {menu();return 0;}
    if(argc!=2) {help();return 1;}
    const char *names[]={"fcfs","rr","priority","compare","banker-safe","banker-unsafe","logger","ipc","worker","limits"};
    if(!strcmp(argv[1],"help")) {help();return 0;}
    if(!strcmp(argv[1],"all"))return integrated();
    for(int i=0;i<10;i++) if(!strcmp(argv[1],names[i])) {
        if(i<4)seed();
        return demo(i+3);
    }
    help();return 1;
}
