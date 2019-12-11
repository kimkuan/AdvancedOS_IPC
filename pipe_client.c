#include "pipe.h"

char msg[MSG_SIZE];
char snd[MSG_SIZE];
char c;
int fd0, fd1;
int nread, i;
int shm1, shm2;
int* shmaddr;//공유 메모리 영역
int* shmaddr2;//공유 메모리 영역
int serverpid, clientpid;//시그널 전송을 위한 pid
int flag = 0;	//성공 여부
int c_flag = 0; //예약한 좌석이 있는지에 대한 확인
int my_seat = 0;	//현재 차지한 좌석 정보
int dr, iw, timer;
int num; //지정한 좌석번호

void input_write(int num);
void writefunc(int flag);
int readfunc();
void input_write(int num);
void data_read();
void time_on();
pthread_t p_thread[3];

int semid;

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

int pipefunc() {	//파이프 읽어오는 함수
	/*fd0은 서버가 읽고 클라가 쓰는용*/
	if ((fd0 = open(PIPENAME0, O_WRONLY)) < 0) {
		printf("fail to open named pipe\n");
		return 0;
	}
	/*fd1은 서버가 쓰고 클라가 읽는용*/
	/* named pipe 열기, Read Write가능 해야 한다 */
	if ((fd1 = open(PIPENAME1, O_RDONLY)) < 0) {
		printf("fail to open named pipe\n");
		return 0;
	}
}

int readfunc() {	//파이프에서 읽어오는 함수
	if ((nread = read(fd1, msg, sizeof(msg))) < 0) {
		printf("fail to call read()\n");
	}
	return atoi(msg);
}

void writefunc(int flag) {	//파이프에 써주는 함수
	snprintf(snd, sizeof(snd), "%d", flag);
	if ((nread = write(fd0, snd, sizeof(snd))) < 0) {
		printf("fail to call write()\n");
	}
}

void time_on() {	//좌석 대여 시간 재줌
	sleep(4000);
	printf("\ntime out!!");
	writefunc(2);	//현재 가지고있는 좌석값을 반환시켜주기 위함
	serverpid = readfunc();
	iw= pthread_create(&p_thread[1], NULL, input_write, num);	//가지고 있는 좌석 반납
	if (iw < 0) {
		perror("thread create error : ");
		exit(0);
	}
}

void data_read() {	//데이터를 읽어옴
	int read;
	sleep(1);

	for (int i = 0; i < LISTSIZ - 4; i++) {
		read = readfunc();	//좌석값을 읽어옴
		printf("%d번 좌석 : %d\n", i , read);	//좌석과 선점여부
	}
	printf("현재 내 자리 = %d\n", my_seat);	//현재 내 좌석
	pthread_exit(NULL);
}

void kill_cli(){
	writefunc(2);
	serverpid = readfunc();
	input_write(3);
	exit(0);
}
void input_write(int num) {	//예약하고자하는 좌석값을 입력해줌
	int pid=getpid();	//클라이언트의 pid값을 가져옴
	semop (semid,&p,1);	//세마포어 시작
	writefunc(num);	//좌석값을 서버에 전송
	writefunc(c_flag);	//클라이언트의 좌석 선점 여부 전송
	writefunc(pid);	//클라이언트의 pid를 넘겨줌
	flag = readfunc();	//성공 여부를 받아옴
	sleep(1);

	if (flag == 1) {	//성공한 경우
		if (c_flag == 1) {	//클라이언트가 좌석을 가지고있는 경우
			printf("반납 성공\n\n");
			my_seat = 0;	//좌석정보 초기화
			c_flag = 0;	
			pthread_cancel(p_thread[2]);	//타이머 꺼줌
		}
		else {
			printf("좌석예약 성공\n\n");
			my_seat = num;	//좌석 정보를 넣어줌
			c_flag = 1;	
			timer = pthread_create(&p_thread[2], NULL, time_on, NULL);	//타이머를 켜줌
			if (timer < 0) {
				perror("thread create error : ");
				exit(1);
			}
		}
	}
	else if (flag == 0) {	//실패라는 결과를 받은 경우
		printf("이미 예약된 좌석\n\n");

	}
	semop (semid,&v,1);	//세마포어 해제
	pthread_exit(NULL); 

}


void main(void) {
	pipefunc();	//파이프 연결
	signal(SIGINT, kill_cli);	
	int status; 
	int read;

	bool exitOuterLoop = false; //바깥 루프 break
	int com;
	while (1) {
		com = 0;
		printf("--------------------\n1 = 좌석현황 보기\n2 = 반납or예약\n3 = 종료\n--------------------\n\n명령  입력 : ");	//메뉴 출력
		scanf("%d", &com);
		if(com<4){
			serverpid = readfunc();	//서버의 pid값을 읽어옴
		}
		if (0 < com < 4) {
			switch (com) {
				case 1:  //공유 메모리에서 좌석정보 읽어옴
					writefunc(1);	//메뉴1을 넘겨줌
					dr = pthread_create(&p_thread[0], NULL, data_read, NULL);
					if (dr < 0) {
						perror("thread create error : ");
						exit(0);
					}
					pthread_join(p_thread[0], (void**)& status);
					break;
				case 2:	//예약 혹은 반납
					printf("좌석 번호 선택: ");	//추가적으로 좌석 정보 입력
					scanf("%d", &num);
					if (c_flag == 1) {	//현재 좌석이 있는 경우
						if (num != my_seat) {	//선택좌석과 현재 좌석이 같지않은 경우
							printf("반납먼저\n");
							break;
						}
					}
					writefunc(2);	//메뉴2을 넘겨줌
					iw = pthread_create(&p_thread[1], NULL, input_write, num);
					if (iw < 0) {
						perror("thread create error : ");
						exit(0);
					}
					pthread_join(p_thread[1], (void**)& status);

					break;
				case 3:
					writefunc(3);	//메뉴 3을 넘겨줌
					if(c_flag){
						writefunc(2);
						serverpid = readfunc();
						input_write(my_seat);
					}
					exitOuterLoop = true;
					printf("안녕! 잘 가!\n");
					break;
				default: printf("command error try again.\n");
					 break;
			}
		}
		if (exitOuterLoop == true) //이중루프 탈출
			break;
	}

}


