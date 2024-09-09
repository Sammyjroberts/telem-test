#include "telemetry_udp.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> // For htons and networking functions

// Prototype for the send_fragment function

// Function to create and send a packet in fragments
void send_packet_in_fragments(const char *payload, uint16_t packet_id, const char *ip, uint16_t port)
{
  TelemetryPacket packet;
  packet.packet_id = htons(packet_id);

  memset(packet.payload, ' ', sizeof(packet.payload));       // Pad with spaces
  strncpy((char *)packet.payload, payload, strlen(payload)); // Copy message

  // Define fragment sizes
  size_t total_length = strlen(payload);   // Length of the message
  size_t fragment_size = total_length / 3; // Divide message into 3 parts

  // Send three fragments, ensuring each only sends the relevant part of the payload
  // Fragment 1
  if (send_fragment(packet.payload, 0, fragment_size, 1, ip, port, 0, 3) != 0) // Sequence 1
  {
    printf("Failed to send fragment 1 of packet ID %d\n", packet_id);
    return;
  }

  // Fragment 3
  if (send_fragment(packet.payload, 2 * fragment_size, total_length - 2 * fragment_size, 3, ip, port, 1, 3) != 0) // Sequence 3, last fragment
  {
    printf("Failed to send fragment 3 of packet ID %d\n", packet_id);
    return;
  }

  // Fragment 2
  if (send_fragment(packet.payload, fragment_size, fragment_size, 2, ip, port, 0, 3) != 0) // Sequence 2
  {
    printf("Failed to send fragment 2 of packet ID %d\n", packet_id);
    return;
  }
}

int main()
{
  // Define IP and port for sending
  const char *ip = "127.0.0.1";
  uint16_t port = 12345;

  // Packet 1
  send_packet_in_fragments("hello world", 1, ip, port);

  // Packet 2
  send_packet_in_fragments("goodbye moon", 2, ip, port);

  // Packet 3
  send_packet_in_fragments("lorem ipsum", 3, ip, port);

  // Packet 4
  send_packet_in_fragments("foo bar baz", 4, ip, port);

  return 0;
}
