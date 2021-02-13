# Synchronous-Workers
A set of workers and a master working synchronously with the use of semaphores locks.

## Information
The project's goal is to create independend programs tha run concurrently and simualte the creation of salads for a store. There are two types of processes, where each one has a different role 
in the simulation. The two types of processes are the chef and the saladmakers. The chef coordinates and manages the production of salads with the help of the saladmakers. The saladmakers
wait for the chef to give them the needed ingredients so they can go on an create salads. There are a total of 1 chef and 3 salad makers. Each salad is made up of tomatoes, peppers and onions.

The project uses semaphores to coordinate the processes execution. There is also a shared memory segment that stores needed information for all processes.

The chef has an unlimited amount of the three ingredients at his disposal. Each salad maker has an unlimited amount of only one ingredient. That means each salad maker has to wait for the chef 
to give him the two other ingredients that he needs to create a salad. When he has all the ingredients that he needs, he proceeds to make a salad, cutting ingredients for a random amount of time.
The chef chooses two ingredients randomly (not the same every time) and places them on the table. Then the corresponding salad maker is notified to begin working and the chef goes on to the next pair of
ingredients, untill all needed salads have been made.

There is also a loggine mechanism for each salad makers and also a unified log that tracks "global" information.

## Using the Simulation

### Compilation

Enter the ``` make ``` command while in the */Synchronous-Workers/* directory to create all the executables.

### Execution 

We can run the chef using ``` ./chef -n numOfSlds -m mantime ```.
The saladmakers can be run using ``` ./saladmakers -t1 lb -t2 ub -s shmid -i makerIndex ```.

Each of the processes should be run in a different tty.

> For your ease the chef prints the exact command needed to run the first salad maker. Then you can just change the -i argument for the other two.

### Command line arguments
You can give the following arguments in the command line:

For the chef:

``` -n <numOfSalads> ``` : The number of salads that need to be created.

``` -m <mantime> ``` : The time that the chef can take for rest between each ingredient deposit.

For the saladamakers:

``` -t1 <lb> ``` : lb is the minimum time it takes for a maker to create a salad.

``` -t2 <ub> ``` : ub is the maximum time it takes for a maker to create a salad.

``` -s <shmid> ``` : This is the key for the shared memory segment.

``` -i <ingredient> ``` : The ingredient this maker has unlimited disposal of.

## About
- This is a fifth semester assignment, by professor [A. Delis](https://www.alexdelis.eu/), for the Department of Informatics and Telecommunications, UoA. It was a part of the Operating Systems course and was given as the third assignment.
