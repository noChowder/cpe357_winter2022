#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#define MATRIX_DIMENSION_XY 10
//SEARCH FOR TODO
//
/***********************************************************************************
*************************************/
// sets one element of the matrix
void set_matrix_elem(float *M,int x,int y,float f){
    M[x + y*MATRIX_DIMENSION_XY] = f;
}
//
/***********************************************************************************
*************************************/
// lets see it both are the same
int quadratic_matrix_compare(float *A,float *B){
    for(int a = 0;a<MATRIX_DIMENSION_XY;a++)
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
            if(A[a +b * MATRIX_DIMENSION_XY]!=B[a +b * MATRIX_DIMENSION_XY]) 
                return 0;
   
    return 1;
}
//
/***********************************************************************************
*************************************/
//print a matrix
void quadratic_matrix_print(float *C){
    printf("\n");
    for(int a = 0;a<MATRIX_DIMENSION_XY;a++){
        printf("\n");
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
            printf("%.2f,",C[a + b* MATRIX_DIMENSION_XY]);
    }
    printf("\n");
}
//
/***********************************************************************************
*************************************/
// multiply two matrices
void quadratic_matrix_multiplication(float *A,float *B,float *C){
//nullify the result matrix first
    for(int a = 0;a<MATRIX_DIMENSION_XY;a++)
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
            C[a + b*MATRIX_DIMENSION_XY] = 0.0;
//multiply
    for(int a = 0;a<MATRIX_DIMENSION_XY;a++) // over all cols a
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++) // over all rows b
            for(int c = 0;c<MATRIX_DIMENSION_XY;c++){ // over all rows/cols left
                C[a + b*MATRIX_DIMENSION_XY] += A[c + b*MATRIX_DIMENSION_XY] * B[a + c*MATRIX_DIMENSION_XY]; 
            }
}
//
/***********************************************************************************
*************************************/
void quadratic_matrix_multiplication_parallel(int par_id, int par_count, float *A, float *B, float *C){
    //make row/id/count
    int rows = MATRIX_DIMENSION_XY / par_count;
    int mod_check = MATRIX_DIMENSION_XY % par_count;
    //printf("%d \n", MATRIX_DIMENSION_XY % par_count);
//nullify the result matrix first
    if(par_id == 0){
        for(int a = 0;a<MATRIX_DIMENSION_XY;a++)
            for(int b = 0;b<MATRIX_DIMENSION_XY;b++)
                C[a + b*MATRIX_DIMENSION_XY] = 0.0;
    }
//multiply
    if(par_id == 0 && mod_check > 0){ // do last mod_check rows
        for(int a = (MATRIX_DIMENSION_XY-mod_check);a<MATRIX_DIMENSION_XY;a++) // over all cols a
            for(int b = 0;b<MATRIX_DIMENSION_XY;b++) // over all rows b
                for(int c = 0;c<MATRIX_DIMENSION_XY;c++){ // over all rows/cols left
                    C[a + b*MATRIX_DIMENSION_XY] += A[c + b*MATRIX_DIMENSION_XY] * B[a + c*MATRIX_DIMENSION_XY]; 
                }
    }
    for(int a = (par_id * rows);a<(par_id * rows + rows);a++) // over all cols a
        for(int b = 0;b<MATRIX_DIMENSION_XY;b++) // over all rows b
            for(int c = 0;c<MATRIX_DIMENSION_XY;c++){ // over all rows/cols left
                C[a + b*MATRIX_DIMENSION_XY] += A[c + b*MATRIX_DIMENSION_XY] * B[a + c*MATRIX_DIMENSION_XY]; 
            }
}
//
/***********************************************************************************
*************************************/
void synch(int par_id,int par_count,int *ready){
//TODO: synch algorithm. make sure, ALL processes get stuck here until all ARE here
    //printf("par_id: %d, ready: %d \n", par_id, *ready);
    if(par_count == 1);
    else{
        // ready[par_id] = 1;
        // int escape = 1;
        // //printf("par_id: %d, ready: %d \n", par_id, *ready);
        // while(ready[par_id] == 1){
        //     if(escape == 1 && ready[par_id] == 0){
        //         continue;
        //     }
        //     for(int i = 0; i < par_count; i++){ // check all
        //         if(ready[i] == 0){
        //             escape = 0;
        //             break;
        //         }
        //     }
        //     if(escape == 1 && ready[par_id] == 1){
        //         for(int j = 0; j < par_count; j++){
        //             ready[j] = 0;
        //         }
        //         //sleep(2);
        //         break;
        //     }
        // }
        ready[par_id] += 1;
        int escape = 1;
        //printf("par_id: %d, ready: %d \n", par_id, ready[par_id]);
        while(1){
            for(int i = 0; i < par_count; i++){
                if(ready[par_id] > ready[i] && par_id != i){
                    // printf("id: %d, ready: %d \n", i, ready[i]);
                    //printf("par_id: %d, ready: %d \n", par_id, ready[par_id]);
                    escape = 0;
                    break;
                }
            }
            while(!escape){
                for(int i = 0; i < par_count; i++){
                    // printf("par_id: %d, ready: %d \n", par_id, ready[par_id]);
                    // printf("id: %d, ready: %d \n", i, ready[i]);
                    if(ready[par_id] < ready[i] && par_id != i){
                        escape = 1;
                        break;
                    }
                    // else if(ready[par_id] == ready[i] && par_id != i){
                    //     escape = 1;
                    //     continue;
                    // }
                }
                if(escape){
                    break;
                }
            }
            if(escape){
                ready[par_id] += 1;
                    break;
                }
        }


        // for(int i = 0; i < par_count; i++){
        //     while(ready[i] == 0);
        // }
        // ready[par_id] = 2;
        // if(par_id == 0){
        //     for(int i = 1; i < par_count; i++){
        //         if(ready[i] != 2){
        //             break;
        //         }
        //         while(ready[par_count-1] == 2);
        //     }
        //     ready[0] = 0;
        // }
        // else{
        //     while(ready[0] == 2);
        // }
    }
}
//
/***********************************************************************************
*************************************/
int main(int argc, char *argv[]){
    int par_id = 0; // the parallel ID of this process
    int par_count = 1; // the amount of processes
    float *A,*B,*C; //matrices A,B and C
    int *ready; //needed for synch
    if(argc!=3){
        printf("no shared\n");
    }
    else{
        par_id= atoi(argv[1]);
        par_count= atoi(argv[2]);
   // strcpy(shared_mem_matrix,argv[3]);
    }
    if(par_count==1){
        printf("only one process\n");
    }
    int fd[4];
    if(par_id==0){
        //TODO: init the shared memory for A,B,C, ready. shm_open with C_CREAT here! 
        //then ftruncate! then mmap
        fd[0] = shm_open("matrixA", O_RDWR | O_CREAT, 0777);
        fd[1] = shm_open("matrixB", O_RDWR | O_CREAT, 0777);
        fd[2] = shm_open("matrixC", O_RDWR | O_CREAT, 0777);
        fd[3] = shm_open("synchobject", O_RDWR | O_CREAT, 0777);
        ftruncate(fd[0], 100 * sizeof(float));
        ftruncate(fd[1], 100 * sizeof(float));
        ftruncate(fd[2], 100 * sizeof(float));
        ftruncate(fd[3], sizeof(int));
        A = (float *)mmap(NULL, 100*sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[0], 0);
        B = (float *)mmap(NULL, 100*sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[1], 0);
        C = (float *)mmap(NULL, 100*sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[2], 0);
        ready = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd[3], 0);
        ready[par_id] = 0;
    }
    else{
        //TODO: init the shared memory for A,B,C, ready. shm_open withOUT C_CREAT 
        //here! NO ftruncate! but yes to mmap
        fd[0] = shm_open("matrixA", O_RDWR, 0777);
        fd[1] = shm_open("matrixB", O_RDWR, 0777);
        fd[2] = shm_open("matrixC", O_RDWR, 0777);
        fd[3] = shm_open("synchobject", O_RDWR, 0777);
        A = (float *)mmap(NULL, 100*sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[0], 0);
        B = (float *)mmap(NULL, 100*sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[1], 0);
        C = (float *)mmap(NULL, 100*sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, fd[2], 0);
        ready = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd[3], 0);
        ready[par_id] = 0;
        //*ready = 0;
        sleep(2); //needed for initalizing synch
    }
    //printf("par_id: %d, ready: %d \n", par_id, *ready);
    synch(par_id,par_count,ready);
    //printf("after synch par_id: %d, ready: %d \n", par_id, *ready);
    if(par_id==0){
        //TODO: initialize the matrices A and B
        for(int c = 0;c<MATRIX_DIMENSION_XY;c++)
            for(int r = 0;r<MATRIX_DIMENSION_XY;r++)
                set_matrix_elem(A, c, r, 2);
        for(int c = 0;c<MATRIX_DIMENSION_XY;c++)
            for(int r = 0;r<MATRIX_DIMENSION_XY;r++)
                set_matrix_elem(B, c, r, 3);
    }
    //*ready++;
    //printf("par_id: %d, ready: %d \n", par_id, *ready);
    synch(par_id,par_count,ready);
    //printf("after synch par_id: %d, ready: %d \n", par_id, *ready);
    //TODO: quadratic_matrix_multiplication_parallel(par_id, par_count,A,B,C, ...);
    quadratic_matrix_multiplication_parallel(par_id, par_count, A, B, C);
    //*ready++;
    //printf("par_id: %d, ready: %d \n", par_id, *ready);
    synch(par_id,par_count,ready);
    //printf("after synch par_id: %d, ready: %d \n", par_id, *ready);
    if(par_id==0)
        quadratic_matrix_print(C);
    //*ready++;
    synch(par_id, par_count, ready);
    //lets test the result:
    float M[MATRIX_DIMENSION_XY * MATRIX_DIMENSION_XY];
    quadratic_matrix_multiplication(A, B, M);
    if (quadratic_matrix_compare(C, M)){
        printf("full points!\n");
        // printf("id: %d \n", par_id);
    }
    else
        printf("buuug!\n");
    close(fd[0]);
    close(fd[1]);
    close(fd[2]);
    close(fd[3]);
    shm_unlink("matrixA");
    shm_unlink("matrixB");
    shm_unlink("matrixC");
    shm_unlink("synchobject");
    return 0;    
}