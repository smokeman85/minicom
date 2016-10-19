#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "list.h"

#define PROXY_PORT 8787
#define STR_ERROR_OUT_MEMORY "Sorry! Out of memory.\n"
#define STR_CONNECTED "Connected\n"

static int socket_proxy;
static pthread_mutex_t client_sock_mutex;

struct client_sock {
	int sock;
	struct list_head list;
};

static struct client_sock client_sock_pool;
static pthread_t accept_thread;

static void close_socket_pool()
{
	struct list_head *pos;
	struct client_sock *obj;

	pthread_mutex_lock(&client_sock_mutex);
redo:
	list_for_each (pos, &client_sock_pool.list) {
		obj = list_entry(pos, struct client_sock, list);
		close(obj->sock);
		list_del(pos);
		free(obj);
		goto redo;
	}
	pthread_mutex_unlock(&client_sock_mutex);
}

void *accept_handler()
{
	int client_sock;
	struct client_sock *cl = NULL;
	struct sockaddr_in client;

	size_t c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(socket_proxy, (struct sockaddr *) &client, (socklen_t *)&c))) {
		cl = malloc(sizeof(struct client_sock));
		if (!cl) {
			printf("%s\n", STR_ERROR_OUT_MEMORY);
			write(client_sock, STR_ERROR_OUT_MEMORY, strlen(STR_ERROR_OUT_MEMORY));
			close(client_sock);
			continue;
		}
		cl->sock = client_sock;

		pthread_mutex_lock(&client_sock_mutex);
		list_add(&(cl->list), &(client_sock_pool.list));
		pthread_mutex_unlock(&client_sock_mutex);

		write(client_sock, STR_CONNECTED, strlen(STR_CONNECTED));
	}

	return NULL;
}

int start_proxy()
{
	struct sockaddr_in proxy;

	socket_proxy = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_proxy == -1)
		return -1;

	memset(&proxy, 0, sizeof(proxy));
	proxy.sin_family = AF_INET;
	proxy.sin_addr.s_addr = INADDR_ANY;
	proxy.sin_port = htons(PROXY_PORT);

	if (bind(socket_proxy, (struct sockaddr *)&proxy, sizeof(proxy)) < 0)
		return -1;

	listen(socket_proxy, 3);

	INIT_LIST_HEAD(&client_sock_pool.list);

	pthread_mutex_init(&client_sock_mutex, NULL);
	if (pthread_create(&accept_thread, NULL, accept_handler, NULL) < 0) {
		pthread_mutex_destroy(&client_sock_mutex);
		close(socket_proxy);
		return -1;
	}

	return 0;
}

void stop_proxy()
{
	pthread_cancel(accept_thread);
	close_socket_pool();
	printf("Proxy is stoped\n");
}

static int check_socket(int sock)
{
	int error = 0, ret = 0;
	socklen_t len = sizeof(error);

	int retval = getsockopt (sock, SOL_SOCKET, SO_ERROR, &error, &len);

	if ((retval != 0) || (error != 0))
		ret = -1;
	return ret;
}

void write_to_proxy(char c)
{
	struct list_head *pos;
	struct client_sock *obj;
	char client_message[2];

	snprintf(client_message,  sizeof(client_message), "%c", c);

	pthread_mutex_lock(&client_sock_mutex);
repeat:
	list_for_each (pos, &client_sock_pool.list) {
		obj = list_entry(pos, struct client_sock, list);
		if (check_socket(obj->sock) != 0) {
			close(obj->sock);
			list_del(pos);
			free(obj);
			goto repeat;
		}
		write(obj->sock , client_message , strlen(client_message));

	}
	pthread_mutex_unlock(&client_sock_mutex);
}

static void send_buffer(char *buffer, int len, void (*send_cb) (int))
{
	int i;

	for(i = 0; i < len; i++)
		send_cb(buffer[i]);
}

void read_from_proxy(void (*send_cb) (int))
{
	fd_set readset;
	int result, iof = -1;
	struct list_head *pos;
	struct client_sock *obj;
	struct timeval tv;
	int fd;
	char buffer[128];

	
	pthread_mutex_lock(&client_sock_mutex);

	list_for_each (pos, &client_sock_pool.list) {
		obj = list_entry(pos, struct client_sock, list);
		fd = obj->sock;
		FD_ZERO(&readset);
		FD_SET(fd, &readset);

		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		result = select(fd+1, &readset, NULL, NULL, &tv);

		if (result < 0)
			continue;
		else if (result > 0 && FD_ISSET(fd, &readset)) {
			if ((iof = fcntl(fd, F_GETFL, 0)) != -1)
				fcntl(fd, F_SETFL, iof | O_NONBLOCK);
			result = recv(fd, buffer, sizeof(buffer), 0);
			if (iof != -1)
				fcntl(fd, F_SETFL, iof);
			send_buffer(buffer, result, send_cb);
		}
	}
	pthread_mutex_unlock(&client_sock_mutex);
}

#if 0
void sig_handler(int signo)
{
	if (signo == SIGINT) {
		stop_proxy();
		exit(0);
	}
}

void print_cb(int c)
{
	printf("%c", c);
}

int main()
{
	int ret;

	signal(SIGINT, sig_handler);
	ret = start_proxy();
	if(ret != 0) {
		printf("Error of starting proxy\n");
		return 0;
	}

	while(1) {
		write_to_proxy('t');
		read_from_proxy(print_cb);
		sleep(2);
	}

	return 0;
}
#endif
