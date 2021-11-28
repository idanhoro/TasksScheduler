# TaskScheduler
---
SaskScheduler is a system which loads runs scheduled tasks by using a cron-like pattern

## Description
---
TasksScheduler automatically executes scheduled tasks.
The system loads tasks from a CSV file and executes them automatically when scheduled.
The tasks are executed sequentially and also supports simultaneously executions by using a multithreaded architecture.

#### Task - 
```c++
typedef struct Task {
    std::string command;
    cron::cronexpr cron_expression;
    std::time_t next_execution;
};
```

The CSV file has 4 columns which represent the command and the time in which it needs to be executed.

Value range:
- Minute: 0-59
- Hour: 0-23
- Day of week: 0-6, where 0 = Monday, 6 = Sunday
- Command: text

#### CSV file structure: 
minute | hour | day_of_week  |  command
------ | ---- |------------  | -------------
24     | 4    | 2            | task1
57     | 17   | 5            | task2
11     | 20   | 0            | task3

## Example

---

#### The time at this run - Sunday 18:34 for CSV the example

#### Task  4 and 5 should execute.
![CSV_example](./images/CSV_example.png)
#### Scheduler start:
![scheduler_start](./images/scheduler_start.png) 
#### Task 4 started and changed to sleep for 2 minutes:
![task4_start_and_sleep](./images/task4_start_and_sleep.png)
#### Task 5 arrive and execute before task 4: 
![task5_start_and_execute](./images/task5_start_and_execute.png)
#### Task 4 finish his sleep time and executed:
![task4_execute](./images/task4_execute.png)

# Installation:
This project was written using Microsoft Visual Studio 2019 and C++14, other versions may not work.
```
gcc source.cpp -o TasksScheduler.exe
```
Then run:
```
TasksScheduler.exe
```

## Credits:
`croncpp.h`: https://github.com/mariusbancila/croncpp