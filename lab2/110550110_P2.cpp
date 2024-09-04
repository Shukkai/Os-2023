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
#include<sys/time.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<vector>
using namespace std;
int main(){
    int size;
    cout<<"input matrix size: ";
    cin>>size;
    cout<<endl;
    unsigned int mat[size][size];
    unsigned int cur = 0;
    for(int i = 0;i < size;i++){
        for(int j = 0;j < size;j++,cur++){
            mat[i][j] = cur;
        }
    }
    int shmid = shmget(0, size*size*sizeof(unsigned int), IPC_CREAT | 0600);
    int* matc = (int*)shmat(shmid, NULL, 0);
    struct timeval start,end;
    vector<int> row;
    vector<vector<int>> stops(16,row);
    for(int i = 0;i < 16;i++){
        int stop = size/(i+1);
        stops[i].push_back(0);
        if(size % (i+1) != 0){
            int left = size % (i+1);
            for(int j = 0;j < i;j++){
                if(left > 0){
                    stops[i].push_back(stops[i].back()+stop+1);
                    left--;
                }
                else{
                    stops[i].push_back(stops[i].back()+stop);
                }
            }
        }
        else{
            for(int j = 0;j < i;j++){
                stops[i].push_back(stop*(j+1));
            }
        }
        stops[i].push_back(size);
    }
    for(int p = 1;p <= 16;p++){
        pid_t pid;
        gettimeofday(&start,0);
        for(int i = 1;i < stops[p-1].size();i++){
            pid = fork();
            if(pid ==0){
                unsigned int tmp = 0;
                for(int l = stops[p-1][i-1];l <= stops[p-1][i];l++){
                    for(int j = 0;j < size;j++,tmp=0){
                        for(int k = 0;k < size;k++){
                            tmp += mat[l][k] * mat[k][j];
                        }
                        matc[l*size+j] = tmp;
                    }
                }
                exit(0);
            }
            else if(pid<0){
                fprintf(stderr,"fork failed\n");
                exit(-1);
            }
        }
        for(int i = 1;i <=p;i++){
            wait(NULL);
        }
        unsigned int sum = 0;
        for(int i = 0;i < size*size;i++){
                sum += matc[i];
        }
        gettimeofday(&end,0);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        if(p == 1){
            cout<<"Multiplying matrices using "<<p<<" process\n"<<"Elapsed time:"<<(sec+(usec/1000000.0))<<" sec, Checksum: "<<sum<<endl;
        }
        else{
            cout<<"Multiplying matrices using "<<p<<" processes\n"<<"Elapsed time:"<<(sec+(usec/1000000.0))<<" sec, Checksum: "<<sum<<endl;
        }
    }
    shmdt(matc);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}