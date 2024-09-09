#include "telemetry_udp.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> // For htons and networking functions

// Prototype for the send_fragment function

int main()
{
  TelemetryPacket packet;
  create_telemetry_packet(&packet); // Create a sample telemetry packet

  // Replace with the correct IP and port for your test
  const char *ip = "127.0.0.1";
  uint16_t port = 12345;
  const char *message = "hello world";
  memset(packet.payload, ' ', sizeof(packet.payload));       // Pad with spaces
  strncpy((char *)packet.payload, message, strlen(message)); // Copy message

  // Define fragment sizes
  size_t total_length = strlen(message);   // Length of the message
  size_t fragment_size = total_length / 3; // Divide message into 3 parts

  // Send three fragments, ensuring each only sends the relevant part of the payload
  // Fragment 1: "hel"
  if (send_fragment(packet.payload, 0, fragment_size, 1, ip, port, 0, 3) != 0) // Sequence 1
  {
    return -1;
  }

  // Fragment 3: "world"
  if (send_fragment(packet.payload, 2 * fragment_size, total_length - 2 * fragment_size, 3, ip, port, 1, 3) != 0) // Sequence 3, last fragment
  {
    return -1;
  }

  // Fragment 2: "lo "
  if (send_fragment(packet.payload, fragment_size, fragment_size, 2, ip, port, 0, 3) != 0) // Sequence 2
  {
    return -1;
  }

  return 0;
}
