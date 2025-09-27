# Bank Queue Simulation Project

This project simulates a bank queueing system to compare single-line vs multiple-line efficiency. I created this for my Data Structures assignment and learned a lot about event-driven programming!

## Why This Matters

Ever wondered why some banks have one line while others have multiple lines? After working on this simulation, I found that it really depends on the situation. Sometimes multiple lines work better (especially when tellers can help each other out), which wasn't what I expected!

## Project Structure

-> bin/           # Compiled binaries
-> include/       # Header files
-> output/        # Simulation output files
-> src/          # Source code files


## Building the Project

To build the project, run:

```bash
make clean
make
```

The executable will be created in the `bin` directory.

## Running the Simulation

The program takes four command-line arguments:
```bash
./qSim <num_customers> <num_tellers> <simulation_time> <avg_service_time>
```

Example:
```bash
./qSim 100 4 60 2.3
```

This will simulate:
- 100 customers
- 4 tellers
- 60 minutes simulation time
- 2.3 minutes average service time

## Test Cases

Here are three recommended test cases:

1. Low traffic:
```bash
./qSim 50 5 60 2.0
```

2. Medium traffic:
```bash
./qSim 100 4 60 2.3
```

3. High traffic:
```bash
./qSim 200 3 60 2.5
```

## Analysis

The simulation compares two queuing strategies:
1. Single Queue: All customers wait in one line and are served by the next available teller
2. Multiple Queues: Each teller has their own line, and customers choose the shortest line

Generally:
- Single queue system is more fair as it follows strict first-come-first-served order
- Single queue system typically results in lower average wait times
- Multiple queue system may have more variance in wait times due to "lucky" or "unlucky" line choices
- Multiple queue system may be more space-efficient in the physical layout of the bank

The program logs every function pointer call to demonstrate their usage throughout the simulation.

## Implementation Details

The program uses:
- Event-driven simulation architecture
- Function pointers for event handling
- Linked lists for queue implementation
- Random number generation for arrival and service times
- Statistics collection and analysis

## Author
Mahfuz Hussain Mondal