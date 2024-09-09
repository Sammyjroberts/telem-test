#include "telemetry_udp.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> // For htons and networking functions
#include <pthread.h>   // For threading

// Prototype for the send_fragment function

// Struct to pass multiple arguments to the thread function
typedef struct
{
  const char *payload;
  uint16_t packet_id;
  const char *ip;
  uint16_t port;
} PacketArgs;

// Function to create and send a packet in fragments
void *send_packet_in_fragments(void *args)
{
  PacketArgs *packetArgs = (PacketArgs *)args;
  TelemetryPacket packet;
  packet.packet_id = htons(packetArgs->packet_id);
  printf("packet.packet_id: %d\n", packet.packet_id);
  memset(packet.payload, ' ', sizeof(packet.payload));                               // Pad with spaces
  strncpy((char *)packet.payload, packetArgs->payload, strlen(packetArgs->payload)); // Copy message

  // Define fragment sizes
  size_t total_length = strlen(packetArgs->payload); // Length of the message
  size_t fragment_size = total_length / 3;           // Divide message into 3 parts

  // Send three fragments, ensuring each only sends the relevant part of the payload
  // Fragment 1
  if (send_fragment(packet.payload, packet.packet_id, 0, fragment_size, 1, packetArgs->ip, packetArgs->port, 0, 3) != 0) // Sequence 1
  {
    printf("Failed to send fragment 1 of packet ID %d\n", packet.packet_id);
    return NULL;
  }

  // Fragment 3
  if (send_fragment(packet.payload, packet.packet_id, 2 * fragment_size, total_length - 2 * fragment_size, 3, packetArgs->ip, packetArgs->port, 1, 3) != 0) // Sequence 3, last fragment
  {
    printf("Failed to send fragment 3 of packet ID %d\n", packetArgs->packet_id);
    return NULL;
  }

  // Fragment 2
  if (send_fragment(packet.payload, packet.packet_id, fragment_size, fragment_size, 2, packetArgs->ip, packetArgs->port, 0, 3) != 0) // Sequence 2
  {
    printf("Failed to send fragment 2 of packet ID %d\n", packetArgs->packet_id);
    return NULL;
  }

  return NULL;
}

int main()
{
  // Define IP and port for sending
  const char *ip = "127.0.0.1";
  uint16_t port = 12345;

  // Define the packet arguments for each thread
  PacketArgs packets[] = {
      {"hello world", 1, ip, port},
      {"goodbye moon", 2, ip, port},
      {"lorem ipsum", 3, ip, port},
      {"foo bar baz", 4, ip, port}};

  // Create threads
  pthread_t threads[4];
  for (int i = 0; i < 4; i++)
  {
    if (pthread_create(&threads[i], NULL, send_packet_in_fragments, &packets[i]) != 0)
    {
      perror("Failed to create thread");
      return -1;
    }
  }

  // Join threads to wait for all to complete
  for (int i = 0; i < 4; i++)
  {
    pthread_join(threads[i], NULL);
  }

  return 0;
}
