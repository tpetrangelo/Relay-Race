#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 6
#define M 6
#define P 6

int A[N][M];
int B[M][P];
int R[N][P];

int C[N][M];
int D[M][P];
int S[N][P];
int num_threads;
int winner = 1;
int countA = 0;
int countB = 0;

pthread_barrier_t barrier;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;;

pthread_mutex_t mut  = PTHREAD_MUTEX_INITIALIZER;;
pthread_mutex_t win  = PTHREAD_MUTEX_INITIALIZER;;

typedef struct{
  long id;
  char team;
}prime_struct;

void computeA(long id, char tTeam);
void computeB(long id, char tTeam);

void* start(void* input) {

  prime_struct *functArgs = input;
  long tid = functArgs->id;
  char tTeam = functArgs->team;

  pthread_barrier_wait(&barrier);
  if(tTeam=='A'){
    computeA(tid,tTeam);
  }
  else{
    computeB(tid,tTeam);
  }

  pthread_exit(NULL);
}

void computeA(long id, char tTeam) {
  pthread_mutex_lock(&mut);
  while(countA != id){
    pthread_cond_wait(&condition,&mut);
  }

  int totalCells = N * P;
  int cellsPerThread = totalCells / num_threads;
  int num_iterations = cellsPerThread;

  if (id == num_threads - 1) {
    num_iterations = totalCells - (cellsPerThread * id);
  }

  for (int i = 0; i < num_iterations; i++) {
    int next = cellsPerThread * id + i;
    int x = (next - next % P) / P;
    int y = next % P;
    R[x][y] = 0;

    for (int k = 0; k < M; k++) {
      R[x][y] += A[x][k]*B[k][y];
    }
  }

  if(id != num_threads-1){
    countA++;
    pthread_cond_broadcast(&condition);
  }
  else if(id == num_threads-1){
    pthread_mutex_lock(&win);
    winner = winner - 1;
    if(winner == 0){
      printf("Red Wins!\n");
    }
    pthread_mutex_unlock(&win);
  }
  pthread_mutex_unlock(&mut);
}

void computeB(long id, char tTeam) {
  pthread_mutex_lock(&mut);
  while(countB != id){
    pthread_cond_wait(&condition,&mut);
  }

  int totalCells = N * P;
  int cellsPerThread = totalCells / num_threads;
  int num_iterations = cellsPerThread;

  if (id == num_threads - 1) {
    num_iterations = totalCells - (cellsPerThread * id);
  }
  for (int i = 0; i < num_iterations; i++) {
    int next = cellsPerThread * id + i;
    int x = (next - next % P) / P;
    int y = next % P;
    S[x][y] = 0;

    for (int k = 0; k < M; k++) {
      S[x][y] += C[x][k]*D[k][y];
    }
  }

  if(id != num_threads-1){
    countB++;
    pthread_cond_broadcast(&condition);
  }
  else if(id == num_threads-1){
    pthread_mutex_lock(&win);
    winner = winner - 1;
    if(winner == 0){
      printf("Blue wins!\n");

    }
    pthread_mutex_unlock(&win);
  }
  pthread_mutex_unlock(&mut);
}


void print(int rows, int columns, int matrix[rows][columns]) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
}

// To compile:
// gcc -pthread RelayRace.c -o relay
// To run with 2 threads:
// ./relay 2
int main(int argc, char** args) {
  num_threads = atoi(args[1]);
  int rcR,rcB;

  pthread_t* Rthreads = malloc(num_threads * sizeof(pthread_t));
  pthread_t* Bthreads = malloc(num_threads * sizeof(pthread_t));

  pthread_barrier_init(&barrier, NULL, num_threads*2);
  pthread_cond_init(&condition, NULL);
  pthread_mutex_init(&mut, NULL);

  prime_struct threadA[num_threads];
  prime_struct threadB[num_threads];

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      A[i][j] = i;
    }
  }
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < P; j++) {
      B[i][j] = i;
    }
  }

  for (int i = 0; i < num_threads; i++) {
    threadA[i].team = 'A';
    threadB[i].team = 'B';
    threadA[i].id = i;
    threadB[i].id = i;
    pthread_create(&Rthreads[i], NULL, start, &threadA[i]);
    pthread_create(&Bthreads[i], NULL, start, &threadB[i]);
  }

  for (int i = 0; i < num_threads; i++) {
    int* result = NULL;
    rcR = pthread_join(Rthreads[i], (void**)(&result));
    rcB = pthread_join(Bthreads[i], (void**)(&result));
  }

  pthread_barrier_destroy(&barrier);
  return 0;
}
