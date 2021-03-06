/* External definitions for single-server queueing system, fixed run length. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "lcgrand.h"  /* Header file for random-number generator. */

#define Q_LIMIT 2500  /* Limit on queue length. */
#define BUSY      1  /* Mnemonics for server's being busy */
#define IDLE      0  /* and idle. */

int   next_event_type, num_custs_delayed, num_events, num_in_q1, num_in_q2, server1_status, server2_status;
float area_num_in_q1, area_num_in_q2, area_server1_status, area_server2_status, mean_interarrival,
      mean_service1, mean_service2, sim_time, time_arrival1[Q_LIMIT + 1], time_arrival2[Q_LIMIT + 1],
      time_end, time_last_event, time_next_event[6], total_of_delays1, total_of_delays2;
FILE  *infile, *outfile;

void  initialize(void);
void  timing(void);
void  arrive(void);
void  second_arrive(void);
void  depart(void);
void  second_depart(void);
void  report(void);
void  update_time_avg_stats(void);
float expon(float mean);


int main()  /* Main function. */
{
    /* Open input and output files. */

    infile  = fopen("mm2alt.in",  "r");
    outfile = fopen("mm2alt.out", "w");

    /* Specify the number of events for the timing function. */

    num_events = 5;

    /* Read input parameters. */

    fscanf(infile, "%f %f %f %f", &mean_interarrival, &mean_service1, &mean_service2, &time_end);

    /* Write report heading and input parameters. */

    fprintf(outfile, "Double-server queueing system with fixed run");
    fprintf(outfile, " length\n\n");
    fprintf(outfile, "Mean interarrival time%11.3f minutes\n\n",
            mean_interarrival);
    fprintf(outfile, "Mean service time 1%16.3f minutes\n\n", mean_service1);
    fprintf(outfile, "Mean service time 2%16.3f minutes\n\n", mean_service2);
    fprintf(outfile, "Length of the simulation%9.3f minutes\n\n", time_end);
    fprintf(outfile, "\n\n*********************************\n\n");

    for(int i = 0; i < 10; i++){

        /* Initialize the simulation. */

        initialize();

        /* Run the simulation until it terminates after an end-simulation event
           (type 3) occurs. */
        do
        {
            /* Determine the next event. */

            timing();

            /* Update time-average statistical accumulators. */

            update_time_avg_stats();

            /* Invoke the appropriate event function. */

            switch (next_event_type)
            {
                case 1:
                    arrive();
                    break;
                case 2:
                    second_arrive();
                    break;
                case 3:
                    depart();
                    break;
                case 4:
                    second_depart();
                    break;
                case 5:
                    report();
                    break;
            }

        /* If the event just executed was not the end-simulation event (type 3),
           continue simulating.  Otherwise, end the simulation. */

        } while (next_event_type != 5);

        fprintf(outfile, "\n\n*********************************");
    }

    fclose(infile);
    fclose(outfile);

    return 0;
}


void initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */

    sim_time = 0.0;

    /* Initialize the state variables. */

    server1_status   = IDLE;
    server2_status   = IDLE;
    num_in_q1        = 0;
    num_in_q2        = 0;
    time_last_event = 0.0;

    /* Initialize the statistical counters. */

    num_custs_delayed  = 0;
    total_of_delays1    = 0.0;
    total_of_delays2    = 0.0;
    area_num_in_q1      = 0.0;
    area_num_in_q2      = 0.0;
    area_server1_status = 0.0;
    area_server2_status = 0.0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration.  The end-
       simulation event (type 3) is scheduled for time time_end. */

    time_next_event[1] = sim_time + expon(mean_interarrival);
    time_next_event[2] = 1.0e+30;
    time_next_event[3] = 1.0e+30;
    time_next_event[4] = 1.0e+30;
    time_next_event[5] = time_end;
}


void timing(void)  /* Timing function. */
{
    int   i;
    float min_time_next_event = 1.0e+29;

    next_event_type = 0;

    /* Determine the event type of the next event to occur. */

    for (i = 1; i <= num_events; ++i)
            if (time_next_event[i] < min_time_next_event) {
            min_time_next_event = time_next_event[i];
            next_event_type     = i;
        }

    /* Check to see whether the event list is empty. */

    if (next_event_type == 0)
    { 
    
        /* The event list is empty, so stop the simulation */

        fprintf(outfile, "\nEvent list empty at time %f", sim_time);
        exit(1);
    }

    /* The event list is not empty, so advance the simulation clock. */

    sim_time = min_time_next_event;
}


void arrive(void)  /* Arrival event function. */
{
    float delay;

    /* Schedule next arrival. */

    time_next_event[1] = sim_time + expon(mean_interarrival);

    /* Check to see whether server is busy. */

    if (server1_status == BUSY) {

        /* Server is busy, so increment number of customers in queue. */

        ++num_in_q1;
        // printf("flag\n");

        /* Check to see whether an overflow condition exists. */

        if (num_in_q1 > Q_LIMIT) {

            /* The queue has overflowed, so stop the simulation. */

            fprintf(outfile, "\nOverflow of the array time_arrival at");
            fprintf(outfile, " time %f", sim_time);
            exit(2);
        }

        /* There is still room in the queue, so store the time of arrival of the
           arriving customer at the (new) end of time_arrival. */

        time_arrival1[num_in_q1] = sim_time;
    } else {

        /* Server is idle, so arriving customer has a delay of zero.  (The
           following two statements are for program clarity and do not affect
           the results of the simulation.) */

        delay            = 0.0;
        total_of_delays1 += delay;

        /* Increment the number of customers delayed, and make server busy. */

        ++num_custs_delayed;
        server1_status = BUSY;

        /* Schedule a departure (service completion). */

        time_next_event[3] = sim_time + expon(mean_service1);

    }
}

void second_arrive(void)
{

    float delay;

    if (server2_status == BUSY){

        /* Server is busy, so increment number of customers in queue. */

        ++num_in_q2;

        /* Check to see whether an overflow condition exists. */

        if (num_in_q2 > Q_LIMIT) {

            /* The queue has overflowed, so stop the simulation. */

            fprintf(outfile, "\nOverflow of the array time_arrival at");
            fprintf(outfile, " time %f\n\n", sim_time);
            fprintf(outfile, "num_in_q2: %d", num_in_q2);
            exit(2);
        }

        /* There is still room in the queue, so store the time of arrival of the
           arriving customer at the (new) end of time_arrival. */

        time_arrival2[num_in_q2] = time_arrival1[0];


    }

    else {

        /* Server is idle, so arriving customer has a delay of zero.  (The
           following two statements are for program clarity and do not affect
           the results of the simulation.) */

        delay            = 0.0;
        total_of_delays2 += delay;

        /* Increment the number of customers delayed, and make server busy. */

        ++num_custs_delayed;
        server2_status = BUSY;

        /* Schedule a departure (service completion). */

        time_next_event[4] = sim_time + expon(mean_service2);
    }
}


void depart(void)  /* Departure event function. */
{
    int   i;
    float delay;

    /* Check to see whether the queue is empty. */

    if (num_in_q1 == 0) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server1_status      = IDLE;
        time_next_event[3] = 1.0e+30;
    } else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        --num_in_q1;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - time_arrival1[1];
        total_of_delays1 += delay;

        time_arrival2[num_in_q2] = time_arrival1[1];

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed;
        time_next_event[3] = sim_time + expon(mean_service1);

        /* Move each customer in queue (if any) up one place. */

        for (i = 0; i <= num_in_q1; ++i)
            time_arrival1[i] = time_arrival1[i + 1];

        second_arrive();
    }
}

void second_depart(void)
{
    int i;
    float delay;

    if (num_in_q2 == 0) {

        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server2_status      = IDLE;
        time_next_event[4] = 1.0e+30;
    } else {

        /* The queue is nonempty, so decrement the number of customers in
           queue. */

        --num_in_q2;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - time_arrival2[1];
        total_of_delays2 += delay;

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed;
        time_next_event[4] = sim_time + expon(mean_service2);

        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q2; ++i)
            time_arrival2[i] = time_arrival2[i + 1];
    }
}


void report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */

    fprintf(outfile, "\n\nAverage delay in queue 1%11.3f minutes\n\n",
            total_of_delays1 / num_custs_delayed);
    fprintf(outfile, "Average delay in queue 2%11.3f minutes\n\n",
            total_of_delays2 / num_custs_delayed);
    fprintf(outfile, "Average number in queue 1%10.3f\n\n",
            area_num_in_q1 / sim_time);
    fprintf(outfile, "Average number in queue 2%10.3f\n\n",
            area_num_in_q2 / sim_time);
    fprintf(outfile, "Server 1 utilization%15.3f\n\n",
            area_server1_status / sim_time);
    fprintf(outfile, "Server 2 utilization%15.3f\n\n",
            area_server2_status / sim_time);
    fprintf(outfile, "Number of delays completed%7d",
            num_custs_delayed);
}


void update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_time - time_last_event;
    time_last_event       = sim_time;

    /* Update area under number-in-queue function. */

    area_num_in_q1      += num_in_q1 * time_since_last_event;
    area_num_in_q2      += num_in_q2 * time_since_last_event;

    /* Update area under server-busy indicator function. */

    area_server1_status += server1_status * time_since_last_event;
    area_server2_status += server2_status * time_since_last_event;
}


float expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */

    return -mean * log(lcgrand(1));
}

