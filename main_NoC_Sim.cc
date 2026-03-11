// Header files
#include "functions.h"
#include "classes.h"

int main() {

    NoC simulation;
    simulation.size            = 8;       // (nxn)
    simulation.time_simulation = 1000;    // packets/step rate
    simulation.injection_rate  = 0.6;     // total steps
    simulation.buffer_size     = 10;      // buffer packet capacity
    // Zero-latency variables
    simulation.packet_length   = 1;       // bits
    simulation.bandwidth       = 1;       // bits/sec

    // Initialiaze routers
    std::vector<Router> routers = initialiaze_routers(
        simulation.size, 
        simulation.buffer_size
    );
    
    int current_step = 0; 

    // Loop Simulation (we control the steps also)
    while (current_step < simulation.time_simulation) {
        
        packet_genarator(
            simulation.size, 
            simulation.injection_rate, 
            routers, 
            current_step, 
            simulation
        );

        process_first_hop(routers, simulation.size, current_step);
        
        DOR_XY_Control(routers, current_step, simulation); 

        packet_transfer(routers, simulation.size, simulation.buffer_size);
        
        current_step++;
    }
    
    // Print outputs
    print_results(simulation);
    
    return 0;
}