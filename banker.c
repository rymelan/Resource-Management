/**
 *  banker.c
 *
 *  Full Name: Yuval Rif
 *  Course section: LE/EECS3221 M
 *  Description of the program: Using command line user input the program
 *  reads a file with information about some tasks and then runs two algorithms
 *  for resource managment and displays the statistics to stdout
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "task.h"

int * optimistic(Task *tasks, int numOfTasks, int *resources, int numOfResources);
int * bankerAlg(Task *tasks, int numOfTasks, int *resources, int numOfResources);
int isSafe(int *resources, Task *tasks, int resourceNum, int taskNum);

int main(int argc, char *argv[]){
    FILE *fp;

    int num_of_Tasks;
    int num_of_rTypes;


    //Reading info from given file and creating tasks based on that info
    //each task has a list of activities associated with it and it's default
    //state is ready 
    fp  = fopen(argv[1],"r");   
    fscanf(fp, "%d %d", &num_of_Tasks, &num_of_rTypes);
    int *resources = malloc(num_of_rTypes * sizeof(int));  
    Task *tasks = malloc(num_of_Tasks * sizeof(Task));
    if (num_of_Tasks > 0 && num_of_rTypes > 0){
	          
        for(int i = 0; i<num_of_rTypes; i++){
            fscanf(fp, "%d", &resources[i]);
        }

	    for (int i=0; i<num_of_Tasks; i++){
            tasks[i].state = 'r';
            tasks[i].activities = 0;
            tasks[i].current = 0;
            tasks[i].hasResources = malloc(sizeof(int) * num_of_rTypes);
            tasks[i].claim = malloc(sizeof(int) * num_of_rTypes);
            tasks[i].id = i+1;
            for(int j = 0; j<num_of_rTypes; j++){
                tasks[i].hasResources[j] = 0;
            }
            tasks[i].list = malloc(sizeof(struct activity) * 100);
            tasks[i].waiting = 0;
		}
        
        struct activity *newAct = malloc(sizeof(struct activity) * num_of_Tasks *100);
        newAct->type = malloc(sizeof(char)*20);
        while (fscanf(fp, "%s %d %d %d", newAct->type, &newAct->task, &newAct->resource, &newAct->num) != EOF)
        {
            for(int i = 0; newAct->type[i]; i++){
                newAct->type[i] = tolower(newAct->type[i]);
            }
            int activities = tasks[newAct->task-1].activities;
            tasks[newAct->task-1].list[activities] = *newAct;
            tasks[newAct->task-1].activities++;
            newAct++;
            newAct->type = malloc(sizeof(char)*20);
        } 
    }
    Task *tasksCpy = malloc(num_of_Tasks * sizeof(Task));
    for(int i = 0; i<num_of_Tasks; i++){
        tasksCpy[i] = tasks[i];
    }
    
    //call banker and FIFO alrgoorithms for resource managment
    //prints out the results 
    int *endTimeBanker = bankerAlg(tasks, num_of_Tasks, resources, num_of_rTypes);
    int waitingBanker[num_of_Tasks];
    int totalWaitB = 0;
    int totalTimeB = 0;
    for(int i = 0; i<num_of_Tasks; i++){
        waitingBanker[i] = tasks[i].waiting;
    }
    int waitingFIFO[num_of_Tasks];
    int totalWaitO = 0;
    int totalTimeO = 0;
    int *endTimeFIFO = optimistic(tasksCpy, num_of_Tasks, resources, num_of_rTypes);
    float percent;
    for(int i = 0; i<num_of_Tasks; i++){
        waitingFIFO[i] = tasksCpy[i].waiting;
    }
    printf("%18s %36s", "FIFO", "BANKER'S");
    for(int i = 0; i<num_of_Tasks; i++){
        if(tasksCpy[i].state == 'a'){
            printf("\n%9s %-6d %-10s", "Task", tasksCpy[i].id, "aborted");
        }else{
            totalTimeO += endTimeFIFO[i];
            totalWaitO += waitingFIFO[i];
            percent = (float)waitingFIFO[i]/(float)endTimeFIFO[i]*100;
            printf("\n%9s %d %6d %3d %3.0f%%", "Task", tasksCpy[i].id, endTimeFIFO[i], waitingFIFO[i], percent);
        }
        if(tasks[i].state == 'a'){
            printf("%15s %-6d %9s", "Task", tasks[i].id, "aborted");
        }else{
            totalTimeB += endTimeBanker[i];
            totalWaitB += waitingBanker[i];
            percent = (float)waitingBanker[i]/(float)endTimeBanker[i]*100;
            printf("%15s %d %8d %3d %3.0f%%", "Task", tasks[i].id, endTimeBanker[i], waitingBanker[i], percent);
        }
    }
    percent = (float)totalWaitO/(float)totalTimeO*100;
    printf("\n%10s %7d %3d %3.0f%%", "total", totalTimeO, totalWaitO, percent);
    percent = (float)totalWaitB/(float)totalTimeB*100;
    printf("%16s %9d %3d %3.0f%%", "total", totalTimeB, totalWaitB, percent);
    return 0;
}

//given an array of tasks, the number of tasks in the array, an array of resources and
//the number or different resources; this program allocates resources based on whether or not 
//the amount requested exists in the system. A program who's request can't be stisfied is suspended
//in case of a deadlock the program will abort the task with the tasks with lowest id till the deadlock is resolved
// return an int array of the end time of each task
int * optimistic(Task *tasks, int numOfTasks, int *resources, int numOfResources){
    int terminated = 0;
    int deadlock = 0;
    int *returnedResources = malloc(sizeof(int)*numOfResources);
    for(int i = 0; i<numOfResources; i++){
        returnedResources[i] = 0;
    }
    int numPending = 0;
    int cycle = 0;
    int *endTimes = malloc(sizeof(int) * numOfTasks);
    Task *pending[numOfTasks];
    //A loop which runs untill all tasks terminate or abort
    while(!terminated){
        terminated = 1;
        //check pending requests first will satisfy any requests if possible
        //satisfied requests have the state n or available next cycle
        for(int i = 0; i < numPending; i++){
            if(pending[i]->state == 'a'){
                for(int j = i; j < numPending; j++){
                    pending[j] = pending[j+1];
                }
                numPending--;
                i--;
            }else{
                terminated = 0;
                int current = pending[i]->current;
                int resource = pending[i]->list[current].resource;
                int num = pending[i]->list[current].num;
                if(pending[i]->list[current].num <= resources[resource-1]){
                    resources[resource-1] -= num;
                    pending[i]->hasResources[resource-1] += num;
                    pending[i]->current++;
                    pending[i]->state = 'n';
                    for(int j = i; j < numPending; j++){
                        pending[j] = pending[j+1];
                    }
                    numPending--;
                }    
            }
        }
        //check all non-pending active tasks and executes their activities
        for(int i = 0; i < numOfTasks; i++){
            int current = tasks[i].current;
            struct activity *nextAct = &tasks[i].list[current];
            // if the task is computing either it computes this cycle 
            // or it's done computing and change its state to ready
            if(tasks[i].state == 'c'){
                if(tasks[i].computing > 0){
                    tasks[i].computing--;
                    
                }else{
                    tasks[i].state = 'r';
                    tasks[i].current++;
                    current = tasks[i].current;
                    nextAct = &tasks[i].list[current];
                }
            }
            // if the task is pending increase its waiting time
            if(tasks[i].state == 'p'){
                tasks[i].waiting++;
            }
            //reads a tasks next activity and executes it 
            if(tasks[i].state == 'r'){
                int resource = nextAct->resource;
                int num = nextAct->num;
                // If the next activity is initiate then set
                // the claim of the task 
                if(!strcmp(nextAct->type, "initiate")){
                    terminated = 0;
                    tasks[i].claim[resource-1] = num;
                    tasks[i].current++;
                }
                // Satisfies requests based on whether we have enough available resources
                // if not the task is pending
                else if(!strcmp(nextAct->type, "request")){
                    terminated = 0;
                    if(resources[resource-1] >= nextAct->num){
                        resources[resource-1] = resources[resource-1] - num;
                        tasks[i].hasResources[resource - 1] += nextAct->num;
                        tasks[i].current++;
                    }else{
                        pending[numPending] = &tasks[i];
                        tasks[i].state = 'p';
                        numPending++;
                        tasks[i].waiting++;
                    }
                }
                // sets the task to compute state and starts of with the 
                // first computing cycle this turn
                else if(!strcmp(nextAct->type, "compute")){
                    terminated = 0;
                    tasks[i].state = 'c';
                    tasks[i].computing = nextAct->resource - 1;
                }
                // take resources from task and lets them be available 
                // next cycle
                else if(!strcmp(nextAct->type, "release")){
                    terminated = 0;
                    returnedResources[nextAct->resource-1] += nextAct->num;
                    tasks[i].hasResources[nextAct->resource-1] -= nextAct->num;
                    tasks[i].current++;
                }
                // terminates the process releases it's resources if it had any
                else if(!strcmp(nextAct->type, "terminate")){
                    tasks[i].state = 't';
                    endTimes[i] = cycle;
                    for(int j = 0; j<numOfResources; j++){
                        resources[j] += tasks[i].hasResources[j];
                        tasks[i].hasResources[j] = 0; 
                    }
                }
            }if(tasks[i].state == 'n'){
                tasks[i].state = 'r';
            }
        }
        int i = 0;
        // make released resources available
        for(int i = 0; i<numOfResources; i++){
            resources[i] += returnedResources[i];
            returnedResources[i] = 0;
        }
        //check for deadlocks and aborts lowest id tasks until deadlock is resolved
        while((tasks[i].state == 'p' || tasks[i].state == 'a' || tasks[i].state == 't') && i<numOfTasks){
            i++;
        }
        if(i>=numOfTasks && !terminated){
            deadlock = 1;
            i = 0;
            while(deadlock && i<=numOfTasks){
                while((tasks[i].state == 'a' || tasks[i].state == 't') && i<numOfTasks){
                    i++;
                }
                tasks[i].state = 'a';
                endTimes[i] = cycle;
                for(int j = 0; j<numOfResources; j++){
                    resources[j] += tasks[i].hasResources[j];
                    tasks[i].hasResources[j] = 0;
                }
                for(int j = 0; j<=numOfTasks; j++){
                    if(tasks[j].state == 'p'){
                        int resource = tasks[j].list[tasks[j].current].resource;
                        int num = tasks[j].list[tasks[j].current].num;
                        if(resources[resource-1] >= num){
                            deadlock = 0;
                            break;
                        }
                    }
                    
                }
            }
        }
        cycle++; 
    }
    return endTimes;
}

//given an array of tasks, the number of tasks in the array, an array of resources and
//the number or different resources; this program allocates resources based on whether or not 
//the amount requested exists in the system and satisfying the request result in a safe state. 
//A program who's request can't be stisfied is suspended
// return an int array of the end time of each task
int * bankerAlg(Task *tasks, int numOfTasks, int *resources, int numOfResources){
    int terminated = 0;
    int *returnedResources = malloc(sizeof(int)*numOfResources);
    int *resourcesAfter = malloc(sizeof(int)*numOfResources);
    for(int i = 0; i<numOfResources; i++){
        returnedResources[i] = 0;
        resourcesAfter[i] = resources[i];
    }
    int numPending = 0;
    int cycle = 0;
    int *endTimes = malloc(sizeof(int) * numOfTasks);
    Task *pending[numOfTasks];
    //A loop which runs untill all tasks terminate or abort
    while(!terminated){
        terminated = 1;
        //check pending requests first will satisfy any requests if possible
        //and will result in a safe state satisfied requests have the state n 
        //or available next cycle
        for(int i = 0; i < numPending; i++){
            if(pending[i]->state == 'a'){
                for(int j = i; j < numPending; j++){
                    pending[j] = pending[j+1];
                }
                numPending--;
                i--;
            }else{
                terminated = 0;
                int current = pending[i]->current;
                int resource = pending[i]->list[current].resource;
                int num = pending[i]->list[current].num;
                resourcesAfter[resource-1] -= num;
                pending[i]->hasResources[resource-1] += num;
                if(resourcesAfter[resource-1] >= 0 && isSafe(resourcesAfter, tasks, numOfResources, numOfTasks)){
                    
                    resources[resource-1] -= num;
                    pending[i]->current++;
                    pending[i]->state = 'n';
                    for(int j = i; j < numPending; j++){
                        pending[j] = pending[j+1];
                    }
                    numPending--;
                    
                }else{
                    pending[i]->hasResources[resource-1] -= num;
                }
                resourcesAfter[resource-1] = resources[resource-1];    
            }
        }
        //check all non-pending active tasks and executes their activities
        for(int i = 0; i < numOfTasks; i++){
            int current = tasks[i].current;
            struct activity *nextAct = &tasks[i].list[current];
            // if the task is computing either it computes this cycle 
            // or it's done computing and change its state to ready
            if(tasks[i].state == 'c'){
                if(tasks[i].computing > 0){
                    tasks[i].computing--;
                    
                }else{
                    tasks[i].state = 'r';
                    tasks[i].current++;
                    current = tasks[i].current;
                    nextAct = &tasks[i].list[current];
                }
            }
            // if the task is pending increase its waiting time
            if(tasks[i].state == 'p'){
                tasks[i].waiting++;
            }
            //reads a tasks next activity and executes it
            if(tasks[i].state == 'r'){
                int resource = nextAct->resource;
                int num = nextAct->num;
                // If the next activity is initiate then set
                // the claim of the task. Aborts any tasks which
                // claims more than the available number of resources
                if(!strcmp(nextAct->type, "initiate")){
                    terminated = 0;
                    if(num > resources[resource-1]){
                        tasks[i].state = 'a';
                        printf("  Banker aborts task %d before run begins:\n", tasks[i].id);
                        printf("\tclaim for resource %d (%d) exceeds number of units present (%d)\n", resource-1, num, resources[resource-1]);
                    }else{
                        tasks[i].claim[resource-1] = num;
                        tasks[i].current++;
                    }
                }
                // Satisfies requests based on whether we have enough available resources
                // if not or if granting the resources will
                // cause an unsafe state the task is pending. 
                // Will abort tasks if request more than their claimed amount.
                else if(!strcmp(nextAct->type, "request")){
                    terminated = 0;
                    resourcesAfter[resource-1] -= num;
                    tasks[i].hasResources[resource - 1] += num;
                    if(num > tasks[i].claim[resource-1]){
                        printf("Banker aborts task %d for requesting more than it's claim:\n", tasks[i].id);
                        printf("\trequest for resource %d (%d) exceeds number of units task claimed (%d)", resource, num, tasks[i].claim[resource-1]);
                        tasks[i].state = 'a';
                        for(int j = 0; j < numOfResources; j++){
                            resources[j] += tasks[i].hasResources[j];
                            tasks[i].hasResources[j] = 0;
                        }
                    }else if(resourcesAfter[resource-1] >= 0 && isSafe(resourcesAfter, tasks, numOfResources, numOfTasks)){
                        resources[resource-1] = resources[resource-1] - num;
                        tasks[i].current++;
                           
                    }else{
                        pending[numPending] = &tasks[i];
                        tasks[i].state = 'p';
                        numPending++;
                        tasks[i].waiting++;
                        tasks[i].hasResources[resource - 1] -= num;
                    }
                    resourcesAfter[resource-1] = resources[resource-1];
                }
                // sets the task to compute state and starts of with the 
                // first computing cycle this turn
                else if(!strcmp(nextAct->type, "compute")){
                    terminated = 0;
                    tasks[i].state = 'c';
                    tasks[i].computing = nextAct->resource - 1;
                }
                // take resources from task and lets them be available 
                // next cycle
                else if(!strcmp(nextAct->type, "release")){
                    terminated = 0;
                    returnedResources[nextAct->resource-1] += nextAct->num;
                    tasks[i].hasResources[nextAct->resource-1] -= nextAct->num;
                    tasks[i].current++;
                }
                // terminates the process releases it's resources if it had any
                else if(!strcmp(nextAct->type, "terminate")){
                    tasks[i].state = 't';
                    endTimes[i] = cycle;
                    for(int j = 0; j<numOfResources; j++){
                        resources[j] += tasks[i].hasResources[j];
                        tasks[i].hasResources[j] = 0; 
                    }
                }
            }if(tasks[i].state == 'n'){
                tasks[i].state = 'r';
            }
        }
        // make released resources available
        for(int i = 0; i<numOfResources; i++){
            resources[i] += returnedResources[i];
            resourcesAfter[i] = resources[i];
            returnedResources[i] = 0;
        }
        cycle++; 
    }
    return endTimes;
}

// Given an array of resources and tasks as well as the number of items in those arrays
// return whether this is a safe state of not whether there is a for all tasks to terminate
// return 1 if the state is safe 0 is it isn't
int isSafe(int *resources, Task *tasks, int resourceNum, int taskNum){
    int isTerminated = 1;
    Task *tasksCpy = malloc(sizeof(Task)*taskNum);
    for (int i = 0; i<taskNum; i++){
        tasksCpy[i] = tasks[i];
    }
    for(int i = 0; i < taskNum; i++){
        int canTerminate = 1;
        if(tasks[i].state != 't' && tasks[i].state != 'a'){
            isTerminated = 0;
            for(int j = 0; j < resourceNum; j++){
                if(resources[j] < tasks[i].claim[j]-tasks[i].hasResources[j]){
                    canTerminate = 0;
                }
            }
            if(canTerminate){
                tasksCpy[i].state = 't';
                for(int j = 0; j < resourceNum; j++){
                    resources[j] += tasksCpy[i].hasResources[j];
                    
                }
                if(isSafe(resources, tasksCpy, resourceNum, taskNum)){
                    return 1;
                }
            }
        }  
    }
    return isTerminated;
}
