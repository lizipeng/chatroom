Chatroom

About

基于c++11和Linux socket API实现的聊天室


服务端采用epoll的多线程

客户端采用父子进程协同操作

实现功能：

服务端：支持多个客户端连接，并将每个客户端发过来的消息发给所有其他的客户端

客户端：能够连接服务器，并向服务器发送消息，同时接收服务器发过来的任何消息

示例代码：

服务端：

while(1)
    {
    
        //epoll_events_count表示就绪事件的数目
        
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);

        if(epoll_events_count < 0) {
            perror("epoll failure");
            break;
        }

        cout << "epoll_events_count =\n" << epoll_events_count << endl;

        //处理这epoll_events_count个就绪事件
        for(int i = 0; i < epoll_events_count; ++i)
        {
            int sockfd = events[i].data.fd;
            //新用户连接
            if(sockfd == listener)
            {
                //服务端的套接字有连接，则创建一条线程进行连接和接下来的信息发送的操作
				//将线程设置为分离的
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
				pthread_create(&tid[num],&attr,routine,(void *)&listener);
				num++;

        }
    }
	}
	
	
 客户端：    
    
    // 创建子进程
    pid = fork();
    
    // 如果创建子进程失败则退出
    if(pid < 0) {
        perror("fork error");
        close(sock);
        exit(-1);
    } else if(pid == 0) {
        // 进入子进程执行流程
        //子进程负责写入管道，因此先关闭读端
        close(pipe_fd[0]); 

        // 输入exit可以退出聊天室
        cout << "Please input 'exit' to exit the chat room" << endl;

        // 如果客户端运行正常则不断读取输入发送给服务端
        while(isClientwork){
            bzero(&message, BUF_SIZE);
            fgets(message, BUF_SIZE, stdin);

            // 客户输出exit,退出
            if(strncasecmp(message, EXIT, strlen(EXIT)) == 0){
                isClientwork = 0;
            }
            // 子进程将信息写入管道
            else {
                if( write(pipe_fd[1], message, strlen(message) - 1 ) < 0 ) { 
                    perror("fork error");
                    exit(-1);
                }
            }
        }
    } else { 
        //pid > 0 父进程
        //父进程负责读管道数据，因此先关闭写端
        close(pipe_fd[1]); 

        // 主循环(epoll_wait)
        while(isClientwork) {
            int epoll_events_count = epoll_wait( epfd, events, 2, -1 );

            //处理就绪事件
            for(int i = 0; i < epoll_events_count ; ++i)
            {
                bzero(&message, BUF_SIZE);

                //服务端发来消息
                if(events[i].data.fd == sock)
                {
                    //接受服务端消息
                    int ret = recv(sock, message, BUF_SIZE, 0);

                    // ret= 0 服务端关闭
                    if(ret == 0) {
                        cout << "Server closed connection: " << sock << endl;
                        close(sock);
                        isClientwork = 0;
                    } else {
                        cout << message << endl;
                    }
                }
                //子进程写入事件发生，父进程处理并发送服务端
                else { 
                    //父进程从管道中读取数据
                    int ret = read(events[i].data.fd, message, BUF_SIZE);

                    // ret = 0
                    if(ret == 0)
                        isClientwork = 0;
                    else {
                        // 将信息发送给服务端
                        send(sock, message, BUF_SIZE, 0);
                    }
                }
            }//for
        }//while
    }
 
