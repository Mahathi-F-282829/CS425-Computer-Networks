import socket
import threading
import random
import time

# Configuration
SERVER_IP = '127.0.0.1'
SERVER_PORT = 12345
NUM_CLIENTS = 250 # Adjust based on your testing needs
MESSAGE_INTERVAL = 0.1  # Seconds between messages

# User credentials from your users.txt
USERS = [
    ('alice', 'password123'),
    ('bob', 'qwerty456'),
    ('charlie', 'secure789'),
    ('david', 'helloWorld!'),
    ('eve', 'trustno1'),
    ('frank', 'letmein'),
    ('grace', 'passw0rd')
]

GROUPS = ['CS425', 'Networks', 'ChatGroup', 'TestGroup']

def client_instance(client_id):
    try:
        # Select random user credentials
        user = random.choice(USERS)
        username, password = user
        
        # Create socket and connect
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER_IP, SERVER_PORT))
        
        # Handle authentication
        auth_data = s.recv(1024).decode()
        if "Enter username" in auth_data:
            s.send(username.encode())
            auth_data = s.recv(1024).decode()
            if "Enter password" in auth_data:
                s.send(password.encode())
                auth_result = s.recv(1024).decode()
                if "Authentication successful" not in auth_result:
                    print(f"Client {client_id} auth failed for {username}")
                    return
            else:
                print(f"Client {client_id} missing password prompt")
                return
        else:
            print(f"Client {client_id} missing username prompt")
            return

        print(f"Client {client_id} ({username}) connected successfully")
        
        # Random actions
        while True:
            # Random command selection
            action = random.choice([
                'broadcast', 
                'private', 
                'create_group',
                'join_group',
                'group_msg'
            ])
            
            if action == 'broadcast':
                msg = f"/broadcast Stress test message from {username}"
                s.send(msg.encode())
                
            elif action == 'private':
                target_user = random.choice(USERS)[0]
                msg = f"/msg {target_user} Private stress test from {username}"
                s.send(msg.encode())
                
            elif action == 'create_group':
                group = random.choice(GROUPS)
                s.send(f"/create {group}".encode())
                
            elif action == 'join_group':
                group = random.choice(GROUPS)
                s.send(f"/join {group}".encode())
                
            elif action == 'group_msg':
                group = random.choice(GROUPS)
                msg = f"/group {group} Group stress test from {username}"
                s.send(msg.encode())
            
            time.sleep(MESSAGE_INTERVAL)
            
    except Exception as e:
        print(f"Client {client_id} error: {str(e)}")
    finally:
        s.close()

if __name__ == "__main__":
    print(f"Starting stress test with {NUM_CLIENTS} clients")
    threads = []
    
    for i in range(NUM_CLIENTS):
        thread = threading.Thread(target=client_instance, args=(i,))
        threads.append(thread)
        thread.start()
        time.sleep(0.1)  # Stagger connection attempts
    
    for thread in threads:
        thread.join()
