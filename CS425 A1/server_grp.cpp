#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 300

struct Client {
    int socket;
    std::string username;
};

struct Group {
    std::string name;
    std::unordered_set<std::string> members;
};

std::unordered_map<std::string, std::string> users;
std::unordered_map<int, Client> clients;
std::unordered_map<std::string, Group> groups;
std::unordered_map<std::string, int> username_to_socket;
std::mutex clients_mutex, groups_mutex, username_mutex;

void load_users() {
    std::ifstream file("users.txt");
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            users[line.substr(0, pos)] = line.substr(pos + 1);
        }
    }
}

void send_message(int client_socket, const std::string& message) {
    send(client_socket, message.c_str(), message.size(), 0);
}
void broadcast_join(const std::string& username) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& pair : clients) {
        if (pair.second.username != username) {
            send_message(pair.first, username + " has joined the chat.");
        }
    }
}

bool authenticate_user(int client_socket, std::string& username) {
    char buffer[BUFFER_SIZE];
    send_message(client_socket, "Enter username: ");
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    username = std::string(buffer);

    send_message(client_socket, "Enter password: ");
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    std::string password = std::string(buffer);

    if (users.find(username) != users.end() && users[username] == password) {
        send_message(client_socket, "Authentication successful. Welcome to the server!\n");
        return true;
    } else {
        send_message(client_socket, "Authentication failed. Disconnecting.\n");
        return false;
    }
}

void broadcast_message(const std::string& sender, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& pair : clients) {
        if (pair.second.username != sender) {
            send_message(pair.first, sender + ": " + message);
        }
    }
}


void private_message(const std::string& sender, const std::string& recipient, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& pair : clients) {
        if (pair.second.username == recipient) {
            send_message(pair.first, "PM from " + sender + ": " + message);
            return;
        }
    }
    for (const auto& pair : clients) {
        if (pair.second.username == sender) {
            send_message(pair.first, "User " + recipient + " not found.");
            return;
        }
    }
}

void create_group(const std::string& group_name, const std::string& creator) {
    std::lock_guard<std::mutex> lock(groups_mutex);
    std::lock_guard<std::mutex> un_lock(username_mutex);
    if (groups.find(group_name) == groups.end()) {
        groups[group_name] = {group_name, {creator}};
        auto it = username_to_socket.find(creator);
        if (it != username_to_socket.end()) {
            send_message(it->second, "Group " + group_name + " created successfully.");
        }
    } else {
        auto it = username_to_socket.find(creator);
        if (it != username_to_socket.end()) {
            send_message(it->second, "Group " + group_name + " already exists.");
        }
    }
}

void join_group(const std::string& group_name, const std::string& username) {
    std::lock_guard<std::mutex> lock(groups_mutex);
    std::lock_guard<std::mutex> un_lock(username_mutex);
    if (groups.find(group_name) != groups.end()) {
        groups[group_name].members.insert(username);
        auto it = username_to_socket.find(username);
        if (it != username_to_socket.end()) {
            send_message(it->second, "Joined group " + group_name + " successfully.");
        }
    } else {
        auto it = username_to_socket.find(username);
        if (it != username_to_socket.end()) {
            send_message(it->second, "Group " + group_name + " does not exist.");
        }
    }
}

void leave_group(const std::string& group_name, const std::string& username) {
    std::lock_guard<std::mutex> lock(groups_mutex);
    std::lock_guard<std::mutex> un_lock(username_mutex);
    
    auto group_it = groups.find(group_name);
    if (group_it == groups.end()) {
        auto user_it = username_to_socket.find(username);
        if (user_it != username_to_socket.end()) {
            send_message(user_it->second, "Group " + group_name + " does not exist.");
        }
        return;
    }

    auto& members = group_it->second.members;
    auto member_it = members.find(username);
    if (member_it == members.end()) {
        auto user_it = username_to_socket.find(username);
        if (user_it != username_to_socket.end()) {
            send_message(user_it->second, "You're not a member of group " + group_name + ".");
        }
        return;
    }

    members.erase(username);
    auto user_it = username_to_socket.find(username);
    if (user_it != username_to_socket.end()) {
        send_message(user_it->second, "Left group " + group_name + " successfully.");
    }

    // Optional: Remove group if empty
    if (members.empty()) {
        groups.erase(group_it);
    }
}


void group_message(const std::string& sender, const std::string& group_name, const std::string& message) {
    std::lock_guard<std::mutex> lock(groups_mutex);
    std::lock_guard<std::mutex> un_lock(username_mutex);
    if (groups.find(group_name) != groups.end()) {
        if (groups[group_name].members.find(sender) != groups[group_name].members.end()) {
            for (const auto& member : groups[group_name].members) {
                if (member != sender) {
                    auto it = username_to_socket.find(member);
                    if (it != username_to_socket.end()) {
                        send_message(it->second, "Group " + group_name + " - " + sender + ": " + message);
                    }
                }
            }
        } else {
            auto it = username_to_socket.find(sender);
            if (it != username_to_socket.end()) {
                send_message(it->second, "You are not a member of group " + group_name + ".");
            }
        }
    } else {
        auto it = username_to_socket.find(sender);
        if (it != username_to_socket.end()) {
            send_message(it->second, "Group " + group_name + " does not exist.");
        }
    }
}

void handle_client(int client_socket) {
    std::string username;
    if (!authenticate_user(client_socket, username)) {
        close(client_socket);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        std::lock_guard<std::mutex> un_lock(username_mutex);
        clients[client_socket] = {client_socket, username};
        username_to_socket[username] = client_socket;
    }
broadcast_join(username);  
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }

        std::string message(buffer);
        std::istringstream iss(message);
        std::string command;
        iss >> command;

        if (command == "/broadcast") {
            std::string content;
            std::getline(iss >> std::ws, content);
            broadcast_message(username, content);
        } else if (command == "/msg") {
            std::string recipient, content;
            iss >> recipient;
            std::getline(iss >> std::ws, content);
            private_message(username, recipient, content);
        } else if (command == "/create") {
            std::string group_name;
            iss >> group_name;
            create_group(group_name, username);
        } else if (command == "/join") {
            std::string group_name;
            iss >> group_name;
            join_group(group_name, username);
        } else if (command == "/leave") {
            std::string group_name;
            iss >> group_name;
            leave_group(group_name, username);
        } else if (command == "/group") {
            std::string group_name, content;
            iss >> group_name;
            std::getline(iss >> std::ws, content);
            group_message(username, group_name, content);
        } else {
            send_message(client_socket, "Unknown command. Available commands: /broadcast, /msg, /create, /join, /leave, /group");
        }
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        std::lock_guard<std::mutex> un_lock(username_mutex);
        clients.erase(client_socket);
        username_to_socket.erase(username);
    }
    close(client_socket);
}

int main() {
    load_users();

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(12345);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error binding socket." << std::endl;
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        std::cerr << "Error listening on socket." << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port 12345..." << std::endl;

    while (true) {
        sockaddr_in client_address{};
        socklen_t client_address_size = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);

        if (client_socket < 0) {
            std::cerr << "Error accepting client connection." << std::endl;
            continue;
        }

        std::thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    close(server_socket);
    return 0;
}
