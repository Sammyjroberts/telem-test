package main

/*
#include <arpa/inet.h>               // Include the header file for ntohs
#include "telemetry_udp.h"           // Include the telemetry header file

// Explicitly declare the function to ensure CGo recognizes it
unsigned short ntohs(unsigned short netshort);
*/
import "C"
import (
	"fmt"
	"log"
	"net"
	"sort"
	"strings"
	"sync"
	"unsafe"
)

type Fragment struct {
	sequenceNumber int
	payload        string
	length         int
	totalFragments int
}

var (
	fragmentsMap    = make(map[int]Fragment) // Store fragments by sequence number
	mu              sync.Mutex               // Mutex for synchronizing access to fragmentsMap
	receivedPackets = 0                      // Count of received unique fragments
	delimiter       = "|||"                  // Delimiter to mark the end of the message
)

func handleTelemetryPacket(data []byte) (Fragment, bool) {
	if len(data) < int(unsafe.Sizeof(C.TelemetryPacket{})) {
		log.Printf("Received packet size %d is less than expected %d bytes\n", len(data), unsafe.Sizeof(C.TelemetryPacket{}))
		return Fragment{}, false
	}

	packet := (*C.TelemetryPacket)(unsafe.Pointer(&data[0]))
	packetID := C.ntohs(packet.packet_id)
	packetLength := C.ntohs(packet.packet_length)
	sequenceNumber := C.ntohs(packet.sequence_number)
	totalFragments := C.ntohs(packet.fragment_total)

	// Extract the payload from the packet as a byte slice
	payload := C.GoBytes(unsafe.Pointer(&packet.payload), C.int(packetLength))

	// Convert the payload to a string
	payloadStr := string(payload)

	// Log details of the packet
	fmt.Printf("Received Packet ID: 0x%04X, Sequence: %d, Total Fragments: %d\n", packetID, sequenceNumber, totalFragments)
	fmt.Printf("Packet Length: %d bytes\n", packetLength)
	fmt.Printf("Payload Fragment: [%s]\n", payloadStr) // Print the fragment

	return Fragment{sequenceNumber: int(sequenceNumber), payload: payloadStr, length: int(packetLength), totalFragments: int(totalFragments)}, true
}

func reassembleAndPrintMessage() {
	// Lock and sort the fragments by sequence number
	mu.Lock()
	defer mu.Unlock()

	// Sort keys of the map (sequence numbers)
	sequenceNumbers := make([]int, 0, len(fragmentsMap))
	for seq := range fragmentsMap {
		sequenceNumbers = append(sequenceNumbers, seq)
	}
	sort.Ints(sequenceNumbers)

	// Print map contents for debugging
	for _, seq := range sequenceNumbers {
		fmt.Printf("Sequence: %d, Payload: [%s]\n", seq, fragmentsMap[seq].payload)
		fmt.Printf("Length: %d\n", fragmentsMap[seq].length)
	}

	// Combine fragments in the correct order
	var messageBuilder strings.Builder
	for _, seq := range sequenceNumbers {
		messageBuilder.WriteString(fragmentsMap[seq].payload)
	}

	fullMessage := strings.TrimSuffix(messageBuilder.String(), delimiter)

	// Output the reassembled full message
	fmt.Printf("Reassembled Full Message: [%s]\n", fullMessage)

	// Clear the fragments map and reset counters for the next set of fragments
	fragmentsMap = make(map[int]Fragment)
	receivedPackets = 0
}

func main() {
	addr := net.UDPAddr{
		Port: 12345,
		IP:   net.ParseIP("0.0.0.0"),
	}

	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		log.Fatalf("Failed to set up UDP listener: %v\n", err)
	}
	defer conn.Close()

	log.Printf("Listening on %s\n", addr.String())

	buffer := make([]byte, 1024)

	for {
		n, remoteAddr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("Error receiving packet: %v\n", err)
			continue
		}

		log.Printf("Received %d bytes from %s\n", n, remoteAddr)

		// Handle the received packet and get the fragment
		fragment, ok := handleTelemetryPacket(buffer[:n])
		if !ok {
			continue
		}

		// Lock and store the fragment in the map using its sequence number
		mu.Lock()
		if _, exists := fragmentsMap[fragment.sequenceNumber]; !exists {
			fragmentsMap[fragment.sequenceNumber] = fragment
			receivedPackets++
		}
		mu.Unlock()

		// Check if all expected fragments are received
		if receivedPackets == fragment.totalFragments {
			fmt.Println("All fragments received. Reassembling full message...")
			reassembleAndPrintMessage()
		}
	}
}
