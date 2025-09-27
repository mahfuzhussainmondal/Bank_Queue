#include "../include/qsim.h"
#include <string.h>

// Global variables
static EventQueue* event_queue;
static TellerQueue* teller_queues;
static Statistics stats;
static float simulation_time;
static float average_service_time;
static Customer* customers;
static Teller* tellers;
static int num_customers;
static int num_tellers;
static int summary_mode = 1; // 1 for summary only, 0 for detailed output// Keep track of function calls for debugging
// This helped me find that infinite loop bug!
void log_function_call(const char* function_name) {
    // Only log when not in summary mode
    // Makes output cleaner for final testing
    if (!summary_mode) {
        printf("[Function Call] %s\n", function_name);
    }
}

// Initialize simulation
// Fixed bug: Was using wrong random seed causing same pattern
// Now using time(NULL) for true randomness
void initialize_simulation(int n_customers, int n_tellers, float sim_time, float avg_service_time) {
    // Keep track of function calls - helps with debugging
    log_function_call("initialize_simulation");
    
    // Set simulation parameters
    num_customers = n_customers;
    num_tellers = n_tellers;
    simulation_time = sim_time;
    average_service_time = avg_service_time;
    
    // Initialize random seed
    srand(time(NULL));
    
    // Create event queue
    event_queue = (EventQueue*)malloc(sizeof(EventQueue));
    event_queue->head = NULL;
    event_queue->size = 0;
    
    // Create tellers
    tellers = (Teller*)malloc(num_tellers * sizeof(Teller));
    for (int i = 0; i < num_tellers; i++) {
        tellers[i].id = i;
        tellers[i].total_service_time = 0;
        tellers[i].total_idle_time = 0;
        tellers[i].last_service_end = 0;
        tellers[i].is_available = 1;
        tellers[i].queue = NULL;
    }

    // Create customers
    customers = (Customer*)malloc(num_customers * sizeof(Customer));
    for (int i = 0; i < num_customers; i++) {
        customers[i].id = i;
        customers[i].arrival_time = generate_arrival_time(simulation_time);
        customers[i].service_start_time = -1;
        customers[i].departure_time = -1;
        customers[i].assigned_queue = NULL;
        
        // Create arrival event
        Event* event = (Event*)malloc(sizeof(Event));
        event->type = EVENT_CUSTOMER_ARRIVAL;
        event->time = customers[i].arrival_time;
        event->actor.customer = &customers[i];
        event->action = customer_arrival_action;
        event->next = NULL;
        
        add_event(event);
    }
    
    // Initialize statistics
    memset(&stats, 0, sizeof(Statistics));
}

// Event queue operations
void add_event(Event* event) {
    log_function_call("add_event");
    
    if (event_queue->head == NULL || event->time < event_queue->head->time) {
        event->next = event_queue->head;
        event_queue->head = event;
    } else {
        Event* current = event_queue->head;
        while (current->next != NULL && current->next->time <= event->time) {
            current = current->next;
        }
        event->next = current->next;
        current->next = event;
    }
    event_queue->size++;
}

Event* remove_next_event(void) {
    log_function_call("remove_next_event");
    
    if (event_queue->head == NULL) {
        return NULL;
    }
    
    Event* event = event_queue->head;
    event_queue->head = event->next;
    event_queue->size--;
    event->next = NULL;
    return event;
}

// Generate random times
float generate_arrival_time(float simulation_time) {
    return simulation_time * rand() / (float)RAND_MAX;
}

float generate_service_time(float average_service_time) {
    return 2 * average_service_time * rand() / (float)RAND_MAX;
}

// Queue operations
TellerQueue* get_shortest_queue(void) {
    log_function_call("get_shortest_queue");
    
    TellerQueue* shortest = teller_queues;
    int min_size = shortest->size;
    
    for (TellerQueue* queue = teller_queues->next; queue != NULL; queue = queue->next) {
        if (queue->size < min_size) {
            shortest = queue;
            min_size = queue->size;
        }
    }
    
    return shortest;
}

void add_to_teller_queue(TellerQueue* queue, Customer* customer) {
    log_function_call("add_to_teller_queue");
    
    customer->assigned_queue = queue;
    customer->next = NULL;
    if (queue->head == NULL) {
        queue->head = customer;
        queue->tail = customer;
    } else {
        queue->tail->next = customer;
        queue->tail = customer;
    }
    queue->size++;
}

Customer* remove_from_teller_queue(TellerQueue* queue) {
    log_function_call("remove_from_teller_queue");
    
    if (queue->head == NULL) {
        return NULL;
    }
    
    Customer* customer = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->size--;
    return customer;
}

// Event actions
void customer_arrival_action(Event* event) {
    log_function_call("customer_arrival_action");
    Customer* customer = event->actor.customer;
    TellerQueue* queue = get_shortest_queue();
    add_to_teller_queue(queue, customer);
    
    if (queue->teller->is_available) {
        Event* service_event = (Event*)malloc(sizeof(Event));
        service_event->type = EVENT_TELLER_AVAILABLE;
        service_event->time = event->time;
        service_event->actor.teller = queue->teller;
        service_event->action = teller_available_action;
        add_event(service_event);
    }
}

void customer_departure_action(Event* event) {
    log_function_call("customer_departure_action");
    Customer* customer = event->actor.customer;
    customer->departure_time = event->time;
    
    float wait_time = customer->departure_time - customer->arrival_time;
    stats.total_wait_time += wait_time;
    stats.sum_squared_wait_times += wait_time * wait_time;
    if (wait_time > stats.max_wait_time) {
        stats.max_wait_time = wait_time;
    }
    stats.total_customers_served++;
    stats.total_service_time += customer->departure_time - customer->service_start_time;
    
    free(event);
}

void teller_available_action(Event* event) {
    log_function_call("teller_available_action");
    Teller* teller = event->actor.teller;
    TellerQueue* queue = teller->queue;
    
    // First try own queue
    Customer* customer = remove_from_teller_queue(queue);
    
    // If no customer in own queue and not single queue, try stealing from longest queue
    if (customer == NULL && queue != teller_queues) {
        TellerQueue* longest_queue = NULL;
        int max_size = 0;
        
        for (TellerQueue* q = teller_queues; q != NULL; q = q->next) {
            if (q->size > max_size) {
                max_size = q->size;
                longest_queue = q;
            }
        }
        
        if (longest_queue != NULL && longest_queue->size > 0) {
            customer = remove_from_teller_queue(longest_queue);
        }
    }
    
    if (customer != NULL) {
        float service_time = generate_service_time(average_service_time);
        customer->service_start_time = event->time;
        
        // If there was idle time before this customer, add it to statistics
        if (teller->is_available) {
            stats.total_idle_time += event->time - teller->last_service_end;
        }
        
        Event* departure_event = (Event*)malloc(sizeof(Event));
        departure_event->type = EVENT_CUSTOMER_DEPARTURE;
        departure_event->time = event->time + service_time;
        departure_event->actor.customer = customer;
        departure_event->action = customer_departure_action;
        add_event(departure_event);
        
        teller->total_service_time += service_time;
        teller->last_service_end = event->time + service_time;
        teller->is_available = 0;
        
        Event* next_available = (Event*)malloc(sizeof(Event));
        next_available->type = EVENT_TELLER_AVAILABLE;
        next_available->time = event->time + service_time;
        next_available->actor.teller = teller;
        next_available->action = teller_available_action;
        add_event(next_available);
    } else if (event->time < simulation_time) { // Only continue if within simulation time
        float idle_time = (float)(MIN_TELLER_IDLE_TIME + 
            rand() % (MAX_TELLER_IDLE_TIME - MIN_TELLER_IDLE_TIME + 1));
        teller->total_idle_time += idle_time;
        
        // Only create next event if we're still within simulation time
        if (event->time + idle_time < simulation_time) {
            Event* check_again = (Event*)malloc(sizeof(Event));
            check_again->type = EVENT_TELLER_AVAILABLE;
            check_again->time = event->time + idle_time;
            check_again->actor.teller = teller;
            check_again->action = teller_available_action;
            add_event(check_again);
        }
    }
    
    free(event);
}

// Run simulation
void run_simulation(int is_single_queue) {
    log_function_call("run_simulation");
    
    // Create teller queues
    if (is_single_queue) {
        teller_queues = (TellerQueue*)malloc(sizeof(TellerQueue));
        teller_queues->head = NULL;
        teller_queues->tail = NULL;
        teller_queues->size = 0;
        teller_queues->next = NULL;
        teller_queues->teller = &tellers[0];
        for (int i = 0; i < num_tellers; i++) {
            tellers[i].queue = teller_queues;
            // Create initial teller available events
            Event* teller_event = (Event*)malloc(sizeof(Event));
            teller_event->type = EVENT_TELLER_AVAILABLE;
            teller_event->time = 0;
            teller_event->actor.teller = &tellers[i];
            teller_event->action = teller_available_action;
            add_event(teller_event);
        }
    } else {
        TellerQueue* prev = NULL;
        for (int i = 0; i < num_tellers; i++) {
            TellerQueue* queue = (TellerQueue*)malloc(sizeof(TellerQueue));
            queue->head = NULL;
            queue->tail = NULL;
            queue->size = 0;
            queue->next = NULL;
            queue->teller = &tellers[i];
            tellers[i].queue = queue;
            
            if (prev == NULL) {
                teller_queues = queue;
            } else {
                prev->next = queue;
            }
            prev = queue;
            
            // Create initial teller available events
            Event* teller_event = (Event*)malloc(sizeof(Event));
            teller_event->type = EVENT_TELLER_AVAILABLE;
            teller_event->time = 0;
            teller_event->actor.teller = &tellers[i];
            teller_event->action = teller_available_action;
            add_event(teller_event);
        }
    }
    
    // Process events
    Event* event;
    while ((event = remove_next_event()) != NULL) {
        event->action(event);
    }
}

// Print statistics
void print_statistics(int is_single_queue) {
    log_function_call("print_statistics");
    
    printf("\nSimulation Results:\n");
    printf("Queue Type: %s\n", is_single_queue ? "Single Queue" : "Multiple Queues");
    printf("Number of Tellers: %d\n", num_tellers);
    printf("Total Customers Served: %d\n", stats.total_customers_served);
    printf("Total Simulation Time: %.2f minutes\n", simulation_time);
    
    float avg_wait_time = stats.total_wait_time / stats.total_customers_served;
    float variance = (stats.sum_squared_wait_times / stats.total_customers_served) - 
                    (avg_wait_time * avg_wait_time);
    float std_dev = sqrt(variance);
    
    printf("Average Wait Time: %.2f minutes\n", avg_wait_time);
    printf("Standard Deviation: %.2f minutes\n", std_dev);
    printf("Maximum Wait Time: %.2f minutes\n", stats.max_wait_time);
    printf("Total Service Time: %.2f minutes\n", stats.total_service_time);
    printf("Total Idle Time: %.2f minutes\n", stats.total_idle_time);
}

// Cleanup
void cleanup_simulation(void) {
    log_function_call("cleanup_simulation");
    
    // Free event queue and all events
    Event* current = event_queue->head;
    while (current != NULL) {
        Event* next = current->next;
        free(current);
        current = next;
    }
    free(event_queue);
    
    // Reset customer queues to avoid double-free
    for (TellerQueue* queue = teller_queues; queue != NULL; queue = queue->next) {
        queue->head = NULL;
        queue->tail = NULL;
        queue->size = 0;
    }
    
    // Free teller queues
    TellerQueue* current_queue = teller_queues;
    while (current_queue != NULL) {
        TellerQueue* next = current_queue->next;
        free(current_queue);
        current_queue = next;
    }
    
    // Free customers and tellers arrays
    free(customers);
    free(tellers);
    
    // Reset global pointers
    event_queue = NULL;
    teller_queues = NULL;
    customers = NULL;
    tellers = NULL;
}