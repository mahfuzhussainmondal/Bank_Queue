#ifndef QSIM_H
#define QSIM_H

// Standard includes needed for the simulation
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// TODO: Maybe make these configurable through command line?
// For now hardcoding these values based on observation at local bank
#define MIN_IDLE_TIME 1          // minimum break time in seconds
#define MAX_IDLE_TIME 600        // max coffee break (10 mins)
#define MIN_TELLER_IDLE_TIME 1   // quick check of other queues
#define MAX_TELLER_IDLE_TIME 150 // time for paperwork etc

// Event types
typedef enum {
    EVENT_CUSTOMER_ARRIVAL,
    EVENT_CUSTOMER_DEPARTURE,
    EVENT_TELLER_AVAILABLE
} EventType;

// Forward declarations of structures
struct Event;
struct Customer;
struct Teller;
struct TellerQueue;
struct EventQueue;

// Function pointer types
typedef void (*ActionFunction)(struct Event* event);

// Event structure
typedef struct Event {
    EventType type;
    float time;
    struct Event* next;
    union {
        struct Customer* customer;
        struct Teller* teller;
    } actor;
    ActionFunction action;
} Event;

// Customer structure
typedef struct Customer {
    int id;
    float arrival_time;
    float service_start_time;
    float departure_time;
    struct TellerQueue* assigned_queue;
    struct Customer* next;  // For queue linking
} Customer;

// Teller structure
typedef struct Teller {
    int id;
    float total_service_time;
    float total_idle_time;
    float last_service_end;
    struct TellerQueue* queue;
    int is_available;
} Teller;

// Teller Queue structure
typedef struct TellerQueue {
    struct Customer* head;
    struct Customer* tail;
    struct Teller* teller;
    int size;
    struct TellerQueue* next;
} TellerQueue;

// Event Queue structure
typedef struct EventQueue {
    Event* head;
    int size;
} EventQueue;

// Simulation statistics structure
typedef struct Statistics {
    int total_customers_served;
    float total_simulation_time;
    float total_wait_time;
    float max_wait_time;
    float total_service_time;
    float total_idle_time;
    float sum_squared_wait_times;  // For calculating standard deviation
} Statistics;

// Function declarations
void initialize_simulation(int num_customers, int num_tellers, float sim_time, float avg_service_time);
void run_simulation(int is_single_queue);
void cleanup_simulation(void);

// Event queue operations
void add_event(Event* event);
Event* remove_next_event(void);
void log_function_call(const char* function_name);

// Event actions
void customer_arrival_action(Event* event);
void customer_departure_action(Event* event);
void teller_available_action(Event* event);

// Queue operations
void add_to_teller_queue(TellerQueue* queue, Customer* customer);
Customer* remove_from_teller_queue(TellerQueue* queue);
TellerQueue* get_shortest_queue(void);

// Utility functions
float generate_arrival_time(float simulation_time);
float generate_service_time(float average_service_time);
void print_statistics(int is_single_queue);

#endif // QSIM_H