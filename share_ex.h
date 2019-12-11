#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>

#define LISTSIZ 54 //50개는 좌석, 좌석번호,선점여부
#define SHMKEY (key_t) 60029 //공유 메모리 키
#define SEMKEY (key_t) 60027 //세마포어 키

int semid;

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short int *aray;
}arg;

struct sembuf p = {0,-1,0};
struct sembuf v = {0,1,0};

void getsem (void){
	if((semid=semget(SEMKEY, 1, IPC_CREAT|0666)) == -1){
		perror("semget failed");
		exit(1);
	}
	arg.val = 1;
	if(semctl(semid, 0, SETVAL, arg) == -1){
		perror("semctl failed");
		exit(1);
	}
}

//int com_buf;
//int *list[LISTSIZ];
//인덱스 49까지 50개 좌석
//인덱스 50 = 클라이언트가 입력한 좌석
//인덱스 51 = 좌석 소유 여부
//인덱스 52 = 클라이언트 pid
//인덱스 53 = 서버 pid
////




