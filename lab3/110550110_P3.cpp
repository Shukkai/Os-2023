/*
Student No.: 110550110
Student Name: 林書愷
Email: kyle.lin0908.nycu.cs10@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/
#include<iostream>
#include<unistd.h>
#include<vector>
#include<semaphore.h>
#include<pthread.h>
#include<fstream>
#include<sys/time.h>
#include<queue>
#include<algorithm>
using namespace std;
vector<int> input;
vector<int> bbounds;
vector<bool> finish;
int lsbounds[] = {0,0,4,0,2,4,6};
int rsbounds[] = {8,4,8,2,4,6,8};
int n;
float ms;
sem_t mutex, job;// mutex for job arriving, job for task running
queue<int> schedule;
void merge(const int st,const int ed){
    const int mid = (st+ed)/2;
    vector<int> l(input.begin()+st,input.begin()+mid);
    vector<int> r(input.begin()+mid,input.begin()+ed);
    l.insert(l.end(),2147483647);
    r.insert(r.end(),2147483647);
    int idl = 0, idr = 0;
    for(int i = st;i < ed; i++){
        if(l[idl] <= r[idr]){
            input[i] = l[idl];
            idl++;
        }
        else{
            input[i] = r[idr];
            idr++;
        }
    }
}
void bubble(const int l, const int r){
    for(int i = l; i < r-1;i++){
        for(int j = i+1;j < r;j++){
            if(input[i] > input[j]){
                swap(input[i],input[j]);
            }
        }
    }
}
void *dispatcher(void *){
    int l,r;
    while (1)
    {
        sem_wait(&mutex);
        if(schedule.empty()){
            sem_post(&mutex);
            sem_post(&job);
            return nullptr;
        }
        int cur = schedule.front();
        schedule.pop();
        sem_post(&mutex);
        if(cur > 6){
            l = bbounds[cur-7];
            r = bbounds[cur-6];
            bubble(l,r);
        }
        else{
            l = bbounds[lsbounds[cur]];
            r = bbounds[rsbounds[cur]];
            merge(l,r);
        }
        finish[cur] = true;
        sem_wait(&mutex);
        if(cur != 0){
            if(cur % 2 == 0 && finish[cur-1] == true){
                int tmp = (cur/2)-1;
                schedule.push(tmp);
            }
            else if(cur % 2 == 1 && finish[cur+1] == true){
                int tmp = (cur/2);
                schedule.push(tmp);
            }
        }
        sem_post(&mutex);
    }
    
}
int main(){
    for(int t = 1;t <= 8; t++){
        pthread_t T[8];
        struct timeval start,end;
        input.clear();
        ifstream in;
        in.open("./input.txt");
        in >> n;
        input.assign(n,0);
        for(int i = 0;i < n;i++){
            in >> input[i];
        }
        in.close();
        finish.assign(15,false);
        int left = n % 8;
        int part = n / 8;
        bbounds.assign(9,0);
        for(int i = 1;i < 9;i++){
            if(left > 0){
                left--;
                bbounds[i] = bbounds[i-1] + part + 1; 
            }
            else{
                bbounds[i] = bbounds[i-1] + part;
            }
        }
        gettimeofday(&start,0);

        sem_init(&job,0,0);
        sem_init(&mutex, 0, 1);
        sem_post(&mutex);
        for(int i = 7;i <= 14;i++){
            schedule.push(i);
        }
        sem_wait(&mutex);
        for(int i = 0; i <= t;i++){
            pthread_create(&T[i],NULL, dispatcher,NULL);
        }
        for(int i = 0; i <= t;i++){
            sem_wait(&job);
        }
        gettimeofday(&end,0);
        ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec)/1000.0;
        cout<<"worker thread #"<<t<<", elapsed "<<ms<<" ms\n";
        ofstream of;
        string outpath = "./out"+ to_string(t) + ".txt";
        of.open(outpath);
        for(int i = 0;i < n;i++){
            of<<input[i]<<" ";
        }
        of.close();

        sem_destroy(&mutex);
        sem_destroy(&job);
    }
    return 0;
}