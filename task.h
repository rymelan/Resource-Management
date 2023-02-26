/**
 *  process.h
 *
 *  Full Name: Yuval Rif
 *  Course section: LE/EECS3221 M
 *  Representation of a task in the system
 *  
 */




// representation of an activity
struct activity{
    int task;//the task associated with this activity
    int num;// this represents the 4th argument in the given activity
    int resource;//the type of resource this activity refers to
    char *type;// a lower case string indicating the type of activity
};

#ifndef TASK_H
#define TAsK_H

typedef struct task {
    int id;//the tasks id
    int *claim;//initial claims of reasources
    struct activity *list;//list of activities for this task
    int activities;//number of overall activities
    int current;//number of the current activity in the array
    int computing;//how much time left for computing 0 if not currently computing
    int *hasResources;//number of reasources assigned to task
    char state; //c is computing, t is terminated, r is ready to give new activity, p pending, a is aborted, and n is available next cycle 
    int waiting;//time the task spends waiting
} Task;
#endif


