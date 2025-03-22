#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> //Internet protocol family
#include <netinet/in.h> //Defines structures and constants related to Internet addresses and ports 
#include <arpa/inet.h>  //Provides functions for manipulating IP addresses and converting them between human-readable strings and binary formats
#include <syslog.h>
#include <signal.h>
//In Linux, signals are a form of inter-process communication (IPC) used to notify a process that a specific event has occurred. 
//e.g SIGINT (2): Sent when the user presses Ctrl+C to interrupt a process.
#include <fcntl.h>
/*The <fcntl.h> header file in Linux provides functions and constants for file control operations. 
These operations allow you to manipulate file descriptors, which are low-level handles to files, pipes, sockets, and other I/O resources. 
*/
#include <errno.h>

#define DATA_FILE "/var/tmp/aesdsocketdata"
#define MAX_PACKET_SIZE 1048576 // 1 MB (adjust as needed)
volatile sig_atomic_t shutdown_flag = 0;


// Signal handler for SIGINT and SIGTERM
void handle_signal(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGINT || sig == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");

        // Delete the file after transmission is complete
        if (remove(DATA_FILE) != 0) {
            syslog(LOG_ERR, "Failed to delete file: %s", DATA_FILE);
        } else {
            syslog(LOG_INFO, "Deleted file: %s", DATA_FILE);
        }

        shutdown_flag = 1;
    }
}

// Function to append data to the file
void append_to_file(const char *file_path, const char *data, size_t data_len) {
    int fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Failed to open file");
        return;
    }
    write(fd, data, data_len);
    close(fd);
}

// Function to read the file and send its content to the client
void send_file_content(int client_fd, const char *file_path) {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return;
    }

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(client_fd, buffer, bytes_read);
    }

    close(fd);
}



int create_socket()
{
    int socket_fd = 0;
    struct sockaddr_in socket_addr; //struct that describes an internet socket address
    
    socket_fd = socket(AF_INET, SOCK_STREAM, 0); //IPv4 Internet protocols, connection-oriented communication using the TCP protocol (stream of bytes)
    if(socket_fd == -1)
    {
        perror("Failed to create socket");
        return -1;
    }

    //Configure the server address
    memset(&socket_addr, 0, sizeof(socket_addr)); //fills the entire server_addr structure with zeros. socket_addr - maps to a server socket location
    socket_addr.sin_family = AF_INET; //IPV4
    socket_addr.sin_port = htons(9000);
    //htons() function makes sure that numbers are stored in memory in network byte order, which is with the most significant byte first

    //Bind socket
    /*it’s common to bind a socket to INADDR_ANY (or 0.0.0.0), which means the socket will accept connections on all available network interfaces. 
      This is often used for servers that need to listen on all IP addresses assigned to the machine.*/
    socket_addr.sin_addr.s_addr = INADDR_ANY; /* Address to accept any incoming messages.  */

    //Bind the socket to port 9000
    if(bind(socket_fd, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) == -1)
    {
        perror("Failed to bind socket");
        close(socket_fd);
        return -1;
    }

    // Listen for connections
    if (listen(socket_fd, 5) == -1) { //Prepare to accept connections on socket FD. 5 connection requests will be queued before further requests are refused.
        perror("Failed to listen");
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

void handle_client(int client_fd, const char *client_ip) {
    char *packet = NULL;
    size_t packet_size = 0;
    size_t total_received = 0;
    char buffer[1024];
    ssize_t bytes_received;

    // Dynamically allocate memory for the packet
    packet = (char *)malloc(MAX_PACKET_SIZE);
    if (packet == NULL) {
        syslog(LOG_ERR, "Failed to allocate memory for packet");
        close(client_fd);
        return;
    }

    // Open file for appending
    FILE *fp = fopen(DATA_FILE, "a+");
    if (fp == NULL) {
        perror("Failed to open file");
        free(packet);
        close(client_fd);
        return;
    }

    // Receive data from client
    while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        // Check if the packet size exceeds the limit
        if (total_received + bytes_received > MAX_PACKET_SIZE) {
            syslog(LOG_ERR, "Packet size exceeds limit, discarding packet");
            free(packet);
            fclose(fp);
            close(client_fd);
            return;
        }

        // Append received data to the packet
        memcpy(packet + total_received, buffer, bytes_received);
        total_received += bytes_received;

        // Check if the received data contains a newline
        if (memchr(buffer, '\n', bytes_received) != NULL) {
            // Append the packet to the file
            fwrite(packet, sizeof(char), total_received, fp);
            fflush(fp);

            // Reset the file pointer to the beginning
            fseek(fp, 0, SEEK_SET);

            // Send the file content back to the client
            char fbuf[1024];
            size_t bytes_read;
            while ((bytes_read = fread(fbuf, 1, sizeof(fbuf), fp)) > 0) {
                ssize_t bytes_sent = send(client_fd, fbuf, bytes_read, 0);
                if (bytes_sent == -1) {
                    syslog(LOG_ERR, "Send error: %s", strerror(errno));
                    break;
                }
            }

            // Reset for the next packet
            total_received = 0;
        }
    }

    if (bytes_received == -1) {
        perror("Failed to receive data");
    }

    // Clean up
    free(packet);
    fclose(fp);
    syslog(LOG_INFO, "Closed connection from %s", client_ip);
    close(client_fd);
}


int main(int argc, char *argv[]) {
    int is_daemon = 0;
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        is_daemon = 1;
    }

    // Daemonize if requested
    if (is_daemon) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Failed to fork");
            exit(1);
        }
        if (pid > 0) {
            exit(0); // Parent process exits
        }
        setsid(); // Create a new session
        chdir("/"); // Change working directory
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    // Open syslog
    openlog("aesdsocket", LOG_PID, LOG_USER);

    // Set up signal handlers
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Create and bind socket
    int socket_fd = create_socket();
    if (socket_fd == -1) {
        return 1;
    }

    // Main loop to accept connections
    while (!shutdown_flag) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            if (shutdown_flag) break; // Exit if shutdown signal received
            perror("Failed to accept connection");
            continue;
        }

        // Get client IP address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        // Handle client connection
        handle_client(client_fd, client_ip);
    }

    // Cleanup
    close(socket_fd);
    closelog();
    return 0;
}