#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define FILE_NAME_MAX_SIZE     512
#define BUFFER_SIZE            1024

int sockfd;//客户端socket
char* IP = "101.76.248.233";//服务器的IP
short PORT = 10222;//服务器服务端口
typedef struct sockaddr SA;
char name[19];//字符实际能用量为申请的大小减一，如此处只能接受19个字符的名字 
char key[9]; //9-1=8位密码 如为9会出错，因为最后一位要留作\0 
int  choose;

void chatInit(){
    sockfd = socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("Can not reach to server\n");
        exit(-1);
    }
    printf("Connect Successful!\n");
    printf("Please enter your name (less than 9 Chinese characters or 18 letters):");
    scanf("%s",name);
    printf("Please enter the 8 digit server password:");
	scanf("%s",key);
    char buf2[100] = {};
    memset(buf2,0,sizeof(buf2));  
    sprintf(buf2,"%d%s%s",choose,key,name);
    //客户第一次发送 
    send(sockfd,buf2,strlen(buf2),0);
}

void FileInit(){
	sockfd = socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);
    if (connect(sockfd,(SA*)&addr,sizeof(addr)) == -1){
        perror("Can not reach to server\n");
        exit(-1);
    }
    printf("The connection is successful.! <bye> can be entered directly to exit chat room.\n");
    printf("Please enter your name (less than 9 Chinese characters or 18 letters): ");
    scanf("%s",name);
    printf("Please enter the 8 digit server password:");
	scanf("%s",key);	
}

void* recv_thread(void* p){
    while(1){
        char buf[100] = {};
        if (recv(sockfd,buf,sizeof(buf),0) <= 0){
            return NULL;
        }
        printf("%s\n",buf);
    }
}

void toChat(){
	//启动线程单独处理聊天接受的消息 
	chatInit();
	pthread_t id;
    void* recv_thread(void*);
    pthread_create(&id,0,recv_thread,0);
    //不断循环有消息则发送 
	while(1){
        char buf[100] = {};
        scanf("%s",buf);
        if (strcmp(buf,"bye") == 0){
            memset(buf,0,sizeof(buf));
            sprintf(buf,"%squit the chat room.",name);
            send(sockfd,buf,strlen(buf),0);
            break;
        }
        char msg[131] = {};
        sprintf(msg,"%s:%s",name,buf);
        if(send(sockfd,msg,strlen(msg),0)<=0){
        	break;
		}
    }
} 

void getFile(){
	FileInit();
	//输入文件名 
	char file_name[FILE_NAME_MAX_SIZE+1]; 
  	memset(file_name, 0, FILE_NAME_MAX_SIZE+1); 
  	printf("Please Input File Name On Server: "); 
  	scanf("%s", file_name);
	
	char buffer[BUFFER_SIZE]; 
  	memset(buffer, 0, BUFFER_SIZE); 
	//向服务器发送功能选择、密码、文件名  
    sprintf(buffer,"%d%s%s",choose,key,file_name);
    //客户第一次发送 
  	if(send(sockfd,buffer,strlen(buffer),0) < 0) 
  	{ 
    	printf("In Choose 2:Send File Name Failed\n"); 
    	return;
  	} 
  	
  	//打开文件，准备写入 
  	FILE * fp = fopen(file_name, "wb"); //windows下是"wb",表示打开一个只写的二进制文件 
  	if(NULL == fp) 
  	{ 
    	printf("In Choose 2:File: %s Can Not Open To Write\n", file_name); 
    	return;
  	} 
  	else
  	{ 
    	memset(buffer, 0, BUFFER_SIZE); 
    	int length = 0; 
    	while ((length = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0) 
    	{ 
      		if (fwrite(buffer, sizeof(char), length, fp) < length) 
      		{ 
        		printf("File: %s Write Failed\n", file_name); 
        		break; 
      		} 
      		memset(buffer, 0, BUFFER_SIZE); 
    	} 
    	printf("Receive File: %s From Server Successful!\n", file_name); 
  	} 
  	
	
}

void sendFile(){
	FileInit();
	//得到文件名 
	char file_name[FILE_NAME_MAX_SIZE + 1]; 
	memset(file_name,0,FILE_NAME_MAX_SIZE+1);  
	printf("Please Input The File Name To Send:");
    scanf("%s", file_name);
    
    char buffer[BUFFER_SIZE]; 
  	memset(buffer, 0, BUFFER_SIZE); 
	//向服务器发送功能选择、密码、文件名  
    sprintf(buffer,"%d%s%s",choose,key,file_name);
    if(send(sockfd,buffer,strlen(buffer),0) < 0) 
  	{ 
    	printf("In Choose 3:Send File Name Failed\n"); 
    	return;
  	}
	
	//打开文件，准备发送
	FILE * fp = fopen(file_name, "rb"); //windows下是"rb",表示打开一个只读的二进制文件 
	if (NULL == fp) 
    { 
      	printf("In Choose 3:File: %s Not Found\n", file_name); 
    } 
    else
    { 
      	memset(buffer, 0, BUFFER_SIZE); 
      	int length = 0; 
  
      	while ((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) 
      	{ 	
      		printf("sending...\n");
        	if (send(sockfd, buffer, length, 0) < 0) 
        	{ 
          		printf("Send File: %s Failed\n", file_name); 
          		break; 
        	} 
        	memset(buffer, 0, BUFFER_SIZE); 
      	} 
      	fclose(fp); 
    	printf("File: %s Send Successful!\n", file_name);
    } 
	   
} 

void getFileList(){
	FileInit();
	char buf2[100] = {};
    memset(buf2,0,sizeof(buf2));  
    sprintf(buf2,"%d%s%s",choose,key,name);
    //客户第一次发送 
    send(sockfd,buf2,strlen(buf2),0);
    
    //准备接收文件目录
	char fileList[BUFFER_SIZE];
	memset(fileList,0,BUFFER_SIZE); 
	if (recv(sockfd,fileList,BUFFER_SIZE,0) <= 0)
	{
        printf("Failed to receive file directory!\n");  
		return;
        
	}
    printf("%s\n",fileList);
}

int main(){
	printf("********<Web Service Programs Start> --BY WB********\n********WELCOME********\n");
	printf("Please enter the corresponding number selection (1:enter the chat room 2:download the file 3:upload file 4:get the file list)\n---->input: ");
	scanf("%d",&choose);
	int reChoose=1;
	while(reChoose==1){ 
		if(choose==1){
			reChoose = 0;
			toChat();
		}	
		else if(choose==2){
			reChoose = 0;
			getFile();
		}
		else if(choose==3){
			reChoose = 0;
			sendFile();
		}
		else if(choose==4){
			reChoose = 0;
			getFileList();
		}	
		else{
			reChoose=0;
			printf("Input error! Please press 1 for reinput, and drop out the other numbers."); 
			scanf("%d",&reChoose); 
		} 
	}	
    close(sockfd);
    printf("finish connection,wish you a happy life ~ \n"); 
    return 0;
}
