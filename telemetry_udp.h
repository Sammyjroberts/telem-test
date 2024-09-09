#ifndef TELEMETRY_UDP_H
#define TELEMETRY_UDP_H
#include <stddef.h> // For size_t
#include <stdint.h> // For uint16_t, uint8_t

// Define the telemetry packet structure
typedef struct
{
  uint16_t packet_id;       // Packet identifier
  uint16_t packet_length;   // Length of the payload
  uint16_t sequence_number; // Sequence number for ordering
  uint16_t fragment_total;  // Offset of the fragment
  uint8_t payload[256];     // Example payload size; adjust as necessary
} TelemetryPacket;

extern const char DELIMITER[4];

// Function to create a sample telemetry packet
void create_telemetry_packet(TelemetryPacket *packet);

// Function to send a telemetry packet over UDP
int send_telemetry_packet(const TelemetryPacket *packet, const char *ip, uint16_t port);
int send_fragment(const uint8_t *payload, size_t offset, size_t length, uint16_t sequence_number, const char *ip, uint16_t port, int is_last, int total_fragments);

#endif // TELEMETRY_UDP_H