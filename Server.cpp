#include <iostream>

#include "Server.h"
#include <pthread.h>
using namespace std;


// 客户端列表
list<int> clients_list;
pthread_rwlock_t rwlock;

// 发送广播消息或者点对点通信给所有客户端
int Broadcast_OR_SINGLE(int clientfd)
{
    // buf[BUF_SIZE] 接收新消息
    // message[BUF_SIZE] 保存格式化的消息
    char buf[BUF_SIZE], message[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    bzero(message, BUF_SIZE);

    // 接收新消息
    cout << "read from client(clientID = " << clientfd << ")" << endl;
    int len = recv(clientfd, buf, BUF_SIZE, 0);

    // 如果客户端关闭了连接
    if(len == 0) 
    {
        close(clientfd);
        
        // 在客户端列表中删除该客户端
        clients_list.remove(clientfd);
        cout << "ClientID = " << clientfd 
             << " closed.\n now there are " 
             << clients_list.size()
             << " client in the char room"
             << endl;

    }
    // 发送广播消息给所有客户端
    else 
    {
        // 判断是否聊天室还有其他客户端
        if(clients_list.size() == 1) { 
            // 发送提示消息
            send(clientfd, CAUTION, strlen(CAUTION), 0);
            return len;
        }
		string ret_1(buf);//初始化ret_1
		istringstream is(ret_1);//初始化is
		string str1,str2;
		is>>str1>>str2;//通过空格划分字符串
		int number=atoi(str1.c_str());
		if(str1=="group")//若为group，则进行广播
		{
        // 格式化发送的消息内容
        sprintf(message, SERVER_MESSAGE, clientfd, strcpy(buf,str2.c_str()));

        // 遍历客户端列表依次发送消息，需要判断不要给来源客户端发
        list<int>::iterator it;
        for(it = clients_list.begin(); it != clients_list.end(); ++it) {
           if(*it != clientfd){
                if( send(*it, message, BUF_SIZE, 0) < 0 ) {
                    return -1;
                }
           }
        }
		}
		else if(number>=0 && number<=100)//若是0到100的数字则根据数字来实现点对点通信
		{
	     sprintf(message, SERVER_MESSAGE, clientfd, strcpy(buf,str2.c_str()));

        // 遍历客户端列表依次发送消息，发送信息给指定方
        list<int>::iterator it;
        for(it = clients_list.begin(); it != clients_list.end(); ++it) {
           if(*it == number){
                if( send(*it, message, BUF_SIZE, 0) < 0 ) {
                    return -1;
                }
		}
    }
		}
		//若输入格式不符合，则不进行任何操作
		else//若是其他情况则不进行任何操作
		{

				;
		}
				
		}
    return len;
}





void *routine(void *arg)
{

    cout<<pthread_self()<<endl;
	int *listener=(int *)arg;
		        pthread_rwlock_rdlock(&rwlock);
	            struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
				
                int clientfd = accept( *listener, ( struct sockaddr* )&client_address, &client_addrLength );

                cout << "client connection from: "
                     << inet_ntoa(client_address.sin_addr) << ":"
                     << ntohs(client_address.sin_port) << ", clientfd = "
                     << clientfd << endl;



                // 服务端用list保存用户连接
                clients_list.push_back(clientfd);
                cout << "Add new clientfd = " << clientfd << " to epoll" << endl;
                cout << "Now there are " << clients_list.size() << " clients int the chat room" << endl;

                // 服务端发送欢迎信息  
                cout << "welcome message" << endl;                
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                sprintf(message, SERVER_WELCOME, clientfd);
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(ret < 0) {
                    perror("send error");
                    close(clientfd);
                    pthread_exit(NULL);
                }
				while(1)
				{
	            int ret =Broadcast_OR_SINGLE(clientfd);
				
                if(ret < 0) {
                    perror("error");
                    close(clientfd);
				    pthread_rwlock_unlock(&rwlock);
                    pthread_exit(NULL);
                }
				}
	
	
}
// 服务端类成员函数

// 服务端类构造函数
Server::Server(){
    
    // 初始化服务器地址和端口
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 初始化socket
    listener = 0;
    
    // epool fd
    epfd = 0;
	
	num=0;
}

// 初始化服务端并启动监听
void Server::Init() {
    cout << "Init Server..." << endl;
    
     //创建监听socket
    listener = socket(PF_INET, SOCK_STREAM, 0);
    if(listener < 0) { perror("listener"); exit(-1);}
    
    //绑定地址
    if( bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind error");
        exit(-1);
    }

    //监听
    int ret = listen(listener, 5);
    if(ret < 0) {
        perror("listen error"); 
        exit(-1);
    }

    cout << "Start to listen: " << SERVER_IP << endl;

    //在内核中创建事件表
    epfd = epoll_create(EPOLL_SIZE);
    
    if(epfd < 0) {
        perror("epfd error");
        exit(-1);
    }

    //往事件表里添加监听事件
    addfd(epfd, listener, true);

}

// 关闭服务，清理并关闭文件描述符
void Server::Close() {

    //关闭socket
    close(listener);
    
    //关闭epoll监听
    close(epfd);
}

// 启动服务端
void Server::Start() {

    // epoll 事件队列
    static struct epoll_event events[EPOLL_SIZE]; 

    // 初始化服务端
    Init();

    //主循环
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

    // 关闭服务
    Close();
}