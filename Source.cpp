#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <queue>
#include <thread>
#include <chrono>
#include <future>

#include "croncpp.h" // Taken from https://github.com/mariusbancila/croncpp for cron pattern


typedef struct Task {
    std::string command;
    cron::cronexpr cron_expression;
    std::time_t next_execution;

    bool operator<(Task const& other_task) { return next_execution < other_task.next_execution; }
    friend std::ostream& operator<<(std::ostream& os, const Task& task) {
        return os << "The task: " << task.command << "\n" << " execute at: " << std::put_time(localtime(&task.next_execution), "%FT%H:%M");
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

/**
 * Loading the CSV file and and initialize the queue of tasks,
 * then calls functions in chronological order.
 */
int Start() {
    std::string file_name = "./tasks.csv";
    std::deque<Task> tasks_queue;
    std::cout << "The scheduler started to run: " << std::endl;
    ParseTasksCSV(tasks_queue, file_name);
    ExecuteTasks(tasks_queue);
    return 0;
}

/**
 * Parse the CSV file and populate the tasks queue.

 * 
 * @param tasks_queue - An empty queue which stores the tasks.
 * @param file_name - The path to the tasks CSV.
 */
void ParseTasksCSV(std::deque<Task>& tasks_queue, std::string& file_name) {
    std::ifstream tasks_csv(file_name);
    if (!tasks_csv.is_open()) std::cerr << "ERROR: Could not open file" << '\n'; // TODO: Add exit

    std::string minute, hour, day_of_week, command;
    getline(tasks_csv, minute, '\n'); // Ignores the first row (CSV title) 

    while (tasks_csv.peek() != EOF) {
        getline(tasks_csv, minute, ',');
        getline(tasks_csv, hour, ',');
        getline(tasks_csv, day_of_week, ',');
        getline(tasks_csv, command, '\n');
        tasks_queue.push_front(CreateTask(minute, hour, day_of_week, command));
    }

    std::sort(std::begin(tasks_queue), std::end(tasks_queue));
    tasks_csv.close();
}
/**
 * Returns a Task object, built from received parameters and the calculated next execution time.
 * 
 * @param minute - Minutes section of the execution time.
 * @param hour - Hours section of the execution time.
 * @param day_of_week - Day of the week section of the execution date.
 * @param command - The command which will be executed.
 * @return task - The constructed task
 */
Task CreateTask(std::string& minute, std::string& hour, std::string& day_of_week, std::string& command) {
    std::string cron_str = "* " + minute + " " + hour + " * * " + day_of_week; // cron expression represnting: at H:M AM, only on (Day of week)
    auto cron = cron::make_cron(cron_str);
    Task task{ command, cron, cron::cron_next(cron, std::time(0)) };
    return task;
}
/**
 * Execute the given command as system call.

 *
 * @param command - The command
 */
void ExecuteTask(std::string command) {
    system(command.c_str());
    auto now{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
    std::cout << "The task: " << command << "\n" << " execute at: " << std::put_time(localtime(&now), "%FT%H:%M") << std::endl;
    
}
/**
 * Pulls tasks while the queue is not empty,
 * changing the thread to sleep until his exection time,
 * executing the tasks in diffrent threads,
 * then pushes the task to the end of the queue to create circle of tasks.
 *
 * @param tasks_queue - A queue full of tasks waiting to be executed.
 */
void ExecuteTasks(std::deque<Task>& tasks_queue) {
    std::vector<std::thread> workers;

    while (!tasks_queue.empty()) {
        Task current_task = tasks_queue.front();
        std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(current_task.next_execution)); //The time that thread should wait until it's scheduled time.

        workers.push_back(std::thread(ExecuteTask, current_task.command));
        tasks_queue.push_back(current_task);
        tasks_queue.pop_front();
    }
    
}
