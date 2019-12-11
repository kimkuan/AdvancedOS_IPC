cc = gcc
all : msg_server msg_client pipe_server pipe_client shm_server shm_client
RM = rm - rf
LDFLAGS = -lpthread

msg_server: msg_server.c
	$(cc) -o $@ $< $(LDFLAGS)
msg_client: msg_client.c
	$(cc) -o $@ $< $(LDFLAGS)

pipe_server: pipe_server.c
	$(cc) -o $@ $< $(LDFLAGS)
pipe_client: pipe_client.c
	$(cc) -o $@ $< $(LDFLAGS)

shm_server: shm_server.c
	$(cc) -o $@ $< $(LDFLAGS)
shm_client: shm_client.c
	$(cc) -o $@ $< $(LDFLAGS)

run:
	$(RM) *.o
