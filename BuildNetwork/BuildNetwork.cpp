#include "BuildNetwork.h"
#include "../BuildingMod.h"
#include "../ZoneSaver/ZoneSaver.h"

BuildNetwork::BuildNetwork(BuildingMod* mod) {
	this->mod = mod;
	this->CSChannel = *(int*)"BLK0";
	this->SCChannel = *(int*)"BLK1";
	UpdatePingTime();
}

void BuildNetwork::SendSingleBlockUpdate(CSteamID steamID, LongVector3& position, cube::Block& block, bool toClient) {
	int channel = toClient ? SCChannel : CSChannel;
	SingleBlockUpdatePacket packet(position, block);
	packet.Send(steamID, channel);
}

void BuildNetwork::SendPing(CSteamID steamID, bool toClient) {
	int channel = toClient ? SCChannel : CSChannel;
	PingPacket packet;
	packet.Send(steamID, channel);
}

void BuildNetwork::SendZoneLoad(CSteamID steamID, IntVector2 position) {
	int channel = CSChannel;
	ZoneLoadPacket packet(position);
	packet.Send(steamID, channel);
}

u32 BuildNetwork::GetPingTime() {
	return timeGetTime() - lastPing;
}

void BuildNetwork::UpdatePingTime() {
	this->lastPing = timeGetTime();
}

void BuildNetwork::Process() {
	ProcessCS();
	ProcessSC();
	DoPings();
}

void BuildNetwork::DoPings() {
	// If you don't periodically send data over a channel, it will be torn down.
	// If it is torn down, it will take longer for the next packet to be transmitted.
	if (GetPingTime() > 10000) {
		cube::Game* game = cube::GetGame();
		for (auto pair : game->host.connections) {
			CSteamID id = pair.first;
			SendPing(id, true);
		}
		SendPing(game->client.host_steam_id, false);
		UpdatePingTime();
	}
}

void BuildNetwork::ProcessCS() {
	cube::Game* game = cube::GetGame();
	u32 size;
	while (cube::SteamNetworking()->IsP2PPacketAvailable(&size, CSChannel)) {
		// Receive packet from client
		auto buffer = std::unique_ptr<u8[]>(new u8[size]);
		u32 receivedSize;
		CSteamID steamID;
		cube::SteamNetworking()->ReadP2PPacket(buffer.get(), size, &receivedSize, &steamID, CSChannel);

		// Ignore it if they aren't connected to us, fuck you
		if (!game->host.connections.count(steamID)) {
			continue;
		}
		BuildNetworkPacket* basePacket = (BuildNetworkPacket*)buffer.get();
		if (size < sizeof(*basePacket)) continue;

		switch (basePacket->id) {
			case BuildNetworkPacket::Ping: {
				//game->PrintMessage(L"PING\n");
				break;
			}
			case BuildNetworkPacket::SingleBlockUpdate: {
				SingleBlockUpdatePacket* packet = (SingleBlockUpdatePacket*)basePacket;
				if (size != sizeof(*packet)) break;
				EnterCriticalSection(&game->host.world.zones_critical_section);
				EnterCriticalSection(&game->host.world.zones_mesh_critical_section);
				game->host.world.SetBlock(packet->position, packet->block, false);
				LeaveCriticalSection(&game->host.world.zones_mesh_critical_section);
				LeaveCriticalSection(&game->host.world.zones_critical_section);
				mod->GetWorldContainer()->SetBlock(packet->position, packet->block.red, packet->block.green, packet->block.blue, packet->block.type);
				mod->GetWorldContainer()->OutputFiles();
				// Update all connected clients
				for (auto connection : game->host.connections) {
					SendSingleBlockUpdate(connection.first, packet->position, packet->block, true);
				}
				break;
			}
			case BuildNetworkPacket::ZoneLoad: {
				ZoneLoadPacket* packet = (ZoneLoadPacket*)basePacket;
				if (size != sizeof(*packet)) break;
				std::vector<ZoneSaver::ZoneBlock*> blocks = mod->GetWorldContainer()->LoadZoneBlocks(packet->position.x, packet->position.y);

				ZoneUpdatePacket responsePacket(packet->position);
				cube::Block cubeBlock;
				cubeBlock.breakable = 0;
				cubeBlock.field_3 = 0;
				for (auto zoneBlock : blocks) {
					cubeBlock.red = zoneBlock->r;
					cubeBlock.green = zoneBlock->g;
					cubeBlock.blue = zoneBlock->b;
					cubeBlock.type = (cube::Block::Type)zoneBlock->type;
					responsePacket.entries.push_back(ZoneUpdatePacket::Entry(zoneBlock->x, zoneBlock->y, zoneBlock->z, cubeBlock));
				}
				responsePacket.Send(steamID, SCChannel);

			}
		}
	}
}

void BuildNetwork::ProcessSC() {
	cube::Game* game = cube::GetGame();
	u32 size;
	while (cube::SteamNetworking()->IsP2PPacketAvailable(&size, SCChannel)) {
		// Receive packet from server
		auto buffer = std::unique_ptr<u8[]>(new u8[size]);
		u32 receivedSize;
		CSteamID steamID;
		cube::SteamNetworking()->ReadP2PPacket(buffer.get(), size, &receivedSize, &steamID, SCChannel);

		// If this packet is not from the host we are supposed to be connected to, ignore it.
		if (steamID != game->client.host_steam_id) {
			continue;
		}

		BuildNetworkPacket* basePacket = (BuildNetworkPacket*)buffer.get();
		if (size < sizeof(*basePacket)) continue;

		switch (basePacket->id) {
			case BuildNetworkPacket::Ping:
				//game->PrintMessage(L"PING\n");
				break;
			case BuildNetworkPacket::SingleBlockUpdate: {
				SingleBlockUpdatePacket* packet = (SingleBlockUpdatePacket*)basePacket;
				if (size != sizeof(*packet)) break;
				mod->PlaceSingleBlock(packet->block, packet->position);  
				break;
			}
			case BuildNetworkPacket::ZoneUpdate: {
				ZoneUpdatePacket packet((u8*)basePacket, size);
				std::vector<std::pair<cube::Block, LongVector3>> blocks;
				blocks.reserve(packet.entries.size());
				for (ZoneUpdatePacket::Entry& entry : packet.entries) {
					LongVector3 position( entry.x + packet.position.x * cube::BLOCKS_PER_ZONE,
									      entry.y + packet.position.y * cube::BLOCKS_PER_ZONE,
										  entry.z);
					blocks.push_back(std::pair<cube::Block, LongVector3>(entry.block, position));
				}
				mod->QueueBlocks(blocks);
			}
		}
	}
}

