#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <sys/time.h>

using namespace std;
struct LRU_node {
	int adder;
	LRU_node *prev, *next;
};

struct LFU_Node {
    int freq, seq, addr;
    LFU_Node(int f, int s, int a) : freq(f), seq(s), addr(a) {}
};

struct LFU_Cache {
    vector<LFU_Node> heap; 
    unordered_map<int, size_t> addr_to_pos; // address check

    LFU_Cache() {}

    void insert(int addr, int seq) {
        if (addr_to_pos.find(addr) != addr_to_pos.end()) {
            // 如果已經在cache中，update freq和seq
            size_t pos = addr_to_pos[addr];
            heap[pos].freq++;
            heap[pos].seq = seq;
            siftDown(pos);
        } else {
            // 如果不在cache中，add new node
            heap.emplace_back(1, seq, addr);
            size_t pos = heap.size() - 1;
            addr_to_pos[addr] = pos;
            siftUp(pos);
        }
    }

    int evict() {
        // 從heap頭部移除一個node(freq最小的)
        LFU_Node node = heap.front();
        swap(heap.front(), heap.back());
        heap.pop_back();
        addr_to_pos.erase(node.addr);
        siftDown(0);
        return node.addr;
    }

    void siftUp(size_t pos) {
        while (pos > 0) {
            size_t parent = (pos - 1) / 2;
            if (heap[pos].freq < heap[parent].freq ||
                (heap[pos].freq == heap[parent].freq && heap[pos].seq < heap[parent].seq)) {
                swap(heap[pos], heap[parent]);
                addr_to_pos[heap[pos].addr] = pos;
                addr_to_pos[heap[parent].addr] = parent;
                pos = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t pos) {
        size_t left, right, smallest;
        while (true) {
            left = 2 * pos + 1;
            right = 2 * pos + 2;
            smallest = pos;

            if (left < heap.size() &&
                (heap[left].freq < heap[smallest].freq ||
                 (heap[left].freq == heap[smallest].freq && heap[left].seq < heap[smallest].seq))) {
                smallest = left;
            }

            if (right < heap.size() &&
                (heap[right].freq < heap[smallest].freq ||
                 (heap[right].freq == heap[smallest].freq && heap[right].seq < heap[smallest].seq))) {
                smallest = right;
            }

            if (smallest != pos) {
                swap(heap[pos], heap[smallest]);
                addr_to_pos[heap[pos].addr] = pos;
                addr_to_pos[heap[smallest].addr] = smallest;
                pos = smallest;
            } else {
                break;
            }
        }
    }
};


int main(int argc, char** argv) {
    // check set
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " trace.txt\n";
        return -1;
    }

    FILE *trace;
    int address, hit = 0, miss = 0;
    timeval start, end;
    double sec;

    // LFU
    gettimeofday(&start, NULL);
    puts("LFU policy:");
    puts("Frame\tHit\t\tMiss\t\tPage fault ratio");

    for (int F = 64; F <= 512; F <<= 1) {
        LFU_Cache cache;
        trace = fopen(argv[1], "r");
        hit = 0;
        miss = 0;
        int seq = 0;

        while (fscanf(trace, "%d", &address) != EOF) {
            seq++;
            if (cache.addr_to_pos.find(address) != cache.addr_to_pos.end()) {
                // Hit
                hit++;
                cache.insert(address, seq);
            } else {
                // Miss
                miss++;
                if (cache.heap.size() >= F) {
                    cache.evict();
                }
                cache.insert(address, seq);
            }
        }

        fclose(trace);
        printf("%d\t%d\t\t%d\t\t%.10f\n", F, hit, miss, miss * 1.0 / (hit + miss));
    }

    gettimeofday(&end, 0);
    sec = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Total elapsed time %.4f sec\n", sec);

    //LRU
    puts("\nLRU policy:");
    gettimeofday(&start,NULL);
    puts("Frame\tHit\t\tMiss\t\tPage fault ratio");

    for(int F=64;F<=512;F<<=1){
        vector<pair<int,LRU_node*>> LRU[1024];
        LRU_node *head=nullptr,*tail=nullptr,*cur=nullptr;
        trace=fopen(argv[1],"r");
        hit=0;
        miss=0;
        int page_num=0;
        while(fscanf(trace,"%d",&address)!=EOF){
             cur=nullptr;
             //printf("address:%d\n",address);
             for(auto &id:LRU[address%1024]){
                if(id.first==address){
                  cur=id.second;
                  break;
                }
             }
             if(cur!=nullptr){
                hit++;               
                if(cur==head){
                    continue;
                }
                cur->prev->next=cur->next;
                if(cur==tail){
                    tail=tail->prev;
                    tail->next=nullptr;
                }
                else{
                    cur->next->prev=cur->prev;
                }
                cur->next=head;
                cur->prev=nullptr;
                head->prev=cur;
                head=cur;
                continue;
                
             }
             miss++;
             while(page_num==F){
                 LRU[tail->adder%1024].erase(remove(LRU[tail->adder%1024].begin(),LRU[tail->adder%1024].end(),make_pair(tail->adder,tail)));
                 tail=tail->prev;
                 tail->next=nullptr;
                 page_num--;
             }
             LRU_node *new_node=new LRU_node;
             new_node->adder=address;
             new_node->next=head;
             new_node->prev=nullptr;
             if(head!=nullptr){
                 head->prev=new_node;
             }
             head=new_node;
             if(page_num==0){
                 tail=new_node;
             }
             LRU[address%1024].push_back(make_pair(address,new_node));
             page_num++;
        }
        fclose(trace);
		printf("%d\t%d\t\t%d\t\t%.10f\n", F, hit, miss, miss*1./(hit+miss));
    }
    gettimeofday(&end, 0);
	sec = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
	printf("Total elapsed tme %.4f sec\n", sec);

	return 0;
}