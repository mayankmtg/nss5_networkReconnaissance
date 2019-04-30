#include <bits/stdc++.h>
#include <thread>
using namespace std;


class syn_thread_handler{
    public:
        void operator()(int port){
            cout << port << endl;

        }
};

class fin_thread_handler{
    public:
        void operator()(int port){
            cout << port << endl;

        }
};
class udp_thread_handler{
    public:
        void operator()(int port){
            cout << port << endl;

        }
};
int main(int argc, char* argv[]){
    int syn_port = 7001;
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
