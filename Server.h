#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H

#include <string>

#include "Common.h"

using namespace std;

// 服务端类，用来处理客户端请求
class Server {

public:
    // 无参数构造函数
    Server();

    // 初始化服务器端设置
    void Init();

    //关闭连接
	void Close();

    // 启动服务端
    void Start();

private:


    // 服务器端serverAddr信息
    struct sockaddr_in serverAddr;
    
    //创建监听的socket
    int listener;

    // epoll_create创建后的返回值
    int epfd;
    

	
	//线程id
	pthread_t tid[10];
	
	//线程的计数
	int num;
};



#endif //CHATROOM_SERVER_H