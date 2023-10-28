#include <libssh2.h>

#include <iostream>

int main(int argc, char* argv[])
{
    int rc = libssh2_init(0);
    if (rc)
    {
        std::cerr << "Cannot initialize libssh2" << std::endl;
        return 1;
    }
    libssh2_exit();
    return 0;
}
