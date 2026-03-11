// functions.cc

#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <bits/stdc++.h>


// Header files
#include "functions.h"
#include "classes.h"

std::vector<Router> initialiaze_routers(int noc_size,
                                        int buffer_size
){
    std::vector<Router> routers;

    for (int y = 0; y < noc_size; y++){
        for (int x = 0; x < noc_size; x++){
            routers.push_back(Router(x, y, buffer_size));
        }
    }

    // Printing the routers, for qualification
    // for (int i = 0; i < pow(noc_size, 2); i++) {
    //     std::cout << "Router " << i
    //          << " -> (x=" << routers[i].coord_x
    //          << ", y=" << routers[i].coord_y
    //          << "), buffer=" << routers[i].input_north
    //          << std::endl;
    // }
    return routers;
}


void packet_genarator(int noc_size,
                      float injection_rate,
                      std::vector<Router>& routers,
                      int step,
                      NoC& stats
){
    float random_num;

    for (int r = 0; r < pow(noc_size, 2); r++){
        random_num = float(rand()) / RAND_MAX;         // so the value will be in the range of [0,1]

        if(injection_rate > random_num){
            Router& src = routers[r];                  // Define source router
        
            // Random destination
            srand(time(0));
            int dst_index;
            do{
                dst_index = rand() % (noc_size * noc_size);
            }while(r == dst_index);

            Router& dst = routers[dst_index];

            // Create a packet and define cordinations for source and destination
            Packet pkt;
            pkt.source_x      = src.coord_x;
            pkt.source_y      = src.coord_y;
            pkt.dest_x        = dst.coord_x;
            pkt.dest_y        = dst.coord_y;
            pkt.hops          = abs(pkt.dest_x - pkt.source_x) + abs(pkt.dest_y - pkt.source_y);
            pkt.creation_time = step;

            stats.packets_generated++;

            src.input_buffer_local.push(pkt);
        }
    }
}

// --------------------------------------
// Input Local ----> Output (same Router)
// --------------------------------------

void process_first_hop(std::vector<Router>& routers,
                       int noc_size,
                       int step
){
    // Access one by one all the routers
    for (Router& r : routers) {

        if (r.input_buffer_local.empty()){ 
            continue;
        }

        Packet& pkt = r.input_buffer_local.front();

        int dx = pkt.dest_x - pkt.source_x;
        int dy = pkt.dest_y - pkt.source_y;

        bool moved = false;

        if (dx > 0) {
            if (r.output_east.size() < r.buffer_size) {               
                r.output_east.push(pkt);
                moved = true;
           }
        } 
        else if (dx < 0) {
            if (r.output_west.size() < r.buffer_size) {
                r.output_west.push(pkt);
                moved = true;
            }
        }
        else {   // (dx == 0)
            if (dy > 0) {
                if (r.output_north.size() < r.buffer_size) {
                    r.output_north.push(pkt);
                    moved = true;
                }
            }
            else if (dy < 0) {
                if (r.output_south.size() < r.buffer_size) {
                    r.output_south.push(pkt);
                    moved = true;
                }
            }
        }
        
        if (moved){
            r.input_buffer_local.pop();
        }
    }
}

// --------------------------------
// Input ----> Output (same Router)
// --------------------------------

void DOR_XY(Router& r,
            std::queue<Packet>& input_q,   // To reuse the function for all the routes
            int current_time,
            NoC& stats
){
    if (input_q.empty()){ 
        return;
    }

    Packet pkt = input_q.front();
    input_q.pop();

    int dx = pkt.dest_x - r.coord_x;
    int dy = pkt.dest_y - r.coord_y;

    // if current_router == destination_router
    if (dx == 0 && dy == 0) {
        int latency = current_time - pkt.creation_time;

        stats.packets_delivered++;
        stats.total_latency += latency;
        
        // TWRA TO PROSTHESA
        stats.total_hops += pkt.hops;

        if (latency >= stats.max_latency){
            stats.max_latency = latency;
        }

        return;
    }

    bool moved = false;

    // Axis x
    if (dx > 0) {
        if (r.output_east.size() < r.buffer_size) {
            r.output_east.push(pkt);
            moved = true;
        }
    }
    else if (dx < 0) {
        if (r.output_west.size() < r.buffer_size) {
            r.output_west.push(pkt);
            moved = true;
        }
    }

    // Axis y
    if (!moved && dx == 0) {

        if (dy > 0) {
            if (r.output_south.size() < r.buffer_size) {
                r.output_south.push(pkt);
                moved = true;
            }
        }
        else if (dy < 0) {
            if (r.output_north.size() < r.buffer_size) {
                r.output_north.push(pkt);
                moved = true;
            }
        }
    }

    // Stall one step if it didn't make a move
    if (!moved) {
        input_q.push(pkt);
        stats.router_latency++;
    }
}


void DOR_XY_Control(std::vector<Router>& routers,
                    int current_time,
                    NoC& stats
){
    for (Router& r : routers) {
                
        // North Input
        DOR_XY(r, r.input_north, current_time, stats); 
        
        // South Input
        DOR_XY(r, r.input_south, current_time, stats);
        
        // East Input
        DOR_XY(r, r.input_east, current_time, stats);
        
        // West Input
        DOR_XY(r, r.input_west, current_time, stats);
    }
}

// ----------------------------
// Output  ----> Neighbor Input 
// ----------------------------

void packet_transfer(std::vector<Router>& routers,
                     int noc_size,
                     int buffer_size
){
    // Iterate through all Routers
    for (Router& r : routers) {

        // Define a lambda
        auto get_router = [&] (int x, int y) -> Router* {  // Pointer to Router (return_type)
            return &routers[y * noc_size + x];
        };

        // East -> Neighbor West
        if (!r.output_east.empty()) {
            int neighbor_x = r.coord_x + 1;
            if (neighbor_x < noc_size) {                                    // x range is (0,noc_size)
                Router* neighbor = get_router(neighbor_x, r.coord_y);
                if (neighbor && neighbor->input_west.size() < neighbor->buffer_size) {
                    neighbor->input_west.push(r.output_east.front());
                    r.output_east.pop();
                }
            // Else nothing, packet should NOT delivered
            }
        }

        // West -> Neighbor East
        if (!r.output_west.empty()) {
            int neighbor_x = r.coord_x - 1;
            if (neighbor_x >= 0) {
                Router* neighbor = get_router(neighbor_x, r.coord_y);
                if (neighbor && neighbor->input_east.size() < neighbor->buffer_size) {
                    neighbor->input_east.push(r.output_west.front());
                    r.output_west.pop();
                }
            }
        }

        // North -> Neighbor South
        if (!r.output_north.empty()) {
            int neighbor_y = r.coord_y - 1;
            if (neighbor_y >= 0) {
                Router* neighbor = get_router(r.coord_x, neighbor_y);
                if (neighbor && neighbor->input_south.size() < neighbor->buffer_size) {
                    neighbor->input_south.push(r.output_north.front());
                    r.output_north.pop();
                }
            }
        }

        // South -> Neighbor North
        if (!r.output_south.empty()) {
            int neighbor_y = r.coord_y + 1;
            if (neighbor_y < noc_size) {
                Router* neighbor = get_router(r.coord_x, neighbor_y);
                if (neighbor && neighbor->input_north.size() < neighbor->buffer_size) {
                    neighbor->input_north.push(r.output_south.front());
                    r.output_south.pop();
                }
            }
        }
    }
}


void print_results(const NoC& stats) {
    
    // Mean latency
    float avg_latency        = 0.0;
    float avg_hops           = 0.0;
    float avg_router_latency = 0.0;
    float zero_latency       = 0.0;

    float theoretical_avg_hops = 0.0;

    if (stats.packets_delivered > 0) {
        avg_latency        = (float)stats.total_latency  / stats.packets_delivered;
        avg_hops           = (float)stats.total_hops     / stats.packets_delivered;   
        avg_router_latency = (float)stats.router_latency / stats.packets_generated;
        
        theoretical_avg_hops = (float)(stats.size + stats.size)/3.0;

        float tr = 3.0;                     // Assumption: Router requires 3 cycles to process a packet
        float tc = theoretical_avg_hops;    // 1 κύκλος per hop
        float serialization = 0.0; 
        
        if (stats.bandwidth > 0)
            serialization = (float)stats.packet_length / stats.bandwidth;

        zero_latency = (theoretical_avg_hops * tr) + tc + serialization;
    }
    
    std::cout << "\n=================================================\n";
    std::cout << "              SIMULATION RESULTS \n";
    std::cout << "=================================================\n";
    std::cout << "-> Packets Generated: " << stats.packets_generated << "\n";
    std::cout << "-> Packets Delivered: " << stats.packets_delivered << "\n";
    std::cout << "-> Max Latency: " << stats.max_latency << " steps\n";
    std::cout << "-> Avg Latency: " << avg_latency << " steps\n";
    std::cout << "-> Average hops: " << avg_hops << " hops\n";
    std::cout << "=================================================\n";
    std::cout << "Theoretical average hops : " << theoretical_avg_hops << "\n";
    std::cout << "-> Zero latency is : " << zero_latency << " steps\n";
    std::cout << "=================================================\n";
}