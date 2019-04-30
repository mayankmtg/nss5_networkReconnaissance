#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
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
    unsigned short udp_length;
    struct udphdr udp;
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
struct iphdr* craft_ip_header(char* datagram, char* src_addr_char, unsigned int d_addr_uint){
	struct iphdr* iph = (struct iphdr*) datagram;
	iph->version = 4;
	iph->ihl = 5;
	iph->tos = 0;
	iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr);
	iph->id = htonl(54321);
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_UDP;
	iph->check = 0;
	iph->saddr = inet_addr(src_addr_char);
	iph->daddr = d_addr_uint;
	iph->check = csum ((unsigned short *) datagram, iph->tot_len);
	return iph;
}

// struct tcphdr* craft_tcp_header(char* packet, int port_no){
// 	struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof (struct ip));
// 	tcph->source = htons (8080);
//     tcph->dest = htons (port_no);
//     tcph->seq = 0;
//     tcph->ack_seq = 0;
//     tcph->doff = 5;
// 	tcph->fin=1;
// 	tcph->syn=0;
// 	tcph->rst=0;
// 	tcph->psh=0;
// 	tcph->ack=0;
// 	tcph->urg=0;
//     tcph->window = htons (5840);
//     tcph->check = 0;
//     tcph->urg_ptr = 0;
// 	return tcph;

// }

struct udphdr* craft_udp_header(char* datagram, int port_no){
    struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct ip));
    udph->source = htons(8080);
    udph->dest = htons(port_no);
    udph->len = htons(8);
    udph->check = 0;
    return udph;
}

struct pseudo_header craft_pseudo_header(char* src_addr_char, unsigned int d_addr_uint){
	struct pseudo_header psh;
	psh.source_address = inet_addr(src_addr_char);
	psh.dest_address = d_addr_uint;
    // psh.dest_address = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons( sizeof( struct udphdr ) );
	return psh;
}

bool send_udp(char* dst_addr_char, int port_no){

	int s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
    int s2 = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(s == -1){
		throw_error("socket create","UDP");
	}
	char datagram[4096];
	struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8080);
    sin.sin_addr.s_addr = inet_addr (dst_addr_char);

	memset(datagram, 0, 4096);
	char src_addr_char[16];
	sprintf(src_addr_char, "%d.%d.%d.%d\n", 192,168,60,56);
	struct iphdr *iph = craft_ip_header(datagram, src_addr_char, sin.sin_addr.s_addr);
	struct udphdr *udph = craft_udp_header(datagram, port_no);
	struct pseudo_header psh = craft_pseudo_header(src_addr_char,sin.sin_addr.s_addr);	

    memcpy(&psh.udp , udph , sizeof (struct udphdr));
    udph->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));

	int one = 1;
    const int *val = &one;
    int sockop = setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one));
    if (sockop < 0){
        throw_error("access", "root access required");
    }
    if(sendto (s, datagram, iph->tot_len, 0, (struct sockaddr *) &sin, sizeof (sin)) < 0){
        throw_error("sending", "sending packet");
	}
	struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    if (setsockopt (s2, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0){
        throw_error("options", "setsockopt failed");
	}
	char recvPacket[4096] = "";
	// struct sockaddr_in *sin2;
	// struct sockaddr temp;
	// socklen_t llen;
	int timeout_flag = 0;
	time_t start, finish;
	time(&start);
	while(true){
		int newData = recv(s2,recvPacket,sizeof(recvPacket),0);
		if(newData == -1 && (errno== EAGAIN || errno==EWOULDBLOCK)){
			timeout_flag = 1;
			break;
		}
		time(&finish);
		// if(difftime(finish, start) >= 5){
		// 	timeout_flag = 1;
		// 	break;
		// }
		struct iphdr *rec_iph = (struct iphdr *) recvPacket;
		struct icmphdr* rec_icmp = (struct icmphdr *) (recvPacket + sizeof (struct ip));
        // cout << port_no<< "\t:\t"<< ntohs(rec_icmp->type) << " " << ntohs(rec_icmp->code) << endl;        
		if(rec_iph->saddr = inet_addr(src_addr_char)){
			cout << port_no<< "\t:\t"<< "ICMP ERROR" << endl;
			break;
		}
		else{
			continue;
		}
	}
	if(timeout_flag == 1){
		cout << port_no<< "\t:\t"<< "TIMEOUT\t";
		cout << "\tUDP OUT" << endl;
	}
	close(s);
	return true;
}

int main(int argc, char* argv[]){
	string dst_addr = "192.168.2.166";
	for (int i =7000;i <=7100; i++){
		send_udp((char *)dst_addr.c_str(), i);
        sleep(1);
	}
	return 0;
}