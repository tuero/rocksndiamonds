/*
 *   A server for a multi-player version of Tetris
 *
 *   Copyright (C) 1996 Roger Espel Llima <roger.espel.llima@pobox.com>
 *
 *   Started: 10 Oct 1996
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation. See the file COPYING for details.
 *
 */

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



extern int verbose;

extern void copydown(char *, char *, int);
extern void fatal(char *);



static int clients = 0;
static int bots = 0;
static int onceonly = 0;
static int timetoplay = 0;
static int is_daemon = 0;
static int level = 5;
static int mode = -1;
static int paused = 0;

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
  char isbot;
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

void syserr(char *s)
{
  if (!is_daemon)
    fprintf(stderr, "fatal: %s failed.\n", s);
  exit(1);
}

void addtobuffer(struct user *u, unsigned char *b, int len)
{
  if (u->nwrite + len >= MAX_BUFFER_SIZE)
    fatal("Internal error: send buffer overflow");
  memcpy(u->writbuf + u->nwrite, b, len);
  u->nwrite += len;
}

void flushuser(struct user *u)
{
  if (u->nwrite)
  {
    write(u->fd, u->writbuf, u->nwrite);
    u->nwrite = 0;
  }
}

void broadcast(struct user *except, int len, int activeonly)
{
  struct user *u;

  realbuf[0] = realbuf[1] = realbuf[2] = 0;
  realbuf[3] = (unsigned char)len;
  for (u=user0; u; u=u->next)
    if (u != except && (u->active || !activeonly) && u->introduced)
      addtobuffer(u, realbuf, 4 + len);
}

void sendtoone(struct user *to, int len)
{
  realbuf[0] = realbuf[1] = realbuf[2] = 0;
  realbuf[3] = (unsigned char)len;
  addtobuffer(to, realbuf, 4 + len);
}

void dropuser(struct user *u)
{
  struct user *v, *w;
  
  if (verbose)
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

  if (u->isbot)
    bots--;

  free(u);
  clients--;

  if (onceonly && clients == bots)
  {
    if (verbose)
    {
      printf("RND_SERVER: no clients left\n");
      printf("RND_SERVER: aborting\n");
    }
    exit(0);
  }

  if (clients == 0)
  {
    mode = -1;
    level = 5;
    timetoplay = 0;
  }
}

void do_play()
{
  struct user *v, *w;

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

  /*
  if (paused)
  {
    paused = 0;
    buf[1] = OP_CONT;
    broadcast(NULL, 2, 0);
  }
  buf[1] = OP_START_PLAYING;
  broadcast(NULL, 2, 0);
  */

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

void new_connect(int fd)
{
  struct user *u, *v;
  unsigned char nxn;

  u = malloc(sizeof (struct user));
  if (!u)
    fatal("Out of memory");
  u->fd = fd;
  u->nick[0] = 0;
  u->next = user0;
  u->nextvictim = NULL;
  u->active = 0;
  u->nread = 0;
  u->nwrite = 0;
  u->playing = 0;
  u->isbot = 0;
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
  if (verbose)
    printf("RND_SERVER: client %d connecting from %s\n", nxn, inet_ntoa(saddr.sin_addr));
  clients++;

  buf[0] = 0;
  buf[1] = OP_YOUR_NUMBER;
  buf[2] = u->number;
  sendtoone(u, 3);
}

void NetworkServer(int port, int onceonly)
{
  int i, sl, on;
  struct user *u, *v, *w;
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
    verbose = 0;
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

  if (verbose)
  {
    printf("rocksndiamonds network server: started up, listening on port %d\n",
	   port);
    printf("rocksndiamonds network server: using protocol version %d.%d.%d\n",
	   PROT_VERS_1, PROT_VERS_2, PROT_VERS_3);
  }

  while(1)
  {
    interrupt = 0;

    if (timetoplay && time(NULL) >= timetoplay)
    {
      buf[0] = 0;
      do_play();
      if (verbose)
	printf("RND_SERVER: everyone lost... restarting game\n");
      timetoplay = 0;
    }

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
      if (errno != EINTR)
	syserr("select");
      else continue;
    
    if (sl < 0)
      continue;

    if (clients > 0 && clients == bots)
    {
      if (verbose)
	printf("RND_SERVER: only bots left... dropping all bots\n");
      while (user0)
	dropuser(user0);
      continue;
    }
    
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
	  if (verbose)
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
	    if (verbose)
	      printf("RND_SERVER: crap from client %d (%s)\n", u->number, u->nick);
	    write(u->fd, "\033]50;kanji24\007\033#8\033(0", 19);
	    dropuser(u);
	    interrupt = 1;
	    break;
	  }
	  memcpy(buf, &u->readbuf[4], len);
	  u->nread -= 4 + len;
	  copydown(u->readbuf, u->readbuf + 4 + len, u->nread);

	  buf[0] = u->number;
	  if (!u->introduced && buf[1] != OP_NICKNAME)
	  {
	    if (verbose)
	      printf("RND_SERVER: !(client %d)->introduced && buf[1]==%d (expected OP_NICKNAME)\n", buf[0], buf[1]);

	    dropuser(u);
	    interrupt = 1;
	    break;
	  }

	  switch(buf[1])
	  {
	    case OP_NICKNAME:
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
	      
	      if (verbose)
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
		if (level != 5)
		{
		  buf[0] = 0;
		  buf[1] = OP_LEVEL;
		  buf[2] = level;
		  sendtoone(u, 3);
		}
		if (mode >= 0)
		{
		  buf[1] = OP_MODE;
		  buf[2] = mode;
		  sendtoone(u, 3);
		}
	      }

	      u->introduced = 1;
	      break;

	    case OP_KILL:
	      for (v=user0; v; v=v->next)
	      {
		if (v->number == buf[2])
		  break;
	      }
	      if (v)
	      {
		if (v->isbot)
		{
		  if (verbose)
		    printf("RND_SERVER: client %d (%s) kills bot %d (%s)\n", u->number, u->nick, v->number, v->nick);

		  dropuser(v);
		  interrupt = 1;
		  break;
		}
		else
		{
		  if (verbose)
		    printf("RND_SERVER: client %d (%s) attempting to kill non-bot %d (%s)\n", u->number, u->nick, v->number, v->nick);
		}
	      }
	      break;

	    case OP_START_PLAYING:
	      if (verbose)
		printf("RND_SERVER: client %d (%s) starts game [level %d from levedir %d (%s)]\n",
		       u->number, u->nick,
		       (buf[2] << 8) + buf[3],
		       (buf[4] << 8) + buf[5],
		       &buf[6]);
	      timetoplay = 0;
	      do_play();
	      break;

	    case OP_MOVE_FIGURE:
	    {
	      int actions_complete = 1;
	      int last_client_nr = 0;

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
		{
		  actions_complete = 0;
		  break;
		}
	      }

	      if (!actions_complete)
		break;

	      /* broadcast actions of all players to all players */
	      for (v=user0; v; v=v->next)
	      {
		buf[6 + v->number-1] = v->action;
		v->action = 0;
		v->action_received = 0;

		if (v->number > last_client_nr)
		  last_client_nr = v->number;
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

	      break;
	    }
	    
	    case OP_MODE:
	      mode = buf[2];
	      if (verbose)
		printf("RND_SERVER: client %d (%s) sets mode %d (%s)\n", u->number, u->nick, buf[2], buf[2] == 0 ? "normal" : (buf[2] == 1 ? "fun" : "unknown"));
	      broadcast(NULL, 3, 0);
	      break;

	    case OP_PAUSE:
	      if (verbose)
		printf("RND_SERVER: client %d (%s) pauses game\n", u->number, u->nick);
	      broadcast(NULL, 2, 0);
	      paused = 1;
	      break;

	    case OP_CONT:
	      if (verbose)
		printf("RND_SERVER: client %d (%s) continues game\n", u->number, u->nick);
	      broadcast(NULL, 2, 0);
	      paused = 0;
	      break;

	    case OP_BOT:
	      if (!u->isbot)
		bots++;
	      u->isbot = 1;
	      if (verbose)
		printf("RND_SERVER: client %d (%s) declares itself to be a bot\n", u->number, u->nick);
	      break;
	    
	    case OP_LEVEL:
	      level = buf[2];
	      if (verbose)
		printf("RND_SERVER: client %d (%s) sets level %d\n", u->number, u->nick, buf[2]);
	      broadcast(NULL, 3, 0);
	      break;

	    case OP_LOST:
	      {
		struct user *won = NULL;

		if (verbose)
		  printf("RND_SERVER: client %d (%s) has lost\n", u->number, u->nick);
		u->playing = 0;
		broadcast(u, 2, 1);
		i = 0;
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
		for (v=user0; v; v=v->next)
		{
		  if (v->playing)
		  {
		    i++;
		    won = v;
		  }
		}
		if (i == 1)
		{
		  buf[0] = won->number;
		  buf[1] = OP_WON;
		  won->games++;
		  broadcast(NULL, 2, 0);
		}
		else if (i == 0)
		{
		  buf[0] = u->number;
		  buf[1] = OP_WON;
		  u->games++;
		  broadcast(NULL, 2, 0);
		}
		if (i < 2 && clients > 1)
		  timetoplay = time(NULL) + 4;
	      }
	      break;
	    
	    case OP_ZERO:
	      broadcast(NULL, 2, 0);
	      if (verbose)
		printf("RND_SERVER: client %d (%s) resets the game counters\n", u->number, u->nick);
	      for (v=user0; v; v=v->next)
		v->games = 0;
	      break;

	    case OP_CLEAR:
	    case OP_GROW:
	      broadcast(u, 2, 1);
	      break;

	    case OP_MSG:
	      buf[len] = '\0';
	      if (verbose)
		printf("RND_SERVER: client %d (%s) sends message: %s\n", u->number, u->nick, &buf[2]);
	      broadcast(u, len, 0);
	      break;

	    case OP_NUMBER_WANTED:
	    {
	      int nr_wanted = buf[2];
	      int nr_is_free = 1;

	      if (verbose)
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

	      if (verbose)
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

	      buf[0] = 0;
	      buf[1] = OP_NUMBER_WANTED;
	      buf[2] = nr_wanted;
	      buf[3] = u->number;
	      sendtoone(u, 4);
	      break;
	    }

	    case OP_PROTOCOL_VERSION:
	      if (len != 5 || buf[2] != PROT_VERS_1 || buf[3] != PROT_VERS_2)
	      {
		if (verbose)
		  printf("RND_SERVER: client %d (%s) has wrong protocol version %d.%d.%d\n", u->number, u->nick, buf[2], buf[3], buf[4]);

		buf[0] = 0;
		buf[1] = OP_BADVERS;
		buf[2] = PROT_VERS_1;
		buf[3] = PROT_VERS_2;
		buf[4] = PROT_VERS_3;
		sendtoone(u, 5);
		flushuser(u);

		dropuser(u);
		interrupt = 1;
	      }
	      else
	      {
		if (verbose)
		  printf("RND_SERVER: client %d (%s) uses protocol version %d.%d.%d\n", u->number, u->nick, buf[2], buf[3], buf[4]);
	      }
	      break;

	    case OP_LINES:
	      if (len != 3)
	      {
		if (verbose)
		  printf("RND_SERVER: client %d (%s) sends crap for an OP_LINES\n", u->number, u->nick);

		dropuser(u);
		interrupt = 1;
		break;
	      }
	      if (u->nextvictim)
	      {
		if (verbose)
		  printf("RND_SERVER: client %d (%s) sends %d %s to client %d (%s)\n", u->number, u->nick, (int)buf[2], buf[2] == 1 ? "line" : "lines", u->nextvictim->number, u->nextvictim->nick);
		sendtoone(u->nextvictim, 3);
		buf[3] = u->nextvictim->number;
		buf[1] = OP_LINESTO;
		broadcast(u->nextvictim, 4, 1);
		for (v=NEXT(u->nextvictim); v!=u->nextvictim; v=NEXT(v))
		{
		  if (v->active && v != u && v->playing)
		  {
		    u->nextvictim = v;
		    break;
		  }
		}
	      }
	      else if (verbose)
		printf("RND_SERVER: client %d (%s) makes %d %s but has no victim\n", u->number, u->nick, (int)buf[2], buf[2] == 1 ? "line" : "lines");
	      break;
	    
	    default:
	      if (verbose)
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
