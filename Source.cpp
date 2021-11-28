#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <queue>
#include <thread>
#include <chrono>
#include <future>

#include "croncpp.h"



typedef struct Task {
    std::string command;
    cron::cronexpr cron_expression;
    std::time_t next_execution;

    bool operator<(Task const& other_task) { return next_execution < other_task.next_execution; }
    friend std::ostream& operator<<(std::ostream& os, const Task& task) {
        return os << "Next task is:" << task.command << " at: " << std::put_time(localtime(&task.next_execution), "%FT%H:%M");
    };
};

int Start();
void ParseTasksCSV(std::deque<Task>&, std::string&);
Task CreateTask(std::string&, std::string&, std::string&, std::string&);
void ExecuteTasks(std::deque<Task>&);
void ExecuteTask(std::string);

int main() {
    return Start();
}

int Start() {
    std::string file_name = "./tasks.csv";
    std::deque<Task> tasks_queue;
    std::cout << "The scheduler started to run : " << std::endl;
    ParseTasksCSV(tasks_queue, file_name);
    ExecuteTasks(tasks_queue);
    return 0;
}
/**
 * Parse the CSV file and fill the queue of tasks.

 * 
 * @param tasks_queue - An empty queue that stores the tasks.
 * @param file_name - A string variable that stores the file name.
 */
void ParseTasksCSV(std::deque<Task>& tasks_queue, std::string& file_name) {
    std::ifstream tasks(file_name);
    if (!tasks.is_open()) std::cerr << "ERROR: File Open or not exist" << '\n';

    std::string minute;
    std::string hour;
    std::string day_of_week;
    std::string command;
    getline(tasks, minute, '\n');

    while (tasks.peek() != EOF) {

        getline(tasks, minute, ',');
        getline(tasks, hour, ',');
        getline(tasks, day_of_week, ',');
        getline(tasks, command, '\n');
        tasks_queue.push_front(CreateTask(minute, hour, day_of_week, command));
    }
    std::sort(std::begin(tasks_queue), std::end(tasks_queue));
    tasks.close();
}
/**
 *Calculate the next execute time base on the cron and current time,  
 *then create task based on varibles sent from the creator 
 * and the calculation result.
 
 * @param minute - A string variable that stores the minute that task should execute at.
 * @param hour - A string variable that stores the hour that task should execute.
 * @param day_of_week - A string variable that stores the day of the week that  task should execute.
 * @param command - A string variable that stores the command the task should execute. 
 * @return task - A struct of task variable (has explained above) that stores all needed for task struct.
 */
Task CreateTask(std::string& minute, std::string& hour, std::string& day_of_week, std::string& command) {
    std::string cron_str = "* " + minute + " " + hour + " * * " + day_of_week;
    auto cron = cron::make_cron(cron_str);
    Task task{ command, cron, cron::cron_next(cron, std::time(0)) };
    return task;
}

void ExecuteTask(std::string command) {
    system((command).c_str());
    auto now{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
    std::cout << command << " has been executed at: " << std::put_time(localtime(&now), "%FT%H:%M") << std::endl;
    
}

void ExecuteTasks(std::deque<Task>& tasks_queue) {
    std::vector<std::thread> workers;


    while (!tasks_queue.empty()) {
        
        Task current_task = tasks_queue.front();
        std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(current_task.next_execution));
        
        workers.push_back(std::thread(ExecuteTask, current_task.command));
        tasks_queue.push_back(current_task);
        tasks_queue.pop_front();
    }
    
}
