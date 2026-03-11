// classes.h
#ifndef CLASSES_H
#define CLASSES_H

#include <vector>
#include <queue>

class NoC{
public:
    int size;                // (size)x(size) Nodes
    float injection_rate;    // packets/step
    int time_simulation;     // total steps
    int buffer_size;         // buffer packet capacity

    // Stats
    long int packets_generated = 0;
    long int packets_delivered = 0;
    int total_latency          = 0;
    int max_latency            = 0;
    
    // Zero-latency, page 103 from slides 
    int total_hops             = 0;    // hops. Also in our simulation network_latency(tc) = hops
    int router_latency         = 0;    // tr
    // Let the user decide
    int packet_length          = 0;    // Lp (bits)
    int bandwidth              = 0;    // b  (bits/sec)  Serialization latency : Lp/b
};


class Packet{
public:
    int source_x;
    int source_y;
    int dest_x;
    int dest_y;
    int creation_time;
    int hops;
};


class Router{
private:
    
public:
    int coord_x;
    int coord_y;
    const int buffer_size;
    
    // Input Buffers
    std::queue<Packet> input_north;
    std::queue<Packet> input_south;
    std::queue<Packet> input_east;
    std::queue<Packet> input_west;

    // Output Buffers
    std::queue<Packet> output_north;
    std::queue<Packet> output_south;
    std::queue<Packet> output_east;
    std::queue<Packet> output_west;
    
    std::queue<Packet> input_buffer_local;          // infinite size

    // Default Constructor
    Router() : coord_x(0), coord_y(0), buffer_size(0) {}

    // Base Constructor
    Router(int x, int y, int buffer_size);
};

#endif