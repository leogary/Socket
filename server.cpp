#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <regex.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include<set>
using namespace std;

#define MAXCLIENT 5


int socketfd[MAXCLIENT]={0};
int clientnum=0;
sockaddr_in socketaddress[MAXCLIENT]={0};
map<string,string>fileowner;
map<string,string>fileright;
vector<string>filelist;
set<string>user;
map<string,string>group;
map<string,int>someonereading;


vector<string> split(string input){
    vector<string>v;
    int index=0;
    v.push_back("");
    for(int i=0; i<input.length();i++){
        //cout<<input[i]<<" "<<index<<endl;
        if(input[i]=='\0'){
            break;
        }
        else if(input[i]==' '){
            index++;
            v.push_back("");    
        }
        else{
            v[index]=v[index]+input[i];
        }
    }
    return v;

}

bool checkaccessright(string filename,string user,int action){
    string owner=fileowner[filename];
    string right=fileright[filename];
    bool samegroup;
    if(group[owner]==group[user]){
        samegroup=true;
    }else{
        samegroup=false;
    }
    if(user==owner){
        if(action==0){
            if(right[0]=='r'){
                return true;
            }
            else{
                return false;
            }
        }
        else{
            if(right[1]=='w'){
                return true;
            }
            else{
                return false;
            }
        }

    }
    else if(samegroup){
        if(action==0){
            if(right[2]=='r'){
                return true;
            }
            else{
                return false;
            }
        }

        else{
            if(right[3]=='w'){
                return true;
            }
            else{
                return false;
            }
        }

    }
    else{
        if(action==0){
            if(right[4]=='r'){
                return true;
            }
            else{
                return false;
            }
        }

        else{
            if(right[5]=='w'){
                return true;
            }
            else{
                return false;
            }
        }
    }

    return false;

}

void load(){
    string path="data/capabilitylist";
    char buffer[1024];
    memset(buffer, 0, 1024);
    FILE*f= fopen(&path[0],"r");
    while (fgets(buffer,sizeof(buffer), f))
    {
        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;
        string a=buffer;
        vector<string>v;
        v=split(a);
        fileowner[v[0]]=v[1];
        fileright[v[0]]=v[2];
        filelist.push_back(v[0]);
        user.insert(v[1]);
        group[v[1]]=v[3];
       
    }
    fclose(f);


}
void save(){
    string path="data/capabilitylist";
    ofstream myfile(path);

    if(myfile.is_open())
    {
        string str;
        for(int i=0;i<filelist.size();i++){
           myfile<<filelist[i]<<" "<<fileowner[filelist[i]]<<" "<<fileright[filelist[i]]<<" "<<group[fileowner[filelist[i]]]<<endl; 
        }
        myfile.close();
    }
    else cout<<"Unable to open file";
}

void clientthread(int index){
    srand(time(NULL));
    char inputBuffer[256] = {};
    bool flag = true;
    
    send(socketfd[index],"connect success\n",16,0);
    recv(socketfd[index],inputBuffer,sizeof(inputBuffer),0);
    printf("%s has connected\n",inputBuffer);
    string name=inputBuffer;
    if(user.count(name)){
        send(socketfd[index],"user is online \n input exit to leave\n",38,0);
        //close(socketfd[index]);
        cout<<name<<" is disconnected"<<endl;
        flag=false ;
        socketfd[index]=0;
        --clientnum;
        close(socketfd[index]);
        return;
    }
    user.insert(name);
    
    if(rand()%2==0){
        group[name]="ASO";
    }
    else{
        group[name]="CSE";
    }
    


    while(flag){
        memset(inputBuffer,0,sizeof(inputBuffer));
        char sendMsg[1024] = {0};
        memset(sendMsg,0,sizeof(sendMsg));
        recv(socketfd[index],inputBuffer,sizeof(inputBuffer),0);
        string command=strtok(inputBuffer,"\n");
        vector<string> v=split(command);
        const char*filename=v[1].data();
        string path = "data/" + v[1];
        if(v[0]=="create"&&v.size()==3){
            //create directory
            mkdir("data",S_IRWXU|S_IRWXG|S_IRWXO);
            //check file exist
            if(access(&path[0],F_OK) == 0){
                //file exist
                cout<<"file existed"<<endl;
                send(socketfd[index],"file exist\n",12,0);
           
            }else{
                //create file
                FILE *fout;
                fout = fopen(&path[0],"w+");
                fclose(fout);
                fileright[v[1]]=v[2];
                fileowner[v[1]]=name;
                filelist.push_back(v[1]);
                cout<<name<<" created "<<v[1]<<" "<<v[2]<<endl;
                send(socketfd[index],"create success\n",16,0);

            }
        }
        else if(v[0]=="read"&&v.size()==2){
            if(access(&path[0],F_OK) == 0){
                if(checkaccessright(v[1],name,0)){
                    //file exist
                    while(someonereading[v[1]]<0){ //someone writing
                        cout<<"someonewriting"<<endl;
                    }
                    someonereading[v[1]]++;
                    cout<<name<<" reading "<<v[1]<<endl;
                    mkdir(&name[0],0777); 
                    string destination=name+"/"+v[1];
                    char buffer[1024];
                    memset(buffer, 0, 1024);
                    FILE*f= fopen(&path[0],"r");
                    ofstream myfile(destination);
                    while (fgets(buffer,sizeof(buffer), f))
                    {
                        // Remove trailing newline
                        buffer[strcspn(buffer, "\n")] = 0;
                        string a=buffer;
                        myfile<<a<<endl;
                    
                    }
                    fclose(f);  
                    myfile.close();
                    cout<<name<<" finish reading "<<v[1]<<endl;
                    send(socketfd[index],"download success",17,0);
                    someonereading[v[1]]--;
                    
                }
                else{
                cout<<name<<" access denied"<<endl;
                send(socketfd[index],"you have no accessright",24,0);
                }
            }
            else{
                    cout<<"file not existed"<<endl;
                    send(socketfd[index],"file not exist",15,0);
            }
 
        }
        else if(v[0]=="write"&&v.size()==3&&(v[2]=="o"||v[2]=="a")){

            if(access(&path[0],F_OK) == 0){
                if(checkaccessright(v[1],name,1)){
                    //file exist
                    while(someonereading[v[1]]!=0){ //someonereading
                        cout<<"someone reading or writing"<<endl;
                    }
                    someonereading[v[1]]-=100;//can't read
                    cout<<name<<" writing "<<v[1]<<endl;
                    
                    if(v[2]=="o"){
                        //FILE *fout;
                        ofstream in;
                        in.open(&path[0],ios::trunc); //ios::trunc表示在開啟檔案前將檔案清空,由於是寫入,檔案不存在則建立
                        int i;
                        char a='a';
                        for(long i=1;i<=100000000;i++)
                        {
                        
                            in<<"0"<<i<<"\t"<<"\n";

                        
                        }
                        in.close();//關閉檔案
                        cout<<name<<" rewrite "<<v[1]<<endl;
                        send(socketfd[index],"rewrite success",16,0);
                        in.close();
                        //fclose(fout);
                    }else{
                        //FILE *fout;
                        ofstream file_out;
                        file_out.open(&path[0], std::ios_base::app);
                        file_out << "Some random text to append." << endl;
                        cout<<name<<" append "<<v[1]<<endl;
                        send(socketfd[index],"append success",15,0);
                        file_out.close();
                        //fclose(fout);
                    }
                    
                    cout<<name<<" finish writing "<<v[1]<<endl;
                    someonereading[v[1]]+=100;
                
           
                }
                else{
                cout<<name<<" access denied"<<endl;
                send(socketfd[index],"you have no accessright",24,0);
                }
            
            }
            else{
                    cout<<"file not existed"<<endl;
                    send(socketfd[index],"file not exist",15,0);
            }

        }
        else if(v[0]=="changeright"&&v.size()==3){
            if(name==fileowner[v[1]]){
                fileright[v[1]]=v[2];
                send(socketfd[index],"change success",15,0);
                cout<<name<<" change "<<v[1]<<" rights"<<endl;
            }
            else{
                send(socketfd[index],"you can not change right",25,0);
                cout<<name<<" change "<<v[1]<<" failed"<<endl;
            }
        }
        else if(command=="exit"){
            socketfd[index]=0;
            break;
        } 
        else{

        }

    }
    socketfd[index]=0;
    --clientnum;
    close(socketfd[index]);

    
}

void serverinput(int* sockfd,bool*closeserver){
    cout<<"server working"<<endl;
    load();
    cout<<"load success"<<endl;
    string input;
    while(1){
        //fflush(stdin);
        cin.clear();
        getline(cin, input);
        if(input == "exit"){
            *closeserver=true;
            save();
            cout<<"save success"<<endl;
            shutdown(*sockfd,SHUT_RD);
            clientnum=0;
            // broadcast server is shutdown
            for(int i=0 ; i < MAXCLIENT ; i++){
                if(socketfd[i] != 0){
                    send(socketfd[i],"exit",5,0);
                }
                
            }
            
            break;
        }
        else if(input =="cl"){
            printf("    file      owner     rights\n");
            printf("----------------------------------\n");
            for(int i=0;i<filelist.size();i++){
                cout<<setw(8)<<filelist[i]<<setw(10)<<fileowner[filelist[i]]<<setw(12)<<fileright[filelist[i]]<<endl;
            }
        }
        else if(input =="ul"){
            printf("    user    groups\n");
            printf("-------------------\n");
            for(set<string>::iterator it=user.begin();it!=user.end();it++){
                cout<<setw(8)<<*it<<setw(8)<<group[*it]<<endl;
            }
        }
        else if(input=="save"){
            save();
            cout<<"save success"<<endl;
        }
        else if(input=="load"){
            load();
            cout<<"load success"<<endl;
        }
        else{
            cout << "-------------------" << endl;
            cout << "ul:         userlist" << endl;
            cout << "load:       load data" << endl;
            cout << "sava:       save  data" << endl;
            cout << "cl:         capabilitylist" << endl;
            cout << "exit:       close server" << endl;
            cout << "-------------------" << endl;
        }
    }
}

int main(int argc , char *argv[])
{
    //socket的建立
    
    clientnum=0;
    bool closeserver=false;
    char message[] = {"Hi,this is server.\n"};
    int sockfd = 0,forClientSockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in serverInfo,clientInfo;
    int addrlen = sizeof(clientInfo);
    socklen_t src_len = sizeof(socketaddress[0]);
    bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(8700);
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,MAXCLIENT);
    thread server(serverinput,&sockfd,&closeserver);
    server.detach();
    string input="";
    while(!closeserver){    
        for(int i=0;i<MAXCLIENT;i++){
            
            if(socketfd[i]==0){
                
                socketfd[i] = accept(sockfd,(struct sockaddr*) &socketaddress[clientnum], &src_len);
                if(!closeserver)clientnum++;
                cout<<"clientnumber:"<<clientnum<<endl;
                
                thread t(clientthread,i);
                t.detach();
                break;
                      
            }
            if(closeserver){
                break;
            }
            
        }
        
    }
    close(sockfd);
    return 0;
}
