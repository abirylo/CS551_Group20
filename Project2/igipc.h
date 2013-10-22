#include <lib.h>
#include <unistd.h>

int IGLookup (int groupName)
{
    message m;
    m.m1_i1 = groupName;
    return( _syscall(PM_PROC_NR, IGLOOKUP, &m) );
}

int IGCreate (int groupName)
{
    message m;
    m.m1_i1 = groupName;
    return( _syscall(PM_PROC_NR, IGCREATE, &m) );
}

int IGPublisher (int groupName)
{
    message m;
    m.m1_i1 = groupName;
    return( _syscall(PM_PROC_NR, IGPUBLISHER, &m) );
}

int IGSubscriber (int groupName)
{
    message m;
    m.m1_i1 = groupName;
    return( _syscall(PM_PROC_NR, IGSUBSCRIBER, &m) );
}

int IGPublish (int groupName, int messageSend)
{
    message m;
    m.m1_i1 = groupName;
    m.m1_i2 = messageSend;
    return( _syscall(PM_PROC_NR, IGPUBLISH, &m) );
}

int IGRetrive (int groupName, int messageSend)
{
    message m;
    m.m1_i1 = groupName;
    m.m1_i2 = messageSend;
    return( _syscall(PM_PROC_NR, IGRETRIVE, &m) );
}