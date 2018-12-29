//
// Created by liork on 8/22/18.
//

#ifndef SCALE_MATRIXMEASUREMENT_H
#define SCALE_MATRIXMEASUREMENT_H

#include <string>
#include <map>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "Timer.h"


using namespace std;
using namespace std::chrono;

// Here we modify the original implementation
// to support multithreading

class MatrixMeasurement
{
private:
    string getcwdStr() const
    {
        char buff[256];
        auto res = getcwd(buff, 255);
        assert(res != NULL);
        return std::string(buff);
    }

    // for each thread keep a set of timers
    vector<map<string, Timer> > timers;

    vector<string> m_tasksNames;
    string m_arguments = "";
    const int k_max_threads;

public:
    MatrixMeasurement(size_t argc, char* argv[], vector<string> tasksNames):
        m_tasksNames(tasksNames), k_max_threads(1000)
    {
        // SCALE has a max of 1000 threads
        timers.resize(k_max_threads);

        for(size_t idx = 0; idx < argc; ++idx)
        {
            string s(argv[idx]);
            if (idx < argc - 1)
                m_arguments += s + "*";
            else
                m_arguments += s;
        }

    }

    void startSubTask(string taskName, int thread_no)
    {
        timers[thread_no][taskName].start();
    }

    void endSubTask(string taskName, int thread_no)
    {
        timers[thread_no][taskName].stop();
    }

    void write_log()
    {
        string logFileName = getcwdStr() + "/MATRIX/logs/" + m_arguments + ".log";
        ofstream logFile(logFileName);
        if (!logFile.is_open())
        {
            cerr << "MatrixMeasurement: Could not open log file '"
                 << logFileName
                 << "'\n";
            return;
        }
        //write to file
        for (const string& taskName: m_tasksNames)
        {
            logFile << taskName + ":";
            cout << "taskName : " << taskName << endl;

            double cur_time = 0;
            double total_time = 0;
            int num_threads_per_task = 0;

            for (int thread_no = 0; thread_no < k_max_threads; ++thread_no)
            {
                // get timings in milliseconds
                double cur_time = timers[thread_no][taskName].elapsed() * 1000;
                if (cur_time != 0) {
                    total_time += cur_time;
                    num_threads_per_task += 1;
                }
            }
            double thread_average = total_time / num_threads_per_task;
            cout << "value : " << thread_average << endl;
            logFile << to_string(thread_average) << ",";
            logFile << "\n";
        }
        logFile.close();
    }
};

#endif //ABY_MATRIXMEASUREMENT_H
