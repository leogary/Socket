#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

//c++ 
#include<iostream>
#include <string>
#include <vector>

using namespace std;

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
void hint(){
    cout << "-------------------" << endl;
    cout << "create:     create $fileName $rights" << endl;
    cout << "read:       read $fileName" << endl;
    cout << "write:      write $fileName o/a (overwrite/append)" << endl;
    cout << "changeright: changeright $fileName $rights" << endl;
    cout << "exit:       close client & disconnect" << endl;
    cout << "-------------------" << endl;
}


int main(int argc , char *argv[])
{

    //socket的建立
    int sockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    string input = "";
    cout<<"input your name"<<endl;
    getline(cin, input);
    string name = input;


    if (sockfd == -1){
        printf("Fail to create a socket.");
        return -1;
    }

    //socket的連線

    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = PF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(8700);

    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1){
        printf("Connection error \n");
        return -1;
    }


    //Send a message to server
    char message[100] ;
    //vector<char>in(input.begin(),input.end());
    const char*a=input.data();
    sprintf(message,"%s",a);
    char receiveMessage[100] = {};
    send(sockfd,message,sizeof(message),0);
    recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
    printf("%s",receiveMessage);
    while(1){
        //cout<<"cin<<"<<endl;
        getline(cin,input);
        const char*ab=input.data();
        sprintf(message,"%s\n",ab);
        vector<string> v;
        v= split(input);
        string path=name+"/"+v[1];
        if(input=="exit"){
            send(sockfd,message,sizeof(message),0);
            break;
        }
        else if(v.size()==3&&v[0]=="create"){
            send(sockfd,message,sizeof(message),0);
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            cout<<receiveMessage;
        }
        else if(v.size()==2&&v[0]=="read"){
            send(sockfd,message,sizeof(message),0);
            /*recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            string cmd=string(strtok(receiveMessage," "));*/
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            cout<<receiveMessage<<endl;
            
        }
    
        else if(v.size()==3&&v[0]=="write"){
            send(sockfd,message,sizeof(message),0);
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            cout<<receiveMessage<<endl;

        }
        else if(v.size()==1&&v[0]=="hint"){
            hint();
        }
        else if(v.size()==3&&v[0]=="changeright"){
            send(sockfd,message,sizeof(message),0);
            recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
            cout<<receiveMessage<<endl;
        }
        else{
            //send(sockfd,message,sizeof(message),0);
            hint();

        }
    }

    

    printf("close Socket\n");
    close(sockfd);
    return 0;
}
