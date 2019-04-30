#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <thread>
using namespace std;


void throw_error(string ss){
	cerr << "error:\t " << ss << endl;
	exit(0);
}
class syn_thread_handler{
    public:
        void operator()(int port){
            int sockfd = socket(AF_INET,SOCK_STREAM,0);

			if(sockfd == -1){
				throw_error("socket creation");
			}
			
			struct sockaddr_in svrAdd;
			bzero((char*) &svrAdd, sizeof(svrAdd));
			svrAdd.sin_family = AF_INET;
			svrAdd.sin_addr.s_addr = INADDR_ANY;
			svrAdd.sin_port = htons(port);
			int bindRet = bind(sockfd, (struct sockaddr*) &svrAdd, sizeof(svrAdd));
			cout << "Listening on port: " << to_string(port) << endl;
			if(bindRet==-1){
				throw_error("binding socket");
			}
			
			int listenRet = listen(sockfd, 5);
			
			if(listenRet==-1){
				throw_error("listening on port");
			}
			
			int svrAddLen = sizeof(svrAdd);
			while(1==1){
                cout << "Accepting on port: " << to_string(port) << endl;
				int acceptFd = accept(sockfd,(struct sockaddr *)&svrAdd,(socklen_t*) &svrAddLen);
				if(acceptFd==-1){
					throw_error("accepting connection");
				}
				char buff[1024];
				int read_size = read(acceptFd,buff,1024);
				
				if(read_size==-1){
					throw_error("reading socket failed");
				}
				else if(read_size == 0){
					return;
				}
                string text = "connected";
				send(acceptFd,text.c_str(),text.length(),0);
			}
        }
};

class fin_thread_handler{
    public:
        void operator()(int port){
            int sockfd = socket(AF_INET,SOCK_STREAM,0);

			if(sockfd == -1){
				throw_error("socket creation");
			}
			
			struct sockaddr_in svrAdd;
			bzero((char*) &svrAdd, sizeof(svrAdd));
			svrAdd.sin_family = AF_INET;
			svrAdd.sin_addr.s_addr = INADDR_ANY;
			svrAdd.sin_port = htons(port);
			int bindRet = bind(sockfd, (struct sockaddr*) &svrAdd, sizeof(svrAdd));
			cout << "Listening on port: " << to_string(port) << endl;
			if(bindRet==-1){
				throw_error("binding socket");
			}
			
			int listenRet = listen(sockfd, 5);
			
			if(listenRet==-1){
				throw_error("listening on port");
			}
			
			int svrAddLen = sizeof(svrAdd);
			while(1==1){
                cout << "Accepting oon port: " << to_string(port) << endl;
				int acceptFd = accept(sockfd,(struct sockaddr *)&svrAdd,(socklen_t*) &svrAddLen);
				if(acceptFd==-1){
					throw_error("accepting connection");
				}
				char buff[1024];
				int read_size = read(acceptFd,buff,1024);
				
				if(read_size==-1){
					throw_error("reading socket failed");
				}
				else if(read_size == 0){
					return;
				}
                string text = "connected";
				send(acceptFd,text.c_str(),text.length(),0);
			}
        }
};
class udp_thread_handler{
    public:
        void operator()(int port){
            int sockfd = socket(AF_INET,SOCK_DGRAM,0);
			if(sockfd == -1){
				throw_error("socket creation");
			}
			struct sockaddr_in svrAdd, clnAdd;
			bzero((char*) &svrAdd, sizeof(svrAdd));
            bzero((char*) &clnAdd, sizeof(clnAdd));

			svrAdd.sin_family = AF_INET;
			svrAdd.sin_addr.s_addr = INADDR_ANY;
			svrAdd.sin_port = htons(port);
			int bindRet = bind(sockfd, (struct sockaddr*) &svrAdd, sizeof(svrAdd));
			cout << "Listening on port: " << to_string(port) << endl;
			if(bindRet==-1){
				throw_error("binding socket");
			}
			
			int svrAddLen = sizeof(svrAdd);
			while(1==1){
				char buff[1024];
                socklen_t llen;
				int read_size = recvfrom(sockfd,buff,1024, MSG_WAITALL, ( struct sockaddr *) &clnAdd, &llen);
				if(read_size==-1){
					throw_error("reading socket failed");
				}
				else if(read_size == 0){
					return;
				}
                buff[read_size] = '\0';
                string text = string(buff);
                cout << text << endl;
				// send(acceptFd,text.c_str(),text.length(),0);
			}
        }
};
int main(int argc, char* argv[]){
    int syn_port = 7010;
    int fin_port = 7002;
    int udp_port = 7003;
    thread syn_thread(syn_thread_handler(), syn_port);
    thread fin_thread(fin_thread_handler(), fin_port);
    thread udp_thread(udp_thread_handler(), udp_port);
    syn_thread.join();
    fin_thread.join();
    udp_thread.join();
    return 0;
}
