/*
* PS#2 Network Systems
* Author : Rishikesh Sundaragiri
* Date : 12/3/2024
*/

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>  
#include <string.h>  
#include <stdlib.h>   
#include <arpa/inet.h>    
#include <pthread.h> 
#include <stdbool.h>

#define FAIL 1
#define SUCCESS 0
#define BACKLOG 5
#define TIMEOUT_PERIOD 10
#define MSG_SIZE 2500
#define BUF_SIZE 8388608 /* 8 MB */
#define SPACE_CHAR 32
#define SIZE_OF_HEADER 320
#define SIZE_OF_PACKET 32


uint8_t get_command[] = "GET";
uint8_t text_file[]="txt";
uint8_t image_png[]="png";
uint8_t image_jpg[]="jpg";
uint8_t image_gif[]="gif";
uint8_t html_file[]="html";
uint8_t css_file[]="css";
uint8_t js_file[]="js";
uint8_t text_file_name[]="text/plain";
uint8_t image_png_name[]="image/png";
uint8_t image_jpg_name[]="image/jpg";
uint8_t image_gif_name[]="image/gif";
uint8_t html_file_name[]="text/html";
uint8_t css_file_name[]="text/css";
uint8_t js_file_name[]="application/javascript";


void exit_handler(int shutdown_fd)
{
    shutdown(shutdown_fd,SHUT_RDWR);
    close(shutdown_fd);
}



bool brute_force_pattern(uint8_t* str,uint8_t* pat)
{
    bool match = false;  
    for(int i= 0;i < (strlen(str) - strlen(pat) + 1);i++)
    {

      for(int j = 0;j < strlen(pat);j++)
      {
          if(str[i+j] != pat[j])
          {
              match = false;
              break;
          }
          else
          {
              match = true;
          }
      }
      if(match)
      {
          return match;
      }
    }
    return match;
}

int str_finder(uint8_t* in, uint8_t* out)
{
    uint8_t path[120];
    bzero(path,120);
    if(brute_force_pattern(in,get_command))
    {
        /* Here we need to add 1 because strlen will not consider the terminator character in C Language*/
        uint8_t get_cmd_len = strlen(get_command) + 1;
        /* Check if there is no file name entered and its empty or space after the GET command*/
        if(*(in + get_cmd_len + 1) == SPACE_CHAR)
		{
			/* Send the default index.html as responce */
            uint8_t* file_name = "www/index.html";
            FILE *file_pointer;
            uint8_t* type_of_file;
            int size_of_file;
            uint8_t header[SIZE_OF_HEADER];
            uint8_t data[SIZE_OF_PACKET];
            bool end_flag = false;
            if(!access(file_name,F_OK))
            {

                file_pointer = fopen(file_name,"r");
		        fseek(file_pointer,0,SEEK_END);
		        size_of_file = ftell(file_pointer);
		        fseek(file_pointer,0,SEEK_SET);
                
                if(type_of_file == NULL)
                {
                    return end_flag;
                }
                sprintf(header,"HTTP/1.1 200 OK\r\n Content-Type:%s\r\nContent-Length:%d\r\n\r\n",type_of_file,size_of_file);
               	memcpy(out,header,strlen(header));
		        buf_data += strlen(header);
		        out+=strlen(header);
                int n;
                while ((n = fread(data, 1, SIZE_OF_PACKET, file_pointer)) > 0) 
                {
                    memcpy(out, data, n); // Copy only the bytes actually read
                    out += n;             
                }
                end_flag = true;
                fclose(file_pointer);
            }
            else{
                return 403;
            }
            return end_flag;
		}
        else
		{
            int index = 3;
			strcpy(path,"www");

			while(*(in + get_cmd_len)!= SPACE_CHAR)
			{
				path[index++]= *(in + get_cmd_len++);
			}
			/* Start sending the file that is at that file path*/
            uint8_t* file_name = path;
            FILE *file_pointer;
            uint8_t* type_of_file;
            int size_of_file;
            uint8_t header[SIZE_OF_HEADER];
            uint8_t data[SIZE_OF_PACKET];
            bool end_flag = 0;
            if(!access(file_name,F_OK))
            {
                file_pointer = fopen(file_name,"r");
		        fseek(file_pointer,0,SEEK_END);
		        size_of_file = ftell(file_pointer);
		        fseek(file_pointer,0,SEEK_SET);
                
                if(type_of_file == NULL)
                {
                    return end_flag;
                }
                sprintf(header,"HTTP/1.1 200 OK\r\n Content-Type:%s\r\nContent-Length:%d\r\n\r\n",type_of_file,size_of_file);
               	memcpy(out,header,strlen(header));
		        buf_data += strlen(header);
		        out+=strlen(header);
                int n;
                while ((n = fread(data, 1, SIZE_OF_PACKET, file_pointer)) > 0) 
                {
                    /* copy data here */
                }
                end_flag = 1;
                fclose(file_pointer);
            }
            else{
                return 403;
            }
            return end_flag;
		}
    }
}

void thread_routine(void *arg)
{
    int ret_recv;
    int sock_fd = *(int*)arg;
    uint8_t msg[MSG_SIZE];
    uint8_t ret_str_finder;
    uint8_t *buf = (uint8_t *)malloc(BUF_SIZE);
    int ret_send;
    while((ret_recv = recv(sock_fd,msg,MSG_SIZE,0)) > 0)
    {
        printf("Msg received : %s\n",msg);
        buf_data = 0;
        ret_str_finder = str_finder(msg,buf);
        if(ret_str_finder == 1)
        {
            write(sock_fd,buf,buf_data);
        }
        else if(ret_str_finder == 404)
        {
            ret_send = send(sock_fd,error_404,strlen(error_404),0);
            exit_handler(sock_fd);
			break;
        } 
        bzero(msg,MSG_SIZE);
    }
    fflush(stdout);
    exit_handler(sock_fd);
    free(buf);
    free(arg);
    puts("\nClosing Client Sock\n");
}

/*
* Reference/Credits : I used the below code from one of my other courses called 'ECEN 5713 AESD'.
* There also I had to handle multiple clients using fork and pthreads. I used the same old method. 
*/
int main(int argc , char *argv[])
{
    int server_sock_fd;
    int client_sock_fd;
    struct timeval timer;
    int verify_parent_or_child = 1;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int size_sock = sizeof(struct sockaddr_in);
    int *client_sock = NULL;
    int client_connection_fd;
    server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == server_sock_fd)
    {
		printf("Failed to create server socket\n");    
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    int bind_status = bind(server_sock_fd,(struct sockaddr *)&server,size_sock);
    if(bind_status < 0)
	{
		perror("Failed the bind function call:");
		return FAIL;
	}

    int listen_status=listen(server_sock_fd,BACKLOG); 
	if(-1 == listen_status)
	{
		printf("Failed the listen function call\n");
		return FAIL;
	}

	printf("About to accept\n");

    /* Create a seperate fork for each client connection with 10secs timer*/
    while((client_connection_fd = accept(server_sock_fd,(struct sockaddr *)&client, (socklen_t*)&size_sock)))
    {
        printf("accept ran\n");

        verify_parent_or_child = (verify_parent_or_child != 0)?fork():0;
        if(!(verify_parent_or_child == 0))
        {
            continue;
        }
        pthread_t client_thread;
        client_sock = (int*)malloc(sizeof(int));
        if(NULL == client_sock)
        {
            printf("Falied to malloc\n");
            return FAIL;
        }
        *client_sock = client_connection_fd;
        timer.tv_sec = TIMEOUT_PERIOD;


        int ret_val = setsockopt(*client_sock,SOL_SOCKET,SO_RCVTIMEO, (const char*)&timer,sizeof(timer));
        if(-1 == ret_val)
        {
            perror("setsockopt failed:");
            return FAIL;
        }
        int return_val = pthread_create(&client_thread,NULL,thread_routine,(void*)client_sock);
        if(return_val != 0)
        {
            perror("pthread failed:");
            return FAIL;
        }
        printf("Thread created\n");
    }
    if(client_connection_fd < 0)
    {
        perror("accept func falied:");
        return FAIL;
    }
    return SUCCESS;
}
