/*
 * ip_test.c
 *
 *  Created on: 05/06/2013
 *      Author: utnso
 */

#include <stdio.h>
#include <stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

char* print_addresses(int index)
{
  int s;
  struct ifconf ifconf;
  struct ifreq ifr[50];
  char* ipLocal=malloc(16*sizeof(char));
  int ifs;
  int i;

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("socket");
    return 0;
  }

  ifconf.ifc_buf = (char *) ifr;
  ifconf.ifc_len = sizeof ifr;

  if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
    perror("ioctl");
    return 0;
  }

  ifs = ifconf.ifc_len / sizeof(ifr[0]);
  //printf("interfaces = %d:\n", ifs);
  for (i = 0; i < ifs; i++) {
    char ip[INET_ADDRSTRLEN];
    struct sockaddr_in *s_in = (struct sockaddr_in *) &ifr[i].ifr_addr;

    if (!inet_ntop(AF_INET, &s_in->sin_addr, ip, sizeof(ip))) {
      perror("inet_ntop");
      return 0;
    }

    if (i==index){
    strcpy(ipLocal, ip);
    }

    //printf("%s - %s\n", ifr[i].ifr_name, ip);
  }

  //printf("Mi ip es: %s\n", ipLocal);
  close(s);

  return ipLocal;
}
