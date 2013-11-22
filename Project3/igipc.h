#include <lib.h>
#include <unistd.h>
#include <minix/callnr.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>

#define MAX_GROUP_MESSAGES		5
#define MAX_SIZE_IG				20
#define MAX_MESSAGE_SIZE		1024
#define MAX_GROUP_NAME_LENGTH	255

#define PUBLIC_GROUP 1
#define SECURE_GROUP 2

#define SUPER_USER_PASSWORD	12345

typedef long unsigned int vir_bytes;

struct message {
	int message_id;
	int sending_pid;
	char message[MAX_MESSAGE_SIZE];
};

struct message_list {
	struct message message;
	struct message_list *next_message;
};

struct subscriber {
	int pid;
	struct message_list read_messages;
};

struct publisher {
	int pid;
};

struct interestGroup {
    int id;
	int group_leader;
	int group_type;
	char group_name[MAX_GROUP_NAME_LENGTH];
	
	int num_subscribers;
	int num_publishers;
	int num_messages;
	
    struct subscriber subscribers[MAX_SIZE_IG];
	struct publisher publishers[MAX_SIZE_IG];
	struct message messages[MAX_GROUP_MESSAGES];
};


// Functions

int IGInit() {
	message m;
	return _syscall(PM_PROC_NR, IGINIT, &m);
}

int IGLookup(struct interestGroup *ig) {
	message m;
	m.m1_p1 = (vir_bytes) ig;
	
	int status = _syscall(PM_PROC_NR, IGLOOKUP, &m);
	
	return status;
}

int IGCreate(int pid, int group_type, char *groupName, int *authorized_pids, int num_authorized_pids) {
    message m;
	m.m1_i1 = pid;
	m.m1_i2 = group_type;
	m.m1_i3 = num_authorized_pids;
    m.m1_p1 = groupName;
	m.m1_p2 = authorized_pids;
	
    return( _syscall(PM_PROC_NR, IGCREATE, &m) );
}

int IGRemove(int pid, int group_id) {
	message m;
	m.m1_i1 = pid;
	m.m1_i2 = group_id;
	
	return(_syscall(PM_PROC_NR, IGREMOVE, &m));
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

int IGRetrive(int requesting_pid, int group_id, char *ret_message) {
    message m;
    m.m1_i1 = requesting_pid;
    m.m1_i2 = group_id;
	m.m1_p1 = ret_message;
	
	int status = _syscall(PM_PROC_NR, IGRETRIVE, &m);
	
	return status;
}

int AuthSuperUser(int pid, int password) {
	message m;
	m.m1_i1 = pid;
	m.m1_i2 = password;
	
	return (_syscall(PM_PROC_NR, AUTHSUPERUSER, &m));
}

int AddGroupLeader(int pid, int group_leader_pid) {
	message m;
	m.m1_i1 = pid;
	m.m1_i2 = group_leader_pid;
	
	return (_syscall(PM_PROC_NR, ADDGROUPLEADER, &m));
}

int RemoveGroupLeader(int pid, int group_leader_pid) {
	message m;
	m.m1_i1 = pid;
	m.m1_i2 = group_leader_pid;
	
	return (_syscall(PM_PROC_NR, REMOVEGROUPLEADER, &m));
}