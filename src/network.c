/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* network.c                                                *
***********************************************************/

#include "libgame/platform.h"

#if defined(PLATFORM_UNIX)

#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "libgame/libgame.h"

#include "network.h"
#include "netserv.h"
#include "game.h"
#include "tape.h"
#include "files.h"
#include "tools.h"
#include "screens.h"

struct NetworkClientPlayerInfo
{
  byte nr;
  char name[MAX_PLAYER_NAME_LEN + 1];
  struct NetworkClientPlayerInfo *next;
};

static struct NetworkClientPlayerInfo first_player =
{
  0,
  EMPTY_PLAYER_NAME,
  NULL
};

/* server stuff */

static int sfd;
static byte realbuffer[512];
static byte readbuffer[MAX_BUFFER_SIZE], writbuffer[MAX_BUFFER_SIZE];
static byte *buffer = realbuffer + 4;
static int nread = 0, nwrite = 0;

static void SendBufferToServer(int size)
{
  if (!options.network)
    return;

  realbuffer[0] = realbuffer[1] = realbuffer[2] = 0;
  realbuffer[3] = (byte)size;
  buffer[0] = 0;

  if (nwrite + 4 + size >= MAX_BUFFER_SIZE)
    Error(ERR_EXIT, "internal error: network send buffer overflow");

  memcpy(writbuffer + nwrite, realbuffer, 4 + size);
  nwrite += 4 + size;

  /* directly send the buffer to the network server */
  write(sfd, writbuffer, nwrite);
  nwrite = 0;
}

struct NetworkClientPlayerInfo *getNetworkPlayer(int player_nr)
{
  struct NetworkClientPlayerInfo *player = NULL;

  for (player = &first_player; player; player = player->next)
    if (player->nr == player_nr)
      break;

  if (player == NULL)	/* should not happen */
    Error(ERR_EXIT, "protocol error: reference to non-existing player %d",
	  player_nr);

  return player;
}

char *getNetworkPlayerName(int player_nr)
{
  struct NetworkClientPlayerInfo *player;

  if (player_nr == 0)
    return("the network game server");
  else if (player_nr == first_player.nr)
    return("you");
  else
    for (player = &first_player; player; player = player->next)
      if (player->nr == player_nr && player->name && strlen(player->name))
	return(player->name);

  return(EMPTY_PLAYER_NAME);
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
	    "cannot create network server process - no network playing");
      options.network = FALSE;
      return;

    default:
      /* we are parent process -- resume normal operation */
      return;
  }
}

boolean ConnectToServer(char *hostname, int port)
{
  struct sockaddr_in s;
  struct protoent *tcpproto;
  int on = 1, i;

  if (hostname)
  {
    if ((s.sin_addr.s_addr = inet_addr(hostname)) == -1)
    {
      struct hostent *host;

      if ((host = gethostbyname(hostname)) == NULL)
	Error(ERR_EXIT, "cannot locate host '%s'", hostname);

      s.sin_addr = *(struct in_addr *)(host->h_addr_list[0]);
    }
  }
  else
    s.sin_addr.s_addr = inet_addr("127.0.0.1");		/* localhost */

  if (port == 0)
    port = DEFAULT_SERVER_PORT;

  s.sin_port = htons(port);
  s.sin_family = AF_INET;

  sfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sfd < 0)
    Error(ERR_EXIT, "out of file descriptors");

  if ((tcpproto = getprotobyname("tcp")) != NULL)
    setsockopt(sfd, tcpproto->p_proto, TCP_NODELAY, (char *)&on, sizeof(int));

  if (connect(sfd, (struct sockaddr *)&s, sizeof(s)) == 0)	/* connected */
    return TRUE;

  if (hostname)	/* connect to specified server failed */
    return FALSE;

  printf("No rocksndiamonds server on localhost - starting up one ...\n");
  StartNetworkServer(port);

  /* wait for server to start up and try connecting several times */
  for (i = 0; i < 6; i++)
  {
    Delay(500);		/* wait 500 ms == 0.5 seconds */
    close(sfd);

    sfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
      Error(ERR_EXIT, "out of file descriptors");

    setsockopt(sfd, tcpproto->p_proto, TCP_NODELAY, (char *)&on, sizeof(int));

    if (connect(sfd, (struct sockaddr *)&s, sizeof(s)) >= 0)	/* connected */
      return TRUE;
  }

  /* when reaching this point, connect to newly started server has failed */
  return FALSE;
}

void SendToServer_PlayerName(char *player_name)
{
  int len_player_name = strlen(player_name);

  buffer[1] = OP_PLAYER_NAME;
  memcpy(&buffer[2], player_name, len_player_name);
  SendBufferToServer(2 + len_player_name);
  Error(ERR_NETWORK_CLIENT, "you set your player name to \"%s\"", player_name);
}

void SendToServer_ProtocolVersion()
{
  buffer[1] = OP_PROTOCOL_VERSION;
  buffer[2] = PROTOCOL_VERSION_1;
  buffer[3] = PROTOCOL_VERSION_2;
  buffer[4] = PROTOCOL_VERSION_3;

  SendBufferToServer(5);
}

void SendToServer_NrWanted(int nr_wanted)
{
  buffer[1] = OP_NUMBER_WANTED;
  buffer[2] = nr_wanted;

  SendBufferToServer(3);
}

void SendToServer_StartPlaying()
{
  unsigned long new_random_seed = InitRND(NEW_RANDOMIZE);

  int dummy = 0;		/* !!! HAS NO MEANING ANYMORE !!! */
				/* the name of the level must be enough */

  buffer[1] = OP_START_PLAYING;
  buffer[2] = (byte)(level_nr >> 8);
  buffer[3] = (byte)(level_nr & 0xff);
  buffer[4] = (byte)(dummy >> 8);
  buffer[5] = (byte)(dummy & 0xff);

  buffer[6] = (unsigned char)((new_random_seed >> 24) & 0xff);
  buffer[7] = (unsigned char)((new_random_seed >> 16) & 0xff);
  buffer[8] = (unsigned char)((new_random_seed >>  8) & 0xff);
  buffer[9] = (unsigned char)((new_random_seed >>  0) & 0xff);

  strcpy((char *)&buffer[10], leveldir_current->identifier);

  SendBufferToServer(10 + strlen(leveldir_current->identifier) + 1);
}

void SendToServer_PausePlaying()
{
  buffer[1] = OP_PAUSE_PLAYING;

  SendBufferToServer(2);
}

void SendToServer_ContinuePlaying()
{
  buffer[1] = OP_CONTINUE_PLAYING;

  SendBufferToServer(2);
}

void SendToServer_StopPlaying()
{
  buffer[1] = OP_STOP_PLAYING;

  SendBufferToServer(2);
}

void SendToServer_MovePlayer(byte player_action)
{
  buffer[1] = OP_MOVE_PLAYER;
  buffer[2] = player_action;

  SendBufferToServer(3);
}

static void Handle_OP_BAD_PROTOCOL_VERSION()
{
  Error(ERR_WARN, "protocol version mismatch");
  Error(ERR_EXIT, "server expects %d.%d.x instead of %d.%d.%d",
	buffer[2], buffer[3],
	PROTOCOL_VERSION_1, PROTOCOL_VERSION_2, PROTOCOL_VERSION_3);
}

static void Handle_OP_YOUR_NUMBER()
{
  int new_client_nr = buffer[2];
  int new_index_nr = new_client_nr - 1;
  struct PlayerInfo *old_local_player = local_player;
  struct PlayerInfo *new_local_player = &stored_player[new_index_nr];

  printf("OP_YOUR_NUMBER: %d\n", buffer[0]);
  first_player.nr = new_client_nr;

  if (old_local_player != new_local_player)
  {
    /* copy existing player settings and change to new player */

    *new_local_player = *old_local_player;
    old_local_player->connected = FALSE;
    local_player = new_local_player;
  }

  if (first_player.nr > MAX_PLAYERS)
    Error(ERR_EXIT, "sorry - no more than %d players", MAX_PLAYERS);

  Error(ERR_NETWORK_CLIENT, "you get client # %d", new_client_nr);
}

static void Handle_OP_NUMBER_WANTED()
{
  int client_nr_wanted = buffer[2];
  int old_client_nr = buffer[0];
  int new_client_nr = buffer[3];
  int old_index_nr = old_client_nr - 1;
  int new_index_nr = new_client_nr - 1;
  int index_nr_wanted = client_nr_wanted - 1;
  struct PlayerInfo *old_player = &stored_player[old_index_nr];
  struct PlayerInfo *new_player = &stored_player[new_index_nr];

  printf("OP_NUMBER_WANTED: %d\n", buffer[0]);

  if (new_client_nr == client_nr_wanted)	/* switching succeeded */
  {
    struct NetworkClientPlayerInfo *player;

    if (old_client_nr != client_nr_wanted)	/* client's nr has changed */
      Error(ERR_NETWORK_CLIENT, "client %d switches to # %d",
	    old_client_nr, new_client_nr);
    else if (old_client_nr == first_player.nr)	/* local player keeps his nr */
      Error(ERR_NETWORK_CLIENT, "keeping client # %d", new_client_nr);

    if (old_client_nr != new_client_nr)
    {
      /* copy existing player settings and change to new player */

      *new_player = *old_player;
      old_player->connected = FALSE;
    }

    player = getNetworkPlayer(old_client_nr);
    player->nr = new_client_nr;

    if (old_player == local_player)		/* local player switched */
      local_player = new_player;
  }
  else if (old_client_nr == first_player.nr)	/* failed -- local player? */
  {
    char *color[] = { "yellow", "red", "green", "blue" };
    char request[100];

    sprintf(request, "Sorry ! %s player still exists ! You are %s player !",
	    color[index_nr_wanted], color[new_index_nr]);
    Request(request, REQ_CONFIRM);

    Error(ERR_NETWORK_CLIENT, "cannot switch -- you keep client # %d",
	  new_client_nr);
  }
}

static void Handle_OP_PLAYER_NAME(unsigned int len)
{
  struct NetworkClientPlayerInfo *player;
  int player_nr = (int)buffer[0];

  printf("OP_PLAYER_NAME: %d\n", player_nr);
  player = getNetworkPlayer(player_nr);
  buffer[len] = 0;
  Error(ERR_NETWORK_CLIENT, "client %d calls itself \"%s\"",
	buffer[0], &buffer[2]);
  strncpy(player->name, (char *)&buffer[2], MAX_PLAYER_NAME_LEN);
}

static void Handle_OP_PLAYER_CONNECTED()
{
  struct NetworkClientPlayerInfo *player, *last_player = NULL;
  int new_client_nr = (int)buffer[0];
  int new_index_nr = new_client_nr - 1;

  printf("OP_PLAYER_CONNECTED: %d\n", new_client_nr);
  Error(ERR_NETWORK_CLIENT, "new client %d connected", new_client_nr);

  for (player = &first_player; player; player = player->next)
  {
    if (player->nr == new_client_nr)
      Error(ERR_EXIT, "multiplayer server sent duplicate player id");

    last_player = player;
  }

  last_player->next = player =
    checked_malloc(sizeof(struct NetworkClientPlayerInfo));
  player->nr = new_client_nr;
  player->name[0] = '\0';
  player->next = NULL;

  stored_player[new_index_nr].connected = TRUE;
}

static void Handle_OP_PLAYER_DISCONNECTED()
{
  struct NetworkClientPlayerInfo *player, *player_disconnected;
  int player_nr = (int)buffer[0];

  printf("OP_PLAYER_DISCONNECTED: %d\n", player_nr);
  player_disconnected = getNetworkPlayer(player_nr);
  Error(ERR_NETWORK_CLIENT, "client %d (%s) disconnected",
	player_nr, getNetworkPlayerName(buffer[0]));

  for (player = &first_player; player; player = player->next)
    if (player->next == player_disconnected)
      player->next = player_disconnected->next;
  free(player_disconnected);
}

static void Handle_OP_START_PLAYING()
{
  LevelDirTree *new_leveldir;
  int new_level_nr;
  int dummy;				/* !!! HAS NO MEANING ANYMORE !!! */
  unsigned long new_random_seed;
  char *new_leveldir_identifier;

  new_level_nr = (buffer[2] << 8) + buffer[3];
  dummy = (buffer[4] << 8) + buffer[5];
  new_random_seed =
    (buffer[6] << 24) | (buffer[7] << 16) | (buffer[8] << 8) | (buffer[9]);
  new_leveldir_identifier = (char *)&buffer[10];

  new_leveldir = getTreeInfoFromIdentifier(leveldir_first,
					   new_leveldir_identifier);
  if (new_leveldir == NULL)
  {
    Error(ERR_WARN, "no such level identifier: '%s'", new_leveldir_identifier);

    new_leveldir = leveldir_first;
    Error(ERR_WARN, "using default level set: '%s'", new_leveldir->identifier);
  }

  printf("OP_START_PLAYING: %d\n", buffer[0]);
  Error(ERR_NETWORK_CLIENT,
	"client %d starts game [level %d from level identifier '%s']\n",
	buffer[0], new_level_nr, new_leveldir->identifier);

  leveldir_current = new_leveldir;
  level_nr = new_level_nr;

  TapeErase();
  LoadTape(level_nr);
  LoadLevel(level_nr);

  if (setup.autorecord)
    TapeStartRecording();

  if (tape.recording)
    tape.random_seed = new_random_seed;

  InitRND(new_random_seed);

  game_status = GAME_MODE_PLAYING;
  InitGame();
}

static void Handle_OP_PAUSE_PLAYING()
{
  printf("OP_PAUSE_PLAYING: %d\n", buffer[0]);
  Error(ERR_NETWORK_CLIENT, "client %d pauses game", buffer[0]);

  tape.pausing = TRUE;
  DrawVideoDisplay(VIDEO_STATE_PAUSE_ON,0);
}

static void Handle_OP_CONTINUE_PLAYING()
{
  printf("OP_CONTINUE_PLAYING: %d\n", buffer[0]);
  Error(ERR_NETWORK_CLIENT, "client %d continues game", buffer[0]);

  tape.pausing = FALSE;
  DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
}

static void Handle_OP_STOP_PLAYING()
{
  printf("OP_STOP_PLAYING: %d\n", buffer[0]);
  Error(ERR_NETWORK_CLIENT, "client %d stops game", buffer[0]);

  game_status = GAME_MODE_MAIN;
  DrawMainMenu();
}

static void Handle_OP_MOVE_PLAYER(unsigned int len)
{
  int server_frame_counter;
  int i;

  if (!network_playing)
    return;

  server_frame_counter =
    (buffer[2] << 24) | (buffer[3] << 16) | (buffer[4] << 8) | (buffer[5]);

  if (server_frame_counter != FrameCounter)
  {
    Error(ERR_RETURN, "client and servers frame counters out of sync");
    Error(ERR_RETURN, "frame counter of client is %d", FrameCounter);
    Error(ERR_RETURN, "frame counter of server is %d", server_frame_counter);
    Error(ERR_EXIT,   "this should not happen -- please debug");
  }

  /* copy valid player actions */
  for (i = 0; i < MAX_PLAYERS; i++)
    stored_player[i].effective_action =
      (i < len - 6 && stored_player[i].active ? buffer[6 + i] : 0);

  network_player_action_received = TRUE;
}

static void HandleNetworkingMessages()
{
  unsigned int message_length;

  while (nread >= 4 && nread >= 4 + readbuffer[3])
  {
    message_length = readbuffer[3];
    if (readbuffer[0] || readbuffer[1] || readbuffer[2])
      Error(ERR_EXIT, "wrong network server line length");

    memcpy(buffer, &readbuffer[4], message_length);
    nread -= 4 + message_length;
    memmove(readbuffer, readbuffer + 4 + message_length, nread);

    switch(buffer[1])
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

      case OP_PLAYER_NAME:
	Handle_OP_PLAYER_NAME(message_length);
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

      case OP_MOVE_PLAYER:
	Handle_OP_MOVE_PLAYER(message_length);
	break;

      case OP_BROADCAST_MESSAGE:
	printf("OP_BROADCAST_MESSAGE: %d\n", buffer[0]);
	Error(ERR_NETWORK_CLIENT, "client %d sends message", buffer[0]);
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

    r = read(sfd, readbuffer + nread, MAX_BUFFER_SIZE - nread);

    if (r < 0)
      Error(ERR_EXIT, "error reading from network server");

    if (r == 0)
      Error(ERR_EXIT, "connection to network server lost");

    nread += r;

    HandleNetworkingMessages();
  }
}

#endif /* PLATFORM_UNIX */
