#include <lib.h>
#include <unistd.h>

int IGLookup ()
{
    message m;
    return( _syscall(PM_PROC_NR, IGLOOKUP, &m) );
}

int IGCreate ()
{
    message m;
    return( _syscall(PM_PROC_NR, IGCREATE, &m) );
}

int IGPublisher ()
{
    message m;
    return( _syscall(PM_PROC_NR, IGPUBLISHER, &m) );
}

int IGSubscriber ()
{
    message m;
    return( _syscall(PM_PROC_NR, IGSUBSCRIBER, &m) );
}

int IGPublish ()
{
    message m;
    return( _syscall(PM_PROC_NR, IGPUBLISH, &m) );
}

int IGRetrive ()
{
    message m;
    return( _syscall(PM_PROC_NR, IGRETRIVE, &m) );
}