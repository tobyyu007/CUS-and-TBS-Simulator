//
//  main.cpp
//  CUS and TBS Simulator
//
//  Created by Toby on 2019/12/25.
//  Copyright © 2019 Toby. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <math.h>
#include <limits.h>
using namespace std;

struct PTask
{
    int TID;  // Task ID
    float Phase; // 抵達時間
    float Period;  // 週期
    float WCET; // Worst-case execution time
    float RDeadline; // Relative Deadline
};

struct APTask
{
    int TID;  // Task ID
    float Phase; // 抵達時間
    float Period;  // 週期
    float WCET; // Worst-case execution time
};

struct PJob
{
    int release_time;
    int remain_execution_time;
    int absolute_deadline;
    int TID; //屬於哪個Task的工作
    int Period;
};

struct APJob
{
    int release_time;
    int remain_execution_time;
    int absolute_deadline;
    int TID; //屬於哪個Task的工作
    int Period;
};

struct PTask pTask[1000];  // 儲存 periodic task 資訊
struct APTask apTask[1000];  // 儲存 aperiodic task 資訊

struct PJob pJob[1000]; // 儲存 periodic job 資訊
struct APJob apJob[1000]; // 儲存 aperiodic job 資訊

list <int> waitingPQ; // Periodic job 執行序列
list <int> waitingAPQ; // Aperiodic job 執行序列

int MaxSysTime = 1000; // 系統總執行時間
int TotalResponseTime = 0;
int FinishedAJobNumber = 0;
int MissPJobNumber = 0;
int TotalPJobNumber = 0; // 總共的 Periodic Job 個數
int TotalAPJobNumber = 0; // 總共的 Aperiodic Job 個數
int Clock = 0;

// CUS 參數設定
float serverSize = 0.2; // us
float CUSDeadline = 0; // d

void showlist(list <int> g);
void readData();
void initialization();
void simulation();

int main()
{
    readData();
    initialization();
    simulation();
    
    cout << "CUS" << endl;
    cout << "Miss Rate: " << MissPJobNumber/TotalPJobNumber << endl;
    cout << "Average Response Time: " << TotalResponseTime/FinishedAJobNumber << endl;
}

// MARK: Step 1
void readData()
{
    // periodic 讀取儲存部分
    fstream perFile;
    string filename = "/Users/toby/Library/Mobile Documents/com~apple~CloudDocs/Collage Classes/Real time system/CUS and TBS Simulator/CUS and TBS Simulator/periodic.txt";
    
    perFile.open(filename.c_str());
    if(!perFile)
        cout << "檔案無法開啟" << endl;
    
    string line; // 暫存讀入的資料
    
    for(TotalPJobNumber = 0; getline(perFile, line); TotalPJobNumber++)
    {
        stringstream line_ss(line);
        vector<float> presult;
        for (float j = 0; line_ss >> j;)
        {
            presult.push_back(j);
            if (line_ss.peek() == ',')
                line_ss.ignore();
        }
        int number = 0;
        
        pTask[TotalPJobNumber].TID = TotalPJobNumber;
        pTask[TotalPJobNumber].Phase = 0;
        pTask[TotalPJobNumber].Period = presult[number++];
        pTask[TotalPJobNumber].RDeadline = apTask[TotalPJobNumber].Period;
        pTask[TotalPJobNumber].WCET = presult[number];
    }
    
    
    cout << "Periodic" << endl;
    cout << "Phase, " << "Period, " << "Execution time, " << "TID" << endl;
    
    for (size_t i = 0; i < TotalPJobNumber; i++)
    {
        cout << pTask[i].Phase << ", ";
        cout << pTask[i].Period << ", ";
        cout << pTask[i].WCET << ", ";
        cout << pTask[i].TID << ", " << endl;
    }
    
    
    perFile.close();
    
    // periodic 讀取儲存部分
    fstream aperFile;
    string aperfilename = "/Users/toby/Library/Mobile Documents/com~apple~CloudDocs/Collage Classes/Real time system/CUS and TBS Simulator/CUS and TBS Simulator/aperiodic.txt";
    
    aperFile.open(aperfilename.c_str());
    if(!aperFile)
        cout << "檔案無法開啟" << endl;
    
    string aperline; // 暫存讀入的資料
    
    for(TotalAPJobNumber = 0; getline(aperFile, line); TotalAPJobNumber++)
    {
        stringstream line_ss(line);
        vector<float> apresult;
        for (float j = 0; line_ss >> j;)
        {
            apresult.push_back(j);
            if (line_ss.peek() == ',')
                line_ss.ignore();
        }
        int number = 0;
        
        apTask[TotalAPJobNumber].TID = TotalAPJobNumber;
        apTask[TotalAPJobNumber].Phase = apresult[number++];
        apTask[TotalAPJobNumber].Period = 0;
        apTask[TotalAPJobNumber].WCET = apresult[number];
    }
    
    aperFile.close();
    
    
    cout << "Aperiodic" << endl;
    cout << "Phase, " << "Period, " << "Execution time, " << "TID" << endl;
    
    for (int i = 0; i < TotalAPJobNumber; i++)
    {
        cout << apTask[i].Phase << ", ";
        cout << apTask[i].Period << ", ";
        cout << apTask[i].WCET << ", ";
        cout << apTask[i].TID << ", " << endl;
    }
    
}

// MARK: Step 2-9
void initialization()
{
    MaxSysTime = 0;
    
    waitingPQ.clear();
    waitingAPQ.clear();
    
    TotalResponseTime = 0;
    FinishedAJobNumber = 0;
    MissPJobNumber = 0;
    Clock = 0;
}

void simulation()
{
    // MARK: Step 10
    while(Clock <= MaxSysTime)
    {
        // MARK: Step 11
        if(!waitingPQ.empty())
        {
            vector<int> jobToRemove; // 儲存不能在 deadline 前完成 job 的號碼
            for(list <int> :: iterator it = waitingPQ.begin(); it != waitingPQ.end(); ++it) // 找出哪些 job 無法在 deadline 完成
            {
                if(pJob[*it].absolute_deadline - Clock - pJob[*it].remain_execution_time < 0)
                {
                    cout << "Job T" << pJob[*it].TID << " 不能在絕對截限時間內完成" << endl;
                    MissPJobNumber++;
                    jobToRemove.push_back(*it);
                }
            }
            for(int i = 0; i < jobToRemove.size(); i++) // 移除 job
            {
                waitingPQ.remove(jobToRemove[i]);
            }
            jobToRemove.clear();
        }
        
        // MARK: Step 12
        for(int i = 0; i < TotalPJobNumber ; i++)
        {
            if(Clock - pTask[i].Phase >= 0 && fmod(Clock, pTask[i].Period) == 0)  // 目前 Clock 是否為 periodic job 之抵達時間
            {
                waitingPQ.push_back(pTask[i].TID);  // 在 Queue 中存入對應到該 job 和 task 相對應的 TID 編號
                pJob[TotalPJobNumber].release_time = Clock;
                pJob[TotalPJobNumber].remain_execution_time = pTask[i].WCET;
                pJob[TotalPJobNumber].absolute_deadline = pTask[i].RDeadline + Clock;
                pJob[TotalPJobNumber].TID = pTask[i].TID;
                pJob[TotalPJobNumber++].Period = pTask[i].Period;
            }
        }
        
        // MARK: Step 13
        for(int i = 0; i < TotalAPJobNumber; i++)
        {
            if(Clock-apTask[i].Phase >= 0 && Clock == apTask[i].Phase)  // 目前 Clock 為 aperiodic job 之抵達時間
            {
                if(waitingAPQ.empty() && Clock == CUSDeadline)  // 如果 ARQ 是空的且 CUS 的 deadline = Clock
                {
                    CUSDeadline = Clock + apTask[i].WCET/serverSize;
                }
                waitingAPQ.push_back(pTask[i].TID);
                apJob[TotalAPJobNumber].release_time = Clock;
                apJob[TotalAPJobNumber].remain_execution_time = pTask[i].WCET;
                apJob[TotalAPJobNumber].absolute_deadline = pTask[i].RDeadline + Clock;
                apJob[TotalAPJobNumber].TID = pTask[i].TID;
                apJob[TotalAPJobNumber++].Period = pTask[i].Period;
                TotalAPJobNumber++;
            }
        }
        
        // MARK: Step 14 & 15
        float leastDeadline = INT_MAX;
        int leastDeadlineTID = 0;
        for(list <int> :: iterator it = waitingPQ.begin(); it != waitingPQ.end(); ++it)
        {
            if(pJob[*it].absolute_deadline < leastDeadline)
            {
                leastDeadline = pJob[*it].absolute_deadline;
                leastDeadlineTID = *it;
            }
        }
        if(leastDeadline < CUSDeadline) // 執行 periodic job
        {
            for(int i = 0; i < TotalPJobNumber; i++)
            {
                if(pJob[i].TID == leastDeadlineTID)
                {
                    pJob[i].remain_execution_time--;
                    cout << "Excuted " << pJob[i].TID << "in " << Clock << endl;
                    if(pJob[i].remain_execution_time == 0)  // 執行時間已為 0，則刪除該工作
                        waitingPQ.remove(pJob[i].TID);
                    break;
                }
            }
        }
        else // 執行 CUS
        {
            list <int> :: iterator it = waitingAPQ.begin();
            apJob[*it].remain_execution_time--;
            cout << "Excuted CUS " << apJob[*it].TID << "in " << Clock << endl;
            if(apJob[*it].remain_execution_time == 0)
            {
                FinishedAJobNumber++;
                TotalResponseTime += Clock + apJob[*it].release_time;
                waitingAPQ.remove(*it);
            }
        }
        
        // MARK: Step 16
        if(CUSDeadline == Clock)
        {
            if(!waitingPQ.empty())
            {
                list <int> :: iterator it = waitingPQ.begin();
                int e = pTask[*it].WCET;
                CUSDeadline = Clock + e/serverSize;
            }
        }
            
        // MARK: Step 17
        Clock++;
    }
}

// MARK: showList
void showlist(list <int> g)
{
    cout << "List is: ";
    list <int> :: iterator it;
    for(it = g.begin(); it != g.end(); ++it)
        cout << *it << " ";
    cout << endl;
}

