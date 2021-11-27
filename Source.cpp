#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <queue>
#include <thread>
#include <chrono>

#include "croncpp.h"


typedef struct Task {
    std::string command;
    cron::cronexpr cron_expression;
    std::time_t next_run;

    bool operator<(Task const& other_task) { return next_run < other_task.next_run; }
    friend std::ostream& operator<<(std::ostream& os, const Task& task) {
        return os << "Next task is:" << task.command << " at: " << std::put_time(localtime(&task.next_run), "%FT%H:%M");
    };
};

void ParseCSVToDequeTasks(std::deque<Task>&, std::string&);
Task CreateTask(std::string&, std::string&, std::string&, std::string&);
void ExecuteTasks(std::deque<Task>&);
void ExecuteTask(std::string);

int main() {
    std::string file_name = "./tasks.csv";
    std::deque<Task> tasks_queue;
    std::cout << "Let's go" << std::endl;
    ParseCSVToDequeTasks(tasks_queue, file_name);
    ExecuteTasks(tasks_queue);
}


void ParseCSVToDequeTasks(std::deque<Task>& tasks_queue, std::string& file_name) {
    std::ifstream tasks(file_name);
    if (!tasks.is_open()) std::cerr << "ERROR: File Open" << '\n';

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
        std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(current_task.next_run));

        workers.push_back(std::thread(ExecuteTask, current_task.command));
        tasks_queue.push_back(current_task);
        tasks_queue.pop_front();
    }
    for (std::thread& t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }
}
