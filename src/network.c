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
#include "netserv.h"
#include "game.h"
#include "tape.h"
#include "files.h"
#include "tools.h"
#include "buttons.h"
#include "screens.h"
#include "misc.h"

#define MAXNICKLEN 14

struct user
{
  byte nr;
  char name[MAXNICKLEN+2];
  struct user *next;
};

struct user me =
{
  0,
  "no name",
  NULL
};

static char msgbuf[300];

/* server stuff */

int sfd;
unsigned char realbuf[512], readbuf[MAX_BUFFER_SIZE], writbuf[MAX_BUFFER_SIZE];
unsigned char *buf = realbuf + 4;
int nread = 0, nwrite = 0;

static void sysmsg(char *s)
{
  if (options.verbose)
  {
    printf("** %s\n", s);
    fflush(stdout);
  }
}

static void u_sleep(int i)
{
  struct timeval tm;
  tm.tv_sec = i / 1000000;
  tm.tv_usec = i % 1000000;
  select(0, NULL, NULL, NULL, &tm);
}

static void flushbuf()
{
  if (nwrite)
  {
    write(sfd, writbuf, nwrite);
    nwrite = 0;
  }
}

static void sendbuf(int len)
{
  if (options.network)
  {
    realbuf[0] = realbuf[1] = realbuf[2] = 0;
    realbuf[3] = (unsigned char)len;
    buf[0] = 0;
    if (nwrite + 4 + len >= MAX_BUFFER_SIZE)
      Error(ERR_EXIT, "internal error: network send buffer overflow");
    memcpy(writbuf + nwrite, realbuf, 4 + len);
    nwrite += 4 + len;


    flushbuf();


  }
}

struct user *finduser(unsigned char c)
{
  struct user *u;

  for (u = &me; u; u = u->next)
    if (u->nr == c)
      return u;
  
  Error(ERR_EXIT, "protocol error: reference to non-existing user %d", c);

  return NULL; /* so that gcc -Wall doesn't complain */
}

char *get_user_name(unsigned char c)
{
  struct user *u;

  if (c == 0)
    return("the server");
  else if (c == me.nr)
    return("you");
  else
    for (u = &me; u; u = u->next)
      if (u->nr == c && u->name && strlen(u->name))
	return(u->name);

  return("no name");
}

static void StartNetworkServer(int port)
{
  switch (fork())
  {
    case 0:
      NetworkServer(port, options.serveronly);

      /* never reached */
      exit(0);

    case -1:
      Error(ERR_WARN,
	    "cannot create network server process - no network games");
      options.network = FALSE;
      return;

    default:
      /* we are parent process -- resume normal operation */
      return;
  }
}

boolean ConnectToServer(char *host, int port)
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
	Error(ERR_EXIT, "cannot locate host '%s'", host);

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
    Error(ERR_EXIT, "out of file descriptors");

  if ((tcpproto = getprotobyname("tcp")) != NULL)
    setsockopt(sfd, tcpproto->p_proto, TCP_NODELAY, (char *)&on, sizeof(int));

  if (connect(sfd, (struct sockaddr *)&s, sizeof(s)) < 0)
  {
    if (!host)
    {
      printf("No rocksndiamonds server on localhost - starting up one ...\n");

      StartNetworkServer(port);

      for (i=0; i<6; i++)
      {
	u_sleep(500000);
	close(sfd);

	sfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	  Error(ERR_EXIT, "out of file descriptors");

	setsockopt(sfd, tcpproto->p_proto, TCP_NODELAY,
		   (char *)&on, sizeof(int));

	if (connect(sfd, (struct sockaddr *)&s, sizeof(s)) >= 0)
	  break;
      }
      if (i==6)
	Error(ERR_EXIT, "cannot connect to server");
    }
    else
      Error(ERR_EXIT, "cannot connect to server");
  }

  return(TRUE);
}

void SendToServer_Nickname(char *nickname)
{
  static char msgbuf[300];

  buf[1] = OP_NICKNAME;
  memcpy(&buf[2], nickname, strlen(nickname));
  sendbuf(2 + strlen(nickname));
  sprintf(msgbuf, "you set your nick to \"%s\"", nickname);
  sysmsg(msgbuf);
}

void SendToServer_ProtocolVersion()
{
  buf[1] = OP_PROTOCOL_VERSION;
  buf[2] = PROT_VERS_1;
  buf[3] = PROT_VERS_2;
  buf[4] = PROT_VERS_3;

  sendbuf(5);
}

void SendToServer_NrWanted(int nr_wanted)
{
  buf[1] = OP_NUMBER_WANTED;
  buf[2] = nr_wanted;

  sendbuf(3);
}

void SendToServer_StartPlaying()
{
  unsigned long new_random_seed = InitRND(NEW_RANDOMIZE);

  buf[1] = OP_START_PLAYING;
  buf[2] = (byte)(level_nr >> 8);
  buf[3] = (byte)(level_nr & 0xff);
  buf[4] = (byte)(leveldir_nr >> 8);
  buf[5] = (byte)(leveldir_nr & 0xff);

  buf[6] = (unsigned char)((new_random_seed >> 24) & 0xff);
  buf[7] = (unsigned char)((new_random_seed >> 16) & 0xff);
  buf[8] = (unsigned char)((new_random_seed >>  8) & 0xff);
  buf[9] = (unsigned char)((new_random_seed >>  0) & 0xff);

  strcpy(&buf[10], leveldir[leveldir_nr].name);

  sendbuf(10 + strlen(leveldir[leveldir_nr].name)+1);
}

void SendToServer_PausePlaying()
{
  buf[1] = OP_PAUSE_PLAYING;

  sendbuf(2);
}

void SendToServer_ContinuePlaying()
{
  buf[1] = OP_CONTINUE_PLAYING;

  sendbuf(2);
}

void SendToServer_StopPlaying()
{
  buf[1] = OP_STOP_PLAYING;

  sendbuf(2);
}

void SendToServer_MovePlayer(byte player_action)
{
  buf[1] = OP_MOVE_FIGURE;
  buf[2] = player_action;

  sendbuf(3);
}

static void Handle_OP_BAD_PROTOCOL_VERSION()
{
  Error(ERR_WARN, "protocol version mismatch");
  Error(ERR_EXIT, "server expects %d.%d.x instead of %d.%d.%d",
	buf[2], buf[3], PROT_VERS_1, PROT_VERS_2, PROT_VERS_3);
}

static void Handle_OP_YOUR_NUMBER()
{
  int new_client_nr = buf[2];
  int new_index_nr = new_client_nr - 1;
  struct PlayerInfo *old_local_player = local_player;
  struct PlayerInfo *new_local_player = &stored_player[new_index_nr];

  printf("OP_YOUR_NUMBER: %d\n", buf[0]);
  me.nr = new_client_nr;

  if (old_local_player != new_local_player)
  {
    /* copy existing player settings and change to new player */

    *new_local_player = *old_local_player;
    old_local_player->connected = FALSE;

    local_player = new_local_player;
  }

  TestPlayer = new_index_nr;

  if (me.nr > MAX_PLAYERS)
    Error(ERR_EXIT, "sorry - no more than %d players", MAX_PLAYERS);

  sprintf(msgbuf, "you get client # %d", new_client_nr);
  sysmsg(msgbuf);
}

static void Handle_OP_NUMBER_WANTED()
{
  int client_nr_wanted = buf[2];
  int old_client_nr = buf[0];
  int new_client_nr = buf[3];
  int old_index_nr = old_client_nr - 1;
  int new_index_nr = new_client_nr - 1;
  int index_nr_wanted = client_nr_wanted - 1;
  struct PlayerInfo *old_player = &stored_player[old_index_nr];
  struct PlayerInfo *new_player = &stored_player[new_index_nr];

  printf("OP_NUMBER_WANTED: %d\n", buf[0]);

  if (new_client_nr == client_nr_wanted)	/* switching succeeded */
  {
    struct user *u;

    if (old_client_nr != client_nr_wanted)	/* client's nr has changed */
    {
      sprintf(msgbuf, "client %d switches to # %d",
	      old_client_nr, new_client_nr);
      sysmsg(msgbuf);
    }
    else if (old_client_nr == me.nr)		/* local player keeps his nr */
    {
      sprintf(msgbuf, "keeping client # %d", new_client_nr);
      sysmsg(msgbuf);
    }

    if (old_client_nr != new_client_nr)
    {
      /* copy existing player settings and change to new player */

      *new_player = *old_player;
      old_player->connected = FALSE;
    }

    u = finduser(old_client_nr);
    u->nr = new_client_nr;

    if (old_player == local_player)		/* local player switched */
      local_player = new_player;


    TestPlayer = new_index_nr;
  }
  else if (old_client_nr == me.nr)		/* failed -- local player? */
  {
    char *color[] = { "yellow", "red", "green", "blue" };

    sprintf(msgbuf, "Sorry ! %s player still exists ! You are %s player !",
	    color[index_nr_wanted], color[new_index_nr]);
    Request(msgbuf, REQ_CONFIRM);

    sprintf(msgbuf, "cannot switch -- you keep client # %d",
	    new_client_nr);
    sysmsg(msgbuf);
  }
}

static void Handle_OP_NICKNAME(unsigned int len)
{
  struct user *u;

  printf("OP_NICKNAME: %d\n", buf[0]);
  u = finduser(buf[0]);
  buf[len] = 0;
  sprintf(msgbuf, "client %d calls itself \"%s\"", buf[0], &buf[2]);
  sysmsg(msgbuf);
  strncpy(u->name, &buf[2], MAXNICKLEN);
}

static void Handle_OP_PLAYER_CONNECTED()
{
  struct user *u, *v = NULL;
  int new_client_nr = buf[0];
  int new_index_nr = new_client_nr - 1;

  printf("OP_PLAYER_CONNECTED: %d\n", new_client_nr);
  sprintf(msgbuf, "new client %d connected", new_client_nr);
  sysmsg(msgbuf);

  for (u = &me; u; u = u->next)
  {
    if (u->nr == new_client_nr)
      Error(ERR_EXIT, "multiplayer server sent duplicate player id");
    else
      v = u;
  }

  v->next = u = checked_malloc(sizeof(struct user));
  u->nr = new_client_nr;
  u->name[0] = '\0';
  u->next = NULL;

  stored_player[new_index_nr].connected = TRUE;
}

static void Handle_OP_PLAYER_DISCONNECTED()
{
  struct user *u, *v;

  printf("OP_PLAYER_DISCONNECTED: %d\n", buf[0]);
  u = finduser(buf[0]);
  sprintf(msgbuf, "client %d (%s) disconnected",
	  buf[0], get_user_name(buf[0]));
  sysmsg(msgbuf);

  for (v = &me; v; v = v->next)
    if (v->next == u)
      v->next = u->next;
  free(u);
}

static void Handle_OP_START_PLAYING()
{
  int new_level_nr, new_leveldir_nr;
  unsigned long new_random_seed;
  unsigned char *new_leveldir_name;

  /*
    if (game_status == PLAYING)
    break;
  */

  new_level_nr = (buf[2] << 8) + buf[3];
  new_leveldir_nr = (buf[4] << 8) + buf[5];
  new_random_seed =
    (buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | (buf[9]);
  new_leveldir_name = &buf[10];

  printf("OP_START_PLAYING: %d\n", buf[0]);
  sprintf(msgbuf, "client %d starts game [level %d from levedir %d (%s)]\n",
	  buf[0],
	  new_level_nr,
	  new_leveldir_nr,
	  new_leveldir_name);
  sysmsg(msgbuf);


  if (strcmp(leveldir[new_leveldir_nr].name, new_leveldir_name) != 0)
    Error(ERR_WARN, "no such level directory: '%s'",new_leveldir_name);

  leveldir_nr = new_leveldir_nr;

  /*
  local_player->leveldir_nr = leveldir_nr;
  */


  /*
  SaveLevelSetup();
  */

  /*
  LoadPlayerInfo(PLAYER_LEVEL);
  SavePlayerInfo(PLAYER_SETUP);
  */


  level_nr = new_level_nr;

  TapeErase();
  LoadLevelTape(level_nr);

  /*
  GetPlayerConfig();
  */

  LoadLevel(level_nr);



  if (setup.autorecord)
    TapeStartRecording();



  if (tape.recording)
    tape.random_seed = new_random_seed;

  InitRND(new_random_seed);


  /*
    printf("tape.random_seed == %d\n", tape.random_seed);
  */

  game_status = PLAYING;
  InitGame();
}

static void Handle_OP_PAUSE_PLAYING()
{
  printf("OP_PAUSE_PLAYING: %d\n", buf[0]);
  sprintf(msgbuf, "client %d pauses game", buf[0]);
  sysmsg(msgbuf);

  tape.pausing = TRUE;
  DrawVideoDisplay(VIDEO_STATE_PAUSE_ON,0);
}

static void Handle_OP_CONTINUE_PLAYING()
{
  printf("OP_CONTINUE_PLAYING: %d\n", buf[0]);
  sprintf(msgbuf, "client %d continues game", buf[0]);
  sysmsg(msgbuf);

  tape.pausing = FALSE;
  DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
}

static void Handle_OP_STOP_PLAYING()
{
  printf("OP_STOP_PLAYING: %d\n", buf[0]);
  sprintf(msgbuf, "client %d stops game", buf[0]);
  sysmsg(msgbuf);

  game_status = MAINMENU;
  DrawMainMenu();
}

static void Handle_OP_MOVE_FIGURE(unsigned int len)
{
  int frame_nr;
  int i;

  if (!network_playing)
    return;

  frame_nr =
    (buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | (buf[5]);

  if (frame_nr != FrameCounter)
  {
    Error(ERR_RETURN, "client and servers frame counters out of sync");
    Error(ERR_RETURN, "frame counter of client is %d", FrameCounter);
    Error(ERR_RETURN, "frame counter of server is %d", frame_nr);
    Error(ERR_EXIT,   "this should not happen -- please debug");
  }

  /* copy valid player actions */
  for (i=0; i<MAX_PLAYERS; i++)
    stored_player[i].effective_action =
      (i < len - 6 && stored_player[i].active ? buf[6 + i] : 0);

  network_player_action_received = TRUE;

  /*
    sprintf(msgbuf, "frame %d: client %d moves player [0x%02x]",
    FrameCounter, buf[0], buf[2]);
    sysmsg(msgbuf);
  */
}

static void HandleNetworkingMessages()
{
  unsigned int len;

  while (nread >= 4 && nread >= 4 + readbuf[3])
  {
    len = readbuf[3];
    if (readbuf[0] || readbuf[1] || readbuf[2])
      Error(ERR_EXIT, "wrong network server line length");

    memcpy(buf, &readbuf[4], len);
    nread -= 4 + len;
    memmove(readbuf, readbuf + 4 + len, nread);

    switch(buf[1])
    {
      case OP_BAD_PROTOCOL_VERSION:
	Handle_OP_BAD_PROTOCOL_VERSION();
	break;

      case OP_YOUR_NUMBER:
	Handle_OP_YOUR_NUMBER();
	break;

      case OP_NUMBER_WANTED:
	Handle_OP_NUMBER_WANTED();
	break;

      case OP_NICKNAME:
	Handle_OP_NICKNAME(len);
	break;

      case OP_PLAYER_CONNECTED:
	Handle_OP_PLAYER_CONNECTED();
	break;
      
      case OP_PLAYER_DISCONNECTED:
	Handle_OP_PLAYER_DISCONNECTED();
	break;

      case OP_START_PLAYING:
	Handle_OP_START_PLAYING();
	break;

      case OP_PAUSE_PLAYING:
	Handle_OP_PAUSE_PLAYING();
	break;

      case OP_CONTINUE_PLAYING:
	Handle_OP_CONTINUE_PLAYING();
	break;

      case OP_STOP_PLAYING:
	Handle_OP_STOP_PLAYING();
	break;

      case OP_MOVE_FIGURE:
	Handle_OP_MOVE_FIGURE(len);
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

  flushbuf();

  FD_ZERO(&rfds);
  FD_SET(sfd, &rfds);

  r = select(sfd + 1, &rfds, NULL, NULL, &tv);

  if (r < 0 && errno != EINTR)
    Error(ERR_EXIT, "HandleNetworking(): select() failed");

  if (r < 0)
    FD_ZERO(&rfds);

  if (FD_ISSET(sfd, &rfds))
  {
    int r;

    r = read(sfd, readbuf + nread, MAX_BUFFER_SIZE - nread);

    if (r < 0)
      Error(ERR_EXIT, "error reading from network server");

    if (r == 0)
      Error(ERR_EXIT, "connection to network server lost");

    nread += r;

    HandleNetworkingMessages();
  }
}
