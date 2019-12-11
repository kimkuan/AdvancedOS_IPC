#include "share_ex.h"

pthread_t p_thread[3];//
int shm1, shm2;
int *shmaddr;//공유 메모리 영역
int serverpid, clientpid;//시그널 전송을 위한 pid
int flag= 0; //예약한 좌석이 있는지에 대한 확인
int my_seat = 0;
int dr, iw, timer;
int num; //지정한 좌석번호

void input_write (int num);
void time_on();
//좌석 시간재는 함수
void time_on(){
	sleep(7200); //2시간 동안 자리 사용할 수 있음
	printf("\ntime out!!");
	iw = pthread_create(&p_thread[1], NULL, input_write, num); //반납 루틴
	if(iw < 0){
		perror("thread create error : ");
		exit(0);
	}
}
// 좌석 데이터 읽어오는 함수
void data_read(){
	kill(serverpid, SIGUSR2);
	sleep(1);

	for(int i = 0; i < LISTSIZ - 4; i++){
		printf("%d번 좌석 : %d\n", i + 1, shmaddr[i]);
	}
	printf("현재 내 자리 = %d\n", my_seat);
	pthread_exit(NULL);
}
//입력 좌석값을 서버에게 전송하는 함수
void input_write (int num){
	semop (semid, &p, 1);//임계영역 진입
	shmaddr[LISTSIZ - 4] = num; //좌석 번호
	shmaddr[LISTSIZ - 3] = flag;
	shmaddr[LISTSIZ - 2] = clientpid; //클라이언트 pid 입력
	kill(serverpid, SIGUSR1);//클라이언트의 요청을 서버에게 확인시키기 위한 시그널
	sleep(1);
	if(shmaddr[LISTSIZ -3] == 1){
		if (flag == 1){	//좌석 반납 성공
			printf("반납 성공\n\n");
			my_seat = 0;
			flag = 0;
			pthread_cancel(p_thread[2]);//반납을 성공했으면 타이머 취소
		}
		else{	//좌석 예약하는 데 성공
			printf("좌석예약 성공\n\n");
			my_seat = num;
			flag = 1;
			
			timer = pthread_create(&p_thread[2], NULL, time_on, NULL);//좌석 점유 시간 재는 루틴
			if(timer <0){
			perror("thread create error : ");
			exit(1);	
			}
		}
	}
	else if(shmaddr[LISTSIZ -3] == 0){
		printf("이미 예약된 좌석\n\n");}

	semop (semid, &v, 1);//세마포어 해제
	pthread_exit(NULL); //세마 해제
}

void main (void){
	signal(SIGINT, input_write);
	int status; //쓰레드 조인을 위한 함수
	/*키 60029의데이터를 위한  공유메모리 생성*/
	if((shm1 = shmget(SHMKEY, sizeof(int)*LISTSIZ, IPC_CREAT|0666)) == -1){
		perror("shm1 failed");
		exit(1);
	}
	//키 60029의 공유메모리 붙이기
	if((shmaddr = shmat(shm1, 0, 0)) == (void *)-1){
		perror("shmat failed");
		exit(1);
	}

	serverpid = shmaddr[LISTSIZ - 1];//서버의 프로세스 아이디
	clientpid = getpid(); //해당 프로세스의 pid 저장

	bool exitOuterLoop = false; //바깥 루프 break
	int com;
        while(1){
		com= 0;
                printf("--------------------\n1 = 좌석현황 보기\n2 = 반납or예약\n3 = 종료\n--------------------\n\n명령  입력 : ");
                scanf("%d", &com);
		if(0<com<4){
			switch (com){
			       	case 1:  //공유 메모리에서 좌석정보 읽어옴
					dr = pthread_create(&p_thread[0], NULL, data_read, NULL);
					if (dr < 0){
						perror("thread create error : ");
						exit(0);
					}
					pthread_join(p_thread[0], (void**)&status);
					break;
				case 2 : //좌석을 예약하거나 반납 할 때
					printf("좌석 번호 선택: ");
					scanf("%d", &num);
					if(flag == 1){
						if(num != my_seat){// 다른 좌석을 예약중일 경우 거절
							printf("반납먼저\n");
							break;}
					}
					iw = pthread_create(&p_thread[1], NULL, input_write, num);
					if(iw < 0){ //좌석예약에 대한 요청
						perror("thread create error : ");
						exit(0);
					}
					pthread_join(p_thread[1], (void**)&status);
					break;
				case 3 : 
					exitOuterLoop = true;//이중 루프를 탈출하여 프로세스를 종료하기 위함
					printf("안녕! 잘 가!\n");
					if (flag ==1)
						input_write(my_seat);
					break;
	                        default : printf("command error try again.\n");
        	                        break;
				}
		}
		if (exitOuterLoop == true) //이중루프 탈출
			break;
        }
}

