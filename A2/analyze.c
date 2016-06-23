#include <stdio.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

#include "headers.h"



void print_connections();
void print_general();
void print_complete();

void parse_packet(const unsigned char *packet, struct timeval ts, unsigned int capture_len);
void check_connection(struct ip *ip, struct TCP_hdr *tcp, struct timeval ts, const char *payload);

struct connection connections[MAX_NUM_CONNECTION];
int total_connections = 0;

int main(int argc, char **argv)
{
  unsigned int packet_counter=0;
  struct pcap_pkthdr header;
  const u_char *packet;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <pcap>\n", argv[0]);
    exit(1);
  }

   pcap_t *handle;
   char errbuf[PCAP_ERRBUF_SIZE];
   handle = pcap_open_offline(argv[1], errbuf);

   if (handle == NULL) {
     fprintf(stderr,"Couldn't open pcap file %s: %s\n", argv[1], errbuf);
     return(2);
   }

   total_connections = 0;
   while (packet = pcap_next(handle,&header)) {
     parse_packet(packet,header.ts,header.caplen);
      packet_counter++;

    }
    pcap_close(handle);


  printf("\nA) Total number of connections: %d\n", total_connections);
  printf("___________________________\n");
  //print_connections();
  //print_general();
  //print_complete();
  return 0;
}

void print_connections(){
  printf("\nB) Connections' details\n\n");
  //while still connection info to print
  printf("Connection :\nSource Address:\nDestination Address:\n,Source Port:\n");
  printf("Destinantion Port:\n");
  printf("Status:\n");
  //if complete printf
  printf("Start Time:\n");
  printf("EndTime:\n");
  printf("Duration:\n");
  printf("Number of packets sent from Source to Destination: \n");
  printf("Number of Packets sent from Destination to Source: \n");
  printf("Total number of packets: \n");
  printf("Number of data bytes sent from Source to Destination: \n");
  printf("Number of data bytes sent from Destination to Source: \n");
  printf("Total number of data bytes: \n");
  //end of connection
  printf("END\n++++++++++++++++++++++++++++++++++++++\n");
}

void print_general(){
  printf("C) General\n\n");
  printf("Total number of complete TCP connections: \n");
  printf("Number of reset TCP connections: \n");
  printf("Number of TCP connections that were still open when the trace capture ended\n");
  printf("\n______________________________________________\n");
}

void print_complete(){
  printf("D) Complete TCP connections\n");
  printf("Minimum time durations: \n");
  printf("Mean time durations: \n");
  printf("Maximum time durations: \n\n");
  printf("Minimum RTT values including both send/received:\n");
  printf("Mean RTT values including both send/received: \n");
  printf("Maximum RTT values including both send/received: \n\n");
  printf("Minimum number of packets including both send/received:\n");
  printf("Mean number of packets including both send/received:\n");
  printf("Maximum number of packets including both send/received:\n\n");
  printf("Minimum received window size including both send/received:\n");
  printf("Mean received window size including both send/received:\n");
  printf("Maximum received window size including both send/received:\n\n");
  printf("_____________________________________________\n");
}

void parse_packet(const unsigned char *packet, struct timeval ts, unsigned int capture_len){
  struct ip *ip; //ip header
  struct TCP_hdr *tcp; //tcp header
  unsigned int IP_header_length; //ip header length
  const char *payload;

  //Skip over Ethernet header
  packet += sizeof(struct ether_header);
  capture_len -= sizeof(struct ether_header);

  if (capture_len < sizeof(struct ip)){
    printf("IP header too short");
    return;
  }

  //get ip header size and ip header
  ip = (struct ip*) packet;
  IP_header_length = ip->ip_hl * 4;

  //check ip header size
  if (capture_len < IP_header_length){
    printf("IP header with options too short");
    return;
  }

  //get to the TCP header
  packet += IP_header_length;
  capture_len -= IP_header_length;

  tcp = (struct TCP_hdr*) packet;

  if (capture_len < sizeof(struct TCP_hdr)){
    printf("TCP Header too short");
    return;
  }

  //get the payload
  packet += TH_OFF(tcp)*4;
  payload = (u_char *)packet;


  check_connection(ip,tcp,ts,payload);
  //char *addr = inet_ntoa(ip->ip_src);
  //printf("src addr=%s dst addr = %s,src_port=%d dst_port=%d\n",addr,inet_ntoa(ip->ip_dst),ntohs(tcp->th_sport),ntohs(tcp->th_dport));

}

void check_connection(struct ip *ip, struct TCP_hdr *tcp, struct timeval ts, const char *payload){
  int i = 0;
  int match = 0;
  if (total_connections == 0){
    strcpy(connections[total_connections].ip_src, inet_ntoa(ip->ip_src));
    strcpy(connections[total_connections].ip_dst, inet_ntoa(ip->ip_dst));
    connections[total_connections].port_src = ntohs(tcp->th_sport);
    connections[total_connections].port_dst = ntohs(tcp->th_dport);
    connections[total_connections].is_set = 1;
    //TODO add the rest of fields
    total_connections++;
    return;
  }
  for (; i <= total_connections; i++){
    if ((connections[i].port_src == ntohs(tcp->th_sport) && connections[i].port_dst == ntohs(tcp->th_dport)
    && !strcmp(connections[i].ip_dst,inet_ntoa(ip->ip_dst)) && !strcmp(connections[i].ip_src,inet_ntoa(ip->ip_src))) ||
    (connections[i].port_src == ntohs(tcp->th_dport) && connections[i].port_dst == ntohs(tcp->th_sport)
    && !strcmp(connections[i].ip_dst,inet_ntoa(ip->ip_src)) && !strcmp(connections[i].ip_src,inet_ntoa(ip->ip_dst)))){
      match = 1;
      break;
    }
  }
  if (match == 0){
  //no match
  total_connections++;
  strcpy(connections[total_connections].ip_src, inet_ntoa(ip->ip_src));
  strcpy(connections[total_connections].ip_dst, inet_ntoa(ip->ip_dst));
  connections[total_connections].port_src = ntohs(tcp->th_sport);
  connections[total_connections].port_dst = ntohs(tcp->th_dport);
  connections[total_connections].is_set = 1;
}




}
