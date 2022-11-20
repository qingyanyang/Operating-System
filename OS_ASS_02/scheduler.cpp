// a1781257, Yanlin Du
// a1846838, Lize Chen
// a1780968, Chang Liu
// a1865304, Qingyan Yang
// Trump
/*
created by Andrey Kan
andrey.kan@adelaide.edu.auz
2021
*/
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <iterator>

// std is a namespace: https://www.cplusplus.com/doc/oldtutorial/namespaces/
//const int TIME_ALLOWANCE = 250;// allow to use up to this number of time slots at once
const int TRAIL_TIME_ALLOWANCE = 1;
const int PRINT_LOG = 0; // print detailed execution trace

class Customer
{
public:
    std::string name;
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int playing_since;
    int waiting_time;

    Customer(std::string par_name, int par_priority, int par_arrival_time, int par_slots_remaining)
    {
        name = par_name;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        playing_since = -1;
        waiting_time = 0;
    }
};

class Event
{
public:
    int event_time;
    int customer_id;  // each event involes exactly one customer

    Event(int par_event_time, int par_customer_id)
    {
        event_time = par_event_time;
        customer_id = par_customer_id;
    }
};

void initialize_system(
        std::ifstream &in_file,
        std::deque<Event> &arrival_events,
        std::vector<Customer> &customers)
{
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        Customer customer_from_file(name, priority, arrival_time, slots_requested);
        customers.push_back(customer_from_file);

        // new customer arrival event
        Event arrival_event(arrival_time, customer_id);
        arrival_events.push_back(arrival_event);

        customer_id++;
    }
}

void print_state(
        std::ofstream &out_file,
        int current_time,
        int current_id,
        const std::deque<Event> &arrival_events,
        const std::deque<int> &customer_queue0,
        const std::deque<int> &customer_queue1,
        const std::deque<int> &queue_trail
        )
{
    out_file << current_time << " " << current_id << '\n';
    if (PRINT_LOG == 0)
    {
        return;
    }
    std::cout << current_time << ", " << current_id << '\n';
    for (int i = 0; i < arrival_events.size(); i++)
    {
        std::cout << "\t" << arrival_events[i].event_time << ", " << arrival_events[i].customer_id << ", ";
    }
    std::cout << '\n';
    for (int i = 0; i < customer_queue0.size(); i++)
    {
        std::cout << "\t" << customer_queue0[i] << ", ";
    }
    std::cout << '\n';
    for (int i = 0; i < customer_queue1.size(); i++)
    {
        std::cout << "\t" << customer_queue1[i] << ", ";
    }
    std::cout << '\n';
    for (int i = 0; i < queue_trail.size(); i++)
    {
        std::cout << "\t" << queue_trail[i] << ", ";
    }
    std::cout << '\n';
}
//sort method
void sort_by_bust_time(std::deque<int> &customer_queue, std::vector<Customer> customers)
{
    int size = (int)customer_queue.size();
    int arr[size];
    for(int k = 0; k < size; k++){
        int customer_id = customer_queue.front();
        customer_queue.pop_front();
        arr[k]= customer_id;
    }
    for(int i = 0; i < size-1; i++){
        int count = 0;
        for(int j = 0; j < size-i-1; j++){
            //if(customers[arr[j]].arrival_time - customers[arr[j+1]].arrival_time >=-60){
                if(customers[arr[j]].slots_remaining > customers[arr[j+1]].slots_remaining){
                    int temp = arr[j];
                    arr[j] = arr[j+1];
                    arr[j+1] = temp;
                    count = 1;
                }
            //}
        }
        if(count == 0){
            break;
        }
    }
    for(int l = 0; l < size; l++){
        customer_queue.push_back(arr[l]);
    }
}
void sort_by_priority(std::deque<int> &customer_queue, std::vector<Customer> customers)
{
    int size = (int)customer_queue.size();
    int arr[size];
    for(int k = 0; k < size; k++){
        int customer_id = customer_queue.front();
        customer_queue.pop_front();
        arr[k]= customer_id;
    }
    for(int i = 0; i < size-1; i++){
        int count = 0;
        for(int j = 0; j < size-i-1; j++){
            if(customers[arr[j]].arrival_time == customers[arr[j+1]].arrival_time){
                if(customers[arr[j]].priority > customers[arr[j+1]].priority){
                    int temp = arr[j];
                    arr[j] = arr[j+1];
                    arr[j+1] = temp;
                    count = 1;
                }
            }
        }
        if(count == 0){
            break;
        }
    }
    for(int l = 0; l < size; l++){
        customer_queue.push_back(arr[l]);
    }
}


// process command line arguments
// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Provide input and output file names." << std::endl;
        return -1;
    }
    std::ifstream in_file(argv[1]);
    std::ofstream out_file(argv[2]);
    if ((!in_file) || (!out_file))
    {
        std::cerr << "Cannot open one of the files." << std::endl;
        return -1;
    }

    // deque: https://www.geeksforgeeks.org/deque-cpp-stl/
    // vector: https://www.geeksforgeeks.org/vector-in-cpp-stl/
    std::deque<Event> arrival_events; // new customer arrivals
    std::vector<Customer> customers; // information about each customer

    // read information from file, initialize events queue
    initialize_system(in_file, arrival_events, customers);

    int current_id = -1; // who is using the machine now, -1 means nobody
    int time_out = -1; // time when current customer will be preempted
    std::deque<int> queue0; // member waiting queue
    std::deque<int> queue1; // nun-member waiting queue
    std::deque<int> queue_trail; //every customer can have a try when they arrive
    // step by step simulation of each time slot
    bool all_done = false;

    for (int current_time = 0; !all_done; current_time++)
    {
        // welcome newly arrived customers
        while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
        {
            queue_trail.push_back(arrival_events[0].customer_id);
            arrival_events.pop_front();
        }
        sort_by_priority(queue_trail, customers);
        // check if we need to take a customer off the machine
        if (current_id >= 0)
        {
            if (current_time == time_out)
            {
                int last_run = current_time - customers[current_id].playing_since;
                customers[current_id].slots_remaining -= last_run;
                if (customers[current_id].slots_remaining > 0)
                {
                    // customer is not done yet, waiting for the next chance to play
                    if(customers[current_id].priority == 0){
                        queue0.push_back(current_id);
                        sort_by_bust_time(queue0,customers);
                    }
                    else
                    {
                        queue1.push_back(current_id);
                        sort_by_bust_time(queue1,customers);
                    }
                    customers[current_id].waiting_time = 0;//the waiting time of this customer is cleared
                }
                current_id = -1; // the machine is free now
            }
            else if(current_time < time_out)
            {
                if (!queue_trail.empty())
                {
                    int last_run = current_time - customers[current_id].playing_since;
                    int temp_slots_remaining = customers[current_id].slots_remaining - last_run;
                    if(customers[queue_trail.front()].slots_remaining < temp_slots_remaining)
                    {
                        customers[current_id].slots_remaining = temp_slots_remaining;
                        int temp = current_id;
                        if(customers[temp].priority == 0)
                        {
                            queue0.push_back(temp);
                            sort_by_bust_time(queue0,customers);
                        }
                        else
                        {
                            queue1.push_back(temp);
                            sort_by_bust_time(queue1,customers);
                        }
                        customers[temp].waiting_time = 0;//the waiting time of this customer is cleared
                        current_id = queue_trail.front();
                        queue_trail.pop_front();
                        time_out = current_time + customers[current_id].slots_remaining;
                        customers[current_id].playing_since = current_time;
                    }
                }
            }
        }
        // if machine is empty, schedule a new customer
        if (current_id == -1)
        {
            if (!queue_trail.empty() || !queue0.empty() || !queue1.empty()) // is anyone waiting?
            {
                if(!queue_trail.empty())
                {
                    current_id = queue_trail.front();
                    queue_trail.pop_front();
                    if (TRAIL_TIME_ALLOWANCE > customers[current_id].slots_remaining)
                    {
                        time_out = current_time + customers[current_id].slots_remaining;
                    }
                    else
                    {
                        time_out = current_time + TRAIL_TIME_ALLOWANCE;
                    }
                    customers[current_id].playing_since = current_time;
                }
                else
                {
                    bool insert = false;
                    if(!queue1.empty()){
                        auto iter = queue1.begin();
                        int maxIndex=0;
                        for (int i = 0; i < (int)queue1.size()-1; i++)
                        {
                            if (customers[queue1[maxIndex]].waiting_time < customers[queue1[i+1]].waiting_time)
                            {
                                maxIndex = i+1;
                            }
                        }
//                        if(customers[queue1[maxIndex]].waiting_time >= 380){
//                            customers[queue1[maxIndex]].priority = 0;
//                            current_id = queue1[maxIndex];
//                            insert = true;
//                            queue1.erase(iter + maxIndex);
//                        }
                    }
                    if(!insert)
                    {
                        if(!queue0.empty())
                        {
                            current_id = queue0.front();
                            queue0.pop_front();
                        }
                        else
                        {
                            current_id = queue1.front();
                            queue1.pop_front();
                        }
                    }
//                    if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
//                    {
//                        time_out = current_time + customers[current_id].slots_remaining;
//                    }
//                    else
//                    {
//                        time_out = current_time + TIME_ALLOWANCE;
//                    }
                    time_out = current_time + customers[current_id].slots_remaining;
                    customers[current_id].playing_since = current_time;
                }
            }
        }

        for (int i = 0; i < (int)queue1.size(); i++)
        {
            customers[queue1[i]].waiting_time++;
        }
        for (int i = 0; i < (int)queue0.size(); i++)
        {
            customers[queue0[i]].waiting_time++;
        }
        print_state(out_file, current_time, current_id, arrival_events, queue0, queue1, queue_trail);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        all_done = (arrival_events.empty() && queue0.empty() && queue1.empty() && queue_trail.empty() && (current_id == -1));
    }

    return 0;
}
