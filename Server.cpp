#include <iostream>

#include "Server.h"
#include <pthread.h>
using namespace std;


// �ͻ����б�
list<int> clients_list;
pthread_rwlock_t rwlock;

// ���͹㲥��Ϣ���ߵ�Ե�ͨ�Ÿ����пͻ���
int Broadcast_OR_SINGLE(int clientfd)
{
    // buf[BUF_SIZE] ��������Ϣ
    // message[BUF_SIZE] �����ʽ������Ϣ
    char buf[BUF_SIZE], message[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    bzero(message, BUF_SIZE);

    // ��������Ϣ
    cout << "read from client(clientID = " << clientfd << ")" << endl;
    int len = recv(clientfd, buf, BUF_SIZE, 0);

    // ����ͻ��˹ر�������
    if(len == 0) 
    {
        close(clientfd);
        
        // �ڿͻ����б���ɾ���ÿͻ���
        clients_list.remove(clientfd);
        cout << "ClientID = " << clientfd 
             << " closed.\n now there are " 
             << clients_list.size()
             << " client in the char room"
             << endl;

    }
    // ���͹㲥��Ϣ�����пͻ���
    else 
    {
        // �ж��Ƿ������һ��������ͻ���
        if(clients_list.size() == 1) { 
            // ������ʾ��Ϣ
            send(clientfd, CAUTION, strlen(CAUTION), 0);
            return len;
        }
		string ret_1(buf);//��ʼ��ret_1
		istringstream is(ret_1);//��ʼ��is
		string str1,str2;
		is>>str1>>str2;//ͨ���ո񻮷��ַ���
		int number=atoi(str1.c_str());
		if(str1=="group")//��Ϊgroup������й㲥
		{
        // ��ʽ�����͵���Ϣ����
        sprintf(message, SERVER_MESSAGE, clientfd, strcpy(buf,str2.c_str()));

        // �����ͻ����б����η�����Ϣ����Ҫ�жϲ�Ҫ����Դ�ͻ��˷�
        list<int>::iterator it;
        for(it = clients_list.begin(); it != clients_list.end(); ++it) {
           if(*it != clientfd){
                if( send(*it, message, BUF_SIZE, 0) < 0 ) {
                    return -1;
                }
           }
        }
		}
		else if(number>=0 && number<=100)//����0��100�����������������ʵ�ֵ�Ե�ͨ��
		{
	     sprintf(message, SERVER_MESSAGE, clientfd, strcpy(buf,str2.c_str()));

        // �����ͻ����б����η�����Ϣ��������Ϣ��ָ����
        list<int>::iterator it;
        for(it = clients_list.begin(); it != clients_list.end(); ++it) {
           if(*it == number){
                if( send(*it, message, BUF_SIZE, 0) < 0 ) {
                    return -1;
                }
		}
    }
		}
		//�������ʽ�����ϣ��򲻽����κβ���
		else//������������򲻽����κβ���
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



                // �������list�����û�����
                clients_list.push_back(clientfd);
                cout << "Add new clientfd = " << clientfd << " to epoll" << endl;
                cout << "Now there are " << clients_list.size() << " clients int the chat room" << endl;

                // ����˷��ͻ�ӭ��Ϣ  
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
// ��������Ա����

// ������๹�캯��
Server::Server(){
    
    // ��ʼ����������ַ�Ͷ˿�
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // ��ʼ��socket
    listener = 0;
    
    // epool fd
    epfd = 0;
	
	num=0;
}

// ��ʼ������˲���������
void Server::Init() {
    cout << "Init Server..." << endl;
    
     //��������socket
    listener = socket(PF_INET, SOCK_STREAM, 0);
    if(listener < 0) { perror("listener"); exit(-1);}
    
    //�󶨵�ַ
    if( bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind error");
        exit(-1);
    }

    //����
    int ret = listen(listener, 5);
    if(ret < 0) {
        perror("listen error"); 
        exit(-1);
    }

    cout << "Start to listen: " << SERVER_IP << endl;

    //���ں��д����¼���
    epfd = epoll_create(EPOLL_SIZE);
    
    if(epfd < 0) {
        perror("epfd error");
        exit(-1);
    }

    //���¼�������Ӽ����¼�
    addfd(epfd, listener, true);

}

// �رշ��������ر��ļ�������
void Server::Close() {

    //�ر�socket
    close(listener);
    
    //�ر�epoll����
    close(epfd);
}

// ���������
void Server::Start() {

    // epoll �¼�����
    static struct epoll_event events[EPOLL_SIZE]; 

    // ��ʼ�������
    Init();

    //��ѭ��
    while(1)
    {
        //epoll_events_count��ʾ�����¼�����Ŀ
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);

        if(epoll_events_count < 0) {
            perror("epoll failure");
            break;
        }

        cout << "epoll_events_count =\n" << epoll_events_count << endl;

        //������epoll_events_count�������¼�
        for(int i = 0; i < epoll_events_count; ++i)
        {
            int sockfd = events[i].data.fd;
            //���û�����
            if(sockfd == listener)
            {
                //����˵��׽��������ӣ��򴴽�һ���߳̽������Ӻͽ���������Ϣ���͵Ĳ���
				//���߳�����Ϊ�����
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
				pthread_create(&tid[num],&attr,routine,(void *)&listener);
				num++;

        }
    }
	}

    // �رշ���
    Close();
}