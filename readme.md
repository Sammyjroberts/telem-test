## Project Overview

This project tests two main functionalities:

1. Sending fragmented packets over UDP with a delimiter.
2. Interoperating between C and Go code.

## How to Run

1. **Build the C Sender:**

   - Navigate to the `c_sender` directory.
   - Run `make` to build the C sender application.

2. **Run the Go Receiver:**

   - At the root level of the project, execute `go run .` to start the Go receiver.

3. **Send Packets:**
   - Run the `c_sender` application to send fragmented packets.

You should see detailed output on the console. Look for the final result, which should display "hello world" as the reassembled message.

## Future Improvements

- Experiment with sending packets with different IDs to ensure the receiver can handle multiple messages simultaneously using the map.
- Explore adding delimiters at the start of packets to simplify reassembly and improve packet handling.

Thank you for checking out this quick test project!
