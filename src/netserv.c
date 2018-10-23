// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// netserv.c
// ============================================================================

#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

#include "libgame/libgame.h"

#include "netserv.h"
#include "main.h"


static int num_clients = 0;
static boolean run_server_only_once = FALSE;

struct NetworkServerPlayerInfo
{
  TCPsocket fd;
  char player_name[MAX_PLAYER_NAME_LEN + 1];
  int number;
  boolean active;
  boolean introduced;
  byte action;
  boolean action_received;

  struct NetworkServerPlayerInfo *next;
};

static struct NetworkServerPlayerInfo *first_player = NULL;

#define NEXT(player) ((player)->next ? (player)->next : first_player)

// TODO: peer address
static TCPsocket lfd;		// listening TCP socket
static UDPsocket udp;		// listening UDP socket
static SDLNet_SocketSet fds;	// socket set

static struct NetworkBuffer *read_buffer = NULL;
static struct NetworkBuffer *write_buffer = NULL;

static unsigned int ServerFrameCounter = 0;


int getNetwork8BitInteger(byte *ptr)
{
  return ptr[0];
}

int putNetwork8BitInteger(byte *ptr, int value)
{
  ptr[0] = value;

  return 1;
}

int getNetwork16BitInteger(byte *ptr)
{
  return ((ptr[0] << 8) |
	  (ptr[1] << 0));
}

int putNetwork16BitInteger(byte *ptr, int value)
{
  ptr[0] = (value >> 8) & 0xff;
  ptr[1] = (value >> 0) & 0xff;

  return 2;
}

int getNetwork32BitInteger(byte *ptr)
{
  return ((ptr[0] << 24) |
	  (ptr[1] << 16) |
	  (ptr[2] <<  8) |
	  (ptr[3] <<  0));
}

int putNetwork32BitInteger(byte *ptr, int value)
{
  ptr[0] = (value >> 24) & 0xff;
  ptr[1] = (value >> 16) & 0xff;
  ptr[2] = (value >>  8) & 0xff;
  ptr[3] = (value >>  0) & 0xff;

  return 4;
}

char *getNetworkString(byte *ptr)
{
  return (char *)ptr;
}

int putNetworkString(byte *ptr, char *s)
{
  strcpy((char *)ptr, s);

  return strlen(s) + 1;
}

struct NetworkBuffer *newNetworkBuffer(void)
{
  struct NetworkBuffer *new = checked_calloc(sizeof(struct NetworkBuffer));

  new->max_size = MAX_BUFFER_SIZE;
  new->size = 0;
  new->pos = 0;

  new->buffer = checked_calloc(new->max_size);

  return new;
}

static void resetNetworkBufferForReading(struct NetworkBuffer *nb)
{
  nb->pos = 0;
}

static void resetNetworkBufferForWriting(struct NetworkBuffer *nb)
{
  nb->size = 0;
  nb->pos = 0;
}

void initNetworkBufferForReceiving(struct NetworkBuffer *nb)
{
  resetNetworkBufferForWriting(nb);
}

void initNetworkBufferForReading(struct NetworkBuffer *nb)
{
  resetNetworkBufferForReading(nb);

  // skip message length header
  getNetworkBuffer32BitInteger(nb);
}

void initNetworkBufferForWriting(struct NetworkBuffer *nb, int message_type,
				 int player_nr)
{
  resetNetworkBufferForWriting(nb);

  // will be replaced with message length before sending
  putNetworkBuffer32BitInteger(nb, 0);

  putNetworkBuffer8BitInteger(nb, message_type);
  putNetworkBuffer8BitInteger(nb, player_nr);
}

static void copyNetworkBufferForWriting(struct NetworkBuffer *nb_from,
					struct NetworkBuffer *nb_to,
					int player_nr)
{
  initNetworkBufferForReading(nb_from);

  int message_type = getNetworkBuffer8BitInteger(nb_from);

  // skip player number
  getNetworkBuffer8BitInteger(nb_from);

  initNetworkBufferForWriting(nb_to, message_type, player_nr);

  while (nb_from->pos < nb_from->size)
  {
    int b = getNetworkBuffer8BitInteger(nb_from);

    putNetworkBuffer8BitInteger(nb_to, b);
  }
}

static void increaseNetworkBuffer(struct NetworkBuffer *nb, int additional_size)
{
  // add some more buffer size than is really required this time
  nb->max_size += additional_size + MAX_BUFFER_SIZE;
  nb->buffer = checked_realloc(nb->buffer, nb->max_size);
}

int receiveNetworkBufferBytes(struct NetworkBuffer *nb, TCPsocket socket,
			     int num_bytes)
{
  if (num_bytes > MAX_PACKET_SIZE)
  {
    Error(ERR_NETWORK_SERVER, "protocol error: invalid packet size %d",
	  num_bytes);

    return -1;
  }

  if (nb->pos + num_bytes > nb->max_size)
    increaseNetworkBuffer(nb, num_bytes);

  int result = SDLNet_TCP_Recv(socket, &nb->buffer[nb->pos], num_bytes);

  if (result != num_bytes)
    return result;

  nb->pos += num_bytes;
  nb->size = nb->pos;

  return num_bytes;
}

int receiveNetworkBufferPacket(struct NetworkBuffer *nb, TCPsocket socket)
{
  int num_bytes, num_bytes_head, num_bytes_body;

  num_bytes_head = 4;
  num_bytes = receiveNetworkBufferBytes(nb, socket, num_bytes_head);

  if (num_bytes != num_bytes_head)
    return num_bytes;

  num_bytes_body = getNetwork32BitInteger(nb->buffer);
  num_bytes = receiveNetworkBufferBytes(nb, socket, num_bytes_body);

  return num_bytes;
}

int getNetworkBuffer8BitInteger(struct NetworkBuffer *nb)
{
  int num_bytes = 1;

  if (nb->pos + num_bytes > nb->size)
    return 0;

  int value = getNetwork8BitInteger(&nb->buffer[nb->pos]);

  nb->pos += num_bytes;

  return value;
}

void putNetworkBuffer8BitInteger(struct NetworkBuffer *nb, int value)
{
  int num_bytes = 1;

  if (nb->pos + num_bytes > nb->max_size)
    increaseNetworkBuffer(nb, num_bytes);

  nb->pos += putNetwork8BitInteger(&nb->buffer[nb->pos], value);
  nb->size = nb->pos;
}

int getNetworkBuffer16BitInteger(struct NetworkBuffer *nb)
{
  int num_bytes = 2;

  if (nb->pos + num_bytes > nb->size)
    return 0;

  int value = getNetwork16BitInteger(&nb->buffer[nb->pos]);

  nb->pos += num_bytes;

  return value;
}

void putNetworkBuffer16BitInteger(struct NetworkBuffer *nb, int value)
{
  int num_bytes = 2;

  if (nb->pos + num_bytes > nb->max_size)
    increaseNetworkBuffer(nb, num_bytes);

  nb->pos += putNetwork16BitInteger(&nb->buffer[nb->pos], value);
  nb->size = nb->pos;
}

int getNetworkBuffer32BitInteger(struct NetworkBuffer *nb)
{
  int num_bytes = 4;

  if (nb->pos + num_bytes > nb->size)
    return 0;

  int value = getNetwork32BitInteger(&nb->buffer[nb->pos]);

  nb->pos += num_bytes;

  return value;
}

void putNetworkBuffer32BitInteger(struct NetworkBuffer *nb, int value)
{
  int num_bytes = 4;

  if (nb->pos + num_bytes > nb->max_size)
    increaseNetworkBuffer(nb, num_bytes);

  nb->pos += putNetwork32BitInteger(&nb->buffer[nb->pos], value);
  nb->size = nb->pos;
}

char *getNetworkBufferString(struct NetworkBuffer *nb)
{
  char *s = getNetworkString(&nb->buffer[nb->pos]);

  nb->pos += strlen(s) + 1;

  return s;
}

void putNetworkBufferString(struct NetworkBuffer *nb, char *s)
{
  int num_bytes = strlen(s) + 1;

  if (nb->pos + num_bytes > nb->max_size)
    increaseNetworkBuffer(nb, num_bytes);

  nb->pos += putNetworkString(&nb->buffer[nb->pos], s);
  nb->size = nb->pos;
}

int getNetworkBufferFile(struct NetworkBuffer *nb, char *filename)
{
  FILE *file;
  int num_bytes = getNetworkBuffer32BitInteger(nb);
  int i;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write file '%s' from network buffer", filename);

    return 0;
  }

  for (i = 0; i < num_bytes; i++)
  {
    int b = getNetworkBuffer8BitInteger(nb);

    putFile8Bit(file, b);
  }

  fclose(file);

  return num_bytes;
}

int putNetworkBufferFile(struct NetworkBuffer *nb, char *filename)
{
  File *file;
  int filesize_pos = nb->pos;
  int num_bytes = 0;

  // will be replaced with file size
  putNetworkBuffer32BitInteger(nb, 0);

  if (!(file = openFile(filename, MODE_READ)))
  {
    Error(ERR_WARN, "cannot read file '%s' to network buffer", filename);

    return 0;
  }

  while (1)
  {
    int b = getFile8Bit(file);

    if (checkEndOfFile(file))
      break;

    putNetworkBuffer8BitInteger(nb, b);

    num_bytes++;
  }

  closeFile(file);

  // set file size
  putNetwork32BitInteger(&nb->buffer[filesize_pos], num_bytes);

  return num_bytes;
}

void dumpNetworkBuffer(struct NetworkBuffer *nb)
{
  int i;

  printf("::: network buffer maximum size: %d\n", nb->max_size);
  printf("::: network buffer size:         %d\n", nb->size);
  printf("::: network buffer position    : %d\n", nb->pos);

  for (i = 0; i < nb->size; i++)
  {
    if ((i % 16) == 0)
      printf("\n::: ");

    printf("%02x ", nb->buffer[i]);
  }

  printf("\n");
}

static void SendNetworkBufferToAllButOne(struct NetworkBuffer *nb,
					 struct NetworkServerPlayerInfo *except)
{
  struct NetworkServerPlayerInfo *player;

  // set message length header
  putNetwork32BitInteger(nb->buffer, nb->size - 4);

  for (player = first_player; player != NULL; player = player->next)
  {
    if (player != except && player->introduced)
    {
      // directly send the buffer to the network client
      SDLNet_TCP_Send(player->fd, nb->buffer, nb->size);
    }
  }
}

static void SendNetworkBufferToAll(struct NetworkBuffer *nb)
{
  SendNetworkBufferToAllButOne(nb, NULL);
}

static void SendNetworkBufferToClient(struct NetworkBuffer *nb,
				      struct NetworkServerPlayerInfo *player)
{
  // set message length header
  putNetwork32BitInteger(nb->buffer, nb->size - 4);

  // directly send the buffer to the network client
  SDLNet_TCP_Send(player->fd, nb->buffer, nb->size);
}

static void RemovePlayer(struct NetworkServerPlayerInfo *player)
{
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "dropping client %d (%s)",
	  player->number, player->player_name);

  SDLNet_TCP_DelSocket(fds, player->fd);
  SDLNet_TCP_Close(player->fd);

  if (player == first_player)
  {
    first_player = player->next;
  }
  else
  {
    struct NetworkServerPlayerInfo *p;

    for (p = first_player; p != NULL; p = p->next)
    {
      if (p->next && p->next == player)
      {
	p->next = player->next;

	break;
      }
    }
  }

  if (player->introduced)
  {
    initNetworkBufferForWriting(write_buffer, OP_PLAYER_DISCONNECTED,
				player->number);

    SendNetworkBufferToAllButOne(write_buffer, player);
  }

  free(player);
  num_clients--;

#if 0	// do not terminate network server if last player disconnected
  if (run_server_only_once && num_clients == 0)
  {
    if (options.verbose)
    {
      Error(ERR_NETWORK_SERVER, "no clients left");
      Error(ERR_NETWORK_SERVER, "aborting");
    }

    exit(0);
  }
#endif
}

static void AddPlayer(TCPsocket fd)
{
  struct NetworkServerPlayerInfo *player, *p;
  int number = 1;
  boolean again = TRUE;

  SDLNet_TCP_AddSocket(fds, fd);

  player = checked_calloc(sizeof(struct NetworkServerPlayerInfo));

  player->fd = fd;
  player->player_name[0] = 0;
  player->active = FALSE;
  player->introduced = FALSE;
  player->action = 0;
  player->action_received = FALSE;
  player->next = first_player;

  first_player = player;

  while (again)
  {
    again = FALSE;
    p = player->next;

    while (p)
    {
      if (p->number == number)
      {
	number++;

	again = TRUE;

	break;
      }

      p = p->next;
    }
  }

  player->number = number;
  num_clients++;

  initNetworkBufferForWriting(write_buffer, OP_YOUR_NUMBER, 0);

  putNetworkBuffer8BitInteger(write_buffer, player->number);

  SendNetworkBufferToClient(write_buffer, player);
}

static void Handle_OP_PROTOCOL_VERSION(struct NetworkServerPlayerInfo *player)
{
  int protocol_version_major = getNetworkBuffer8BitInteger(read_buffer);
  int protocol_version_minor = getNetworkBuffer8BitInteger(read_buffer);
  int protocol_version_patch = getNetworkBuffer8BitInteger(read_buffer);

  if (protocol_version_major != PROTOCOL_VERSION_MAJOR ||
      protocol_version_minor != PROTOCOL_VERSION_MINOR)
  {
    if (options.verbose)
      Error(ERR_NETWORK_SERVER,
	    "client %d (%s) has wrong protocol version %d.%d.%d",
	    player->number, player->player_name,
	    protocol_version_major,
	    protocol_version_minor,
	    protocol_version_patch);

    initNetworkBufferForWriting(write_buffer, OP_BAD_PROTOCOL_VERSION, 0);

    putNetworkBuffer8BitInteger(write_buffer, PROTOCOL_VERSION_MAJOR);
    putNetworkBuffer8BitInteger(write_buffer, PROTOCOL_VERSION_MINOR);
    putNetworkBuffer8BitInteger(write_buffer, PROTOCOL_VERSION_PATCH);

    SendNetworkBufferToClient(write_buffer, player);

    RemovePlayer(player);
  }
  else
  {
    if (options.verbose)
      Error(ERR_NETWORK_SERVER,
	    "client %d (%s) uses protocol version %d.%d.%d",
	    player->number, player->player_name,
	    protocol_version_major,
	    protocol_version_minor,
	    protocol_version_patch);
  }
}

static void Handle_OP_NUMBER_WANTED(struct NetworkServerPlayerInfo *player)
{
  int nr_wanted = getNetworkBuffer8BitInteger(read_buffer);
  int client_nr = player->number;
  boolean nr_is_free = TRUE;
  struct NetworkServerPlayerInfo *p;

  if (options.verbose)
      Error(ERR_NETWORK_SERVER, "client %d (%s) wants to switch to # %d",
	    player->number, player->player_name, nr_wanted);

  for (p = first_player; p != NULL; p = p->next)
  {
    if (p->number == nr_wanted)
    {
      nr_is_free = FALSE;

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

  initNetworkBufferForWriting(write_buffer, OP_NUMBER_WANTED, client_nr);

  putNetworkBuffer8BitInteger(write_buffer, nr_wanted);
  putNetworkBuffer8BitInteger(write_buffer, player->number);

  SendNetworkBufferToAll(write_buffer);
}

static void Handle_OP_PLAYER_NAME(struct NetworkServerPlayerInfo *player)
{
  char *player_name = getNetworkBufferString(read_buffer);
  int i;

  strncpy(player->player_name, player_name, MAX_PLAYER_NAME_LEN);
  player->player_name[MAX_PLAYER_NAME_LEN] = '\0';

  for (i = 0; i < MAX_PLAYER_NAME_LEN; i++)
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
    initNetworkBufferForWriting(write_buffer, OP_PLAYER_CONNECTED,
				player->number);

    SendNetworkBufferToAllButOne(write_buffer, player);
  }
	      
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d calls itself \"%s\"",
	  player->number, player->player_name);

  copyNetworkBufferForWriting(read_buffer, write_buffer, player->number);

  SendNetworkBufferToAllButOne(write_buffer, player);

  if (!player->introduced)
  {
    struct NetworkServerPlayerInfo *p;

    for (p = first_player; p != NULL; p = p->next)
    {
      if (p != player && p->introduced)
      {
	initNetworkBufferForWriting(write_buffer, OP_PLAYER_CONNECTED,
				    p->number);

	SendNetworkBufferToClient(write_buffer, player);

	initNetworkBufferForWriting(write_buffer, OP_PLAYER_NAME, p->number);

	putNetworkBufferString(write_buffer, p->player_name);

	SendNetworkBufferToClient(write_buffer, player);
      }
    }
  }

  player->introduced = TRUE;
}

static void Handle_OP_START_PLAYING(struct NetworkServerPlayerInfo *player)
{
  char *new_leveldir_identifier = getNetworkBufferString(read_buffer);
  int level_nr = getNetworkBuffer16BitInteger(read_buffer);

  if (options.verbose)
    Error(ERR_NETWORK_SERVER,
	  "client %d (%s) starts game [level %d from level set '%s']",
	  player->number, player->player_name, level_nr,
	  new_leveldir_identifier);

  struct NetworkServerPlayerInfo *p;

  // reset frame counter
  ServerFrameCounter = 0;

  Error(ERR_NETWORK_SERVER, "resetting ServerFrameCounter to 0");

  // reset player actions
  for (p = first_player; p != NULL; p = p->next)
  {
    p->action = 0;
    p->action_received = FALSE;

    if (p->introduced)
      p->active = TRUE;
  }

  copyNetworkBufferForWriting(read_buffer, write_buffer, player->number);

  SendNetworkBufferToAll(write_buffer);
}

static void Handle_OP_PAUSE_PLAYING(struct NetworkServerPlayerInfo *player)
{
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) pauses game",
	  player->number, player->player_name);

  copyNetworkBufferForWriting(read_buffer, write_buffer, player->number);

  SendNetworkBufferToAll(write_buffer);
}

static void Handle_OP_CONTINUE_PLAYING(struct NetworkServerPlayerInfo *player)
{
  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) continues game",
	  player->number, player->player_name);

  copyNetworkBufferForWriting(read_buffer, write_buffer, player->number);

  SendNetworkBufferToAll(write_buffer);
}

static void Handle_OP_STOP_PLAYING(struct NetworkServerPlayerInfo *player)
{
  int cause_for_stopping = getNetworkBuffer8BitInteger(read_buffer);

  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) stops game [%d]",
	  player->number, player->player_name, cause_for_stopping);

  copyNetworkBufferForWriting(read_buffer, write_buffer, player->number);

  SendNetworkBufferToAll(write_buffer);
}

static void Handle_OP_MOVE_PLAYER(struct NetworkServerPlayerInfo *player)
{
  int player_action = getNetworkBuffer8BitInteger(read_buffer);
  int last_client_nr = 0;
  struct NetworkServerPlayerInfo *p;
  int i;

  // store player action
  for (p = first_player; p != NULL; p = p->next)
  {
    if (p->number == player->number)
    {
      p->action = player_action;
      p->action_received = TRUE;
    }
  }

  // check if server received action from each player
  for (p = first_player; p != NULL; p = p->next)
  {
    if (!p->action_received)
      return;

    if (p->number > last_client_nr)
      last_client_nr = p->number;
  }

  int player_action_all[last_client_nr];

  // initialize all player actions to zero
  for (i = 0; i < last_client_nr; i++)
    player_action_all[i] = 0;

  // broadcast actions of all players to all players
  for (p = first_player; p != NULL; p = p->next)
  {
    player_action_all[p->number - 1] = p->action;

    p->action = 0;
    p->action_received = FALSE;
  }

  initNetworkBufferForWriting(write_buffer, OP_MOVE_PLAYER, player->number);

  putNetworkBuffer32BitInteger(write_buffer, ServerFrameCounter);

  for (i = 0; i < last_client_nr; i++)
    putNetworkBuffer8BitInteger(write_buffer, player_action_all[i]);

  SendNetworkBufferToAll(write_buffer);

  ServerFrameCounter++;
}

static void Handle_OP_BROADCAST_MESSAGE(struct NetworkServerPlayerInfo *player)
{
  char *message = getNetworkBufferString(read_buffer);

  if (options.verbose)
    Error(ERR_NETWORK_SERVER, "client %d (%s) sends message: %s",
	  player->number, player->player_name, message);

  copyNetworkBufferForWriting(read_buffer, write_buffer, player->number);

  SendNetworkBufferToAllButOne(write_buffer, player);
}

static void Handle_OP_LEVEL_FILE(struct NetworkServerPlayerInfo *player)
{
  copyNetworkBufferForWriting(read_buffer, write_buffer, player->number);

  SendNetworkBufferToAllButOne(write_buffer, player);
}

static void ExitNetworkServer(int exit_value)
{
  Error(ERR_NETWORK_SERVER, "exiting network server");

  exit(exit_value);
}

// the following is not used for a standalone server;
// the pointer points to an integer containing the port-number
int NetworkServerThread(void *ptr)
{
  NetworkServer(*((int *) ptr), 0);

  // should never be reached
  return 0;
}

void NetworkServer(int port, int serveronly)
{
  struct NetworkServerPlayerInfo *player;
  IPaddress ip;

#if defined(PLATFORM_UNIX) && !defined(PLATFORM_NEXT)
  struct sigaction sact;
#endif

  if (read_buffer == NULL)
    read_buffer = newNetworkBuffer();

  if (write_buffer == NULL)
    write_buffer = newNetworkBuffer();

  if (port == 0)
    port = DEFAULT_SERVER_PORT;

  // if only running the network server, exit on Ctrl-C
  if (serveronly)
    signal(SIGINT, ExitNetworkServer);

  if (!serveronly)
    run_server_only_once = TRUE;

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

  if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_ResolveHost() failed: %s",
          SDLNet_GetError());

  if ((fds = SDLNet_AllocSocketSet(MAX_PLAYERS + 1 + 1)) == NULL)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_AllocSocketSet() failed: %s"),
      SDLNet_GetError();

  if ((lfd = SDLNet_TCP_Open(&ip)) == NULL)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_TCP_Open() failed: %s"),
      SDLNet_GetError();

  if (SDLNet_TCP_AddSocket(fds, lfd) == -1)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_TCP_AddSocket() failed: %s"),
      SDLNet_GetError();

  if ((udp = SDLNet_UDP_Open(port)) == NULL)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_UDP_Open() failed: %s",
          SDLNet_GetError());

  if (SDLNet_UDP_AddSocket(fds, udp) == -1)
    Error(ERR_EXIT_NETWORK_SERVER, "SDLNet_TCP_AddSocket() failed: %s"),
      SDLNet_GetError();

  if (options.verbose)
  {
    Error(ERR_NETWORK_SERVER, "started up, listening on port %d", port);
    Error(ERR_NETWORK_SERVER, "using protocol version %d.%d.%d",
	  PROTOCOL_VERSION_MAJOR,
	  PROTOCOL_VERSION_MINOR,
	  PROTOCOL_VERSION_PATCH);
  }

  while (1)
  {
    // wait for 100 ms for activity on open network sockets
    if (SDLNet_CheckSockets(fds, 100) < 1)
      continue;

    // accept incoming TCP connections
    if (SDLNet_SocketReady(lfd))
    {
      Error(ERR_DEBUG, "got TCP packet");

      TCPsocket newsock;

      newsock = SDLNet_TCP_Accept(lfd);

      if (newsock)
	AddPlayer(newsock);
    }

    // accept incoming UDP packets
    if (SDLNet_SocketReady(udp))
    {
      Error(ERR_DEBUG, "got UDP packet");

      static UDPpacket packet;

      int num_packets = SDLNet_UDP_Recv(udp, &packet);

      if (num_packets == 1)
      {
        // bounce packet
        SDLNet_UDP_Send(udp, -1, &packet);
      }
    }

    for (player = first_player; player != NULL; player = player->next)
    {
      if (!SDLNet_SocketReady(player->fd))
	continue;

      initNetworkBufferForReceiving(read_buffer);

      int num_bytes = receiveNetworkBufferPacket(read_buffer, player->fd);

      if (num_bytes <= 0)
      {
	if (options.verbose)
	  Error(ERR_NETWORK_SERVER, "EOF from client %d (%s)",
		player->number, player->player_name);

	RemovePlayer(player);

	break;
      }

      initNetworkBufferForReading(read_buffer);

      int message_type = getNetworkBuffer8BitInteger(read_buffer);

      // skip player number
      getNetworkBuffer8BitInteger(read_buffer);

      if (!player->introduced &&
	  message_type != OP_PLAYER_NAME &&
	  message_type != OP_PROTOCOL_VERSION)
      {
	if (options.verbose)
	  Error(ERR_NETWORK_SERVER, "got opcode %d for client %d which is not introduced yet (expected OP_PLAYER_NAME or OP_PROTOCOL_VERSION)", message_type, player->number);

	RemovePlayer(player);

	break;
      }

      switch (message_type)
      {
	case OP_PROTOCOL_VERSION:
	  Handle_OP_PROTOCOL_VERSION(player);
	  break;

	case OP_NUMBER_WANTED:
	  Handle_OP_NUMBER_WANTED(player);
	  break;

	case OP_PLAYER_NAME:
	  Handle_OP_PLAYER_NAME(player);
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
	  Handle_OP_BROADCAST_MESSAGE(player);
	  break;

	case OP_LEVEL_FILE:
	  Handle_OP_LEVEL_FILE(player);
	  break;

	default:
	  if (options.verbose)
	    Error(ERR_NETWORK_SERVER,
		  "unknown opcode %d from client %d (%s)",
		  message_type, player->number, player->player_name);
      }
    }
  }
}
