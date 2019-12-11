#include "pipe.h"

int fd0, fd1;
int semid;
int nread, rc;
char msg[MSG_SIZE];	//받아온 메세지
char snd[MSG_SIZE];	//보내는 메세지
int data[LISTSIZ][2];	//좌석 정보
int flag;	//성공여부
int shm1, shm2, shm3;

int status; //thread_join을 위해
pthread_t p_thread[3]; //쓰레드 생성하려면

int semid;
void input_read();
void data_send();


void getsem(void) {
	if ((semid = semget(SEMKEY, 1, IPC_CREAT | 0666)) == -1) {
		perror("semget failed");
		exit(1);
	}
	arg.val = 1;
	if (semctl(semid, 0, SETVAL, arg) == -1) {
		perror("semctl failed");
		exit(1);
	}
}

int makePipe() {	//파이프 생성
	/* 기존에 named pipe가 있으면 삭제 */
	if (access(PIPENAME0, F_OK) == 0) {
		unlink(PIPENAME0);
	}
	if (access(PIPENAME1, F_OK) == 0) {
		unlink(PIPENAME1);
	}

	/* named pipe 생성하기 */
	if ((rc = mkfifo(PIPENAME0, 0666)) < 0) {
		printf("fail to make named pipe\n");
		return 0;
	}
	/* named pipe 생성하기 */
	if ((rc = mkfifo(PIPENAME1, 0666)) < 0) {
		printf("fail to make named pipe\n");
		return 0;
	}

}

int pipefunc() {	//파이프 오픈
	/*fd0은 서버가 읽고 클라가 쓰는용*/
	if ((fd0 = open(PIPENAME0, O_RDONLY)) < 0) {
		printf("fail to open named pipe\n");
		return 0;
	}
	/*fd1은 서버가 쓰고 클라가 읽는용*/
	/* named pipe 열기, Read Write가능 해야 한다 */
	if ((fd1 = open(PIPENAME1, O_WRONLY)) < 0) {
		printf("fail to open named pipe\n");
		return 0;
	}
}
void writefunc(int flag) {		//파이프에 쓰는 함수
	snprintf(snd, sizeof(snd), "%d", flag);	//char 배열 형으로 변환
	if ((nread = write(fd1, snd, sizeof(snd))) < 0) {
		printf("fail to call write()\n");
	}
}

int readfunc() {	//파이프에서 읽어오는 함수
	if ((nread = read(fd0, msg, sizeof(msg))) < 0) {
		printf("fail to call read()\n");
	}
	return atoi(msg);	//int형으로 변환해서 전달
}

void input_write() {			
	pthread_create(&p_thread[1], NULL, input_read, NULL);
	pthread_join(p_thread[1], (void**)& status);
	return;
}

void data_read() {
	pthread_create(&p_thread[2], NULL, data_send, NULL);
	pthread_join(p_thread[2], (void**)& status);
	return;
}

void input_read() {		//입력값을 읽어오는 함수
	int num, flag,clipid;
	num = readfunc();	//좌석
	flag = readfunc();	//현재 좌석 선점 여부
	clipid=readfunc();	//client의 pid값

	if (flag == 1) { //반납의 경우
		data[num][0] = 0; //서버 데이터 수정
		writefunc(1);	//성공했다고 반환
		printf("반납\n");
	}
	if ((flag == 0) && (data[num][0] == 1)) {	//예약하려고했지만 이미 차있는 경우
		writefunc(0);	//실패했다고 반환
		printf("예약실패");
	}
	if ((flag == 0) && (data[num][0] == 0)) {	//예약하려고했고 좌석이 비어있는 경우
		writefunc(1);	//성공했다고 반환
		data[num][0] = 1;	//좌석 선점
		data[num][1]=clipid;	//좌석에 client pid저장
		printf("예약 성공");
	}
}

void data_send() {		//데이터 전송
	for (int i = 0; i < LISTSIZ-4; i++) {
		writefunc(data[i][0]);	//데이터 선점 여부 전송
	}
	printf("전송 완료\n");
}
void close_time() {	//열람실 종료시간 함수
	sleep(1000);
	printf("count\n");

	for (int i = 0; i < LISTSIZ - 4; i++) {		//클라이언트의 pid로 모두 종료시킴
		kill(data[i][1], SIGINT);
	}

}

void kill_client(){	//서버가 종료될때 클라이언트도 종료시킴
	for(int i = 0; i<LISTSIZ - 4; i++){
		if(data[i][1]){
			kill(data[i][1], SIGINT);	//클라이언트의 pid로 모두 종료시킴
			printf("client죽음\n");
		}
	}
	exit(0);
}
void main() {
	signal(SIGINT,kill_client);	//서버가 종료될때 kill client호출
	pthread_create(&p_thread[0], NULL, close_time, NULL);	//열람실 종료시간에 대한 쓰레드
	int menu;

	getsem();//서버가 세마포어 생성

	for (int i = 0; i < LISTSIZ; i++) {
		data[i][0] = 0;    //50개의 빈좌석
		data[i][1] = 0;
	}
	makePipe();	//파이프 생성
	pipefunc();	//파이프 오픈
	int pid=getpid();	//서버의 pid

	while (1) {	
		writefunc(pid);	//pid값을 넘겨줌
		menu = readfunc();	//메뉴를 읽어옴
		// printf("%d\n",menu);	//메뉴 출력
		switch (menu) {
			case 1:
				data_send();	//좌석 데이터 전송
				break;
			case 2:
				input_read();	//입력값 읽어옴
				break;
			case 3:
				printf("client 종료\n");	//종료
				break;
			default:
				break;
		}
	}
}


