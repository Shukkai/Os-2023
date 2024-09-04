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
#include<fstream>
#include<list>
#include<utility>
#include<queue>
using namespace std;
int sz = 100003;
struct LFUnode
{
    int times,seq,page;
};
typedef struct LRUnode
{
    int page;
    LRUnode *prev, *next;
}LRUnode;
int main(int argc, char *argv[]){
    timeval start,end;
    double elapsed;
    string tmp;
    cout<<"LFU policy:\nFrame\tHit\t\tMiss\t\tPage fault ratio\n";
    gettimeofday(&start, 0);
    int f,h,m;
    for(int i = 64; i <= 512; i<<=1){
        ifstream in;
        in.open(argv[1]);
        f = 0;h = 0; m = 0;
        LFUnode cache[i];
        list<pair<int,int>> hash[sz];
        int seq = 0;
        while (getline(in,tmp))
        {
            int page = stoi(tmp);
            int flag = 1;
            for(auto &j: hash[(page % sz)]){
                if(j.first == page){
                    cache[j.second].times++;
                    cache[j.second].seq = seq;
                    h++;
                    flag = 0;
                    break;
                }
            }
            if(flag){
                m++;
                if(f < i){
                    hash[page%sz].push_back(make_pair(page,f));
                    cache[f++] = {1,seq,page};
                }
                else{
                    int to = 0;
                    for (int j=1; j<i; j++){
                        if (cache[to].times > cache[j].times){
                            to = j;
                        }
                        else if (cache[to].times == cache[j].times && cache[to].seq > cache[j].seq){
                            to = j;
                        }
                    }
                    hash[cache[to].page%sz].remove({cache[to].page,to});
                    hash[page%sz].push_back(make_pair(page,to));
                    cache[to] = {1,seq,page};
                    
                }   
            }
            seq++;
        }
        double pfr = (double)m/(h+m);
        printf("%d\t%d\t\t%d\t\t%.10f\n",i,h,m,pfr);
        in.close();
    }
    gettimeofday(&end, 0);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Total elapsed time %.4f sec\n\n", elapsed);
    cout<<"LRU policy:\nFrame\tHit\t\tMiss\t\tPage fault ratio\n";
    ifstream in2;
    gettimeofday(&start, 0);
    for(int i = 64; i <= 512; i<<=1){
        in2.open(argv[1]);
        f = 0;h = 0; m = 0;
        LRUnode *head = NULL, *tail = NULL, *cur;
        list<pair<int,LRUnode*>> hash[sz];
        while (getline(in2,tmp))
        {
            int page = stoi(tmp);
            cur = NULL;
            for(auto &j: hash[(page % sz)]){
                if(j.first == page){
                    cur = j.second;
                    break;
                }
            }
            if(cur != NULL){
                h++;
                if(cur != head){
                    cur -> prev -> next = cur -> next;
                    if(tail == cur){
                        tail = tail -> prev;
                        free(tail->next);
                        tail -> next = NULL;
                    }
                    else{
                        cur -> next -> prev = cur -> prev;
                    }
                    cur -> next = head; 
                    cur -> prev = NULL;
                    head -> prev = cur;
                    head = cur;
                }
            }
            else{
                m++;
                while(f >= i){
                    hash[tail->page%sz].remove(make_pair(tail->page,tail));
                    tail = tail -> prev;
                    free(tail->next);
                    tail -> next = NULL;
                    f--;
                }
                cur = new(LRUnode);
                cur -> page = page;
                cur -> next = head;
                cur -> prev = NULL;
                if(head != NULL){
                    head -> prev = cur;
                }
                head = cur; 
                if(f == 0){
                    tail = cur;
                }
                hash[page%sz].push_back(make_pair(page,cur));
                f++;   
            }
        }
        double pfr = (double)m/(h+m);
        printf("%d\t%d\t\t%d\t\t%.10f\n",i,h,m,pfr);
        in2.close();
    }
    gettimeofday(&end, 0);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Total elapsed time %.4f sec\n", elapsed);
    return 0;
}
