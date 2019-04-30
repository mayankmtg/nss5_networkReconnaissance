#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <errno.h>
#include <ctime>

using namespace std;

struct pseudo_header
{
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;
    unsigned char protocol;
    unsigned short tcp_length;
    struct tcphdr tcp;
};

unsigned short swaps( unsigned short val)
{
    return ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
}

void throw_error(string type, string s){
	cerr <<"error\t"<< type << " : " << s << endl;
	exit(0);
}


unsigned short csum(unsigned short *ptr,int nbytes) {
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }

    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;

    return(answer);
}
struct iphdr* craft_ip_header(char* packet, char* src_addr_char, unsigned int d_addr_uint){
	struct iphdr* iph = (struct iphdr*) packet;
	iph->version = 4;
	iph->ihl = 5;
	iph->tos = 0;
	iph->tot_len = sizeof(struct ip) + sizeof(struct tcphdr);
	iph->id = htonl(54321);
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0;
	iph->saddr = inet_addr(src_addr_char);
	iph->daddr = d_addr_uint;
	iph->check = csum ((unsigned short *) packet, iph->tot_len >> 1);
	return iph;
}

struct tcphdr* craft_tcp_header(char* packet, int port_no){
	struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof (struct ip));
	tcph->source = htons (8080);
    tcph->dest = htons (port_no);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;
	tcph->fin=1;
	tcph->syn=0;
	tcph->rst=0;
	tcph->psh=0;
	tcph->ack=0;
	tcph->urg=0;
    tcph->window = htons (5840);
    tcph->check = 0;
    tcph->urg_ptr = 0;
	return tcph;

}
struct pseudo_header craft_pseudo_header(char* src_addr_char, unsigned int d_addr_uint){
	struct pseudo_header psh;
	psh.source_address = inet_addr(src_addr_char);
	psh.dest_address = d_addr_uint;
    // psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(20);
	return psh;
}

bool send_fin(char* dst_addr_char, int port_no){

	int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
	char packet[4096];
	// struct iphdr* iph = (struct iphdr*) packet;
	struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8080);
    sin.sin_addr.s_addr = inet_addr (dst_addr_char);

	memset(packet, 0, 4096);
	char src_addr_char[16];
	sprintf(src_addr_char, "%d.%d.%d.%d\n", 192,168 ,60,56);
	struct iphdr *iph = craft_ip_header(packet, src_addr_char, sin.sin_addr.s_addr);
	struct tcphdr *tcph = craft_tcp_header(packet, port_no);
	struct pseudo_header psh = craft_pseudo_header(src_addr_char,sin.sin_addr.s_addr);	

    memcpy(&psh.tcp , tcph , sizeof (struct tcphdr));
    tcph->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));

	int one = 1;
    const int *val = &one;
    int sockop = setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one));
    if (sockop < 0){
        throw_error("access", "root access required");
    }
    if(sendto (s, packet, iph->tot_len, 0, (struct sockaddr *) &sin, sizeof (sin)) < 0){
        throw_error("sending", "sending packet");
	}
	struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt (s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0){
        throw_error("options", "setsockopt failed");
	}
	// sleep(0.04);
	char recvPacket[4096] = "";
	// struct sockaddr_in *sin2;
	// struct sockaddr temp;
	// socklen_t llen;
	int timeout_flag = 0;
	time_t start, finish;
	time(&start);
	while(true){
		int newData = recv(s,recvPacket,sizeof(recvPacket),0);
		if(newData == -1 && (errno== EAGAIN || errno==EWOULDBLOCK)){
			timeout_flag = 1;
			break;
		}
		time(&finish);
		if(difftime(finish, start) >= 5){
			timeout_flag = 1;
			break;
		}
		struct iphdr *rec_iph = (struct iphdr *) recvPacket;
		struct tcphdr* rec_tcp = (struct tcphdr *) (recvPacket + sizeof (struct ip));
		if(ntohs(rec_tcp->dest) == 8080){
			cout << port_no<< "\t:\t"<< 'f' << rec_tcp->fin << 's' << rec_tcp->syn << 'r' << rec_tcp->rst << 'p' << rec_tcp->psh << 'a' << rec_tcp->ack << 'u' << rec_tcp->urg;
			if(rec_tcp->fin == 0 && rec_tcp->syn == 0 && rec_tcp->rst == 1 && rec_tcp->psh == 0 && rec_tcp->ack == 1 && rec_tcp->urg == 0){
				// RST 1 && ACK 1
				cout << "\tRST ACK" << endl;
			}
			else if(rec_tcp->fin == 0 && rec_tcp->syn == 1 && rec_tcp->rst == 0 && rec_tcp->psh == 0 && rec_tcp->ack == 1 && rec_tcp->urg == 0){
				cout << "\tSYN ACK" << endl;
			}
			else{
				cout << "\tELSE" << endl;
			}
			break;
		}
		else{
			continue;
		}
	}
	if(timeout_flag == 1){
		cout << port_no<< "\t:\t"<< 'f' << 0 << 's' << 0 << 'r' << 0 << 'p' << 0 << 'a' << 0 << 'u' << 0;
		cout << "\tFIN OUT" << endl;
	}
	close(s);
	return true;
}

int main(int argc, char* argv[]){
	string dst_addr = "192.168.2.166";
	for (int i =7000;i <=7010; i++){
		send_fin((char *)dst_addr.c_str(), i);
	}
	return 0;
}