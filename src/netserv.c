/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2006 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* netserv.c                                                *
***********************************************************/

#include "libgame/platform.h"

#if defined(NETWORK_AVALIABLE)

#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

#if defined(TARGET_SDL)
#include "main.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>			/* apparently needed for OS/2 port */
#endif

#include "libgame/libgame.h"

#include "netserv.h"

static int clients = 0;
static int onceonly = 0;

struct NetworkServerPlayerInfo
{
#if defined(TARGET_SDL)
  TCPsocket fd;
#else
  int fd;
#endif

  char player_name[16];
  unsigned char number;
  struct NetworkServerPlayerInfo *next;
  char active;
  char introduced;
  unsigned char readbuffer[MAX_BUFFER_SIZE];
  unsigned char writbuffer[MAX_BUFFER_SIZE];
  int nread, nwrite;
  byte action;
  boolean action_received;
};

static struct NetworkServerPlayerInfo *first_player = NULL;

#define NEXT(player) ((player)->next ? (player)->next : first_player)

#if defined(TARGET_SDL)
/* TODO: peer address */
static TCPsocket lfd;		/* listening socket */
static SDLNet_SocketSet fds;	/* socket set */
#else
static struct sockaddr_in saddr;
static int lfd;			/* listening socket */
static fd_set fds;		/* socket set */
static int tcp = -1;
#endif

static unsigned char realbuffer[512], *buffer = realbuffer + 4;

static int interrupt;

static unsigned long ServerFrameCounter = 0;

static void addtobuffer(struct NetworkServerPlayerInfo *player,
			unsigned char *b, int len)
{
  if (player->nwrite + len >= MAX_BUFFER_SIZE)
    Error(ERR_EXIT_NETWORK_SERVER,
	  "internal error: network send buffer overflow");

  memcpy(player->writbuffer + player->nwrite, b, len);
  player->nwrite += len;
}

static void flushuser(struct NetworkServerPlayerInfo *player)
{
  if (player->nwrite)
  {
#if defined(TARGET_SDL)
    SDLNet_TCP_Send(player->fd, player->writbuffer, player->nwrite);
#else
    write(player->fd, player->writbuffer, player->nwrite);
#endif
    player->nwrite = 0;
  }
}

static void broadcast(struct NetworkServerPlayerInfo *except,
		      int len, int activeonly)
{
  struct NetworkServerPlayerInfo *player;

  realbuffer[0] = realbuffer[1] = realbuffer[2] = 0;
  realbuffer[3] = (unsigned char)len;
  for (player = first_player; player; player = player->next)
    if (player != except && player->introduced &&
	(player->active || !activeonly))
      addtobuffer(player, realbuffer, 4 + len);
}

static void sendtoone(struct NetworkServerPlayerInfo *to, int len)
{
  realbuffer[0] = realbuffer[1] = realbuffer[2] = 0;
  realbuffer[3] = (unsigned char)len;
  addtobuffer(to, realbuffer, 4 + len);
}

static void RemovePlayer(struct NetworkServerPlayerInfo *player)
{
  struct NetworkServerPlayerInfo *v;
  
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "dropping client %d (%s)",
	  player->number, player->player_name);

  if (player == first_player)
    first_player = player->next;
  else
  {
    for (v = first_player; v; v = v->next)
    {
      if (v->next && v->next == player)
      {
	v->next = player->next;
	break;
      }
    }
  }

#if defined(TARGET_SDL)
  SDLNet_TCP_DelSocket(fds, player->fd);
  SDLNet_TCP_Close(player->fd);
#else
  close(player->fd);
#endif

  if (player->introduced)
  {
    buffer[0] = player->number;
    buffer[1] = OP_PLAYER_DISCONNECTED;
    broadcast(player, 2, 0);
  }

  free(player);
  clients--;

  if (onceonly && clients == 0)
  {
    if (options.verbose)
    {
      Error(ERR_NETWORK_SERVER, "no clients left");
      Error(ERR_NETWORK_SERVER, "aborting");
    }
    exit(0);
  }
}

#if defined(TARGET_SDL)
static void AddPlayer(TCPsocket fd)
#else
static void AddPlayer(int fd)
#endif
{
  struct NetworkServerPlayerInfo *player, *v;
  unsigned char nxn;
  boolean again = TRUE;

  player = checked_malloc(sizeof (struct NetworkServerPlayerInfo));

  player->fd = fd;
  player->player_name[0] = 0;
  player->next = first_player;
  player->active = 0;
  player->nread = 0;
  player->nwrite = 0;
  player->introduced = 0;
  player->action = 0;
  player->action_received = FALSE;

#if defined(TARGET_SDL)
  SDLNet_TCP_AddSocket(fds, fd);
#endif

  first_player = player;

  nxn = 1;

  while (again)
  {
    again = FALSE;
    v = player->next;

    while (v)
    {
      if (v->number == nxn)
      {
	nxn++;

	again = TRUE;
	break;
      }
      v = v->next;
    }
  }

  player->number = nxn;
#if !defined(TARGET_SDL)
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d connecting from %s",
	  nxn, inet_ntoa(saddr.sin_addr));
#endif
  clients++;

  buffer[0] = 0;
  buffer[1] = OP_YOUR_NUMBER;
  buffer[2] = player->number;
  sendtoone(player, 3);
}

static void Handle_OP_PROTOCOL_VERSION(struct NetworkServerPlayerInfo *player,
				       unsigned int len)
{
  if (len != 5 ||
      buffer[2] != PROTOCOL_VERSION_1 ||
      buffer[3] != PROTOCOL_VERSION_2)
  {
    if (options.verbose)
      Error(ERR_NETWORK_SERVER,
	    "client %d (%s) has wrong protocol version %d.%d.%d",
	    player->number, player->player_name, buffer[2], buffer[3], buffer[4]);

    buffer[0] = 0;
    buffer[1] = OP_BAD_PROTOCOL_VERSION;
    buffer[2] = PROTOCOL_VERSION_1;
    buffer[3] = PROTOCOL_VERSION_2;
    buffer[4] = PROTOCOL_VERSION_3;
    sendtoone(player, 5);
    flushuser(player);

    RemovePlayer(player);
    interrupt = 1;
  }
  else
  {
    if (options.verbose)
      Error(ERR_NETWORK_SERVER,
	    "client %d (%s) uses protocol version %d.%d.%d",
	    player->number, player->player_name, buffer[2], buffer[3], buffer[4]);
  }
}

static void Handle_OP_NUMBER_WANTED(struct NetworkServerPlayerInfo *player)
{
  struct NetworkServerPlayerInfo *v;
  int client_nr = player->number;
  int nr_wanted = buffer[2];
  int nr_is_free = 1;

  if (options.verbose)
      Error(ERR_NETWORK_SERVER, "client %d (%s) wants to switch to # %d",
	    player->number, player->player_name, nr_wanted);

  for (v = first_player; v; v = v->next)
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
      Error(ERR_NETWORK_SERVER, "client %d (%s) switches to # %d",
	    player->number, player->player_name, nr_wanted);
    else if (player->number == nr_wanted)
      Error(ERR_NETWORK_SERVER, "client %d (%s) already has # %d",
	    player->number, player->player_name, nr_wanted);
    else
      Error(ERR_NETWORK_SERVER,
	    "client %d (%s) cannot switch (client %d already exists)",
	    player->number, player->player_name, nr_wanted);
  }

  if (nr_is_free)
    player->number = nr_wanted;

  buffer[0] = client_nr;
  buffer[1] = OP_NUMBER_WANTED;
  buffer[2] = nr_wanted;
  buffer[3] = player->number;

  /*
  sendtoone(player, 4);
  */

  broadcast(NULL, 4, 0);
}

static void Handle_OP_PLAYER_NAME(struct NetworkServerPlayerInfo *player,
				  unsigned int len)
{
  struct NetworkServerPlayerInfo *v;
  int i;

  if (len>16)
    len=16;
  memcpy(player->player_name, &buffer[2], len-2);
  player->player_name[len-2] = 0;
  for (i = 0; i < len - 2; i++)
  {
    if (player->player_name[i] < ' ' || 
	((unsigned char)(player->player_name[i]) > 0x7e &&
	 (unsigned char)(player->player_name[i]) <= 0xa0))
    {
      player->player_name[i] = 0;
      break;
    }
  }

  if (!player->introduced)
  {
    buffer[0] = player->number;
    buffer[1] = OP_PLAYER_CONNECTED;
    broadcast(player, 2, 0);
  }
	      
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d calls itself \"%s\"",
	  player->number, player->player_name);
  buffer[1] = OP_PLAYER_NAME;
  broadcast(player, len, 0);

  if (!player->introduced)
  {
    for (v = first_player; v; v = v->next)
    {
      if (v != player && v->introduced)
      {
	buffer[0] = v->number;
	buffer[1] = OP_PLAYER_CONNECTED;
	sendtoone(player, 2);
	buffer[1] = OP_PLAYER_NAME;
	memcpy(&buffer[2], v->player_name, 14);
	sendtoone(player, 2+strlen(v->player_name));
      }
    }
  }

  player->introduced = 1;
}

static void Handle_OP_START_PLAYING(struct NetworkServerPlayerInfo *player)
{
  struct NetworkServerPlayerInfo *v, *w;

  if (options.verbose)
    Error(ERR_NETWORK_SERVER,
	  "client %d (%s) starts game [level %d from leveldir %d (%s)]",
	  player->number, player->player_name,
	  (buffer[2] << 8) + buffer[3],
	  (buffer[4] << 8) + buffer[5],
	  &buffer[10]);

  for (w = first_player; w; w = w->next)
    if (w->introduced)
      w->active = 1;

  /* reset frame counter */
  ServerFrameCounter = 0;

  Error(ERR_NETWORK_SERVER, "resetting ServerFrameCounter to 0");

  /* reset player actions */
  for (v = first_player; v; v = v->next)
  {
    v->action = 0;
    v->action_received = FALSE;
  }

  broadcast(NULL, 10 + strlen((char *)&buffer[10])+1, 0);
}

static void Handle_OP_PAUSE_PLAYING(struct NetworkServerPlayerInfo *player)
{
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) pauses game",
	  player->number, player->player_name);
  broadcast(NULL, 2, 0);
}

static void Handle_OP_CONTINUE_PLAYING(struct NetworkServerPlayerInfo *player)
{
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) continues game",
	  player->number, player->player_name);
  broadcast(NULL, 2, 0);
}

static void Handle_OP_STOP_PLAYING(struct NetworkServerPlayerInfo *player)
{
  int cause_for_stopping = buffer[2];

  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) stops game [%d]",
	  player->number, player->player_name, cause_for_stopping);
  broadcast(NULL, 3, 0);
}

static void Handle_OP_MOVE_PLAYER(struct NetworkServerPlayerInfo *player)
{
  struct NetworkServerPlayerInfo *v;
  int last_client_nr = 0;
  int i;

  /* store player action */
  for (v = first_player; v; v = v->next)
  {
    if (v->number == player->number)
    {
      v->action = buffer[2];
      v->action_received = TRUE;
    }
  }

  /* check if server received action from each player */
  for (v = first_player; v; v = v->next)
  {
    if (!v->action_received)
      return;

    if (v->number > last_client_nr)
      last_client_nr = v->number;
  }

  /* initialize all player actions to zero */
  for (i = 0; i < last_client_nr; i++)
    buffer[6 + i] = 0;

  /* broadcast actions of all players to all players */
  for (v = first_player; v; v = v->next)
  {
    buffer[6 + v->number-1] = v->action;
    v->action = 0;
    v->action_received = FALSE;
  }

  buffer[2] = (unsigned char)((ServerFrameCounter >> 24) & 0xff);
  buffer[3] = (unsigned char)((ServerFrameCounter >> 16) & 0xff);
  buffer[4] = (unsigned char)((ServerFrameCounter >>  8) & 0xff);
  buffer[5] = (unsigned char)((ServerFrameCounter >>  0) & 0xff);

  broadcast(NULL, 6 + last_client_nr, 0);

#if 0
  Error(ERR_NETWORK_SERVER, "sending ServerFrameCounter value %d",
	ServerFrameCounter);
#endif

  ServerFrameCounter++;
}

#if defined(TARGET_SDL)
/* the following is not used for a standalone server;
   the pointer points to an integer containing the port-number */
int NetworkServerThread(void *ptr)
{
  NetworkServer(*((int *) ptr), 0);

  /* should never be reached */
  return 0;
}
#endif

void NetworkServer(int port, int serveronly)
{
  int sl;
  struct NetworkServerPlayerInfo *player;
  int r; 
  unsigned int len;
#if defined(TARGET_SDL)
  IPaddress ip;
#else
  int i, on;
  int is_daemon = 0;
  struct protoent *tcpproto;
  struct timeval tv;
  int mfd;
#endif

#if defined(PLATFORM_UNIX) && !defined(PLATFORM_NEXT)
  struct sigaction sact;
#endif

  if (port == 0)
    port = DEFAULT_SERVER_PORT;

  if (!serveronly)
    onceonly = 1;

#if !defined(TARGET_SDL)
  if ((tcpproto = getprotobyname("tcp")) != NULL)
    tcp = tcpproto->p_proto;
#endif

#if defined(PLATFORM_UNIX)
#if defined(PLATFORM_NEXT)
  signal(SIGPIPE, SIG_IGN);
#else
  sact.sa_handler = SIG_IGN;
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = 0;
  sigaction(SIGPIPE, &sact, NULL);
#endif
#endif

#if defined(TARGET_SDL)

  /* assume that SDL is already initialized */
#if 0
  if (SDLNet_Init() == -1)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_Init() failed");
  atexit(SDLNet_Quit);
#endif

  if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_ResolveHost() failed");

  lfd = SDLNet_TCP_Open(&ip);
  if (!lfd)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_TCP_Open() failed");

  fds = SDLNet_AllocSocketSet(MAX_PLAYERS+1);
  SDLNet_TCP_AddSocket(fds, lfd);

#else

  if ((lfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    Error(ERR_EXIT_NETWORK_SERVER, "socket() failed");

  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons(port);

  on = 1;

  setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
  if (bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
    Error(ERR_EXIT_NETWORK_SERVER, "bind() failed");

  listen(lfd, 5);
#endif

#if !defined(TARGET_SDL)
  if (is_daemon)
  {
    /* become a daemon, breaking all ties with the controlling terminal */
    options.verbose = FALSE;
    for (i = 0; i < 255; i++)
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
#endif

  if (options.verbose)
  {
    Error(ERR_NETWORK_SERVER, "started up, listening on port %d", port);
    Error(ERR_NETWORK_SERVER, "using protocol version %d.%d.%d",
	  PROTOCOL_VERSION_1, PROTOCOL_VERSION_2, PROTOCOL_VERSION_3);
  }

  while (1)
  {
    interrupt = 0;

    for (player = first_player; player; player = player->next)
      flushuser(player);

#if defined(TARGET_SDL)
    if ((sl = SDLNet_CheckSockets(fds, 500000)) < 1)
    {
      Error(ERR_NETWORK_SERVER, SDLNet_GetError());
      perror("SDLNet_CheckSockets");
    }

#else

    FD_ZERO(&fds);
    mfd = lfd;
    player = first_player;
    while (player)
    {
      FD_SET(player->fd, &fds);
      if (player->fd > mfd)
	mfd = player->fd;
      player = player->next;
    }
    FD_SET(lfd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    if ((sl = select(mfd + 1, &fds, NULL, NULL, &tv)) < 0)
    {
      if (errno != EINTR)
	Error(ERR_EXIT_NETWORK_SERVER, "select() failed");
      else
	continue;
    }
#endif

    if (sl < 0)
      continue;
    
    if (sl == 0)
      continue;

    /* accept incoming connections */
#if defined(TARGET_SDL)
    if (SDLNet_SocketReady(lfd))
    {
      TCPsocket newsock;

      newsock = SDLNet_TCP_Accept(lfd);

      if (newsock)
	AddPlayer(newsock);
    }

#else

    if (FD_ISSET(lfd, &fds))
    {
      int newfd, slen;

      slen = sizeof(saddr);
      newfd = accept(lfd, (struct sockaddr *)&saddr, &slen);
      if (newfd < 0)
      {
	if (errno != EINTR)
	  Error(ERR_EXIT_NETWORK_SERVER, "accept() failed");
      }
      else
      {
	if (tcp != -1)
	{
	  on = 1;
	  setsockopt(newfd, tcp, TCP_NODELAY, (char *)&on, sizeof(int));
	}
	AddPlayer(newfd);
      }
      continue;
    }
#endif

    player = first_player;

    do
    {
#if defined(TARGET_SDL)
      if (SDLNet_SocketReady(player->fd))
#else
      if (FD_ISSET(player->fd, &fds))
#endif
      {
#if defined(TARGET_SDL)
	/* read only 1 byte, because SDLNet blocks when we want more than is
	   in the buffer */
	r = SDLNet_TCP_Recv(player->fd, player->readbuffer + player->nread, 1);
#else
	r = read(player->fd, player->readbuffer + player->nread,
		 MAX_BUFFER_SIZE - player->nread);
#endif

	if (r <= 0)
	{
	  if (options.verbose)
	    Error(ERR_NETWORK_SERVER, "EOF from client %d (%s)",
		  player->number, player->player_name);
	  RemovePlayer(player);
	  interrupt = 1;
	  break;
	}
	player->nread += r;
	while (player->nread >= 4 && player->nread >= 4 + player->readbuffer[3])
	{
	  len = player->readbuffer[3];
	  if (player->readbuffer[0] || player->readbuffer[1] || player->readbuffer[2])
	  {
	    if (options.verbose)
	      Error(ERR_NETWORK_SERVER, "crap from client %d (%s)",
		    player->number, player->player_name);
	    RemovePlayer(player);
	    interrupt = 1;
	    break;
	  }
	  memcpy(buffer, &player->readbuffer[4], len);
	  player->nread -= 4 + len;
	  memmove(player->readbuffer, player->readbuffer + 4 + len, player->nread);

	  buffer[0] = player->number;
	  if (!player->introduced && buffer[1] != OP_PLAYER_NAME)
	  {
	    if (options.verbose)
	      Error(ERR_NETWORK_SERVER, "!(client %d)->introduced && buffer[1]==%d (expected OP_PLAYER_NAME)", buffer[0], buffer[1]);

	    RemovePlayer(player);
	    interrupt = 1;
	    break;
	  }

	  switch (buffer[1])
	  {
	    case OP_PLAYER_NAME:
	      Handle_OP_PLAYER_NAME(player, len);
	      break;

	    case OP_PROTOCOL_VERSION:
	      Handle_OP_PROTOCOL_VERSION(player, len);
	      break;

	    case OP_NUMBER_WANTED:
	      Handle_OP_NUMBER_WANTED(player);
	      break;

	    case OP_START_PLAYING:
	      Handle_OP_START_PLAYING(player);
	      break;

	    case OP_PAUSE_PLAYING:
	      Handle_OP_PAUSE_PLAYING(player);
	      break;

	    case OP_CONTINUE_PLAYING:
	      Handle_OP_CONTINUE_PLAYING(player);
	      break;

	    case OP_STOP_PLAYING:
	      Handle_OP_STOP_PLAYING(player);
	      break;

	    case OP_MOVE_PLAYER:
	      Handle_OP_MOVE_PLAYER(player);
	      break;

	    case OP_BROADCAST_MESSAGE:
	      buffer[len] = '\0';
	      if (options.verbose)
		Error(ERR_NETWORK_SERVER, "client %d (%s) sends message: %s",
		      player->number, player->player_name, &buffer[2]);
	      broadcast(player, len, 0);
	      break;
	    
	    default:
	      if (options.verbose)
		Error(ERR_NETWORK_SERVER,
		      "unknown opcode %d from client %d (%s)",
		      buffer[0], player->number, player->player_name);
	  }
	}
      }

      if (player && !interrupt)
	player = player->next;
    }
    while (player && !interrupt);
  }
}

#endif /* NETWORK_AVALIABLE */
