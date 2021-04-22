// Server side C/C++ program to demonstrate Socket programming

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>

#define PORT 80
int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[102] = {0};
    char *hello = "Hello from server";
    pid_t pid;

    printf("execve=0x%p\n", execve);

    //determine whether process is re-exec by child by arguments amount
    if (argc == 2)
    {
        printf("Process replaced by exec with uid %d\n", getuid());

        //extract socket value from argument
        new_socket = atoi(argv[1]);

        //process data from client
        valread = read(new_socket, buffer, 1024);
        printf("%s\n", buffer);
        send(new_socket, hello, strlen(hello), 0);
        printf("Hello message sent\n");

        return 0;
    }

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to port 80
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 80
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    //child process
    if (pid == 0)
    {
        printf("child process created with uid %d\n", getuid());

        //drop privilege by setting uid to nobody
        if (setuid(65534) == -1)
        {
            perror("setuid failed");
            exit(EXIT_FAILURE);
        }
        printf("uid after privilege drop: %d\n", getuid());

        //store socket in string
        char socketString[10];
        snprintf(socketString, 10, "%d", new_socket);

        //copy filename from argv
        char filename[20];
        snprintf(filename, 20, "%s", argv[0]);

        //exec and pass in arguments
        char *const arguments[] = {filename, socketString, NULL};
        execvp(argv[0], arguments);
    }

    //parent process
    else if (pid > 0)
    {
        //wait for child process terminate
        wait(NULL);
    }

    //fork failled
    else
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
