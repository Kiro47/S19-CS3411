#include "utils.h"


static void genericHandler(int signum)
{
    print("Signal [%d] detected\n", signum);
}
