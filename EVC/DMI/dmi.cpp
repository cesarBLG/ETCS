#ifdef unix
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
using namespace std;
int dmi_pid;
void start_dmi()
{
    printf("Starting Driver Machine Interface...\n");
    dmi_pid = fork();
    if(dmi_pid == 0)
    {
        chdir("../DMI");
        int fd = open("dmi.log.o", O_WRONLY | O_CREAT);
        dup2(fd, 1);
        execl("dmi", "dmi", nullptr);
    }
}
#endif