//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     Networking module which uses SDL_net
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../utils/memory.hpp"
#include "doomtype.hpp"
#include "i_system.hpp"
#include "m_argv.hpp"
#include "m_misc.hpp"
#include "net_defs.hpp"
#include "net_io.hpp"
#include "net_packet.hpp"
#include "net_sdl.hpp"
#include "z_zone.hpp"
#include <map>

//
// NETWORKING
//

#include <SDL_net.h>
#include <cstring>

#define DEFAULT_PORT 2342

static boolean initted = false;
static int port = DEFAULT_PORT;
static UDPsocket udpsocket;
static UDPpacket *recvpacket;

typedef struct
{
    net_addr_t net_addr;
    IPaddress sdl_addr;
} addrpair_t;

struct CmpByHostThenPort {
    bool operator()(const IPaddress& a, const IPaddress& b) const {
       return a.host != b.host ? a.host < b.host : a.port < b.port;
    }
};

std::map<IPaddress, addrpair_t,CmpByHostThenPort> ip_to_net_map;

// Finds an address by searching the table.  If the address is not found,
// it is added to the table.

static net_addr_t *NET_SDL_FindAddress(IPaddress *addr)
{
   auto x = ip_to_net_map.find( *addr );
   if (x != ip_to_net_map.end()) {
      return &x->second.net_addr;
   }

   // Was not found in list.  We need to add it.
   // Add a new entry

   addrpair_t &new_entry = ip_to_net_map[ *addr ];

   new_entry.sdl_addr = *addr;
   new_entry.net_addr.refcount = 0;
   new_entry.net_addr.handle = &new_entry.sdl_addr;
   new_entry.net_addr.module = &net_sdl_module;

   return &new_entry.net_addr;
}

static void NET_SDL_FreeAddress(net_addr_t *addr)
{
   // FIXME, you could.... use an offset of hack to get back to
   // the IPAddress to lookup using the map to avoid this loop.
   for (auto it=ip_to_net_map.begin(); it!=ip_to_net_map.end(); ++it)
   {
      if (addr == &it->second.net_addr)
      {
         ip_to_net_map.erase( it );
         return;
      }
   }

   I_Error("NET_SDL_FreeAddress: Attempted to remove an unused address!");
}

static boolean NET_SDL_InitClient(void)
{
    int p;

    if (initted)
        return true;

    //!
    // @category net
    // @arg <n>
    //
    // Use the specified UDP port for communications, instead of 
    // the default (2342).
    //

    p = M_CheckParmWithArgs("-port", 1);
    if (p > 0)
       port = std::stoi(myargv[p+1]);

    SDLNet_Init();

    udpsocket = SDLNet_UDP_Open(0);

    if (udpsocket == NULL)
    {
        I_Error("NET_SDL_InitClient: Unable to open a socket!");
    }
    
    recvpacket = SDLNet_AllocPacket(1500);

#ifdef DROP_PACKETS
    srand(time(NULL));
#endif

    initted = true;

    return true;
}

static boolean NET_SDL_InitServer(void)
{
    int p;

    if (initted)
        return true;

    p = M_CheckParmWithArgs("-port", 1);
    if (p > 0)
       port = std::stoi(myargv[p+1]);

    SDLNet_Init();

    udpsocket = SDLNet_UDP_Open(port);

    if (udpsocket == NULL)
    {
        I_Error("NET_SDL_InitServer: Unable to bind to port %i", port);
    }

    recvpacket = SDLNet_AllocPacket(1500);
#ifdef DROP_PACKETS
    srand(time(NULL));
#endif

    initted = true;

    return true;
}

static void NET_SDL_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    UDPpacket sdl_packet;
    IPaddress ip;
   
    if (addr == &net_broadcast_addr)
    {
        SDLNet_ResolveHost(&ip, NULL, port);
        ip.host = INADDR_BROADCAST;
    }
    else
    {
        ip = *((IPaddress *) addr->handle);
    }

#if 0
    {
        static int this_second_sent = 0;
        static int lasttime;

        this_second_sent += packet->len + 64;

        if (I_GetTime() - lasttime > TICRATE)
        {
            printf("%i bytes sent in the last second\n", this_second_sent);
            lasttime = I_GetTime();
            this_second_sent = 0;
        }
    }
#endif

#ifdef DROP_PACKETS
    if ((rand() % 4) == 0)
        return;
#endif

    sdl_packet.channel = 0;
    sdl_packet.data = packet->data;
    sdl_packet.len = packet->len;
    sdl_packet.address = ip;

    if (!SDLNet_UDP_Send(udpsocket, -1, &sdl_packet))
    {
        I_Error("NET_SDL_SendPacket: Error transmitting packet: %s",
                SDLNet_GetError());
    }
}

static boolean NET_SDL_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    int result;

    result = SDLNet_UDP_Recv(udpsocket, recvpacket);

    if (result < 0)
    {
        I_Error("NET_SDL_RecvPacket: Error receiving packet: %s",
                SDLNet_GetError());
    }

    // no packets received

    if (result == 0)
        return false;

    // Put the data into a new packet structure

    *packet = NET_NewPacket(recvpacket->len);
    memcpy((*packet)->data, recvpacket->data, recvpacket->len);
    (*packet)->len = recvpacket->len;

    // Address

    *addr = NET_SDL_FindAddress(&recvpacket->address);

    return true;
}

void NET_SDL_AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{
    IPaddress *ip;
    uint32_t host;
    uint16_t port;

    ip = (IPaddress *) addr->handle;
    host = SDLNet_Read32(&ip->host);
    port = SDLNet_Read16(&ip->port);

    // If we are using the default port we just need to show the IP address,
    // but otherwise we need to include the port. This is important because
    // we use the string representation in the setup tool to provided an
    // address to connect to.
    if (port != DEFAULT_PORT)
    {
       M_snprintf(buffer, buffer_len, "%i.%i.%i.%i:%i",
               (host >> 24) & 0xff, (host >> 16) & 0xff,
                 (host >> 8) & 0xff, host & 0xff, port);
    }
    else
    {
       M_snprintf(buffer, buffer_len, "%i.%i.%i.%i",
               (host >> 24) & 0xff, (host >> 16) & 0xff,
               (host >> 8) & 0xff, host & 0xff);
    }
}

net_addr_t *NET_SDL_ResolveAddress(const char *address)
{
    IPaddress ip;
    int addr_port;
    int result;

    const auto *colon = std::strchr(address, ':');

    auto deleter = [](char *ptr) { free ( ptr ); };
    auto addr_hostname = std::unique_ptr<char, decltype(deleter) >( M_StringDuplicate(address), deleter );

    if (colon != NULL)
    {
        addr_hostname.get()[colon - address] = '\0';
	addr_port = atoi(colon + 1);
    }
    else
    {
	addr_port = port;
    }
    
    result = SDLNet_ResolveHost(&ip, addr_hostname.get(), addr_port);

    if (result)
    {
        // unable to resolve

        return NULL;
    }
    else
    {
        return NET_SDL_FindAddress(&ip);
    }
}

// Complete module

net_module_t net_sdl_module =
{
    NET_SDL_InitClient,
    NET_SDL_InitServer,
    NET_SDL_SendPacket,
    NET_SDL_RecvPacket,
    NET_SDL_AddrToString,
    NET_SDL_FreeAddress,
    NET_SDL_ResolveAddress,
};

