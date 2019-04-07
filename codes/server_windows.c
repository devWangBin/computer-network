#include <winsock2.h>
#include<windows.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUFFER_SIZE     1024
#define cSIZE            10
#define FILE_NAME_MAX_SIZE     512
#pragma comment(lib,"ws2_32.lib")

SOCKET sockfd;//������socket
SOCKET fds[cSIZE];//�ͻ��˵�socketfd,100��Ԫ�أ�fds[0]~fds[99]
int biaoji[cSIZE] = {0};//���ÿ���̣߳�0��δռ�ã�1���ʾ���߳�æ����
int bChat[cSIZE] = {0};
char* IP = "101.76.248.233";
short PORT = 10222;
typedef struct sockaddr SA;
HANDLE hThread[cSIZE];
int count = 0;//ͳ�������ҵ�ǰ������
char key[8] = {'k','e','y','t','o','s','e','v'};
 

//��ʼ�������� 
void init(){
	WORD sockVision = MAKEWORD(2,2);
	WSADATA wsadata;
	if(WSAStartup(sockVision,&wsadata) != 0)
    {  
        printf("WSA��ʼ��ʧ��\n");  
        return;  
    }  
    if ( LOBYTE( wsadata.wVersion ) != 2 || HIBYTE( wsadata.wVersion ) != 2 ) { // ����Ƿ�2.2�汾��socket��  
        WSACleanup( );  
        return;   
    }  
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if (sockfd <= 0){
        perror("����socketʧ��");
        exit(-1);
    }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (bind(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("��ʧ��");
        exit(-1);
    }
    printf("��ʼ����...\n");
    if (listen(sockfd,10) == -1){
        perror("���ü���ʧ��");
        exit(-1);
    }
}

void SendMsgToAll(char* msg){
    int i;
    for (i = 0;i < cSIZE;i++){
        if (bChat[i] != 0){
            printf("Chat send to%d\n",i);
            send(fds[i],msg,strlen(msg),0);
        }
    }
}

//�߳���תִ�еĺ��� 
DWORD WINAPI service_thread(PVOID p){
    SOCKET fd = *(SOCKET*)p;
    
	int i;//�̺߳����Ǻ� important!
    for(i = 0;i<cSIZE;i++){
    	if(fd==fds[i]){
    		break;
    	}
	} 
	
	//��һ�ν������Կͻ��˵���Ϣ���ж��������Ƿ���ȷ����������ѡ��Ĺ��ܽ�����ת����һλ��ѡ��1-9λΪ���� 
	char bufFirst[FILE_NAME_MAX_SIZE+1] = {};
	if (recv(fd,bufFirst,sizeof(bufFirst),0) <= 0){
		if (fd == fds[i]){
            count--;
            biaoji[i] = 0;
            closesocket(fds[i]);
			CloseHandle(hThread[i]);
        }
        printf("�˳����ͻ��� = %dquit\n",i); 
        return;
	}
	
	//���ж������Ƿ���ȷ 
	int j;
	for(j = 0;j<8;j++){
		if(bufFirst[j+1]!=key[j]){
			printf("�˳������������ǿ�ƿͻ��� = %dquit\n",i);
			char backMsg[] = {"The password is wrong, you have been forced to break the line!"};
			send(fds[i],backMsg,strlen(backMsg),0);
			count--;
            biaoji[i] = 0;
            closesocket(fds[i]);
			CloseHandle(hThread[i]);
			return;
		}
	} 
	
    printf("pthread = [%d] start\n",i);
    int choos = bufFirst[0]-48;
    strcpy(bufFirst,bufFirst+9);//��ȥѡ�������ʣ�µ��ַ��� 
	
	switch(choos){
    	case 1:
    		bChat[i]=1;//�������������� 
    		char buf[BUFFER_SIZE] = {};
    		memset(buf,0,sizeof(buf));  
    		sprintf(buf,"Welcome %s to join the Chat Room!",bufFirst);
    		SendMsgToAll(buf);
    		while(1){
    			memset(buf,0,sizeof(buf));
    			if (recv(fd,buf,sizeof(buf),0) <= 0){
                	if (fd == fds[i]){
                		count--;
                		biaoji[i] = 0;
                		bChat[i]=0;//������Ѵ��������뿪 
                    	closesocket(fds[i]);
						CloseHandle(hThread[i]);
						printf("��������ɾ����%d\n",i); 
                	}
                	printf("�˳����ͻ��� = %dquit\n",i); 
					break;
        		}	
    			SendMsgToAll(buf);
			}
			break;
		case 2:
			printf("�����ļ�%s\n", bufFirst);//��ӡ�ļ��� 
			char buffer[BUFFER_SIZE]; 
  			//���ļ�׼������ 
    		FILE * fp = fopen(bufFirst, "rb"); //windows����"rb",��ʾ��һ��ֻ���Ķ������ļ� 
    		if (NULL == fp) 
    		{ 
      			printf("File: %s Not Found\n", bufFirst);
      			count--;
            	biaoji[i] = 0;
            	closesocket(fds[i]);
				CloseHandle(hThread[i]);
				return;
    		} 
        	else
    		{ 
      			memset(buffer, 0, BUFFER_SIZE); 
      			int length = 0; 
  
      			while ((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) 
      			{ 	
      				printf("sending...\n");
        			if (send(fd, buffer, length, 0) < 0) 
        			{ 
          				printf("Send File: %s Failed\n", bufFirst); 
          				break; 
        			} 
        		memset(buffer, 0, BUFFER_SIZE); 
      			} 
  
      			fclose(fp); 
      			printf("File: %s Transfer Successful!\n", bufFirst);
				count--;
            	biaoji[i] = 0;
            	closesocket(fds[i]);
				CloseHandle(hThread[i]);
    		} 
    		break;
    	case 3:
  	  		printf("�����ļ���%s\n", bufFirst);//��ӡ�ļ��� 
  	  		//���ļ���д��FileLise.txt�ļ���
			FILE *fList = fopen("D://wangbin/FileList.txt","ab"); //"ab"��ʽ�򿪱�ʾ���ļ�ĩβ��ʼд�����Զ�������ʽ 
			if (NULL == fList) 
    		{ 
      			printf("Can Not Open The FileList.txt To Write\n"); 
      			count--;
            	biaoji[i] = 0;
            	closesocket(fds[i]);
				CloseHandle(hThread[i]);
				return;
    		} 
			else
			{
				char fileName[100];
				memset(fileName, 0, sizeof(fileName));
				sprintf(fileName,"<%s>\\n",bufFirst);
				if(fwrite(fileName, sizeof(char),strlen(fileName),fList)<strlen(fileName))
      			{ 
        			printf("File: %s Write To The FileList.txt Failed\n", bufFirst); 
        			break; 
      			}
      			else
      			{
      				printf("File: %s Write To The FileList.txt Successful\n", bufFirst);
				}
				fclose(fList);  	
			}
			 
			char Buffer[BUFFER_SIZE]; 
			
  			//���ļ�׼��д�� 
    		FILE * fpGet = fopen(bufFirst, "wb"); 
			//windows����"wb",��ʾ��һ��ֻд�Ķ������ļ� �������������д�����������½� 
    		if (NULL == fpGet) 
    		{ 
      			printf("File: %s Can Not Open To Write\n", bufFirst); 
      			count--;
            	biaoji[i] = 0;
            	closesocket(fds[i]);
				CloseHandle(hThread[i]);
				return;
    		} 
    		else
  			{ 
    			memset(Buffer, 0, BUFFER_SIZE); 
    			int length3 = 0; 
    			while ((length3 = recv(fd, Buffer, BUFFER_SIZE, 0)) > 0) 
    			{ 
      				if (fwrite(Buffer, sizeof(char), length3, fpGet) < length3) 
      				{ 
        				printf("File: %s Write Failed\n", bufFirst); 
        				break; 
      				} 
      				memset(Buffer, 0, BUFFER_SIZE); 
    			}
				fclose(fpGet);  
    			printf("Receive File: %s From Client Successful!\n", bufFirst);
				count--;
            	biaoji[i] = 0;
            	closesocket(fds[i]);
				CloseHandle(hThread[i]); 
  			} 
  			break;
  		case 4:
  			//�����ļ�Ŀ¼
			printf("sending file list...\n"); 
  			FILE *fl = fopen("D://wangbin/FileList.txt","rb");
  			if(fl==NULL)
  			{
				char back[]={"Open File List Failed"};
				printf("%s\n",back);
				send(fd,back,sizeof(back),0);
				return;  
			} 
			else
			{
				int len = 0; 
  				char bufferList[BUFFER_SIZE];
  				memset(bufferList,0,BUFFER_SIZE);
      			while ((len = fread(bufferList, sizeof(char), BUFFER_SIZE, fl)) > 0) 
      			{ 	
        			if (send(fd, bufferList, len, 0) < 0) 
        			{ 
          				printf("Send File List Failed\n"); 
          				break; 
        			} 
        		memset(buffer, 0, BUFFER_SIZE); 
      			} 
  
      			fclose(fl); 
      			printf("File List Transfer Successful!\n");
				count--;
            	biaoji[i] = 0;
            	closesocket(fd);
				CloseHandle(hThread[i]);
			}
  			break;
	}    
}


void service(){
    while(1){
        struct sockaddr_in fromaddr;
        memset(&fromaddr,0,sizeof(fromaddr));
        int len = sizeof(fromaddr);
        SOCKET fd = accept(sockfd,(SA*)&fromaddr,&len);
        if (fd == -1){
            printf("�ͻ������ӳ���...\n");
            continue;
        }
        int i;
        for(i = 0;i<cSIZE;i++){
        	if (biaoji[i] == 0){
                //��¼�ͻ��˵�socket
                fds[i] = fd;
				count++;//������1 
                printf("�¿ͻ��˳ɹ�����,������Ϊ[%d] %s:%d\n", count,inet_ntoa(fromaddr.sin_addr), ntohs(fromaddr.sin_port)); 
                //�пͻ�������֮�������̸߳��˿ͻ�����
        		biaoji[i]=1;
				hThread[i] = CreateThread(NULL,0,service_thread,&fd,0,0);//�������߳� 
        		break;
            }
        	if (cSIZE == i){
            //���͸��ͻ���˵����������
            char* str = "�Բ����������Ѿ�����!";
            send(fd,str,strlen(str),0); 
            closesocket(fd);
        	}
   	 	}
    }
}

int main(){
	printf("����������\n");
    init();
    service();
    WSACleanup();     //�ͷ�WS2_32.DLL
    printf("�������ر�\n");
}
