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
#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif
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
    float release_time;
    float remain_execution_time;
    float absolute_deadline;
    int TID; //屬於哪個Task的工作
    int JID; // Job ID
    float Period;
};

struct APJob
{
    float release_time;
    float remain_execution_time;
    float absolute_deadline;
    int TID; //屬於哪個Task的工作
    int JID; // Job ID
    float Period;
};

struct PTask pTask[10000];  // 儲存 periodic task 資訊
struct APTask apTask[10000];  // 儲存 aperiodic task 資訊

list <int> waitingPQ; // Periodic job 執行序列
list <int> waitingAPQ; // Aperiodic job 執行序列

int MaxSysTime = 1000; // 系統總執行時間
float TotalResponseTime = 0;
float FinishedAJobNumber = 0;
float MissPJobNumber = 0;
int TotalPTaskNumber = 0; // 總共的 Periodic Task 個數
int TotalAPTaskNumber = 0; // 總共的 Periodic Task 個數
int TotalPJobNumber = 0; // 總共的 Periodic Job 個數
int TotalAPJobNumber = 0; // 總共的 Aperiodic Job 個數
float Clock = 0;

bool firstAP = true;
float budget = 0;
// CUS 參數設定

// ********************************************************************* 要記得改回去 0.2
float serverSize = 0.2; // us
// ********************************************************************* 要記得改回去 0.2
float CUSDeadline = INT_MAX; // d

void showlist(list <int> g);
void readData();
void initialization();
void CUS();
void TBS();

int main()
{
    readData();
    
    initialization();
    CUS();
    cout << "CUS" << endl;
    cout << "Miss Rate: " << MissPJobNumber/TotalPJobNumber << endl;
    cout << "Average Response Time: " << TotalResponseTime/FinishedAJobNumber << endl << endl;
    
    initialization();
    TBS();
    cout << "TBS" << endl;
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
    
    for(TotalPTaskNumber = 0; getline(perFile, line); TotalPTaskNumber++)
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
        
        pTask[TotalPTaskNumber].TID = TotalPTaskNumber;
        pTask[TotalPTaskNumber].Phase = 0;
        pTask[TotalPTaskNumber].Period = presult[number++];
        pTask[TotalPTaskNumber].RDeadline = pTask[TotalPTaskNumber].Period;
        pTask[TotalPTaskNumber].WCET = presult[number];
    }
    
    /*
    cout << "Periodic" << endl;
    cout << "Phase, " << "Period, " << "Execution time, " << "TID" << endl;
    
    for (size_t i = 0; i < TotalPTaskNumber; i++)
    {
        cout << pTask[i].Phase << ", ";
        cout << pTask[i].Period << ", ";
        cout << pTask[i].WCET << ", ";
        cout << pTask[i].TID << ", " << endl;
    }
    */
    
    
    perFile.close();
    
    // periodic 讀取儲存部分
    fstream aperFile;
    string aperfilename = "/Users/toby/Library/Mobile Documents/com~apple~CloudDocs/Collage Classes/Real time system/CUS and TBS Simulator/CUS and TBS Simulator/aperiodic.txt";
    
    aperFile.open(aperfilename.c_str());
    if(!aperFile)
        cout << "檔案無法開啟" << endl;
    
    string aperline; // 暫存讀入的資料
    
    for(TotalAPTaskNumber = 0; getline(aperFile, line); TotalAPTaskNumber++)
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
        
        apTask[TotalAPTaskNumber].TID = TotalAPTaskNumber;
        apTask[TotalAPTaskNumber].Phase = apresult[number++];
        apTask[TotalAPTaskNumber].Period = 0;
        apTask[TotalAPTaskNumber].WCET = apresult[number];
    }
    
    aperFile.close();
    
    /*
    cout << "Aperiodic" << endl;
    cout << "Phase, " << "Period, " << "Execution time, " << "TID" << endl;
    
    for (int i = 0; i < TotalAPTaskNumber; i++)
    {
        cout << apTask[i].Phase << ", ";
        cout << apTask[i].Period << ", ";
        cout << apTask[i].WCET << ", ";
        cout << apTask[i].TID << ", " << endl;
    }
    */
}

// MARK: Step 2-9
void initialization()
{
    // ********************************************************************* 要記得改回去 1000
    MaxSysTime = 1000;
    // ********************************************************************* 要記得改回去 1000
    
    waitingPQ.clear();
    waitingAPQ.clear();
    
    TotalResponseTime = 0;
    FinishedAJobNumber = 0;
    MissPJobNumber = 0;
    TotalPJobNumber = 0;
    TotalAPJobNumber = 0;
    
    firstAP = true;
    budget = 0;
    CUSDeadline = INT_MAX;
    Clock = 0;
}

// MARK: CUS Part

void CUS()
{
    struct PJob pJob[10000] = {}; // 儲存 periodic job 資訊
    struct APJob apJob[10000] = {}; // 儲存 aperiodic job 資訊
    
    // MARK: Step 10
    while(Clock <= MaxSysTime)
    {
        // MARK: Step 11
        if(!waitingPQ.empty())
        {
            vector<int> jobToRemove; // 儲存不能在 deadline 前完成 job 的號碼
            for(list <int> :: iterator it = waitingPQ.begin(); it != waitingPQ.end(); ++it) // 找出哪些 periodic job 無法在 deadline 完成
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
        for(int i = 0; i < TotalPTaskNumber ; i++)
        {
            if(Clock - pTask[i].Phase >= 0 && fmod(Clock, pTask[i].Period) == 0)  // 目前 Clock 是否為 periodic job 之抵達時間
            {
                pJob[TotalPJobNumber].release_time = Clock;
                pJob[TotalPJobNumber].remain_execution_time = pTask[i].WCET;
                pJob[TotalPJobNumber].absolute_deadline = pTask[i].RDeadline + Clock;
                pJob[TotalPJobNumber].TID = pTask[i].TID;
                pJob[TotalPJobNumber].JID = TotalPJobNumber;
                pJob[TotalPJobNumber].Period = pTask[i].Period;
                
                waitingPQ.push_back(pJob[TotalPJobNumber++].JID);  // 在 Queue 中存入對應到該 periodic job 的 Job ID
            }
        }
        
        // MARK: Step 13
        for(int i = 0; i < TotalAPTaskNumber; i++)
        {
            if(Clock-apTask[i].Phase >= 0 && Clock == apTask[i].Phase)  // 目前 Clock 為 aperiodic job 之抵達時間
            {
                if(firstAP)  // 如果 ARQ 是空的且 CUS 的 deadline = Clock
                {
                    CUSDeadline = Clock + apTask[i].WCET/serverSize;
                    cout << "Deadline first is: " << CUSDeadline << endl;
                    budget = apTask[i].WCET;
                    firstAP = false;
                }
                apJob[TotalAPJobNumber].release_time = Clock;
                apJob[TotalAPJobNumber].remain_execution_time = apTask[i].WCET;
                apJob[TotalAPJobNumber].TID = apTask[i].TID;
                apJob[TotalAPJobNumber].JID = TotalAPJobNumber;
                apJob[TotalAPJobNumber].Period = apTask[i].Period;
                
                waitingAPQ.push_back(apJob[TotalAPJobNumber++].JID);  // 在 Queue 中存入對應到該 aperiodic job 的 Job ID
            }
        }
        //showlist(waitingAPQ);
        
        // MARK: Step 16
        // Deadline 和 Budget 更新
        if(Clock >= CUSDeadline && !waitingAPQ.empty())
        {
            list <int> :: iterator it = waitingAPQ.begin();
            int e = apTask[*it].WCET;
            CUSDeadline = Clock + e/serverSize;
            budget = apTask[*it].WCET;
            cout << "Deadline is: " << CUSDeadline << endl;
        }
        
        // MARK: Step 14 & 15
        float leastDeadline = INT_MAX;
        int leastDeadlineJID = 0;
        for(list <int> :: iterator it = waitingPQ.begin(); it != waitingPQ.end(); ++it)
        {
            if(pJob[*it].absolute_deadline - Clock < leastDeadline)
            {
                leastDeadline = pJob[*it].absolute_deadline - Clock;
                leastDeadlineJID = pJob[*it].JID;
            }
        }
        if(!waitingAPQ.empty() && CUSDeadline - Clock <= leastDeadline && budget > 0) // 執行 CUS Aperiodic Job
        {
            list <int> :: iterator it = waitingAPQ.begin();
            // ********************************************************************* 要記得改回去 1
            apJob[*it].remain_execution_time-=1;
            budget -= 1;
            // ********************************************************************* 要記得改回去 1
            cout << "Excuted aperiodic " << apJob[*it].TID << " in " << Clock << endl;
            if(apJob[*it].remain_execution_time == 0)
            {
                FinishedAJobNumber++;
                TotalResponseTime += Clock + 1 - apJob[*it].release_time;
                waitingAPQ.remove(*it);
            }
        }
        else if(!waitingPQ.empty())// 執行 periodic job
        {
            for(int i = 0; i < TotalPJobNumber; i++)
            {
                if(pJob[i].JID == leastDeadlineJID)
                {
                    // ********************************************************************* 要記得改回去 1
                    pJob[i].remain_execution_time-=1;
                    // ********************************************************************* 要記得改回去 1
                    cout << "Excuted " << pJob[i].TID << " in " << Clock << endl;
                    if(pJob[i].remain_execution_time == 0)  // 執行時間已為 0，則刪除該工作
                        waitingPQ.remove(pJob[i].JID);
                    break;
                }
            }
        }
        leastDeadline = INT_MAX;
        leastDeadlineJID = 0;
            
        // MARK: Step 17
        // ********************************************************************* 要記得改回去 1
        Clock+=1;
        // ********************************************************************* 要記得改回去 1
    }
}

// MARK: TBS Part

void TBS()
{
    struct PJob pJob[10000] = {}; // 儲存 periodic job 資訊
    struct APJob apJob[10000] = {}; // 儲存 aperiodic job 資訊
    
    // MARK: Step 10
    while(Clock <= MaxSysTime)
    {
        // MARK: Step 11
        if(!waitingPQ.empty())
        {
            vector<int> jobToRemove; // 儲存不能在 deadline 前完成 job 的號碼
            for(list <int> :: iterator it = waitingPQ.begin(); it != waitingPQ.end(); ++it) // 找出哪些 periodic job 無法在 deadline 完成
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
        for(int i = 0; i < TotalPTaskNumber ; i++)
        {
            if(Clock - pTask[i].Phase >= 0 && fmod(Clock, pTask[i].Period) == 0)  // 目前 Clock 是否為 periodic job 之抵達時間
            {
                pJob[TotalPJobNumber].release_time = Clock;
                pJob[TotalPJobNumber].remain_execution_time = pTask[i].WCET;
                pJob[TotalPJobNumber].absolute_deadline = pTask[i].RDeadline + Clock;
                pJob[TotalPJobNumber].TID = pTask[i].TID;
                pJob[TotalPJobNumber].JID = TotalPJobNumber;
                pJob[TotalPJobNumber].Period = pTask[i].Period;
                
                waitingPQ.push_back(pJob[TotalPJobNumber++].JID);  // 在 Queue 中存入對應到該 periodic job 的 Job ID
            }
        }
        
        // MARK: Step 13
        for(int i = 0; i < TotalAPTaskNumber; i++)
        {
            if(Clock-apTask[i].Phase >= 0 && Clock == apTask[i].Phase)  // 目前 Clock 為 aperiodic job 之抵達時間
            {
                if(firstAP)  // 如果 ARQ 是空的且 CUS 的 deadline = Clock
                {
                    CUSDeadline = Clock + apTask[i].WCET/serverSize;
                    cout << "Deadline first is: " << CUSDeadline << endl;
                    budget = apTask[i].WCET;
                    firstAP = false;
                }
                apJob[TotalAPJobNumber].release_time = Clock;
                apJob[TotalAPJobNumber].remain_execution_time = apTask[i].WCET;
                apJob[TotalAPJobNumber].TID = apTask[i].TID;
                apJob[TotalAPJobNumber].JID = TotalAPJobNumber;
                apJob[TotalAPJobNumber].Period = apTask[i].Period;
                
                waitingAPQ.push_back(apJob[TotalAPJobNumber++].JID);  // 在 Queue 中存入對應到該 aperiodic job 的 Job ID
            }
        }
        //showlist(waitingAPQ);
        
        // MARK: Step 16
        if(!waitingAPQ.empty() && budget == 0)
        {
            list <int> :: iterator it = waitingAPQ.begin();
            budget = apTask[*it].WCET;
            int e = apTask[*it].WCET;
            CUSDeadline = max(CUSDeadline, Clock) + e/serverSize;
        }
        
        // MARK: Step 14 & 15
        float leastDeadline = INT_MAX;
        int leastDeadlineJID = 0;
        for(list <int> :: iterator it = waitingPQ.begin(); it != waitingPQ.end(); ++it)
        {
            if(pJob[*it].absolute_deadline - Clock < leastDeadline)
            {
                leastDeadline = pJob[*it].absolute_deadline - Clock;
                leastDeadlineJID = pJob[*it].JID;
            }
        }
        if(!waitingAPQ.empty() && CUSDeadline - Clock <= leastDeadline && budget > 0) // 執行 TBS Aperiodic Job
        {
            list <int> :: iterator it = waitingAPQ.begin();
            // ********************************************************************* 要記得改回去 1
            apJob[*it].remain_execution_time-=1;
            budget -= 1;
            // ********************************************************************* 要記得改回去 1
            cout << "Excuted aperiodic " << apJob[*it].TID << " in " << Clock << endl;
            if(apJob[*it].remain_execution_time == 0)
            {
                FinishedAJobNumber++;
                TotalResponseTime += Clock + 1 - apJob[*it].release_time;
                waitingAPQ.remove(*it);
            }
        }
        else if(!waitingPQ.empty())// 執行 periodic job
        {
            for(int i = 0; i < TotalPJobNumber; i++)
            {
                if(pJob[i].JID == leastDeadlineJID)
                {
                    // ********************************************************************* 要記得改回去 1
                    pJob[i].remain_execution_time-=1;
                    // ********************************************************************* 要記得改回去 1
                    cout << "Excuted " << pJob[i].TID << " in " << Clock << endl;
                    if(pJob[i].remain_execution_time == 0)  // 執行時間已為 0，則刪除該工作
                        waitingPQ.remove(pJob[i].JID);
                    break;
                }
            }
        }
        leastDeadline = INT_MAX;
        leastDeadlineJID = 0;
            
        // MARK: Step 17
        // ********************************************************************* 要記得改回去 1
        Clock+=1;
        // ********************************************************************* 要記得改回去 1
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

