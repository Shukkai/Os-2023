/*
Student No.: 110550110
Student Name: 林書愷
Email: kyle.lin0908.nycu.cs10@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
typedef struct  block
{
    size_t size;
    int free;
    struct block *prev;
    struct block *next;
}block;
void *all = NULL;
#define length  20000
block* pool;
void *malloc(size_t size){
    
    if(size == 0){
        size_t largest = 0;
        for(block* cur = pool; cur != NULL; cur = cur->next){
            if(cur->size > largest && cur->free == 1){
                largest = cur->size;
            }
        }
        char message[100]={};
        int m_length = sprintf(message,"Max Free Chunk size = %zu\n", largest);
        write(1,message,strlen(message));
        if(pool != NULL){
            munmap(pool, length);
        }
        return NULL;
    }
    size = (size+31)/32*32;
    if(all == 0){
        all = mmap(0, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        pool = all;
        pool->size = length - sizeof(block);
        pool->free = 1;
        pool->prev = NULL;
        pool->next = NULL;

    }
    for(block* cur = pool; cur != NULL; cur = cur->next){
        if(cur->size > (size)){
            if(cur->free == 0){
                continue;
            }
            else{
                block* new_block;
                new_block = cur + 1 + (size/32);
                new_block->free = 1;
                new_block->size = cur->size - size - sizeof(block);
                new_block->next = cur->next;
                new_block->prev = cur;
                if(cur->next != NULL){
                    cur->next->prev = new_block;
                }
                cur->size = size;
                cur->free = 0;
                cur->next = new_block;
                
                return cur+1;
            }
        }
        else if(cur->size == (size)){
            if(cur->free == 0){
                continue;
            }
            else{
                cur->size = size;
                cur->free = 0;
                return cur+1;
            }
        }
    }
    return 0;
}
void free(void *ptr){
    if(ptr == NULL){
        return;
    }
    block* cur = ptr;
    cur--;
    cur->free = 1;
    if(cur->next != NULL && cur->next->free == 1){
        cur->size += sizeof(block) + cur->next->size;
        if(cur->next->next != NULL){
            cur->next->next->prev = cur;
            cur->next = cur->next->next;
        }
        else{
            cur->next = NULL;
        }
    }
    if(cur->prev != NULL && cur->prev->free == 1){
        cur->prev->size += sizeof(block) + cur->size;
        cur->prev->next = cur->next;
        if(cur->next != NULL){
            cur->next->prev = cur-> prev;
        }
    }
}