#ifndef FUNTIONS_H
#define FUNCTIONS_H

#include <vector>
#include "classes.h"


std::vector<Router> initialiaze_routers(int noc_size,
                                        int size_buffer
);


void packet_genarator(int noc_size,
                      float injection_rate,
                      std::vector<Router>& routers,
                      int step,
                      NoC& stats
);


void process_first_hop(std::vector<Router>& routers,
                       int noc_size,
                       int step
);


void DOR_XY(Router& r,
            std::queue<Packet>& input_q,   // To recall it for all routers
            int current_time,
            NoC& stats
);


void DOR_XY_Control(std::vector<Router>& routers,
                    int current_time,
                    NoC& stats
);


void packet_transfer(std::vector<Router>& routers,
                     int noc_size,
                     int buffer_size
);


void print_results(const NoC& stats);

#endif