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
using namespace std;
int sz = 100003;
struct LFUfreqnode;
struct LFUnode{
    int page;
    LFUnode *prev, *next;
    LFUfreqnode *times;
};
struct LFUfreqnode
{
    int cnt,cnums;
    LFUfreqnode *prev, *next;
    LFUnode *children;
};
typedef struct LRUnode
{
    int page;
    LRUnode *prev, *next;
}LRUnode;
void insert_node(int page, LFUfreqnode *tail, vector<list<pair<int,LFUnode*>>> &hash){
    LFUnode *toadd = tail -> children;
    for(;toadd != NULL && toadd -> next != NULL; ){
        toadd = toadd -> next;
    }
    LFUnode *add = new(LFUnode);
    add -> times = tail;
    add -> next = NULL;
    add -> page = page;
    add -> prev = toadd;
    if(toadd){
        toadd -> next = add;
    }
    if(tail -> children == NULL){
        tail -> children = add;
    }
    tail -> cnums ++;
    hash[page%sz].push_back(make_pair(page,add));
}
void move_node(LFUnode *cur, LFUfreqnode *newfre){
    LFUnode *toadd = newfre -> children;
    for(;toadd != NULL && toadd -> next != NULL; ){
        toadd = toadd -> next;
    }
    cur -> next = NULL;
    cur -> prev = toadd;
    if(toadd){
        toadd -> next = cur;
    }
    cur -> times = newfre;
    cur -> times -> cnums ++;
    if(newfre -> children == NULL){
        newfre -> children = cur;
    }
}
int main(int argc, char *argv[]){
    timeval start,end;
    double elapsed;
    string tmp;
    cout<<"LFU policy:\nFrame\tHit\tMiss\tPage fault ratio\n";
    gettimeofday(&start, 0);
    double f,h,m;
    for(int i = 64; i <= 512; i<<=1){
        ifstream in;
        in.open(argv[1]);
        f = 0;h = 0; m = 0;
        LFUfreqnode *head = NULL, *tail = NULL;
        LFUnode *cur;
        vector<list<pair<int,LFUnode*>>> hash;
        hash.resize(sz);
        int now = 1;
        while (getline(in,tmp))
        {
            cout<<now<<": ";
            now++;
            int page = stoi(tmp);
            // cout<<page<<"\n";
            cur = NULL;
            for(auto &j: hash[(page % sz)]){
                if(j.first == page){
                    cur = j.second;
                    break;
                }
            }
            if(cur != NULL){
                h++;
                // check need to create a newfre node or not
                if(cur -> times == head || cur -> times -> prev -> cnt != cur -> times -> cnt + 1){
                    LFUfreqnode *newfre = new(LFUfreqnode);
                    // check need to remove or not
                    if(cur -> times -> cnums == 1){
                        //cout<<"1hit at "<<cur->times->cnt<<endl;
                        newfre -> cnt = cur -> times -> cnt + 1;
                        newfre -> cnums = 0;
                        newfre -> prev = cur -> times -> prev;
                        //cur times need to delete
                        newfre -> next = cur -> times -> next;
                        newfre -> children = NULL;
                        if(cur -> times == head){
                            head = newfre;
                        }
                        else{
                            cur -> times -> prev -> next = newfre;
                        }
                        if(cur -> times == tail){
                            tail = newfre;
                        }
                        else{
                            cur -> times -> next -> prev = newfre;
                        }
                        free(cur->times);
                        move_node(cur, newfre);
                    }
                    else{
                        cout<<"2hit at "<<cur->times->cnt<<" ";
                        newfre -> cnt = cur -> times -> cnt + 1;
                        newfre -> cnums = 0;
                        newfre -> children = NULL;
                        newfre -> next = cur -> times;
                        if(cur -> times == head){
                            head = newfre;
                        }
                        else{
                            cur -> times -> prev -> next = newfre;
                        }
                        newfre -> prev = cur -> times -> prev;
                        cur -> times -> prev = newfre;
                        cur -> times -> cnums--;
                        // means the node is first child
                        if(cur -> prev == NULL){
                            cur -> next -> prev = NULL;
                            cur -> times -> children = cur -> next;
                        }
                        else{
                            // means not the last child
                            if(cur -> next != NULL){
                                cur -> next -> prev = cur -> prev;
                            }
                            cur -> prev -> next = cur -> next;   
                        }
                        move_node(cur, cur -> times -> prev);
                    }
                    // cout<<page<<": ";
                    // for(LFUfreqnode* s = head; s != NULL; s = s->next){
                    //     cout<<"node frequency: " <<s -> cnt<<" child num: "<< s->cnums<<" "; 
                    // }
                    // cout<<endl;
                }
                else{
                    // check if the node need to remove or not
                    //cout<<"3hit at "<<cur->times->cnt<<endl;
                    if(cur -> times -> cnums == 1){
                        cur -> times -> prev -> next = cur -> times -> next;      
                        if(cur -> times == tail){
                            tail = tail -> prev;
                        }
                        else{
                            cur -> times -> next -> prev = cur -> times -> prev;
                        }
                        free(cur -> times);
                        move_node(cur, cur -> times -> prev);
                    }
                    else{
                        cur -> times -> cnums--;
                        // means the node is first child
                        if(cur -> prev == NULL){
                            cur -> next -> prev = NULL;
                            cur -> times -> children = cur -> next;
                        }
                        else{
                            // means not the last child
                            if(cur -> next != NULL){
                                cur -> next -> prev = cur -> prev;
                            }
                            cur -> prev -> next = cur -> next;   
                        }
                        cur -> times = cur -> times -> prev;
                        move_node(cur, cur -> times);
                    }
                }
                if(cur -> page == 16385){
                    if(cur -> times ->prev != NULL){
                        cout<<"front"<<cur->times->prev->cnt;
                    if(cur -> times -> prev -> next != NULL){
                        cout<<"front to"<<cur->times -> prev -> next -> cnt;
                    }
                    }
                    cout<<" own"<<cur->times->cnt;
                    if(cur -> times -> next != NULL){
                        cout<<" back"<<cur->times->next->cnt;
                    }
                }
            }
            else{
                m++;
                // kill extra from tail's last
                if(f == i){
                    if(tail -> cnums > 1){
                        LFUnode* del = tail->children;
                        for(int j = 0;j < tail->cnums; j++, del = del->next);
                        tail->cnums--;
                        hash[del->page%sz].remove(make_pair(del->page,del));
                        del = del -> prev;
                        free(del->next);
                        del -> next = NULL;
                    }
                    else{
                        hash[tail->children->page%sz].remove(make_pair(tail->children->page,tail->children));
                        free(tail->children);
                        tail = tail -> prev;
                        free(tail->next);
                        tail -> next = NULL;
                    }
                    f--;
                }
                // head = null, no nodes, also tail == head
                if(head == NULL){
                    // create new frenode and point head and tail
                    LFUfreqnode* newfre = new(LFUfreqnode);
                    newfre -> cnt = 1;
                    newfre -> cnums = 0;
                    newfre -> children = NULL;
                    newfre -> next = NULL;
                    newfre -> prev = NULL;
                    // add lfu node to tail
                    insert_node(page, newfre, hash);
                    head = newfre;
                    tail = newfre;
                    tail -> next = NULL;
                    head -> prev = NULL;
                }
                else{
                    // just need to consider tail cnt and add newfre node or put at tail
                    if(tail -> cnt == 1){
                        insert_node(page, tail, hash);
                    }
                    else{
                        LFUfreqnode *newfree = new(LFUfreqnode);
                        newfree -> cnt = 1;
                        newfree -> children = NULL;
                        newfree -> next = NULL;
                        newfree -> prev = tail;
                        insert_node(page, newfree, hash);
                        if(head == tail){
                            head -> next = newfree;
                        }
                        tail -> next = newfree;
                        tail = newfree;
                    }
                }
                f++;
            }
            // for(LFUfreqnode* s = head; s != NULL; s = s-> next){
            //     cout<<"cnt: "<<s->cnt<<" nums: "<<s->cnums<<" children: ";
            //     for(LFUnode *k = s -> children; k != NULL; k = k -> next){
            //         cout<< k -> page << " ";
            //     }
            //     cout<<endl;
            // }
            // cout<<"hits "<<h<<endl;
            cout<<endl;
        }
        double pfr = m/(h+m);
        cout<<i<<"\t"<<h<<"\t"<<m<<"\t"<<pfr<<"\n";
        in.close();
    }
    gettimeofday(&end, 0);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    cout<<"Total elapsed time "<<elapsed<<" sec\n";

    cout<<"LRU policy:\nFrame\tHit\tMiss\tPage fault ratio\n";
    ifstream in2;
    gettimeofday(&start, 0);
    for(int i = 64; i <= 512; i<<=1){
        in2.open(argv[1]);
        f = 0;h = 0; m = 0;
        LRUnode *head = NULL, *tail = NULL, *cur;
        vector<list<pair<int,LRUnode*>>> hash;
        hash.resize(sz);
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
        double pfr = m/(h+m);
        cout<<i<<"\t"<<h<<"\t"<<m<<"\t"<<pfr<<"\n";
        in2.close();
    }
    gettimeofday(&end, 0);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    cout<<"Total elapsed time "<<elapsed<<" sec\n";
    return 0;
}
