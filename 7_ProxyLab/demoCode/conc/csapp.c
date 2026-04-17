#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define CACHE_LINES (MAX_CACHE_SIZE / MAX_OBJECT_SIZE)

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* Forward declarations */
void proxy_conn(int clientfd); 
void clienterror(int clientfd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void *thread(void *vargp);
unsigned long hash_func(char *str);
int get_write_idx(); 

typedef struct {
    unsigned long hash;
    int timestamp;
    int valid;
    int length;                     // how big is the response?
    char object[MAX_OBJECT_SIZE];   // the actual response
} cacheLines_t;

cacheLines_t* cache;
unsigned long curr_time = 0;
int read_cnt = 0;
sem_t mutex, w;


/* Main Entry */
int main(int argc, char **argv)
{
    int listenfd;       // listener file descriptor
    int* connfdp;       // pointer to connection file descriptor 
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    Sem_init(&mutex, 0, 1);                                                      
    Sem_init(&w, 0, 1);

    /* Check for args, print usage message */
    if (argc != 2) {
          fprintf(stderr, "usage: %s <port>\n", argv[0]);           
          exit(1);
    }

    cache = Calloc(CACHE_LINES, sizeof(cacheLines_t));  // allocate cache == CACHE_LINES * sizeof struct

    listenfd = Open_listenfd(argv[1]);  // Open listener
    while (1) { // loop infinitely until manually killed (Ctrl+c)
	clientlen = sizeof(clientaddr);
    connfdp = Malloc(sizeof(int));
    /* Wait for a connection, store the client's info in clientaddr, and put   
    the new file descriptor into the memory connfdp points to */
	*connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen); //Accept connection 
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                    port, MAXLINE, 0);  // Get hostname info of requestor
        printf("Accepted connection from (%s, %s)\n", hostname, port);
    Pthread_create(&tid, NULL, thread, connfdp);  // call thread routine 
    }
}

/* Thread routine */
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);  /* Take the generic pointer, treat it as a 
                                    pointer to an int, and grab the int value (file descriptor) */
    Pthread_detach(pthread_self());
    Free(vargp);
    proxy_conn(connfd);  // individual thread routine calls proxy request handler 
    Close(connfd);
    return NULL;
}

void clienterror(int clientfd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Proxy Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Proxy server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(clientfd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(clientfd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(clientfd, buf, strlen(buf));
    Rio_writen(clientfd, body, strlen(body));
}

/* Proxy Request Handler */
void proxy_conn(int clientfd) 
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char host[MAXLINE], port[MAXLINE], path[MAXLINE];
    char request[MAXLINE]; 
    rio_t rio;  // robust io buffer (pipe)                                       
     

    /* Read request line and headers */
    Rio_readinitb(&rio, clientfd); // set up rio so that future reads pull data from clientfd
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //using rio as the source, pull out the next line and put it in buf
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       // read from buf: method, uri, version (%s stops at first whitespace)
    if (strcasecmp(method, "GET")) {                     // if method is not GET
        clienterror(clientfd, method, "501", "Not Implemented", // send error to client and stop
                    "Proxy does not implement this method");  // proxy only deals with GET requests 
        return;
    }
    /* Parse URL: http://host:port/path */
    if (sscanf(uri, "http://%[^:/]:%[^/]%s", host, port, path) == 3) {
        /* got all three */
    } else if (sscanf(uri, "http://%[^/]%s", host, path) == 2) {
        strcpy(port, "80");
    } else {
        strcpy(port, "80");
        strcpy(path, "/");
    }

    unsigned long request_hash = hash_func(uri);
    P(&mutex);      // lock read_cnt variable so only one thread changes counter at a time
    read_cnt++;
    if (read_cnt == 1) // if 1 reader
        P(&w);          // lock out writers
    V(&mutex);          // unlock counter after updating  
    for (int i = 0; i < CACHE_LINES; i++) {
        if (request_hash == cache[i].hash && cache[i].valid == 1){
            //printf("Cache hit!\n");
            Rio_writen(clientfd, cache[i].object, cache[i].length);  // write cached object to client file descriptor 
            P(&mutex);      // lock read_cnt variable
            read_cnt--;
            if (read_cnt == 0)      // if read count is none
                V(&w);              // let writers in 
            V(&mutex);              // unlock read_cnt 
            return;
        }
    }
    P(&mutex);  // lock read_cnt variable
    read_cnt--; 
    if (read_cnt == 0) // if read count is none
        V(&w);  // let writers in
    V(&mutex);  // unlock read_cnt


    Rio_readlineb(&rio, buf, MAXLINE); // using rio as source, read next line into buf up to MAXLINE bytes
    sprintf(request, "GET %s HTTP/1.0\r\n", path);  // build the request line
    /* loop through client request and keep or discard headers */
    while (strcmp(buf, "\r\n")) {  //while /r/n is not in the buf, keep comparing, else if they are equal, stop
        /* if substring in each header do nothing, throw away*/
        if (strstr(buf, "User-Agent")) {
            ;
        }else if (strstr(buf, "Proxy-Connection"))
            ;
        else if (strstr(buf, "Connection"))
            ;
        else
            strcat(request, buf);   // if not one of the above, append to request
        Rio_readlineb(&rio, buf, MAXLINE);
    }
    /* Append required headers and concatenate to a new request variable */ 
    sprintf(buf, "Host: %s\r\n", host);
    strcat(request, buf);
    strcat (request, user_agent_hdr);
    sprintf(buf, "Connection: close\r\n");
    strcat(request, buf);
    sprintf(buf, "Proxy-Connection: close\r\n");
    strcat(request, buf);
    strcat(request, "\r\n");

    // Open connection to external server
    int serverfd = Open_clientfd(host, port);
    // Send request from proxy
    Rio_writen(serverfd, request, strlen(request));
    
    /* read server response */
    rio_t server_rio;  // buf for external server (pipe)                                              
    Rio_readinitb(&server_rio, serverfd);  // set up pipe to read from server
    /* read a chunk, keep going as longs as not 0 */
    ssize_t n; // size of response chunk 
    char cache_buf[MAX_OBJECT_SIZE]; // buffer for complete server response                                             
    int total = 0;  

    while ((n = Rio_readnb(&server_rio, buf, MAXLINE)) > 0) {
        Rio_writen(clientfd, buf, n);   // write to client fd
        if (total + n <= MAX_OBJECT_SIZE) {
            memcpy(cache_buf + total, buf, n);
        }
        total += n;                                 
    }
    if (total <= MAX_OBJECT_SIZE) {
        P(&w);
        int idx = get_write_idx();
        cache[idx].hash = request_hash;
        cache[idx].length = total;
        memcpy(cache[idx].object, cache_buf, total);
        cache[idx].valid = 1;
        cache[idx].timestamp = ++curr_time;
        V(&w);
    }


    Close(serverfd);
}

unsigned long hash_func(char *str) {                                         
    unsigned long hash = 5381;                                               
    int c;                                                                   
    while ((c = *(str++)))
        hash = ((hash << 5) + hash) + c;                                     
    return hash;                                                             
}

int get_write_idx() {
    int lru_idx = 0;                                                         
    for (int i = 0; i < CACHE_LINES; i++) {
        if (cache[i].valid == 0)                                             
            return i;                                                        
        if (cache[i].timestamp < cache[lru_idx].timestamp)
            lru_idx = i;                                                     
    }           
    return lru_idx;
} 