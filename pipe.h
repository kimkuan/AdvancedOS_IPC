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
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>


#define LISTSIZ 54 //50개는 좌석, 좌석번호,선점여부
#define SHMKEY (key_t) 60024 //공유 메모리 키
#define COMKEY (key_t) 60025 //두번째 공유메모리 키
#define SEMKEY (key_t) 60026 //세마포어 키

#define MSG_SIZE 80
#define PIPENAME0 "./named_pipe_file0"	//서버 read용
#define PIPENAME1 "./named_pipe_file1"	//서버 write용


union semun {
	int val;
	struct semid_ds* buf;
	unsigned short int* aray;
}arg;

struct sembuf p = { 0,-1,0 };
struct sembuf v = { 0,1,0 };

