Using command line input the program reads a file with information about some 
tasks and then runs an optimistic and the banker algorithms for resource managment 
and displays the statistics to stdout.

input is a text file in the following format:


numberOfTasks numberOfResourceTypes amountOfEachResourceInOrder
activityType taskNumber resourceType numberOfResourcesUsed(As many lines of this as needed)



An example of a valid input:

3 1 4
initiate  1 1 3
request   1 1 1
release   1 1 1
request   1 1 3
release   1 1 3
terminate 1 0 0
initiate  2 1 3
request   2 1 1
request   2 1 1
release   2 1 2
terminate 2 0 0
initiate  3 1 3
request   3 1 2
request   3 1 1
release   3 1 3
terminate 3 0 0


Each task must be initiated before it can do anything else
tasks may request resources when being initialized
tasks may request and release resources
tasks must terminate before end of file




task.h:
Defines the representations of task and activity in the 
system
has no dependencies

banker.c:
Given a text file with information about the tasks and their activities and prints 
out to stdout the end time, waiting time and precentage spent waiting
for every task using FIFO resource allocating and banker's algorithm 
dependencies: task.h
