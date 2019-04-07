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

SOCKET sockfd;//服务器socket
SOCKET fds[cSIZE];//客户端的socketfd,100个元素，fds[0]~fds[99]
int biaoji[cSIZE] = {0};//标记每个线程，0则未占用，1则表示该线程忙线中
int bChat[cSIZE] = {0};
char* IP = "101.76.248.233";
short PORT = 10222;
typedef struct sockaddr SA;
HANDLE hThread[cSIZE];
int count = 0;//统计聊天室当前的人数
char key[8] = {'k','e','y','t','o','s','e','v'};
 

//初始化服务器 
void init(){
	WORD sockVision = MAKEWORD(2,2);
	WSADATA wsadata;
	if(WSAStartup(sockVision,&wsadata) != 0)
    {  
        printf("WSA初始化失败\n");  
        return;  
    }  
    if ( LOBYTE( wsadata.wVersion ) != 2 || HIBYTE( wsadata.wVersion ) != 2 ) { // 检测是否2.2版本的socket库  
        WSACleanup( );  
        return;   
    }  
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    if (sockfd <= 0){
        perror("创建socket失败");
        exit(-1);
    }
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (bind(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("绑定失败");
        exit(-1);
    }
    printf("开始监听...\n");
    if (listen(sockfd,10) == -1){
        perror("设置监听失败");
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

//线程跳转执行的函数 
DWORD WINAPI service_thread(PVOID p){
    SOCKET fd = *(SOCKET*)p;
    
	int i;//线程号与标记号 important!
    for(i = 0;i<cSIZE;i++){
    	if(fd==fds[i]){
    		break;
    	}
	} 
	
	//第一次接受来自客户端的信息，判断其密码是否正确，并根据其选择的功能进行跳转，第一位是选择，1-9位为密码 
	char bufFirst[FILE_NAME_MAX_SIZE+1] = {};
	if (recv(fd,bufFirst,sizeof(bufFirst),0) <= 0){
		if (fd == fds[i]){
            count--;
            biaoji[i] = 0;
            closesocket(fds[i]);
			CloseHandle(hThread[i]);
        }
        printf("退出：客户端 = %dquit\n",i); 
        return;
	}
	
	//先判断密码是否正确 
	int j;
	for(j = 0;j<8;j++){
		if(bufFirst[j+1]!=key[j]){
			printf("退出：密码错误，已强制客户端 = %dquit\n",i);
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
    strcpy(bufFirst,bufFirst+9);//除去选择和密码剩下的字符串 
	
	switch(choos){
    	case 1:
    		bChat[i]=1;//标记其进入聊天室 
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
                		bChat[i]=0;//标记其已从聊天室离开 
                    	closesocket(fds[i]);
						CloseHandle(hThread[i]);
						printf("从聊天室删除号%d\n",i); 
                	}
                	printf("退出：客户端 = %dquit\n",i); 
					break;
        		}	
    			SendMsgToAll(buf);
			}
			break;
		case 2:
			printf("发送文件%s\n", bufFirst);//打印文件名 
			char buffer[BUFFER_SIZE]; 
  			//打开文件准备发送 
    		FILE * fp = fopen(bufFirst, "rb"); //windows下是"rb",表示打开一个只读的二进制文件 
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
  	  		printf("接收文件：%s\n", bufFirst);//打印文件名 
  	  		//将文件名写入FileLise.txt文件中
			FILE *fList = fopen("D://wangbin/FileList.txt","ab"); //"ab"方式打开表示从文件末尾开始写数据以二进制形式 
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
			
  			//打开文件准备写入 
    		FILE * fpGet = fopen(bufFirst, "wb"); 
			//windows下是"wb",表示打开一个只写的二进制文件 ，存在则清空重写，不存在则新建 
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
  			//发送文件目录
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
            printf("客户端连接出错...\n");
            continue;
        }
        int i;
        for(i = 0;i<cSIZE;i++){
        	if (biaoji[i] == 0){
                //记录客户端的socket
                fds[i] = fd;
				count++;//人数加1 
                printf("新客户端成功接入,总人数为[%d] %s:%d\n", count,inet_ntoa(fromaddr.sin_addr), ntohs(fromaddr.sin_port)); 
                //有客户端连接之后，启动线程给此客户服务
        		biaoji[i]=1;
				hThread[i] = CreateThread(NULL,0,service_thread,&fd,0,0);//启动新线程 
        		break;
            }
        	if (cSIZE == i){
            //发送给客户端说聊天室满了
            char* str = "对不起，聊天室已经满了!";
            send(fd,str,strlen(str),0); 
            closesocket(fd);
        	}
   	 	}
    }
}

int main(){
	printf("服务器启动\n");
    init();
    service();
    WSACleanup();     //释放WS2_32.DLL
    printf("服务器关闭\n");
}
