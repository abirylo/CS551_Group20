#include <lib.h>
#include <unistd.h>

int IGInit()
{
    message m;
    return( _syscall(PM_PROC_NR, IGINIT, &m) );
}

int IGLookup (int groupName)
{
    message m;
    m.m1_i2 = groupName;
    return( _syscall(PM_PROC_NR, IGLOOKUP, &m) );
}

int IGCreate (int groupName)
{
    message m;
    m.m1_i2 = groupName;
    return( _syscall(PM_PROC_NR, IGCREATE, &m) );
}

int IGPublisher (int id, int groupName)
{
    message m;
    m.m1_i1 = id;
    m.m1_i2 = groupName;
    return( _syscall(PM_PROC_NR, IGPUBLISHER, &m) );
}

int IGSubscriber (int id, int groupName)
{
    message m;
    m.m1_i1 = id;
    m.m1_i2 = groupName;
    return( _syscall(PM_PROC_NR, IGSUBSCRIBER, &m) );
}

int IGPublish (int id, int groupName, char *messageSend)
{
    message m;
    m.m1_i1 = id;
    m.m1_i2 = groupName;
    m.m1_p1 = messageSend;
    return( _syscall(PM_PROC_NR, IGPUBLISH, &m) );
}

int IGRetrive (int id, int groupName, char *messageRecive)
{
    message m;
    m.m1_i1 = id;
    m.m1_i2 = groupName;
    m.m1_p1 = messageRecive; 
    int r = _syscall(PM_PROC_NR, IGRETRIVE, &m);
    return r;
}
