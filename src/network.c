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
#include "init.h"

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

// server stuff

static TCPsocket sfd;		// TCP server socket
static UDPsocket udp;		// UDP server socket
static SDLNet_SocketSet rfds;	// socket set

static struct NetworkBuffer *read_buffer = NULL;
static struct NetworkBuffer *write_buffer = NULL;

static boolean stop_network_game = FALSE;
static boolean stop_network_client = FALSE;
static char stop_network_client_message[MAX_OUTPUT_LINESIZE + 1];

static struct NetworkLevelInfo network_level;

static void DrawNetworkTextExt(char *message, int font_nr, boolean initialize)
{
  static int xpos = 0, ypos = 0;
  static int max_line_width = 0;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int ypos_1 = 120;
  int ypos_2 = 150;

  if (initialize)
  {
    if (game_status == GAME_MODE_LOADING)
    {
      max_line_width = WIN_XSIZE;

      xpos = (max_line_width - getTextWidth(message, font_nr)) / 2;
      ypos = ypos_1;

      DrawText(xpos, ypos, message, font_nr);

      xpos = 0;
      ypos = ypos_2;
    }
    else
    {
      max_line_width = SXSIZE;

      DrawTextSCentered(ypos_1, font_nr, message);

      // calculate offset to x position caused by rounding
      int max_chars_per_line = max_line_width / font_width;
      int xoffset = (max_line_width - max_chars_per_line * font_width) / 2;

      xpos = SX + xoffset;
      ypos = SY + ypos_2;
    }

    Error(ERR_DEBUG, "========== %s ==========", message);
  }
  else
  {
    int max_chars_per_line = max_line_width / font_width;
    int max_lines_per_text = 10;
    int num_lines_spacing = (font_nr == FC_YELLOW ? 1 : 3);
    int num_lines_printed = DrawTextBuffer(xpos, ypos, message, font_nr,
					   max_chars_per_line, -1,
					   max_lines_per_text, 0, -1,
					   TRUE, TRUE, FALSE);

    ypos += (num_lines_printed + num_lines_spacing) * font_height;

    Error(ERR_DEBUG, "%s", message);
  }

  BackToFront();
}

static void DrawNetworkText(char *message)
{
  DrawNetworkTextExt(message, FC_YELLOW, FALSE);
}

static void DrawNetworkText_Success(char *message)
{
  DrawNetworkTextExt(message, FC_GREEN, FALSE);
}

static void DrawNetworkText_Failed(char *message)
{
  DrawNetworkTextExt(message, FC_RED, FALSE);
}

static void DrawNetworkText_Title(char *message)
{
  DrawNetworkTextExt(message, FC_GREEN, TRUE);
}

static void SendNetworkBufferToServer(struct NetworkBuffer *nb)
{
  if (!network.enabled)
    return;

  // set message length header
  putNetwork32BitInteger(nb->buffer, nb->size - 4);

  // directly send the buffer to the network server
  SDLNet_TCP_Send(sfd, nb->buffer, nb->size);
}

static struct NetworkClientPlayerInfo *getNetworkPlayer(int player_nr)
{
  struct NetworkClientPlayerInfo *player = NULL;

  for (player = &first_player; player; player = player->next)
    if (player->nr == player_nr)
      break;

  if (player == NULL)	// should not happen
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

boolean hasStartedNetworkGame(void)
{
  return !network_level.use_network_level_files;
}

static boolean hasPathSeparator(char *s)
{
  return (strchr(s, '/') != NULL);
}

static void StartNetworkServer(int port)
{
  static int p;

  p = port;

  server_thread = SDL_CreateThread(NetworkServerThread,
				   "NetworkServerThread", &p);
  network_server = TRUE;
}

boolean ConnectToServer(char *hostname, int port)
{
  IPaddress ip;
  int server_host = 0;
  int i;

  if (read_buffer == NULL)
    read_buffer = newNetworkBuffer();

  if (write_buffer == NULL)
    write_buffer = newNetworkBuffer();

  DrawNetworkText_Title("Initializing Network");

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

    SDLNet_Write32(0xffffffff, &ip_address.host);	// 255.255.255.255
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

    DrawNetworkText("Looking for nearby network server ...");

    // wait for any nearby network server to answer UDP broadcast
    for (i = 0; i < 5; i++)
    {
      if (SDLNet_CheckSockets(udp_socket_set, 0) == 1)
      {
	int num_packets = SDLNet_UDP_Recv(udp, &packet);

	if (num_packets == 1)
	{
	  char message[100];

	  server_host = SDLNet_Read32(&packet.address.host);

	  sprintf(message, "Network server found at %d.%d.%d.%d!",
		  (server_host >> 24) & 0xff,
		  (server_host >> 16) & 0xff,
		  (server_host >>  8) & 0xff,
		  (server_host >>  0) & 0xff);

	  DrawNetworkText_Success(message);
	}
	else
	{
	  DrawNetworkText_Failed("No answer from network server!");
	}

	break;
      }
      else
      {
	Delay_WithScreenUpdates(100);
      }
    }

    if (server_host == 0)
      DrawNetworkText_Failed("No nearby network server found!");
  }

  rfds = SDLNet_AllocSocketSet(1);

  if (hostname)
  {
    char message[100];

    SDLNet_ResolveHost(&ip, hostname, port);

    if (ip.host == INADDR_NONE)
    {
      sprintf(message, "Failed to resolve network server hostname '%s'!",
	      hostname);

      DrawNetworkText_Failed(message);

      return FALSE;
    }
    else
    {
      server_host = SDLNet_Read32(&ip.host);
    }

    sprintf(message, "Connecting to network server host %s ...", hostname);

    DrawNetworkText(message);
  }
  else
  {
    // if no hostname was given and no network server was auto-detected in the
    // local network, try to connect to a network server at the local host
    if (server_host == 0)
    {
      server_host = 0x7f000001;			// 127.0.0.1

      DrawNetworkText("Looking for local network server ...");
    }
    else
    {
      DrawNetworkText("Connecting to network server ...");
    }

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

    DrawNetworkText_Success("Successfully connected!");

    return TRUE;
  }
  else
  {
    if (hostname)
      DrawNetworkText_Failed("Failed to connect to network server!");
    else
      DrawNetworkText_Failed("No local network server found!");

    printf("SDLNet_TCP_Open(): %s\n", SDLNet_GetError());
  }

  if (hostname)			// connect to specified server failed
    return FALSE;

  DrawNetworkText("Starting new local network server ...");

  StartNetworkServer(port);

  // wait for server to start up and try connecting several times
  for (i = 0; i < 30; i++)
  {
    if ((sfd = SDLNet_TCP_Open(&ip)))		// connected
    {
      DrawNetworkText_Success("Successfully connected to newly started network server!");

      SDLNet_TCP_AddSocket(rfds, sfd);

      return TRUE;
    }

    Delay_WithScreenUpdates(100);
  }

  DrawNetworkText_Failed("Failed to connect to newly started network server!");

  // when reaching this point, connect to newly started server has failed
  return FALSE;
}

void SendToServer_PlayerName(char *player_name)
{
  initNetworkBufferForWriting(write_buffer, OP_PLAYER_NAME, 0);

  putNetworkBufferString(write_buffer, player_name);

  SendNetworkBufferToServer(write_buffer);

  Error(ERR_NETWORK_CLIENT, "you set your player name to \"%s\"", player_name);
}

void SendToServer_ProtocolVersion(void)
{
  initNetworkBufferForWriting(write_buffer, OP_PROTOCOL_VERSION, 0);

  putNetworkBuffer8BitInteger(write_buffer, PROTOCOL_VERSION_MAJOR);
  putNetworkBuffer8BitInteger(write_buffer, PROTOCOL_VERSION_MINOR);
  putNetworkBuffer8BitInteger(write_buffer, PROTOCOL_VERSION_PATCH);

  SendNetworkBufferToServer(write_buffer);
}

void SendToServer_NrWanted(int nr_wanted)
{
  initNetworkBufferForWriting(write_buffer, OP_NUMBER_WANTED, 0);

  putNetworkBuffer8BitInteger(write_buffer, nr_wanted);

  SendNetworkBufferToServer(write_buffer);
}

void SendToServer_LevelFile(void)
{
  initNetworkBufferForWriting(write_buffer, OP_LEVEL_FILE, 0);

  putNetworkBufferString(      write_buffer, leveldir_current->identifier);
  putNetworkBuffer16BitInteger(write_buffer, level.file_info.nr);
  putNetworkBuffer8BitInteger( write_buffer, level.file_info.type);
  putNetworkBuffer8BitInteger( write_buffer, level.file_info.packed);
  putNetworkBufferString(      write_buffer, level.file_info.basename);
  putNetworkBufferFile(        write_buffer, level.file_info.filename);
  putNetworkBuffer8BitInteger( write_buffer, level.use_custom_template);

  if (level.use_custom_template)
  {
    putNetworkBufferString(write_buffer, level_template.file_info.basename);
    putNetworkBufferFile(  write_buffer, level_template.file_info.filename);
  }

  SendNetworkBufferToServer(write_buffer);

  setString(&network_level.leveldir_identifier, leveldir_current->identifier);

  // the sending client does not use network level files (but the real ones)
  network_level.use_network_level_files = FALSE;

#if 0
  printf("::: '%s'\n", leveldir_current->identifier);
  printf("::: '%d'\n", level.file_info.nr);
  printf("::: '%d'\n", level.file_info.type);
  printf("::: '%d'\n", level.file_info.packed);
  printf("::: '%s'\n", level.file_info.basename);
  printf("::: '%s'\n", level.file_info.filename);

  if (level.use_custom_template)
    printf("::: '%s'\n", level_template.file_info.filename);
#endif
}

void SendToServer_StartPlaying(void)
{
  unsigned int new_random_seed = InitRND(level.random_seed);

  initNetworkBufferForWriting(write_buffer, OP_START_PLAYING, 0);

  putNetworkBufferString(      write_buffer, leveldir_current->identifier);
  putNetworkBuffer16BitInteger(write_buffer, level_nr);
  putNetworkBuffer32BitInteger(write_buffer, new_random_seed);

  SendNetworkBufferToServer(write_buffer);
}

void SendToServer_PausePlaying(void)
{
  initNetworkBufferForWriting(write_buffer, OP_PAUSE_PLAYING, 0);

  SendNetworkBufferToServer(write_buffer);
}

void SendToServer_ContinuePlaying(void)
{
  initNetworkBufferForWriting(write_buffer, OP_CONTINUE_PLAYING, 0);

  SendNetworkBufferToServer(write_buffer);
}

void SendToServer_StopPlaying(int cause_for_stopping)
{
  initNetworkBufferForWriting(write_buffer, OP_STOP_PLAYING, 0);

  putNetworkBuffer8BitInteger(write_buffer, cause_for_stopping);

  SendNetworkBufferToServer(write_buffer);
}

void SendToServer_MovePlayer(byte player_action)
{
  initNetworkBufferForWriting(write_buffer, OP_MOVE_PLAYER, 0);

  putNetworkBuffer8BitInteger(write_buffer, player_action);

  SendNetworkBufferToServer(write_buffer);
}

static void Handle_OP_BAD_PROTOCOL_VERSION(void)
{
  int protocol_version_major = getNetworkBuffer8BitInteger(read_buffer);
  int protocol_version_minor = getNetworkBuffer8BitInteger(read_buffer);

  Error(ERR_WARN, "protocol version mismatch");
  Error(ERR_WARN, "server expects %d.%d.x instead of %d.%d.%d",
	protocol_version_major,
	protocol_version_minor,
	PROTOCOL_VERSION_MAJOR,
	PROTOCOL_VERSION_MINOR,
	PROTOCOL_VERSION_PATCH);

  sprintf(stop_network_client_message, "Network protocol version mismatch! Server expects version %d.%d.x instead of %d.%d.%d!",
	  protocol_version_major,
	  protocol_version_minor,
	  PROTOCOL_VERSION_MAJOR,
	  PROTOCOL_VERSION_MINOR,
	  PROTOCOL_VERSION_PATCH);

  stop_network_client = TRUE;
}

static void Handle_OP_YOUR_NUMBER(void)
{
  int old_client_nr = getNetworkBuffer8BitInteger(read_buffer);
  int new_client_nr = getNetworkBuffer8BitInteger(read_buffer);
  int new_index_nr = new_client_nr - 1;
  struct PlayerInfo *old_local_player = local_player;
  struct PlayerInfo *new_local_player = &stored_player[new_index_nr];

  printf("OP_YOUR_NUMBER: %d\n", old_client_nr);
  first_player.nr = new_client_nr;

  if (old_local_player != new_local_player)
  {
    // set relevant player settings and change to new player

    local_player = new_local_player;

    old_local_player->connected_locally = FALSE;
    new_local_player->connected_locally = TRUE;

    old_local_player->connected_network = FALSE;
    new_local_player->connected_network = TRUE;
  }

  if (first_player.nr > MAX_PLAYERS)
    Error(ERR_EXIT, "sorry, more than %d players not allowed", MAX_PLAYERS);

  Error(ERR_NETWORK_CLIENT, "you get client # %d", new_client_nr);

  stored_player[new_index_nr].connected_network = TRUE;
}

static void Handle_OP_NUMBER_WANTED(void)
{
  int old_client_nr    = getNetworkBuffer8BitInteger(read_buffer);
  int client_nr_wanted = getNetworkBuffer8BitInteger(read_buffer);
  int new_client_nr    = getNetworkBuffer8BitInteger(read_buffer);
  int old_index_nr = old_client_nr - 1;
  int new_index_nr = new_client_nr - 1;
  int index_nr_wanted = client_nr_wanted - 1;
  struct PlayerInfo *old_player = &stored_player[old_index_nr];
  struct PlayerInfo *new_player = &stored_player[new_index_nr];

  printf("OP_NUMBER_WANTED: %d\n", old_client_nr);

  if (new_client_nr == client_nr_wanted)	// switching succeeded
  {
    struct NetworkClientPlayerInfo *player;

    if (old_client_nr != client_nr_wanted)	// client's nr has changed
      Error(ERR_NETWORK_CLIENT, "client %d switches to # %d",
	    old_client_nr, new_client_nr);
    else if (old_client_nr == first_player.nr)	// local player keeps his nr
      Error(ERR_NETWORK_CLIENT, "keeping client # %d", new_client_nr);

    if (old_client_nr != new_client_nr)
    {
      // set relevant player settings and change to new player

      old_player->connected_network = FALSE;
      new_player->connected_network = TRUE;
    }

    player = getNetworkPlayer(old_client_nr);
    player->nr = new_client_nr;

    if (old_player == local_player)		// local player switched
    {
      local_player = new_player;

      old_player->connected_locally = FALSE;
      new_player->connected_locally = TRUE;
    }
  }
  else if (old_client_nr == first_player.nr)	// failed -- local player?
  {
    char request[100];

    sprintf(request, "Sorry! Player %d already exists! You are player %d!",
	    index_nr_wanted + 1, new_index_nr + 1);

    Request(request, REQ_CONFIRM);

    Error(ERR_NETWORK_CLIENT, "cannot switch -- you keep client # %d",
	  new_client_nr);
  }

  if (game_status == GAME_MODE_MAIN)
    DrawNetworkPlayers();
}

static void Handle_OP_PLAYER_NAME(void)
{
  int player_nr = getNetworkBuffer8BitInteger(read_buffer);
  char *player_name = getNetworkBufferString(read_buffer);
  struct NetworkClientPlayerInfo *player = getNetworkPlayer(player_nr);

  printf("OP_PLAYER_NAME: %d\n", player_nr);

  strncpy(player->name, player_name, MAX_PLAYER_NAME_LEN);
  player->name[MAX_PLAYER_NAME_LEN] = '\0';

  Error(ERR_NETWORK_CLIENT, "client %d calls itself \"%s\"",
	player_nr, player->name);
}

static void Handle_OP_PLAYER_CONNECTED(void)
{
  struct NetworkClientPlayerInfo *player, *last_player = NULL;
  int new_client_nr = getNetworkBuffer8BitInteger(read_buffer);
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

static void Handle_OP_PLAYER_DISCONNECTED(void)
{
  struct NetworkClientPlayerInfo *player, *player_disconnected;
  int player_nr = getNetworkBuffer8BitInteger(read_buffer);
  int index_nr = player_nr - 1;

  printf("OP_PLAYER_DISCONNECTED: %d\n", player_nr);
  player_disconnected = getNetworkPlayer(player_nr);
  Error(ERR_NETWORK_CLIENT, "client %d (%s) disconnected",
	player_nr, getNetworkPlayerName(player_nr));

  for (player = &first_player; player; player = player->next)
    if (player->next == player_disconnected)
      player->next = player_disconnected->next;
  free(player_disconnected);

  stored_player[index_nr].connected_locally = FALSE;
  stored_player[index_nr].connected_network = FALSE;

  if (game_status == GAME_MODE_PLAYING)
  {
    char message[100];

    sprintf(message, "Player %d left network server! Network game stopped!",
	    player_nr);

    Request(message, REQ_CONFIRM | REQ_STAY_CLOSED);

    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();
  }
  else if (game_status == GAME_MODE_MAIN)
  {
    DrawNetworkPlayers();
  }
}

static void Handle_OP_START_PLAYING(void)
{
  int player_nr = getNetworkBuffer8BitInteger(read_buffer);
  char *new_leveldir_identifier = getNetworkBufferString(read_buffer);
  int new_level_nr = getNetworkBuffer16BitInteger(read_buffer);
  unsigned int new_random_seed = getNetworkBuffer32BitInteger(read_buffer);

  if (!strEqual(new_leveldir_identifier, network_level.leveldir_identifier))
  {
    Error(ERR_WARN, "no such level identifier: '%s'", new_leveldir_identifier);

    stop_network_game = TRUE;

    return;
  }

  printf("OP_START_PLAYING: %d\n", player_nr);
  Error(ERR_NETWORK_CLIENT,
	"client %d starts game [level %d from level identifier '%s']\n",
	player_nr, new_level_nr, new_leveldir_identifier);

  LevelDirTree *new_leveldir =
    getTreeInfoFromIdentifier(leveldir_first, new_leveldir_identifier);

  if (new_leveldir != NULL)
  {
    leveldir_current = new_leveldir;
    level_nr = new_level_nr;
  }

  // needed if level set of network game changed graphics, sounds or music
  ReloadCustomArtwork(0);

  TapeErase();

  if (network_level.use_network_level_files)
    LoadNetworkLevel(&network_level);
  else
    LoadLevel(level_nr);

  StartGameActions(FALSE, setup.autorecord, new_random_seed);
}

static void Handle_OP_PAUSE_PLAYING(void)
{
  int player_nr = getNetworkBuffer8BitInteger(read_buffer);

  printf("OP_PAUSE_PLAYING: %d\n", player_nr);
  Error(ERR_NETWORK_CLIENT, "client %d pauses game", player_nr);

  if (game_status == GAME_MODE_PLAYING)
  {
    tape.pausing = TRUE;
    DrawVideoDisplay(VIDEO_STATE_PAUSE_ON, 0);
  }
}

static void Handle_OP_CONTINUE_PLAYING(void)
{
  int player_nr = getNetworkBuffer8BitInteger(read_buffer);

  printf("OP_CONTINUE_PLAYING: %d\n", player_nr);
  Error(ERR_NETWORK_CLIENT, "client %d continues game", player_nr);

  if (game_status == GAME_MODE_PLAYING)
  {
    tape.pausing = FALSE;
    DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF, 0);
  }
}

static void Handle_OP_STOP_PLAYING(void)
{
  int client_nr = getNetworkBuffer8BitInteger(read_buffer);
  int cause_for_stopping = getNetworkBuffer8BitInteger(read_buffer);

  printf("OP_STOP_PLAYING: %d [%d]\n", client_nr, cause_for_stopping);
  Error(ERR_NETWORK_CLIENT, "client %d stops game [%d]",
	client_nr, cause_for_stopping);

  if (game_status == GAME_MODE_PLAYING)
  {
    int index_nr = client_nr - 1;
    struct PlayerInfo *client_player = &stored_player[index_nr];
    boolean stopped_by_remote_player = (!client_player->connected_locally);
    char message[100];

    if (cause_for_stopping == NETWORK_STOP_BY_PLAYER)
      sprintf(message, "Network game stopped by player %d!", client_nr);
    else
      sprintf(message, (cause_for_stopping == NETWORK_STOP_BY_ERROR ?
			"Network game stopped due to internal error!" :
			"Network game stopped!"));

    if (cause_for_stopping != NETWORK_STOP_BY_PLAYER ||
	stopped_by_remote_player)
      Request(message, REQ_CONFIRM | REQ_STAY_CLOSED);

    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();
  }
}

static void Handle_OP_MOVE_PLAYER(void)
{
  int player_nr = getNetworkBuffer8BitInteger(read_buffer);
  int server_frame_counter = getNetworkBuffer32BitInteger(read_buffer);
  int i;

  if (!network_playing)
    return;

  if (server_frame_counter != FrameCounter)
  {
    Error(ERR_INFO, "frame counters of client %d and server out of sync",
	  player_nr);
    Error(ERR_INFO, "frame counter of client is %d", FrameCounter);
    Error(ERR_INFO, "frame counter of server is %d", server_frame_counter);
    Error(ERR_INFO, "this should not happen -- please debug");

    stop_network_game = TRUE;

    return;
  }

  // copy valid player actions (will be set to 0 for not connected players)
  for (i = 0; i < MAX_PLAYERS; i++)
    stored_player[i].effective_action =
      getNetworkBuffer8BitInteger(read_buffer);

  network_player_action_received = TRUE;
}

static void Handle_OP_BROADCAST_MESSAGE(void)
{
  int player_nr = getNetworkBuffer8BitInteger(read_buffer);

  printf("OP_BROADCAST_MESSAGE: %d\n", player_nr);
  Error(ERR_NETWORK_CLIENT, "client %d sends message", player_nr);
}

static void Handle_OP_LEVEL_FILE(void)
{
  int player_nr = getNetworkBuffer8BitInteger(read_buffer);
  char *leveldir_identifier;
  char *network_level_dir;
  struct LevelFileInfo *file_info = &network_level.file_info;
  struct LevelFileInfo *tmpl_info = &network_level.tmpl_info;
  boolean use_custom_template;

  setString(&network_level.leveldir_identifier, NULL);
  setString(&network_level.file_info.basename,  NULL);
  setString(&network_level.file_info.filename,  NULL);
  setString(&network_level.tmpl_info.basename,  NULL);
  setString(&network_level.tmpl_info.filename,  NULL);

  printf("OP_LEVEL_FILE: %d\n", player_nr);

  leveldir_identifier = getStringCopy(getNetworkBufferString(read_buffer));

  if (hasPathSeparator(leveldir_identifier))
    Error(ERR_EXIT, "protocol error: invalid filename from network client");

  InitNetworkLevelDirectory(leveldir_identifier);

  network_level_dir   = getNetworkLevelDir(leveldir_identifier);

  file_info->nr       = getNetworkBuffer16BitInteger(read_buffer);
  file_info->type     = getNetworkBuffer8BitInteger(read_buffer);
  file_info->packed   = getNetworkBuffer8BitInteger(read_buffer);
  file_info->basename = getStringCopy(getNetworkBufferString(read_buffer));
  file_info->filename = getPath2(network_level_dir, file_info->basename);

  if (hasPathSeparator(file_info->basename))
    Error(ERR_EXIT, "protocol error: invalid filename from network client");

  int num_bytes = getNetworkBufferFile(read_buffer, file_info->filename);

  // if received level file is empty, remove it (as being non-existent)
  if (num_bytes == 0)
    remove(file_info->filename);

  use_custom_template = getNetworkBuffer8BitInteger(read_buffer);
  if (use_custom_template)
  {
    *tmpl_info = *file_info;

    tmpl_info->basename = getStringCopy(getNetworkBufferString(read_buffer));
    tmpl_info->filename = getPath2(network_level_dir, tmpl_info->basename);

    if (hasPathSeparator(tmpl_info->basename))
      Error(ERR_EXIT, "protocol error: invalid filename from network client");

    getNetworkBufferFile(read_buffer, tmpl_info->filename);

    // if received level file is empty, use level template file instead
    if (num_bytes == 0)
      setString(&file_info->filename,  tmpl_info->filename);
  }

  network_level.leveldir_identifier = leveldir_identifier;
  network_level.use_custom_template = use_custom_template;

  // the receiving client(s) use(s) the transferred network level files
  network_level.use_network_level_files = TRUE;

#if 0
  printf("::: '%s'\n", leveldir_identifier);
  printf("::: '%d'\n", file_info->nr);
  printf("::: '%d'\n", file_info->type);
  printf("::: '%d'\n", file_info->packed);
  printf("::: '%s'\n", file_info->basename);
  printf("::: '%s'\n", file_info->filename);

  if (use_custom_template)
    printf("::: '%s'\n", tmpl_info->filename);
#endif
}

static void HandleNetworkingMessage(void)
{
  stop_network_game = FALSE;

  initNetworkBufferForReading(read_buffer);

  int message_type = getNetworkBuffer8BitInteger(read_buffer);

  switch (message_type)
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
      Handle_OP_PLAYER_NAME();
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
      Handle_OP_MOVE_PLAYER();
      break;

    case OP_BROADCAST_MESSAGE:
      Handle_OP_BROADCAST_MESSAGE();
      break;

    case OP_LEVEL_FILE:
      Handle_OP_LEVEL_FILE();
      break;

    default:
      if (options.verbose)
	Error(ERR_NETWORK_CLIENT,
	      "unknown opcode %d from server", message_type);
  }

  fflush(stdout);

  // in case of internal error, stop network game
  if (stop_network_game)
    SendToServer_StopPlaying(NETWORK_STOP_BY_ERROR);
}

static char *HandleNetworkingPackets(void)
{
  while (1)
  {
    // ---------- check network server for activity ----------

    int num_active_sockets = SDLNet_CheckSockets(rfds, 0);

    if (num_active_sockets < 0)
      return "Error checking network sockets!";

    if (num_active_sockets == 0)
      break;	// no active sockets, stop here

    // ---------- read packets from network server ----------

    initNetworkBufferForReceiving(read_buffer);

    int num_bytes = receiveNetworkBufferPacket(read_buffer, sfd);

    if (num_bytes < 0)
      return "Error reading from network server!";

    if (num_bytes == 0)
      return "Connection to network server lost!";

    HandleNetworkingMessage();

    if (stop_network_client)
      return stop_network_client_message;
  }

  return NULL;
}

static void FreeNetworkClientPlayerInfo(struct NetworkClientPlayerInfo *player)
{
  if (player == NULL)
    return;

  if (player->next)
    FreeNetworkClientPlayerInfo(player->next);

  checked_free(player);
}

static void HandleNetworkingDisconnect(void)
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

  FreeNetworkClientPlayerInfo(first_player.next);

  first_player.nr = 0;
  first_player.next = NULL;
}

void HandleNetworking(void)
{
  // do not handle any networking packets if request dialog is active
  if (game.request_active)
    return;

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

      if (game_status == GAME_MODE_MAIN)
	ClearNetworkPlayers();
    }
  }
}

void DisconnectFromNetworkServer(void)
{
  DrawNetworkText_Title("Terminating Network");
  DrawNetworkText("Disconnecting from network server ...");

  HandleNetworkingDisconnect();

  DrawNetworkText_Success("Successfully disconnected!");
}
