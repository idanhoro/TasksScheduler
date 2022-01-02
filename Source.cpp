#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <queue>
#include <thread>
#include <chrono>

#include "croncpp.h" // Taken from https://github.com/mariusbancila/croncpp for cron pattern


struct Task {
    std::string command;
    cron::cronexpr cron_expression;
    std::time_t next_execution;

    bool operator<(Task const &other_task) {
        return next_execution < other_task.next_execution;
    } // Used in order to sort the queue.

    friend std::ostream &operator<<(std::ostream &os, const Task &task) {
        return os << "The task: " << task.command << "\n" << " execute at: "
                  << std::put_time(localtime(&task.next_execution), "%FT%H:%M");
    };
};

int Start();

void ParseTasksCSV(std::deque <Task> &, std::string &);

Task CreateTask(std::string &, std::string &, std::string &, std::string &);

void ExecuteTasks(std::deque <Task> &);

void ExecuteTask(std::string);

const char *const DAYS[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};

int main() {
    return Start();
}

/**
 * Runs TasksScheduler.
 */
int Start() {
    auto now{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
    std::string file_name = "./tasks.csv";
    std::deque <Task> tasks_queue;

    std::cout << "The scheduler started to run at: " << std::put_time(localtime(&now), "%FT%H:%M") << std::endl;
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
void ParseTasksCSV(std::deque <Task> &tasks_queue, std::string &file_name) {
    std::ifstream tasks_csv(file_name);
    if (!tasks_csv.is_open()) {
        std::cerr << "ERROR: Could not open file" << '\n';
        exit(1);
    }
    std::string minute, hour, day_of_week, command;
    getline(tasks_csv, minute, '\n'); // Ignores the first row (CSV title) 

    while (tasks_csv.peek() != EOF) {
        getline(tasks_csv, minute, ',');
        getline(tasks_csv, hour, ',');
        getline(tasks_csv, day_of_week, ',');
        getline(tasks_csv, command, '\n');
        tasks_queue.push_front(CreateTask(minute, hour, day_of_week, command));
    }

    tasks_csv.close();
    std::sort(std::begin(tasks_queue), std::end(tasks_queue));
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
Task CreateTask(std::string &minute, std::string &hour, std::string &day_of_week, std::string &command) {
    std::string cron_str = "* " + minute + " " + hour + " * * " + DAYS[stoi(day_of_week)];
    // cron expression representing: at H:M AM, only on (Day of week)
    auto cron = cron::make_cron(cron_str);
    Task task{command, cron, cron::cron_next(cron, std::time(0))};

    return task;
}

/**
 * Execute the given command as system call.
 *
 * @param command - The command
 */
void ExecuteTask(std::string command) {
    system(command.c_str());
    auto now{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
    std::cout << "The task: " << command << "\n" << " execute at: "
    << std::put_time(localtime(&now), "%FT%H:%M") << std::endl;

}

/**
 * Fetches tasks from the task queue and runs them according to their execution time
 * The tasks are then returned to the bottom of the queue in order to create a never-ending
 * cycle of tasks (Possible due to the fact that they all run once a week)
 *
 * @param tasks_queue - A queue full of tasks waiting to be executed.
 */
void ExecuteTasks(std::deque <Task> &tasks_queue) {
    std::vector <std::thread> workers;

    while (!tasks_queue.empty()) {
        Task current_task = tasks_queue.front();
        Task next_task = current_task;

        std::cout << "The next task will run at: "
        << std::put_time(localtime(&next_task.next_execution), "%FT%H:%M") << std::endl;

        next_task.next_execution = cron::cron_next(next_task.cron_expression, next_task.next_execution);
        std::this_thread::sleep_until(std::chrono::system_clock::from_time_t(current_task.next_execution));
        //The time that thread should wait until it's scheduled time.

        workers.push_back(std::thread(ExecuteTask, current_task.command));
        tasks_queue.push_back(next_task);
        tasks_queue.pop_front();
    }

}
