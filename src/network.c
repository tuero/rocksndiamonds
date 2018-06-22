// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// network.c
// ============================================================================

#include <signal.h>
#include <sys/time.h>

#include "libgame/libgame.h"

#include "network.h"
#include "netserv.h"
#include "main.h"
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

static TCPsocket sfd;		/* TCP server socket */
static UDPsocket udp;		/* UDP server socket */
static SDLNet_SocketSet rfds;	/* socket set */

static byte realbuffer[512];
static byte readbuffer[MAX_BUFFER_SIZE], writbuffer[MAX_BUFFER_SIZE];
static byte *buffer = realbuffer + 4;
static int nread = 0, nwrite = 0;
static boolean stop_network_game = FALSE;

static void SendBufferToServer(int size)
{
  if (!network.enabled)
    return;

  realbuffer[0] = realbuffer[1] = realbuffer[2] = 0;
  realbuffer[3] = (byte)size;
  buffer[0] = 0;

  if (nwrite + 4 + size >= MAX_BUFFER_SIZE)
    Error(ERR_EXIT, "internal error: network send buffer overflow");

  memcpy(writbuffer + nwrite, realbuffer, 4 + size);
  nwrite += 4 + size;

  /* directly send the buffer to the network server */
  SDLNet_TCP_Send(sfd, writbuffer, nwrite);
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
      if (player->nr == player_nr && strlen(player->name) > 0)
	return(player->name);

  return(EMPTY_PLAYER_NAME);
}

static void StartNetworkServer(int port)
{
  static int p;

  p = port;
#if defined(TARGET_SDL2)
  server_thread = SDL_CreateThread(NetworkServerThread,
				   "NetworkServerThread", &p);
#else
  server_thread = SDL_CreateThread(NetworkServerThread, &p);
#endif
  network_server = TRUE;
}

boolean ConnectToServer(char *hostname, int port)
{
  IPaddress ip;
  int server_host = 0;
  int i;

  if (port == 0)
    port = DEFAULT_SERVER_PORT;

  if (hostname == NULL)
  {
    // if no hostname given, try to auto-detect network server in local network
    // by doing a UDP broadcast on the network server port and wait for answer

    SDLNet_SocketSet udp_socket_set = SDLNet_AllocSocketSet(1);
    if (!udp_socket_set)
      Error(ERR_EXIT, "SDLNet_AllocSocketSet() failed: %s"), SDLNet_GetError();

    udp = SDLNet_UDP_Open(0);
    if(!udp)
      Error(ERR_EXIT, "SDLNet_UDP_Open() failed: %s", SDLNet_GetError());

    if (SDLNet_UDP_AddSocket(udp_socket_set, udp) == -1)
      Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_TCP_AddSocket() failed: %s"),
        SDLNet_GetError();

    char *data_ptr = "network server UDB broadcast";
    int data_len = strlen(data_ptr) + 1;
    IPaddress ip_address;

    SDLNet_Write32(0xffffffff, &ip_address.host);	/* 255.255.255.255 */
    SDLNet_Write16(port,       &ip_address.port);

    UDPpacket packet =
    {
      -1,
      (Uint8 *)data_ptr,
      data_len,
      data_len,
      0,
      ip_address
    };

    SDLNet_UDP_Send(udp, -1, &packet);

    Error(ERR_DEBUG, "doing UDP broadcast for local network server ...");

    if (SDLNet_CheckSockets(udp_socket_set, 1000) == 1)
    {
      int num_packets = SDLNet_UDP_Recv(udp, &packet);

      if (num_packets == 1)
      {
        Error(ERR_DEBUG, "network server found");

        server_host = SDLNet_Read32(&packet.address.host);
      }
      else
      {
	Error(ERR_DEBUG, "no answer from network server");
      }
    }
    else
    {
      Error(ERR_DEBUG, "no network server found");
    }
  }

  rfds = SDLNet_AllocSocketSet(1);

  if (hostname)
  {
    SDLNet_ResolveHost(&ip, hostname, port);

    if (ip.host == INADDR_NONE)
      Error(ERR_EXIT, "cannot locate host '%s'", hostname);
    else
      server_host = SDLNet_Read32(&ip.host);
  }
  else
  {
    // if no hostname was given and no network server was auto-detected in the
    // local network, try to connect to a network server at the local host
    if (server_host == 0)
      server_host = 0x7f000001;			/* 127.0.0.1 */

    SDLNet_Write32(server_host, &ip.host);
    SDLNet_Write16(port,        &ip.port);
  }

  Error(ERR_DEBUG, "trying to connect to network server at %d.%d.%d.%d ...",
        (server_host >> 24) & 0xff,
        (server_host >> 16) & 0xff,
        (server_host >>  8) & 0xff,
        (server_host >>  0) & 0xff);

  sfd = SDLNet_TCP_Open(&ip);

  if (sfd)
  {
    SDLNet_TCP_AddSocket(rfds, sfd);
    return TRUE;
  }
  else
  {
    printf("SDLNet_TCP_Open(): %s\n", SDLNet_GetError());
  }

  if (hostname)			/* connect to specified server failed */
    return FALSE;

  printf("No rocksndiamonds server on localhost -- starting up one ...\n");
  StartNetworkServer(port);

  /* wait for server to start up and try connecting several times */
  for (i = 0; i < 6; i++)
  {
    Delay(500);			/* wait 500 ms == 0.5 seconds */

    if ((sfd = SDLNet_TCP_Open(&ip)))		/* connected */
    {
      SDLNet_TCP_AddSocket(rfds, sfd);
      return TRUE;
    }
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
  unsigned int new_random_seed = InitRND(level.random_seed);

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

void SendToServer_StopPlaying(int cause_for_stopping)
{
  buffer[1] = OP_STOP_PLAYING;
  buffer[2] = cause_for_stopping;

  SendBufferToServer(3);
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
    /* set relevant player settings and change to new player */

    local_player = new_local_player;

    old_local_player->connected_locally = FALSE;
    new_local_player->connected_locally = TRUE;

    old_local_player->connected_network = FALSE;
    new_local_player->connected_network = TRUE;
  }

  if (first_player.nr > MAX_PLAYERS)
    Error(ERR_EXIT, "sorry, more than %d players not allowed", MAX_PLAYERS);

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
      /* set relevant player settings and change to new player */

      old_player->connected_network = FALSE;
      new_player->connected_network = TRUE;
    }

    player = getNetworkPlayer(old_client_nr);
    player->nr = new_client_nr;

    if (old_player == local_player)		/* local player switched */
    {
      local_player = new_player;

      old_player->connected_locally = FALSE;
      new_player->connected_locally = TRUE;
    }
  }
  else if (old_client_nr == first_player.nr)	/* failed -- local player? */
  {
    char request[100];

    sprintf(request, "Sorry! Player %d already exists! You are player %d!",
	    index_nr_wanted + 1, new_index_nr + 1);

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

  stored_player[new_index_nr].connected_network = TRUE;
}

static void Handle_OP_PLAYER_DISCONNECTED()
{
  struct NetworkClientPlayerInfo *player, *player_disconnected;
  int player_nr = (int)buffer[0];
  int index_nr = player_nr - 1;

  printf("OP_PLAYER_DISCONNECTED: %d\n", player_nr);
  player_disconnected = getNetworkPlayer(player_nr);
  Error(ERR_NETWORK_CLIENT, "client %d (%s) disconnected",
	player_nr, getNetworkPlayerName(buffer[0]));

  for (player = &first_player; player; player = player->next)
    if (player->next == player_disconnected)
      player->next = player_disconnected->next;
  free(player_disconnected);

  stored_player[index_nr].connected_locally = FALSE;
  stored_player[index_nr].connected_network = FALSE;
}

static void Handle_OP_START_PLAYING()
{
  LevelDirTree *new_leveldir;
  int new_level_nr;
  unsigned int new_random_seed;
  char *new_leveldir_identifier;

  new_level_nr = (buffer[2] << 8) + buffer[3];
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

  StartGameActions(FALSE, setup.autorecord, new_random_seed);
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
  printf("OP_STOP_PLAYING: %d [%d]\n", buffer[0], buffer[2]);
  Error(ERR_NETWORK_CLIENT, "client %d stops game [%d]", buffer[0], buffer[2]);

  if (game_status == GAME_MODE_PLAYING)
  {
    int client_nr = buffer[0];
    int index_nr = client_nr - 1;
    struct PlayerInfo *client_player = &stored_player[index_nr];
    boolean stopped_by_remote_player = (!client_player->connected_locally);
    char *message = (buffer[2] == NETWORK_STOP_BY_PLAYER ?
		     "Network game stopped by player!" :
		     buffer[2] == NETWORK_STOP_BY_ERROR ?
		     "Network game stopped due to internal error!" :
		     "Network game stopped!");

    if (buffer[2] != NETWORK_STOP_BY_PLAYER || stopped_by_remote_player)
      Request(message, REQ_CONFIRM | REQ_STAY_CLOSED);
  }

  SetGameStatus(GAME_MODE_MAIN);

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
    Error(ERR_INFO, "client and servers frame counters out of sync");
    Error(ERR_INFO, "frame counter of client is %d", FrameCounter);
    Error(ERR_INFO, "frame counter of server is %d", server_frame_counter);
    Error(ERR_INFO, "this should not happen -- please debug");

    stop_network_game = TRUE;

    return;
  }

  /* copy valid player actions */
  for (i = 0; i < MAX_PLAYERS; i++)
    stored_player[i].effective_action =
      (i < len - 6 ? buffer[6 + i] : 0);

  network_player_action_received = TRUE;
}

static void HandleNetworkingMessages()
{
  unsigned int message_length;

  stop_network_game = FALSE;

  while (nread >= 4 && nread >= 4 + readbuffer[3])
  {
    message_length = readbuffer[3];
    if (readbuffer[0] || readbuffer[1] || readbuffer[2])
      Error(ERR_EXIT, "wrong network server line length");

    memcpy(buffer, &readbuffer[4], message_length);
    nread -= 4 + message_length;
    memmove(readbuffer, readbuffer + 4 + message_length, nread);

    switch (buffer[1])
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

  /* in case of internal error, stop network game */
  if (stop_network_game)
    SendToServer_StopPlaying(NETWORK_STOP_BY_ERROR);
}

static char *HandleNetworkingPackets()
{
  while (1)
  {
    /* ---------- check network server for activity ---------- */

    int num_active_sockets = SDLNet_CheckSockets(rfds, 1);

    if (num_active_sockets < 0)
      return "Error checking network sockets!";

    if (num_active_sockets == 0)
      break;	// no active sockets, stop here

    /* ---------- read packets from network server ---------- */

    int num_bytes = SDLNet_TCP_Recv(sfd, readbuffer + nread, 1);

    if (num_bytes < 0)
      return "Error reading from network server!";

    if (num_bytes == 0)
      return "Connection to network server lost!";

    nread += num_bytes;

    HandleNetworkingMessages();
  }

  return NULL;
}

static void HandleNetworkingDisconnect()
{
  int i;

  SDLNet_TCP_DelSocket(rfds, sfd);
  SDLNet_TCP_Close(sfd);

  network_playing = FALSE;

  network.enabled = FALSE;
  network.connected = FALSE;

  setup.network_mode = FALSE;

  for (i = 0; i < MAX_PLAYERS; i++)
    stored_player[i].connected_network = FALSE;
}

void HandleNetworking()
{
  char *error_message = HandleNetworkingPackets();

  if (error_message != NULL)
  {
    HandleNetworkingDisconnect();

    if (game_status == GAME_MODE_PLAYING)
    {
      Request(error_message, REQ_CONFIRM | REQ_STAY_CLOSED);

      SetGameStatus(GAME_MODE_MAIN);

      DrawMainMenu();
    }
    else
    {
      Request(error_message, REQ_CONFIRM);
    }
  }
}
