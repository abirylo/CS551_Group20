#include <lib.h>
#include <unistd.h>
#include <minix/callnr.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int IGLookup (int groupName);
int IGCreate (int groupName);
int IGPublisher (int id, int groupName);
int IGSubscriber (int id, int groupName);
int IGPublish (int id, int groupName, int messageSend);
int IGRetrive (int id, int groupName, int *messageRecive);