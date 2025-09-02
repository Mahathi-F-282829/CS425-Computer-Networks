# Chat Server with Groups and Private Messages

## Features Implemented

- User authentication
- Private messaging
- Group creation and management
- Group messaging
- Broadcasting messages
- Multi-threaded server for handling multiple concurrent clients

## Features Not Implemented

- File transfer
- Encryption of messages
- User registration (users are loaded from a file)

## Design Decisions

### 1. Threading Model

We chose to create a new thread for each client connection because it allows multiple clients to interact with the server simultaneously without blocking. This ensures efficient concurrency and responsiveness. We considered an event-driven model using select() or epoll(), but this would have increased complexity significantly while handling multiple message types and commands. Instead, using std::thread allows us to focus more on core functionality rather than managing non-blocking I/O.

### 2. Data Structures

We used unordered maps to store client information and groups because they provide average O(1) lookup time, making them optimal for quick user retrieval. This decision was made to ensure that our server can handle multiple users efficiently. Using vectors or lists would have resulted in O(n) lookups, making it less efficient as the number of users increased.

- Unordered Maps:
  - std::unordered_map<int, Client>: Maps client sockets to usernames.
  - std::unordered_map<std::string, Group>: Stores group memberships.
  - std::unordered_map<std::string, int>: Maps usernames to their respective socket descriptors.

### 3. Synchronization

Since multiple clients interact with shared data structures, race conditions could arise. To prevent this, we use mutex locks (********`std::mutex`********\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*) on shared data structures, ensuring only one thread modifies them at a time.

We considered using read-write locks (********`std::shared_mutex`********\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*), which allow multiple readers but only one writer, but since writes occur frequently in a chat server, the performance gain would be minimal compared to using basic mutexes.

### 4. Socket Programming

We opted for TCP sockets over UDP because TCP provides reliable, ordered, and error-checked message delivery, which is crucial for chat applications. UDP would have resulted in packet loss and out-of-order messages, making it unsuitable for our use case.

### 5. Error Handling and Fault Tolerance

To prevent crashes and unexpected behaviors, we implemented robust error handling:

- Invalid commands receive an appropriate error message instead of causing crashes.
- Clients who disconnect abruptly are removed from the system to avoid stale connections.
- Failed authentication results in immediate disconnection to prevent brute-force attacks.

We considered implementing automatic reconnection, but keeping this on the client-side allows more flexibility without overcomplicating the server.

### 6. Message Parsing and Command Handling

Commands are parsed efficiently using string tokenization:

- /msg bob hello is parsed into command = "/msg", recipient = "bob", message = "hello".
- /group CS425 hey everyone! follows a similar parsing logic.

We initially considered using JSON-based messages for structure but decided against it due to unnecessary complexity and parsing overhead.

### 7. Scalability Considerations

- The current limit is \*\*100 clients (MAX_CLIENTS defined as 100 concurrent clients sending messages).

## Implementation

### Key Functions
1. `handle_client(int client_socket)` - Manages client connection lifecycle, including authentication and message handling.
2. `authenticate_user(int client_socket, std::string& username)` - Verifies user credentials against the loaded user database.
3. `broadcast_message(const std::string& sender, const std::string& message)` - Sends a message to all connected clients except the sender.
4. `private_message(const std::string& sender, const std::string& recipient, const std::string& message)` - Sends a message to a specific user.
5. `create_group(const std::string& group_name, const std::string& creator)` - Creates a new group.
6. `join_group(const std::string& group_name, const std::string& username)` - Allows a user to join an existing group.
7. `leave_group(const std::string& group_name, const std::string& username)` - Removes a user from a group.
8. `group_message(const std::string& sender, const std::string& group_name, const std::string& message)` - Sends a message to all members of a group.

## Code Flow
          +--------------------+
          |  Start Server      |
          +--------------------+
                   |
                   v
          +----------------------+
          | Load Users from File  |
          +----------------------+
                   |
                   v
          +----------------------+
          | Start Listening on    |
          | Port 12345            |
          +----------------------+
                   |
                   v
          +----------------------+
          | Accept Client         |
          | Connection            |
          +----------------------+
                   |
                   v
          +----------------------+
          | Authenticate User     |
          +----------------------+
                   |
                   v
          +----------------------+
          | Handle Client         |
          | Messages              |
          +----------------------+
                   |
                   v
          +----------------------+
          | Process Commands      |
          +----------------------+
             |       |       |       |
             v       v       v       v
     +--------------+  +---------------+  +---------------+  +----------------+
     | Broadcast    |  | Private Msg    |  | Group Msg     |  | Create/Join/   |
     | Message      |  |               |  |               |  | Leave Group     |
     +--------------+  +---------------+  +---------------+  +----------------+
                   |
                   v
          +----------------------+
          | Client Disconnects    |
          +----------------------+
                   |
                   v
          +----------------------+
          | Remove from Active    |
          | Connections           |
          +----------------------+
                   |
                   v
          +----------------------+
          | Server Continues      |
          | Accepting Clients     |
          +----------------------+

## How to Run and Test

### Compilation
1. Ensure you have a C++ compiler installed (e.g., g++)
2. Open a terminal in the project directory
3. Run make to compile both the server and client executables

### Running the Server
1. Start the server: `./server_grp`
2. The server will start listening on port 12345

### Running Clients
1. Open a new terminal for each client
2. Start a client: `./client_grp`
3. Enter username and password when prompted

### Testing Features
1. Private message: `/msg <username> <message>`
2. Broadcast: `/broadcast <message>`
3. Create group: `/create <group_name>`
4. Join group: `/join <group_name>`
5. Group message: `/group <group_name> <message>`
6. Leave group: `/leave <group_name>`

### Testing Scenarios
1. Ran multiple clients and test all commands
2. Verifed that messages are received by intended recipients
3. Tested group functionality with multiple users
4. Attempted to join non-existent groups and send messages to non-existent users and creating already existing froups to test error handling


## Stress Testing

- Used a Python script to simulate 100-150 concurrent clients
- Message interval of 0.1 seconds between client actions
- Monitored server performance and resource usage

## Server Restrictions

- Maximum Clients: 100 (MAX_CLIENTS defined in code).
- Maximum Message Size: 1024 bytes (BUFFER_SIZE).
- Maximum Groups: No explicit limit in implementation.
- Maximum Members Per Group: No explicit limit in implementation.

## Challenges Faced

1. Ensuring thread safety when accessing shared resources.
3. Implementing efficient group messaging without excessive locking.
4. Parsing user input correctly to handle various commands properly.
5. Managing multiple concurrent clients efficiently without thread overload.


## Sources Referred
- Stack Overflow (for specific implementation questions)

## Declaration

I declare that this assignment is our own work and that we have not indulged in plagiarism of any kind.

## Feedback

- The assignment was a great learning experience in socket programming, multithreading, and synchronization.
