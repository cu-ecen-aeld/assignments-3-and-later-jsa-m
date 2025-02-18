#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

//Definition of the writer function.

int main(int argc, char *argv[])
{
    int fd;
    ssize_t nr;

    //Setup syslog logging for your utility using the LOG_USER facility.

    openlog("writer", LOG_PID | LOG_CONS, LOG_USER);

    //Check if there are three arguments, program name, file_path and user_string
    if(argc != 3)
    {
        syslog(LOG_ERR, "Not enough arguments passed, only %d: %s", argc, strerror(errno));
        return 1;
    }

    //fd returns the lowest numbered unused file descriptor.

    const char *file_path = argv[1]; //"/home/joaquin/Documents/test/test.txt";
    const char *user_string = argv[2];//"Hello World";

    fd = open(file_path, O_WRONLY|O_CREAT|O_TRUNC, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

    if (fd ==-1)
    {
        syslog(LOG_ERR, "Failed to open file %s: %s", file_path, strerror(errno));
        closelog();
    }
    else
    {
        nr = write(fd, user_string, strlen(user_string));
        if(nr == -1)
        {
            syslog(LOG_ERR, "Failed to write file %s: %s", user_string, strerror(errno));
            close(fd);
            closelog();
        }
    }

    syslog(LOG_DEBUG, "Writting %s to %s", user_string, file_path);
    close(fd);
    closelog();

    return 0;
}


