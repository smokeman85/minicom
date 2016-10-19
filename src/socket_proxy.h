#ifndef __SOCKET_PROXY_H__
#define __SOCKET_PROXY_H__

int  start_proxy();
void stop_proxy();
void write_to_proxy(char c);
void read_from_proxy(void (*send_cb) (int));

#endif
