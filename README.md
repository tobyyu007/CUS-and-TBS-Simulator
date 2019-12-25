# CUS-and-TBS-Simulator

This is a modification from the previous [EDF-and-RM-Simulator](https://github.com/tobyyu007/EDF-and-RM-Simulator)

- 請實作出Constant Utilization Server與Total Bandwidth Server兩個處理非週期性工作的演算法，server size固定為0.2。

- 輸入檔格式(文字格式)
- 輸入檔分成兩個，一為檔名periodic.txt，另一為檔名aperiodic.txt

periodic.txt
每一行即為一個週期性的任務，第一個數字為週期，第二個為執行時間，且都為正整數，且以逗號隔開。任務個數不固定。
Ex:
3,1
2,1

aperiodic.txt
每一行即為一個非週期性的工作，第一個數字為抵達時間，第二個為執行時間，且都為正整數，且以逗號隔開。工作個數不固定。
Ex:
1,3 
5,1
12,4

輸出格式：
	模擬執行時間為1000單位時間，
(1)	過程中週期性工作miss rate(=miss deadline週期性工作個數/週期性工作總個數)，因為週期性的任務總使用率可能會超過0.8
(2)	非週期性工作的平均反應時間 ，|A|為已完成的非週期性工作集合， 與 分別為工作i之抵達時間與完成時間。
	

繳交時間：  1/6

Demo時間： 另訂

繳交內容：  原始程式、2-3頁報告
 
Pseudocode
1.	先將periodic.txt中的每個任務資料讀入，並儲存在適當資料結構中。N為任務總個數
2.	MaxSimTime=100
3.	將系統的periodic job就緒佇列PQ初始化成空佇列
4.	將系統的aperiodic job就緒佇列AQ初始化成空佇列
5.	TotalResponseTime=0
6.	FinishedAJobNumber=0
7.	MissPJobNumber=0
8.	TotalPJobNumber=0;
9.	初始化時間Clock為0
10.	While(Clock<=MaxSimTime)
{
    1.	判斷PQ中的每一個periodic job是否能在它的絕對截限時間之前完成((d-Clock-rem_exe)>0)，若不能，則輸出該工作訊息，並紀錄miss工作加1和刪除，MissPJobNumber++
    2.	針對每一個任務判斷目前時間Clock是否為它的工作之抵達時間(Clock%pi)==0。若是，則在PQ中加入工作，並紀錄進入系統的工作個數加1，TotalPJobNumber++
    3.	將在時間點clock抵達的aperiodic job加入到AQ中(FIFO)。如果原AQ是空的且CUS的deadline=Clock，則設定CUS的截限時間為Clock+e/us。
    4.	從PQ中找到截限時間最小的工作，並與CUS的截限時間做比較。較小者為優先權最高的，如果是periodic job，其執行時間減1;如果是CUS則AQ中的第一個job執行時間減一。
    5.	如果執行時間已為0，則刪除該工作。如果為週期性的，FinishedAJobNumber++，TotalResponseTime= TotalResponseTime+(clock-該工作抵達時間)
    6.	判斷CUS截限時間是否等於Clock。若是，如果AQ中還有其他的工作，則設定CUS的截限時間為Clock+e/us，e為AQ中第一個工作的執行時間
    7.	Clock++;
}

Miss_rate= MissPJobNumber /TotalPJobNumber;

Avweage_Response_Time= TotalResponseTime/FinishedAJobNumber;
