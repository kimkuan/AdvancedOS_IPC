#include "share_ex.h"

int *data[LISTSIZ][2];

int flag;
int shm1;
int *shmaddr;

int status; //thread_join을 위해
pthread_t p_thread[3]; //쓰레드 생성하려면
//클라이언트에 입력된 데이터 처리
void input_read(){
	int num, clipid, flag;//좌석번호, 클라이언트의 pid, 클라이언트의 flag
	num = shmaddr[LISTSIZ -4] - 1;
	flag = shmaddr[LISTSIZ - 3];
	clipid = shmaddr[LISTSIZ -2];
	
	if (flag == 1){ //반납의 경우
		data[num][0] = 0; //서버 데이터 수정
		shmaddr[LISTSIZ-3] = 1;//성공 알림
		data[num][1] = NULL;
	}
	//좌석 예약, 좌석이 이미 예약
	if ((flag ==0) && (data[num][0] == 1)){
		shmaddr[LISTSIZ -3 ] = 0;
	}
	//좌석 예약 좌석이 비어있음
	if ((flag == 0) && (data[num][0] == 0)){
		shmaddr[LISTSIZ -3] = 1;
		data[num][0] = 1;
		data[num][1] = clipid;
	}

}
//클라이언트의 요청에 따라 서버 데이터 전송
void data_send(){
	for(int i = 0; i < LISTSIZ; i++){
		shmaddr[i] = data[i][0];
	}
}
//클라이언트의 input_write로 인한 시그널에 대한 쓰레드 생성
void input_write(){
	pthread_create(&p_thread[1], NULL, input_read, NULL);
	pthread_join(p_thread[1], (void**)&status);
	return ;
}
//클라이언트의 data_read로 인한 시그널에 대한 쓰레드 생성
void data_read(){
	pthread_create(&p_thread[2], NULL, data_send, NULL);
	pthread_join(p_thread[2], (void**)&status);
	return ;
}
//실시간으로 종료시간 확인
void close_time(){
	struct tm tm;
	//현재시간 받음
	time_t ct;
	ct = time(NULL);
	tm = *localtime(&ct);
	//종료시간 21시에서 현재시간 뺌
	int hour = 2 - tm.tm_hour;
	int min = 33 - tm.tm_min;;
	int sec = 0 - tm.tm_sec;
	if(min < 0){
		hour -= 1;
		min = 60- tm.tm_min;
	}
	if(sec < 0){
		min -= 1;
		sec = 60 - tm.tm_sec;
	}
	//지정한 시간까지 남은 시간 계산
	int left_time = (hour * 3600) + (min * 60) + sec;
	sleep(left_time);

	for(int i = 0; i<LISTSIZ - 4; i++){
		if(data[i][1])
			kill(data[i][1], SIGINT);
		else
			continue;
	}
}

void main (){
	pthread_create(&p_thread[0], NULL, close_time, NULL);

	signal(SIGUSR1, input_write);//시그널핸들러
	signal(SIGUSR2, data_read);
	getsem();//서버가 세마포어 생성

	/*키 60029의데이터를 위한  공유메모리 생성*/
	if((shm1 = shmget(SHMKEY, sizeof(int)*LISTSIZ, IPC_CREAT|0666)) == -1){
		perror("shm1 failed");
		exit(1);
	}
	//키 60029의 공유메모리 붙임
	if((shmaddr = (int *)shmat(shm1, NULL, 0)) == (void *)-1){
		perror("shmat failed");
		exit(1);
	}

	for(int i = 0; i < LISTSIZ; i++){
		data[i][0] = 0;    //50개의 빈좌석
		data[i][0] = NULL;
	}
	data[LISTSIZ - 1][0] = getpid(); //서버 pid


	for (int i = 0; i < LISTSIZ; i++){ //공유 메모리에 좌석 정보입력
		shmaddr[i] = data[i][0];
	}
	printf("server start\n");
	while(1){}
}


