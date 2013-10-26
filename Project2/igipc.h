#include <lib.h>
#include <unistd.h>
#include <callnr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GROUP_MESSAGES		5
#define MAX_SIZE_IG				100
#define MAX_MESSAGE_SIZE		1024
#define MAX_GROUP_NAME_LENGTH	255

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
	char group_name[MAX_GROUP_NAME_LENGTH];
	
	int num_subscribers;
	int num_publishers;
	int num_messages;
	
    struct subscriber subscribers[MAX_SIZE_IG];
	struct publisher publishers[MAX_SIZE_IG];
	struct message messages[MAX_GROUP_MESSAGES];
};

int IGLookup();
int IGCreate(char *groupName);
int IGPublisher(int pid, int group_id);
int IGSubscriber(int pid, int group_id);
int IGPublish(int sending_pid, int group_id, char *sending_message);
int IGRetrive(int requesting_pid, int group_id);