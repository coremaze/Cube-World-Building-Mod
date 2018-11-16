
#ifndef PACKETS_H
#define PACKETS_H
#include <vector>

class QueuePacket{
public:
    SOCKET socket;
    char* data;
    unsigned int data_size;
    QueuePacket(SOCKET _socket, char* _data, unsigned int _data_size){
        this->socket = _socket;
        this->data = _data;
        this->data_size = _data_size;
    }
};
std::vector<QueuePacket*> packetQueue;
CRITICAL_SECTION packet_queue_critical_section;

std::vector<SOCKET> knownSockets;
CRITICAL_SECTION known_sockets_critical_section;

void PurgeSocket(SOCKET socket){
    EnterCriticalSection(&known_sockets_critical_section);

    std::vector<SOCKET>::iterator iter = knownSockets.begin();
    while (iter != knownSockets.end()){
        SOCKET sock = (*iter);
        if (socket == sock){
            iter = knownSockets.erase(iter);
        }
        else {
            ++iter;
        }
    }

    LeaveCriticalSection(&known_sockets_critical_section);
}

bool AddSocket(SOCKET socket){
    EnterCriticalSection(&known_sockets_critical_section);

    for (SOCKET s : knownSockets){
        if (s == socket){
            LeaveCriticalSection(&known_sockets_critical_section);
            return false;
        }
    }

    knownSockets.push_back(socket);
    LeaveCriticalSection(&known_sockets_critical_section);
    return true;
}

void AddPacket(SOCKET socket, char* data, unsigned int data_size){
    EnterCriticalSection(&packet_queue_critical_section);
    char* newData = new char[data_size];
    memcpy(newData, data, data_size);
    QueuePacket* pkt = new QueuePacket(socket, newData, data_size);
    packetQueue.push_back(pkt);
    LeaveCriticalSection(&packet_queue_critical_section);
}

void SendQueuedPackets(SOCKET socket){
    //AddSocket(socket);
    EnterCriticalSection(&packet_queue_critical_section);

    std::vector<QueuePacket*>::iterator iter = packetQueue.begin();
    while (iter != packetQueue.end()){
        QueuePacket* pkt = (*iter);
        if (pkt->socket == socket){
//            printf("To %d: ", pkt->socket);
//            for (int i = 0; i<pkt->data_size; i++){
//                printf("%X ", (unsigned int)(unsigned char)pkt->data[i]);
//            }
//            printf("\n");
            send(socket, pkt->data, pkt->data_size, 0);
            delete[] pkt->data;
            delete pkt;
            iter = packetQueue.erase(iter);
        }
        else {
            ++iter;
        }
    }

    LeaveCriticalSection(&packet_queue_critical_section);
}

void PacketsInit(){
    InitializeCriticalSection(&packet_queue_critical_section);
    InitializeCriticalSection(&known_sockets_critical_section);
}

#endif // PACKETS_H
