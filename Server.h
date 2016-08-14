#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H

#include <string>

#include "Common.h"

using namespace std;

// ������࣬��������ͻ�������
class Server {

public:
    // �޲������캯��
    Server();

    // ��ʼ��������������
    void Init();

    //�ر�����
	void Close();

    // ���������
    void Start();

private:


    // ��������serverAddr��Ϣ
    struct sockaddr_in serverAddr;
    
    //����������socket
    int listener;

    // epoll_create������ķ���ֵ
    int epfd;
    

	
	//�߳�id
	pthread_t tid[10];
	
	//�̵߳ļ���
	int num;
};



#endif //CHATROOM_SERVER_H