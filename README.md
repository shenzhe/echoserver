# echoserver
原始阻塞、 基于select 、 epoll 的各版本echoserver实现，主要用于内部培训的知识普及以及了解相关的系统调用。
=====

gcc server1.c -o serv
gcc server_epoll.c -o epoll
gcc server_select.c -o select
