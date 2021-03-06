#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned char BYTE;
#define PAGESIZE 4096

typedef struct chunkhead{
    unsigned int size;
    unsigned int info;
    unsigned char *next, *prev;
}chunkhead;

unsigned int heapsize = 0;
void *startofheap = NULL;

chunkhead *get_last_chunk(){
    if(!startofheap){
        return NULL;
    }
    chunkhead *ch = (chunkhead *)startofheap;
    for( ; ch->next; ch = (chunkhead *)ch->next);
    return ch;
}

chunkhead *get_best_chunk(unsigned int reqSize, chunkhead *currChunk){
    if(!startofheap){
        return NULL;
    }
    chunkhead *ch = (chunkhead *)startofheap;
    chunkhead *best = NULL;
    for( ; ch->next; ch = (chunkhead *)ch->next){
        if(ch->info == 0 && ch->size >= reqSize && ch->size <= currChunk->size){
            best = ch;
        }
    }
    if(best){
        return best;
    }
    else{
        return ch; // no free chunks
    }
}

unsigned char *mymalloc(unsigned int size){
    chunkhead *currChunk; // current working chunk
    chunkhead *nextChunk; // next working chunk
    //currChunk = (chunkhead *)startofheap;

    /* define required size of chunk */
    unsigned int reqSize = 0;
    if(size / PAGESIZE == 0){
        reqSize = PAGESIZE;
    }
    else{
        reqSize = ((int)(size / PAGESIZE) + 1) * PAGESIZE;
    }

    
    if(heapsize == 0){ // no heap 
        //sbrk(0);
        heapsize = 1;
        currChunk = sbrk(reqSize);
        currChunk->info = 1;
        currChunk->size = reqSize;
        heapsize = 1;
        startofheap = currChunk; // set start of heap
        return (BYTE *)currChunk + sizeof(chunkhead);
    }
    currChunk = (chunkhead *)startofheap;

    currChunk = get_best_chunk(reqSize, currChunk);
    if(currChunk->next == NULL){
        nextChunk = sbrk(reqSize);
        nextChunk->info = 1;
        nextChunk->size = reqSize;
        nextChunk->next = NULL;
        nextChunk->prev = (BYTE *)currChunk;
        currChunk->next = (BYTE *)nextChunk;
        return (BYTE *)nextChunk + sizeof(chunkhead);
    }
    else{
        currChunk->info = 1;
        //nextChunk = (chunkhead *)currChunk->next;
        if(currChunk->size > reqSize){
            unsigned int free = currChunk->size - reqSize;
            BYTE *address = (BYTE *)currChunk + reqSize;
            nextChunk = (chunkhead *)address;
            nextChunk->info = 0;
            nextChunk->size = free;
            nextChunk->next = currChunk->next;
            nextChunk->prev = (BYTE *)currChunk;
            ((chunkhead *)currChunk->next)->prev = (BYTE *)nextChunk;
            currChunk->size = reqSize;
            currChunk->next = (BYTE *)nextChunk;
        }
        return (BYTE *)currChunk + sizeof(chunkhead);
    }

    return NULL;
}

void myfree(unsigned char *address){
    chunkhead *currChunk = (chunkhead *)(address - sizeof(chunkhead)); // current working chunk
    chunkhead *nextChunk = (chunkhead *)(currChunk->next); // next working chunk
    chunkhead *prevChunk = (chunkhead *)(currChunk->prev); // previous working chunk
    currChunk->info = 0;

    /* prev NULL && next NULL */
    if(!prevChunk && !nextChunk){
        startofheap = NULL;
        heapsize = 0;
        brk(address-sizeof(chunkhead));
    }
    /* prev NULL && next 0 */
    else if(!prevChunk && nextChunk->info == 0){
        currChunk->next = nextChunk->next;
        currChunk->size += nextChunk->size;
        if(nextChunk->next){
            ((chunkhead *)nextChunk->next)->prev = (BYTE *)currChunk;
        }
    }
    /* prev NULL && next 1 */
    else if(!prevChunk && nextChunk->info == 1){
        // do nothing
    }
    /* prev 0 && next NULL */
    else if(prevChunk->info == 0 && !nextChunk){
        prevChunk->next = NULL;
        prevChunk->size += currChunk->size;
        currChunk = (chunkhead *)prevChunk;
        brk(address-sizeof(chunkhead));
    }
    /* prev 0 && next 0 */
    else if(prevChunk->info == 0 && nextChunk->info == 0){
        prevChunk->next = nextChunk->next;
        prevChunk->size += currChunk->size + nextChunk->size;
        currChunk = prevChunk;
        if(nextChunk->next){
            ((chunkhead *)nextChunk->next)->prev = (BYTE *)currChunk;
        }
        if(prevChunk->prev){
            ((chunkhead *)prevChunk->prev)->next = (BYTE *)currChunk;
        }
    }
    /* prev 0 && next 1 */
    else if(prevChunk->info == 0 && nextChunk->info == 1){
        prevChunk->next = (BYTE *)nextChunk;
        prevChunk->size += currChunk->size;
        nextChunk->prev = (BYTE *)prevChunk;
    }
    /* prev 1 && next NULL */
    else if(prevChunk->info == 1 && !nextChunk){
        prevChunk->next = NULL;
        currChunk = prevChunk;
        brk(address-sizeof(chunkhead));
    }
    /* prev 1 && next 0 */
    else if(prevChunk->info == 1 && nextChunk->info == 0){
        currChunk->next = nextChunk->next;
        currChunk->size += nextChunk->size;
        if(nextChunk->next){
            ((chunkhead *)nextChunk->next)->prev = (BYTE *)currChunk;
        }
    }
    /* prev 1 && next 1 */
    else if(prevChunk->info == 1 && nextChunk->info == 1){
        // do nothing
    }

    /* no heap */
    if(currChunk->info == 0 && currChunk->prev == NULL && currChunk->next == NULL){
        startofheap = NULL;
        heapsize = 0;
        brk(currChunk);
    }
}

void analyze(){
    printf("\n--------------------------------------------------------------\n");
    if(!startofheap){
        printf("no heap, ");
            printf("program break on address %x", sbrk(0));
        return;
    }
    chunkhead *ch = (chunkhead *)startofheap;
    for(int no = 0; ch; ch = (chunkhead *)ch->next, no++){
        printf("%d | current addr: %x |", no, ch);
        printf("size: %d | ", ch->size);
        printf("info: %d | ", ch->info);
        printf("next: %x | ", ch->next);
        printf("prev: %x", ch->prev);
        printf("      \n");
    }
    printf("program break on address %x\n", sbrk(0));
}

void main(){
    BYTE *a[100];
    analyze(); // 50% points
    for(int i = 0; i < 100; i++){
        a[i] = mymalloc(1000);
    }
    for(int i = 0; i < 90; i++){
        myfree(a[i]);
    }
    analyze(); // 50% of points if this is correct
    myfree(a[95]);
    //myfree(a[96]);
    //myfree(a[98]);
    //analyze();
    a[95] = mymalloc(1000);
    //analyze();
    //a[96] = mymalloc(1000);
    //analyze();
    //a[98] = mymalloc(1000);
    analyze(); // 25% points, this new chunk should fill the smaller free one
    // (best fit)
    for(int i = 90; i < 100; i++){
        myfree(a[i]);
    }
    analyze(); // 25% should be an empty heap now with the start address
    // from the program start
    printf("\n");
}