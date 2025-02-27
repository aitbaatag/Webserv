import socket
import sys

def send_message(message = "hello world"):
    # Define the host and port
    host = 'localhost'
    port = 8080
    # // http request 
    message = " "
    # Create a socket object
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # Connect to the server
        s.connect((host, port))
        
        # Send the message
        s.sendall(message.encode('utf-8'))
        
        # Optionally, receive a response (if the server sends one)
        # set timeout to 5 seconds
        response = s.recv(1024)
        print(f"Received: {response.decode('utf-8')}")

if __name__ == "__main__":
    send_message(sys.argv[1])
