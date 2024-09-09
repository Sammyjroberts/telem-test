#include "telemetry_udp.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

const char DELIMITER[4] = "|||"; // Includes null terminator

// Function to send a telemetry packet over UDP
int send_telemetry_packet(const TelemetryPacket *packet, const char *ip, uint16_t port)
{
  int sockfd;
  struct sockaddr_in server_addr;

  // Create a UDP socket
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("Socket creation failed");
    return -1;
  }

  // Configure the server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip);

  // Send the telemetry packet over UDP
  ssize_t sent_bytes = sendto(sockfd, packet, sizeof(*packet), 0,
                              (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (sent_bytes < 0)
  {
    perror("Failed to send packet");
    close(sockfd);
    return -1;
  }

  printf("Telemetry packet sent successfully (%ld bytes)\n", sent_bytes);

  // Close the socket
  close(sockfd);
  return 0;
}

int send_fragment(const uint8_t *payload, int packetID, size_t offset, size_t length, uint16_t sequence_number, const char *ip, uint16_t port, int is_last, int total_fragments)
{
  TelemetryPacket fragment;
  fragment.packet_id = htons(packetID);              // Example packet ID for fragments
  fragment.packet_length = htons(length);            // Length of the fragment
  fragment.sequence_number = htons(sequence_number); // Set the sequence number
  fragment.fragment_total = htons(total_fragments);  // Set the total number of fragments
  // Copy only the relevant fragment payload from the specified offset
  memcpy(fragment.payload, payload + offset, length); // Copy only the required fragment without padding

  // Print the fragment payload to verify contents
  printf("Fragment %d payload: [%.*s]\n", sequence_number, (int)length, fragment.payload);

  // Add delimiter at the end of the last fragment
  if (is_last)
  {
    size_t delimiter_len = strlen(DELIMITER);
    if (length + delimiter_len <= sizeof(fragment.payload))
    {
      memcpy(fragment.payload + length, DELIMITER, delimiter_len); // Append delimiter
      fragment.packet_length = htons(length + delimiter_len);      // Update length
    }
    else
    {
      printf("Error: Fragment size with delimiter exceeds payload limit.\n");
      return -1;
    }
  }

  // Send the fragment using the existing send function
  printf("Sending fragment %d to %s:%d\n", sequence_number, ip, port);
  if (send_telemetry_packet(&fragment, ip, port) == 0)
  {
    printf("Fragment %d sent successfully.\n", sequence_number);
    return 0;
  }
  else
  {
    printf("Failed to send fragment %d.\n", sequence_number);
    return -1;
  }
}