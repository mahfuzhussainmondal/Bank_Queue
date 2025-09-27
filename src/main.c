#include "../include/qsim.h"

int main(int argc, char* argv[]) {
    // Check if we have enough arguments
    if (argc != 5) {
        // Show usage and example
        printf("Usage: %s <num_customers> <num_tellers> <simulation_time> <avg_service_time>\n", argv[0]);
        printf("Example: %s 100 4 60 2.3\n", argv[0]);
        printf("Note: time is in minutes!\n");  // Added this after confusion in testing
        return 1;
    }
    
    int num_customers = atoi(argv[1]);
    int num_tellers = atoi(argv[2]);
    float simulation_time = atof(argv[3]);
    float avg_service_time = atof(argv[4]);
    
    if (num_customers <= 0 || num_tellers <= 0 || simulation_time <= 0 || avg_service_time <= 0) {
        printf("Error: All parameters must be positive numbers\n");
        return 1;
    }
    
    // Run simulation with single queue
    printf("\nRunning simulation with single queue...\n");
    initialize_simulation(num_customers, num_tellers, simulation_time, avg_service_time);
    run_simulation(1);
    print_statistics(1);
    cleanup_simulation();
    
    // Run simulation with multiple queues
    printf("\nRunning simulation with multiple queues...\n");
    initialize_simulation(num_customers, num_tellers, simulation_time, avg_service_time);
    run_simulation(0);
    print_statistics(0);
    cleanup_simulation();
    
    return 0;
}