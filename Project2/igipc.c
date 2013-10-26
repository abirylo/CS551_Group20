#include "igipc.h"

int IGLookup() {
	message m;
	return _syscall(PM_PROC_NR, IGLOOKUP, &m);
}

int IGCreate(char *groupName) {
    message m;
	m.m1_i1 = 0;
	m.m1_i2 = 0;
	m.m1_i3 = 0;
    m.m1_p1 = groupName;
	m.m1_p2 = NULL;
	m.m1_p3 = NULL;
	printf("Pushing to sys group %s...", groupName);
    return( _syscall(PM_PROC_NR, IGCREATE, &m) );
}

int IGPublisher(int pid, int group_id) {
    message m;
    m.m1_i1 = pid;
    m.m1_i2 = group_id;
    return( _syscall(PM_PROC_NR, IGPUBLISHER, &m) );
}

int IGSubscriber(int pid, int group_id) {
    message m;
    m.m1_i1 = pid;
    m.m1_i2 = group_id;
    return( _syscall(PM_PROC_NR, IGSUBSCRIBER, &m) );
}

int IGPublish(int sending_pid, int group_id, char *sending_message) {
	message m;
    m.m1_i1 = sending_pid;
    m.m1_i2 = group_id;
    m.m1_p1 = sending_message;
    return( _syscall(PM_PROC_NR, IGPUBLISH, &m) );
}

int IGRetrive(int requesting_pid, int group_id) {
    message m;
    m.m1_i1 = requesting_pid;
    m.m1_i2 = group_id;
	
	return (_syscall(PM_PROC_NR, IGRETRIVE, &m));
}