#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <strstream>
#include <queue>
#include <map>
#include <pthread.h>


#include <sys/param.h>

using namespace std;

#define BUF_SIZE 200


struct mapvalue{
    int bucketscount;
    int ntime;
};

struct datavalue{
    int number;
    int ntime;
};

int g_count;
int flag_connected = 0;
bool g_bfinish = false;
int g_nport = 0;
string g_host ;
vector<int>  datalist;
//queue<datavalue> kqueue; 
queue<datavalue> inputqueue; 
vector<map<int, mapvalue> >  vector_map;

void addtoMap(map<int, mapvalue> &map_number, int ntime, int bucketsize){
    map<int, mapvalue>::iterator it;
    it = map_number.find(bucketsize);
    int curtime = ntime;
    int pretime = 0;
    int newsize = bucketsize;
    while((it!=map_number.end()) && (it->second.bucketscount>1)){
        pretime = it->second.ntime;
        it->second.bucketscount = 1;
        it->second.ntime = curtime;
        curtime = pretime;
        newsize = newsize*2;
        it=map_number.find(newsize);
    }
    if(it!=map_number.end()){
        it->second.bucketscount += 1;
    }
    else{
        mapvalue newvalue;
        newvalue.bucketscount = 1;
        newvalue.ntime = curtime;
        map_number.insert(make_pair(newsize, newvalue));
    }        
    return; 
}

int addtoVector(datavalue data){
    int mask = 1;
    int number = 0;
    for(int i = 15; i>=0; i--){
        if (data.number & mask){
            addtoMap(vector_map[i], data.ntime, 1);
        }
        mask <<= 1;
    }
    return 0;
}

int processData(){
    while(!g_bfinish){
        if(!inputqueue.empty()){
            datavalue data = inputqueue.front();
            addtoVector(data);
            inputqueue.pop();
        }
    }
    return 0;
}

long int sumofMap(map<int, mapvalue> &map_number, int timestamp){
    float res = 0;
    map<int, mapvalue>::iterator it;
    it = map_number.begin();
    int buckettime = 0;
    while(it != map_number.end() && (it->second.ntime >= timestamp)){
        res += (it->first) * (it->second.bucketscount);
        buckettime = it->second.ntime;
        it++;
    }
    if((it!=map_number.end()) && (buckettime>timestamp)){
        res +=  ((it->first) * (it->second.bucketscount))* (buckettime - timestamp)/(buckettime - it->second.ntime); 
    }
    return res;
}

long int sumofVectors(int K, int count){
    long int res = 0;
    int timestamp = count - K;
    int vectorsize = vector_map.size();
    for(int i = 0; i<vectorsize; i++){
        res <<= 1;
        res += sumofMap(vector_map[i], timestamp);
    }
    return res;
}

bool converIP(string hostname){
    hostent * record = gethostbyname(hostname.c_str());
	if(record == NULL)
	{
		cout << "hostname is incorrect!" << endl;
		return false;
	}
	in_addr * address = (in_addr * )record->h_addr;
	g_host = inet_ntoa(* address);
    return true;
}

bool readinArg(){
    char buffer[60];
    char * pch;
    bool result = true;
    memset(buffer,0,sizeof(buffer));
    cin.getline(buffer, 60);
    string info(buffer);
    int n = info.find(':');
    if(n!=string::npos){
        if(!converIP(info.substr(0,n))){
            return false;
        }
        for(int i=n+1; i<info.length(); i++){
            g_nport *= 10;
            if (info[i]<='9' && info[i]>='0'){
                g_nport+=info[i]-'0';
            }
            else{
                cout << "Wrong server or port information! " << endl;
                result = false;
                break;
            }
        }
    }
    else{
        cout << "Wrong server or port information! " << endl;
        result = false;
    }
    return result;
}

long int truesum(int num, int count){
    long int res = 0;
    for(int i=1; i<=num; i++){
        res += datalist[count-i];
    }
    return res;
}

void* query_func(void* arg){
    char buffer[80];
    string info;
    while(info!="end"){
        memset(buffer, 0 , sizeof(buffer));
        cin.getline(buffer, 80);
        info.assign(buffer);
        cout<<"info = "<<info<<endl;
        if(info=="end"){
            cout << "end" << endl;
        }
        else{
            int size = info.length(); 
            if(size<26){
                cout << "info = "<<info  << endl;
                cout << "Wrong query, program exit." << endl;
                break;
            }
            else if(info.substr(0,25)!="What is the sum for last "){
                cout << info.substr(0,26) << "What is the sum for last " << endl;
                cout << "Wrong query, program exit." << endl;
                break;
            }
            int pos = 25;
            int num = 0;
            while(pos < size){
                if(info[pos]>= '0' && info[pos]<='9'){
                    num*=10;
                    num+=info[pos]-'0';
                }
                else if(num>0){
                    break;
                }
                pos++;
            }       
            cout << "num = " << num << endl;         
            if(num>0){
                while(num>g_count){
                        sleep(1);
                }
                int count = g_count;
                cout << info << endl;
                cout << "The sum of last "<< num <<" integers is "   <<  sumofVectors(num, g_count) << endl;
                cout << "The sum of last "<< num <<" integers is "   <<  truesum(num, count) << "array" <<endl;
                num = 0;
            }
       }    
    }     
    g_bfinish = true;
    usleep(10000);
    pthread_exit(0);
}  

void* receive_func(void* arg){
    int sock;
    long int sumnumbers = 0;
    struct sockaddr_in server;
    char revnum[BUF_SIZE];
    datavalue newdata;
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        flag_connected = -1;
        cout << "g_bfinish = true" << endl;
        cout << "Could not create socket" << endl;
        pthread_exit(0);
    }
    
    server.sin_addr.s_addr = inet_addr(g_host.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(g_nport);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        cout << "connect failed. Error" << endl;
        close(sock);    
        flag_connected = -1;
        pthread_exit(0);
    }
    else{
        flag_connected = 1;
    }
    vector_map.resize(16);
    memset(revnum, 0, sizeof(revnum));
    while((recv(sock , revnum , 2000 , 0) > 0) && !g_bfinish)
    {
        cout << revnum << endl;
        newdata.number=atoi(revnum);
        newdata.ntime=g_count;
        inputqueue.push(newdata);
        datalist.push_back(newdata.number);
        g_count++;
        memset(revnum, 0, sizeof(revnum));
    }
    close(sock);    
    pthread_exit(0);
}

int main()
{
    g_count = 0;
    
    if(readinArg()==false){
        return(0);
    }
    
    void *status_rec = 0;
    void *status_que = 0;
    // Thread ID:
    pthread_t tid_rec;
    pthread_t tid_que;
    
    // Create attributes:
    pthread_attr_t attr_rec;
    pthread_attr_t attr_que;
    
    // Create receiving data thread 
    pthread_attr_init(&attr_rec);    
    pthread_create(&tid_rec, &attr_rec, receive_func, NULL);
    
    while(flag_connected==0){
        
    }
    if(flag_connected==1){
        pthread_attr_init(&attr_que);    
        pthread_create(&tid_que, &attr_que, query_func, NULL);

        processData();
        pthread_join(tid_que, &status_que);
        pthread_join(tid_rec, &status_rec);
        return 0;
   }
    else{
        pthread_join(tid_rec, &status_rec);
        return 0;
    }
}
