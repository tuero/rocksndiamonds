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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "netserv.h"
#include "misc.h"

static int clients = 0;
static int onceonly = 0;
static int is_daemon = 0;

struct user
{
  int fd;
  unsigned char nick[16];
  unsigned char number;
  struct user *next, *nextvictim;
  char active;
  char introduced;
  unsigned char readbuf[MAX_BUFFER_SIZE];
  int nread;
  unsigned char writbuf[MAX_BUFFER_SIZE];
  int nwrite;
  char playing;
  int lines;
  unsigned int games;
  unsigned char action;
  int action_received;
};

static struct user *user0 = NULL;

#define NEXT(u) ((u)->next ? (u)->next : user0)

static struct sockaddr_in saddr;
static int lfd;
static unsigned char realbuf[512], *buf = realbuf + 4;

static int interrupt;
static int tcp = -1;

static unsigned long frame_counter = 0;

static fd_set fds;

static void syserr(char *s)
{
  if (!is_daemon)
    fprintf(stderr, "fatal: %s failed.\n", s);
  exit(1);
}

static void addtobuffer(struct user *u, unsigned char *b, int len)
{
  if (u->nwrite + len >= MAX_BUFFER_SIZE)
    Error(ERR_EXIT, "internal error: network send buffer overflow");

  memcpy(u->writbuf + u->nwrite, b, len);
  u->nwrite += len;
}

static void flushuser(struct user *u)
{
  if (u->nwrite)
  {
    write(u->fd, u->writbuf, u->nwrite);
    u->nwrite = 0;
  }
}

static void broadcast(struct user *except, int len, int activeonly)
{
  struct user *u;

  realbuf[0] = realbuf[1] = realbuf[2] = 0;
  realbuf[3] = (unsigned char)len;
  for (u=user0; u; u=u->next)
    if (u != except && (u->active || !activeonly) && u->introduced)
      addtobuffer(u, realbuf, 4 + len);
}

static void sendtoone(struct user *to, int len)
{
  realbuf[0] = realbuf[1] = realbuf[2] = 0;
  realbuf[3] = (unsigned char)len;
  addtobuffer(to, realbuf, 4 + len);
}

static void dropuser(struct user *u)
{
  struct user *v, *w;
  
  if (options.verbose)
    printf("RND_SERVER: dropping client %d (%s)\n", u->number, u->nick);

  if (u == user0)
    user0 = u->next;
  else
  {
    for (v=user0; v; v=v->next)
    {
      if (v->next && v->next == u)
      {
	v->next = u->next;
	break;
      }
    }
  }
  close(u->fd);

  if (u->introduced)
  {
    buf[0] = u->number;
    buf[1] = OP_PLAYER_DISCONNECTED;
    broadcast(u, 2, 0);
  }

  for (v=user0; v; v=v->next)
  {
    if (v->nextvictim == u)
    {
      for (w=NEXT(v); w!=v; w=NEXT(w))
      {
	if (w->active && w->playing)
	{
	  v->nextvictim = w;
	  break;
	}
      }
      if (v->nextvictim == u)
	v->nextvictim = NULL;
    }
  }

  free(u);
  clients--;

  if (onceonly && clients == 0)
  {
    if (options.verbose)
    {
      printf("RND_SERVER: no clients left\n");
      printf("RND_SERVER: aborting\n");
    }
    exit(0);
  }
}

static void new_connect(int fd)
{
  struct user *u, *v;
  unsigned char nxn;

  u = checked_malloc(sizeof (struct user));

  u->fd = fd;
  u->nick[0] = 0;
  u->next = user0;
  u->nextvictim = NULL;
  u->active = 0;
  u->nread = 0;
  u->nwrite = 0;
  u->playing = 0;
  u->introduced = 0;
  u->games = 0;
  u->action = 0;
  u->action_received = 0;

  user0 = u;

  nxn = 1;

 again:
  v = u->next;
  while(v)
  {
    if (v->number == nxn)
    {
      nxn++;
      goto again;
    }
    v = v->next;
  }

  u->number = nxn;
  if (options.verbose)
    printf("RND_SERVER: client %d connecting from %s\n", nxn, inet_ntoa(saddr.sin_addr));
  clients++;

  buf[0] = 0;
  buf[1] = OP_YOUR_NUMBER;
  buf[2] = u->number;
  sendtoone(u, 3);
}

static void Handle_OP_PROTOCOL_VERSION(struct user *u, unsigned int len)
{
  if (len != 5 || buf[2] != PROTOCOL_VERSION_1 || buf[3] != PROTOCOL_VERSION_2)
  {
    if (options.verbose)
      printf("RND_SERVER: client %d (%s) has wrong protocol version %d.%d.%d\n", u->number, u->nick, buf[2], buf[3], buf[4]);

    buf[0] = 0;
    buf[1] = OP_BADVERS;
    buf[2] = PROTOCOL_VERSION_1;
    buf[3] = PROTOCOL_VERSION_2;
    buf[4] = PROTOCOL_VERSION_3;
    sendtoone(u, 5);
    flushuser(u);

    dropuser(u);
    interrupt = 1;
  }
  else
  {
    if (options.verbose)
      printf("RND_SERVER: client %d (%s) uses protocol version %d.%d.%d\n", u->number, u->nick, buf[2], buf[3], buf[4]);
  }
}

static void Handle_OP_NUMBER_WANTED(struct user *u)
{
  struct user *v;
  int client_nr = u->number;
  int nr_wanted = buf[2];
  int nr_is_free = 1;

  if (options.verbose)
    printf("RND_SERVER: client %d (%s) wants to switch to # %d\n",
	   u->number, u->nick, nr_wanted);

  for (v=user0; v; v=v->next)
  {
    if (v->number == nr_wanted)
    {
      nr_is_free = 0;
      break;
    }
  }

  if (options.verbose)
  {
    if (nr_is_free)
      printf("RND_SERVER: client %d (%s) switches to # %d\n",
	     u->number, u->nick, nr_wanted);
    else if (u->number == nr_wanted)
      printf("RND_SERVER: client %d (%s) still has # %d\n",
	     u->number, u->nick, nr_wanted);
    else
      printf("RND_SERVER: client %d (%s) cannot switch (client %d still exists)\n",
	     u->number, u->nick, nr_wanted);
  }

  if (nr_is_free)
    u->number = nr_wanted;

  buf[0] = client_nr;
  buf[1] = OP_NUMBER_WANTED;
  buf[2] = nr_wanted;
  buf[3] = u->number;

  /*
  sendtoone(u, 4);
  */

  broadcast(NULL, 4, 0);
}

static void Handle_OP_NICKNAME(struct user *u, unsigned int len)
{
  struct user *v;
  int i;

  if (len>16)
    len=16;
  memcpy(u->nick, &buf[2], len-2);
  u->nick[len-2] = 0;
  for (i=0; i<len-2; i++)
  {
    if (u->nick[i] < ' ' || 
	(u->nick[i] > 0x7e && u->nick[i] <= 0xa0))
    {
      u->nick[i] = 0;
      break;
    }
  }

  if (!u->introduced)
  {
    buf[0] = u->number;
    buf[1] = OP_PLAYER_CONNECTED;
    broadcast(u, 2, 0);
  }
	      
  if (options.verbose)
    printf("RND_SERVER: client %d calls itself \"%s\"\n", u->number, u->nick);
  buf[1] = OP_NICKNAME;
  broadcast(u, len, 0);

  if (!u->introduced)
  {
    for (v=user0; v; v=v->next)
    {
      if (v != u && v->introduced)
      {
	buf[0] = v->number;
	buf[1] = OP_PLAYER_CONNECTED;
	buf[2] = (v->games >> 8);
	buf[3] = (v->games & 0xff);
	sendtoone(u, 4);
	buf[1] = OP_NICKNAME;
	memcpy(&buf[2], v->nick, 14);
	sendtoone(u, 2+strlen(v->nick));
      }
    }
  }

  u->introduced = 1;
}

static void Handle_OP_START_PLAYING(struct user *u)
{
  struct user *v, *w;

  if (options.verbose)
    printf("RND_SERVER: client %d (%s) starts game [level %d from levedir %d (%s)]\n",
	   u->number, u->nick,
	   (buf[2] << 8) + buf[3],
	   (buf[4] << 8) + buf[5],
	   &buf[6]);

  for (w=user0; w; w=w->next)
  {
    if (w->introduced)
    {
      w->active = 1;
      w->playing = 1;
      w->lines = 0;
      w->nextvictim = NULL;
      for (v=NEXT(w); v!=w; v=NEXT(v))
      {
	if (v->introduced)
	{
	  w->nextvictim = v;
	  break;
	}
      }
    }
  }

  /* reset frame counter */
  frame_counter = 0;

  /* reset player actions */
  for (v=user0; v; v=v->next)
  {
    v->action = 0;
    v->action_received = 0;
  }

  broadcast(NULL, 10 + strlen(&buf[10])+1, 0);
}

static void Handle_OP_PAUSE_PLAYING(struct user *u)
{
  if (options.verbose)
    printf("RND_SERVER: client %d (%s) pauses game\n", u->number, u->nick);
  broadcast(NULL, 2, 0);
}

static void Handle_OP_CONTINUE_PLAYING(struct user *u)
{
  if (options.verbose)
    printf("RND_SERVER: client %d (%s) continues game\n", u->number, u->nick);
  broadcast(NULL, 2, 0);
}

static void Handle_OP_STOP_PLAYING(struct user *u)
{
  if (options.verbose)
    printf("RND_SERVER: client %d (%s) stops game\n", u->number, u->nick);
  broadcast(NULL, 2, 0);
}

static void Handle_OP_MOVE_FIGURE(struct user *u)
{
  struct user *v;
  int last_client_nr = 0;
  int i;

  /* store player action */
  for (v=user0; v; v=v->next)
  {
    if (v->number == u->number)
    {
      v->action = buf[2];
      v->action_received = 1;
    }
  }

  /* check if server received action from each player */
  for (v=user0; v; v=v->next)
  {
    if (!v->action_received)
      return;

    if (v->number > last_client_nr)
      last_client_nr = v->number;
  }

  /* initialize all player actions to zero */
  for (i=0; i<last_client_nr; i++)
    buf[6 + i] = 0;

  /* broadcast actions of all players to all players */
  for (v=user0; v; v=v->next)
  {
    buf[6 + v->number-1] = v->action;
    v->action = 0;
    v->action_received = 0;
  }

  buf[2] = (unsigned char)((frame_counter >> 24) & 0xff);
  buf[3] = (unsigned char)((frame_counter >> 16) & 0xff);
  buf[4] = (unsigned char)((frame_counter >>  8) & 0xff);
  buf[5] = (unsigned char)((frame_counter >>  0) & 0xff);

  broadcast(NULL, 6 + last_client_nr, 0);

  frame_counter++;

  /*
    if (verbose)
    printf("RND_SERVER: frame %d: client %d (%s) moves player [0x%02x]\n",
    frame_counter,
    u->number, u->nick, buf[2]);
  */
}

void NetworkServer(int port, int serveronly)
{
  int i, sl, on;
  struct user *u;
  int mfd;
  int r; 
  unsigned int len;
  struct protoent *tcpproto;
  struct timeval tv;
  int is_daemon = 0;

#ifndef NeXT
  struct sigaction sact;
#endif

  if (port == 0)
    port = DEFAULTPORT;

  if (!serveronly)
    onceonly = 1;

  if ((tcpproto = getprotobyname("tcp")) != NULL)
    tcp = tcpproto->p_proto;

#ifdef NeXT
  signal(SIGPIPE, SIG_IGN);
#else
  sact.sa_handler = SIG_IGN;
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
  sigaction(SIGPIPE, &sact, NULL);
#endif


  lfd = socket(PF_INET, SOCK_STREAM, 0);
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons(port);

  if (lfd < 0)
    syserr("socket");
  on = 1;

  setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
  if (bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
    syserr("bind");

  listen(lfd, 5);

  if (is_daemon)
  {
    /* become a daemon, breaking all ties with the controlling terminal */
    options.verbose = 0;
    for (i=0; i<255; i++)
    {
      if (i != lfd)
	close(i);
    }

    if (fork())
      exit(0);
    setsid();
    if (fork())
      exit(0);
    chdir("/");

    /* open a fake stdin, stdout, stderr, just in case */
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
  }

  if (options.verbose)
  {
    printf("rocksndiamonds network server: started up, listening on port %d\n",
	   port);
    printf("rocksndiamonds network server: using protocol version %d.%d.%d\n",
	   PROTOCOL_VERSION_1, PROTOCOL_VERSION_2, PROTOCOL_VERSION_3);
  }

  while(1)
  {
    interrupt = 0;

    for (u=user0; u; u=u->next)
      flushuser(u);

    FD_ZERO(&fds);
    mfd = lfd;
    u = user0;
    while (u)
    {
      FD_SET(u->fd, &fds);
      if (u->fd > mfd)
	mfd = u->fd;
      u = u->next;
    }
    FD_SET(lfd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    if ((sl = select(mfd + 1, &fds, NULL, NULL, &tv)) < 0)
    {
      if (errno != EINTR)
	syserr("select");
      else
	continue;
    }

    if (sl < 0)
      continue;
    
    if (sl == 0)
      continue;

    if (FD_ISSET(lfd, &fds))
    {
      int newfd, slen;

      slen = sizeof(saddr);
      newfd = accept(lfd, (struct sockaddr *)&saddr, &slen);
      if (newfd < 0)
      {
	if (errno != EINTR)
	  syserr("accept");
      }
      else
      {
	if (tcp != -1)
	{
	  on = 1;
	  setsockopt(newfd, tcp, TCP_NODELAY, (char *)&on, sizeof(int));
	}
	new_connect(newfd);
      }
      continue;
    }

    u = user0;

    do
    {
      if (FD_ISSET(u->fd, &fds))
      {
	r = read(u->fd, u->readbuf + u->nread, MAX_BUFFER_SIZE - u->nread);
	if (r <= 0)
	{
	  if (options.verbose)
	    printf("RND_SERVER: EOF from client %d (%s)\n", u->number, u->nick);
	  dropuser(u);
	  interrupt = 1;
	  break;
	}
	u->nread += r;
	while (u->nread >= 4 && u->nread >= 4 + u->readbuf[3])
	{
	  len = u->readbuf[3];
	  if (u->readbuf[0] || u->readbuf[1] || u->readbuf[2])
	  {
	    if (options.verbose)
	      printf("RND_SERVER: crap from client %d (%s)\n", u->number, u->nick);
	    write(u->fd, "\033]50;kanji24\007\033#8\033(0", 19);
	    dropuser(u);
	    interrupt = 1;
	    break;
	  }
	  memcpy(buf, &u->readbuf[4], len);
	  u->nread -= 4 + len;
	  memmove(u->readbuf, u->readbuf + 4 + len, u->nread);

	  buf[0] = u->number;
	  if (!u->introduced && buf[1] != OP_NICKNAME)
	  {
	    if (options.verbose)
	      printf("RND_SERVER: !(client %d)->introduced && buf[1]==%d (expected OP_NICKNAME)\n", buf[0], buf[1]);

	    dropuser(u);
	    interrupt = 1;
	    break;
	  }

	  switch(buf[1])
	  {
	    case OP_NICKNAME:
	      Handle_OP_NICKNAME(u, len);
	      break;

	    case OP_PROTOCOL_VERSION:
	      Handle_OP_PROTOCOL_VERSION(u, len);
	      break;

	    case OP_NUMBER_WANTED:
	      Handle_OP_NUMBER_WANTED(u);
	      break;

	    case OP_START_PLAYING:
	      Handle_OP_START_PLAYING(u);
	      break;

	    case OP_PAUSE_PLAYING:
	      Handle_OP_PAUSE_PLAYING(u);
	      break;

	    case OP_CONTINUE_PLAYING:
	      Handle_OP_CONTINUE_PLAYING(u);
	      break;

	    case OP_STOP_PLAYING:
	      Handle_OP_STOP_PLAYING(u);
	      break;

	    case OP_MOVE_FIGURE:
	      Handle_OP_MOVE_FIGURE(u);
	      break;

	    case OP_MSG:
	      buf[len] = '\0';
	      if (options.verbose)
		printf("RND_SERVER: client %d (%s) sends message: %s\n", u->number, u->nick, &buf[2]);
	      broadcast(u, len, 0);
	      break;
	    
	    default:
	      if (options.verbose)
		printf("RND_SERVER: opcode %d from client %d (%s) not understood\n", buf[0], u->number, u->nick);
	  }
	}
      }

      if (u && !interrupt)
	u = u->next;
    }
    while (u && !interrupt);
  }
}
