/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  network.c                                               *
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "network.h"

int norestart = 0;
int nospeedup = 0;

#define DEFAULTPORT 19503

#define PROT_VERS_1 1
#define PROT_VERS_2 0
#define PROT_VERS_3 1

#define OP_NICK 1
#define OP_PLAY 2
#define OP_FALL 3
#define OP_DRAW 4
#define OP_LOST 5
#define OP_GONE 6
#define OP_CLEAR 7
#define OP_NEW 8
#define OP_LINES 9
#define OP_GROW 10
#define OP_MODE 11
#define OP_LEVEL 12
#define OP_BOT 13
#define OP_KILL 14
#define OP_PAUSE 15
#define OP_CONT 16
#define OP_VERSION 17
#define OP_BADVERS 18
#define OP_MSG 19
#define OP_YOUARE 20
#define OP_LINESTO 21
#define OP_WON 22
#define OP_ZERO 23

/* server stuff */

#define BUFLEN		4096

int sfd;
unsigned char realbuf[512], readbuf[BUFLEN], writbuf[BUFLEN];
unsigned char *buf = realbuf + 4;
int nread = 0, nwrite = 0;

/* like memcpy, but guaranteed to handle overlap when s <= t */
void copydown(char *s, char *t, int n)
{
  for (; n; n--)
    *(s++) = *(t++);
}

void sysmsg(char *s)
{
  printf("** %s\n", s);
  fflush(stdout);
}

void fatal(char *s)
{
  fprintf(stderr, "%s.\n", s);
  exit(1);
}

void u_sleep(int i)
{
  struct timeval tm;
  tm.tv_sec = i / 1000000;
  tm.tv_usec = i % 1000000;
  select(0, NULL, NULL, NULL, &tm);
}

void flushbuf()
{
  if (nwrite)
  {
    write(sfd, writbuf, nwrite);
    nwrite = 0;
  }
}

void sendbuf(int len)
{
  if (!standalone)
  {
    realbuf[0] = realbuf[1] = realbuf[2] = 0;
    realbuf[3] = (unsigned char)len;
    buf[0] = 0;
    if (nwrite + 4 + len >= BUFLEN)
      fatal("Internal error: send buffer overflow");
    memcpy(writbuf + nwrite, realbuf, 4 + len);
    nwrite += 4 + len;
  }
}

void startserver()
{
  char *options[2];
  int n = 0;

  options[0] = options[1] = NULL;
  if (norestart)
    options[n++] = "-norestart";
  if (nospeedup)
    options[n++] = "-nospeedup";

  switch (fork())
  {
    case 0:
      execlp(
#ifdef XTRISPATH
      XTRISPATH "/rnd_server",
#else
      "rnd_server",
#endif
      "rnd_server", "-once", "-v", options[0], options[1], NULL);

      fprintf(stderr, "Can't start server '%s'.\n",
#ifdef XTRISPATH
	XTRISPATH "/rnd_server"
#else
	"rnd_server"
#endif
	      );

      _exit(1);
    
    case -1:
      fatal("fork() failed");
    
    default:
      return;
  }
}

BOOL ConnectToServer(char *host, int port)
{
  struct hostent *hp;
  struct sockaddr_in s;
  struct protoent *tcpproto;
  int on = 1, i;

  if (host)
  {
    if ((s.sin_addr.s_addr = inet_addr(host)) == -1)
    {
      hp = gethostbyname(host);
      if (!hp)
	fatal("Host not found");
      s.sin_addr = *(struct in_addr *)(hp->h_addr_list[0]);
    }
  }
  else
    s.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (port == 0)
    port = DEFAULTPORT;

  s.sin_port = htons(port);
  s.sin_family = AF_INET;
  sfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sfd < 0)
    fatal("Out of file descriptors");
  if ((tcpproto = getprotobyname("tcp")) != NULL)
    setsockopt(sfd, tcpproto->p_proto, TCP_NODELAY, (char *)&on, sizeof(int));

  if (connect(sfd, (struct sockaddr *)&s, sizeof(s)) < 0)
  {
    if (!host)
    {
      printf("No rocksndiamonds server on localhost - starting up one ...\n");
      startserver();
      for (i=0; i<6; i++)
      {
	u_sleep(500000);
	close(sfd);
	sfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	  fatal("Out of file descriptors");
	setsockopt(sfd, tcpproto->p_proto, TCP_NODELAY, (char *)&on, sizeof(int));
	if (connect(sfd, (struct sockaddr *)&s, sizeof(s)) >= 0)
	  break;
      }
      if (i==6)
	fatal("Can't connect to server");
    }
    else
      fatal("Can't connect to server");
  }

  return(TRUE);
}

void SendNicknameToServer(char *nickname)
{
  static char msgbuf[300];

  buf[1] = OP_NICK;
  memcpy(&buf[2], nickname, strlen(nickname));
  sendbuf(2 + strlen(nickname));
  sprintf(msgbuf, "you set your nick to \"%s\"", nickname);
  sysmsg(msgbuf);
}

void SendProtocolVersionToServer()
{
  buf[1] = OP_VERSION;
  buf[2] = PROT_VERS_1;
  buf[3] = PROT_VERS_2;
  buf[4] = PROT_VERS_3;
  sendbuf(5);
}

void handlemessages()
{
  unsigned int len;
  static char msgbuf[300];

  while (nread >= 4 && nread >= 4 + readbuf[3])
  {
    len = readbuf[3];
    if (readbuf[0] || readbuf[1] || readbuf[2])
      fatal("Wrong server line length");

    memcpy(buf, &readbuf[4], len);
    nread -= 4 + len;
    copydown(readbuf, readbuf + 4 + len, nread);

    switch(buf[1])
    {
      case OP_YOUARE:
	printf("OP_YOUARE: %d\n", buf[0]);
	break;

      case OP_NEW:
	printf("OP_NEW: %d\n", buf[0]);
	sprintf(msgbuf, "new client %d connected", buf[0]);
	sysmsg(msgbuf);
	break;
      
      case OP_GONE:
	printf("OP_GONE: %d\n", buf[0]);
	sprintf(msgbuf, "client %d disconnected", buf[0]);
	sysmsg(msgbuf);
	break;

      case OP_BADVERS:
	{
	  static char tmpbuf[128];

	  sprintf(tmpbuf, "Protocol version mismatch: server expects %d.%d.x instead of %d.%d.%d\n", buf[2], buf[3], PROT_VERS_1, PROT_VERS_2, PROT_VERS_3);
	  fatal(tmpbuf);
	}
	break;
      
      case OP_PLAY:
	printf("OP_PLAY: %d\n", buf[0]);
	sprintf(msgbuf, "client %d starts game", buf[0]);
	sysmsg(msgbuf);
	break;

      case OP_PAUSE:
	printf("OP_PAUSE: %d\n", buf[0]);
	sprintf(msgbuf, "client %d pauses game", buf[0]);
	sysmsg(msgbuf);
	break;

      case OP_CONT:
	printf("OP_CONT: %d\n", buf[0]);
	sprintf(msgbuf, "client %d continues game", buf[0]);
	sysmsg(msgbuf);
	break;

      case OP_WON:
	printf("OP_WON: %d\n", buf[0]);
	sprintf(msgbuf, "client %d wins the game", buf[0]);
	sysmsg(msgbuf);
	break;

      case OP_ZERO:
	printf("OP_ZERO: %d\n", buf[0]);
	sprintf(msgbuf, "client %d resets game counters", buf[0]);
	sysmsg(msgbuf);
	break;

      case OP_NICK:
	printf("OP_NICK: %d\n", buf[0]);
        sprintf(msgbuf, "client %d calls itself \"%s\"", buf[0], &buf[2]);
        sysmsg(msgbuf);
	break;

      case OP_MSG:
	printf("OP_MSG: %d\n", buf[0]);
	sprintf(msgbuf, "client %d sends message", buf[0]);
	break;
      
      case OP_LOST:
	printf("OP_MSG: %d\n", buf[0]);
	sprintf(msgbuf, "client %d has lost", buf[0]);
	break;
      
      case OP_LEVEL:
	printf("OP_MSG: %d\n", buf[0]);
	sprintf(msgbuf, "client %d sets level to %d", buf[0], buf[2]);
	break;
    }
  }

  fflush(stdout);
}

void HandleNetworking()
{
  static struct timeval tv = { 0, 0 };
  fd_set rfds;
  int r = 0;

  if (standalone)
    return;

  flushbuf();

  FD_ZERO(&rfds);
  FD_SET(sfd, &rfds);

  r = select(sfd + 1, &rfds, NULL, NULL, &tv);

  if (r < 0 && errno != EINTR)
  {
    perror("select");
    fatal("fatal: select() failed");
  }

  if (r < 0)
    FD_ZERO(&rfds);

  if (FD_ISSET(sfd, &rfds))
  {
    int r;

    r = read(sfd, readbuf + nread, BUFLEN - nread);

    if (r < 0)
      fatal("Error reading from server");
    if (r == 0)
      fatal("Connection to server lost");
    nread += r;

    handlemessages();
  }
}
