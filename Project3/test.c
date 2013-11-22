#include <igipc.h>

int testOverfillIG();
int testAlreadyPublisher();
int testNotPublisher();
int testPublisherInvalidInterestGroup();
int testAlreadySubscriber();
int testNotSubscriber();
int testSubscribeInvalidInterestGroup();
int testRetriveNoMessages();
int testAlreadyRetrivedAllMessages();
int testOverfillMessages();

int testAuthSuperUser();
int testAddGroupLeader();
int testRemoveGroupLeader();
int testPublicGroup();
int testPrivateGroup();
int testRemoveGroup();

void printIGs() {
	struct interestGroup ig[MAX_SIZE_IG];
	int status = IGLookup(ig);
	
	if (status != 0) {
		puts("Lookup FAIL!");
	}
	else {
		puts("Interest groups available:");
		
		int i;
		for (i = 0; i < MAX_SIZE_IG; i++) {
			if (ig[i].id != 0 || i == 0) {
				char *type;
				if (ig[i].group_type == PUBLIC_GROUP) {
					type = "Public";
				}
				else if (ig[i].group_type == SECURE_GROUP) {
					type = "Secure";
				}
				else {
					type = "Unknown";
				}
				
				printf("\t%d - Name: %s, Type: %s, Leader: %d\n", ig[i].id, ig[i].group_name, type, ig[i].group_leader);
				
				if (ig[i].group_type == SECURE_GROUP) {
					printf("\t\t%d Authorized PIDs:\n", ig[i].num_subscribers);
					
					int j;
					for (j = 0; j < ig[i].num_subscribers; j++) {
						if (ig[i].subscribers[j].pid != 0) {
							printf("\t\t\t%d: %d\n", j + 1, ig[i].subscribers[j].pid);
						}
					}
				}
			}
		}
	}
}

int testOverfillIG() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int i;
	for (i = 0; i < MAX_SIZE_IG; i++) {
		char thisGroupName[MAX_GROUP_NAME_LENGTH];
		snprintf(thisGroupName, MAX_GROUP_NAME_LENGTH, "Group %d", i + 1);
		
		int thisGroup = IGCreate(100, PUBLIC_GROUP, thisGroupName, NULL, 0);
		
		if (thisGroup < 0) {
			printf("Overfill IG test failed!\n");
			return -1;
		}
	}
	
	printIGs();
	
	// This should fail.
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "Invalid", NULL, 0);
	
	if (thisGroup > 0) {
		printf("Overfill IG test failed!\n");
		return -2;
	}
	else {
		puts("Overfill IG test passed.");
	}
	
	return 0;
}

int testAlreadyPublisher() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int status;
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "TestGroup", NULL, 0);
	
	status = IGPublisher(100, thisGroup);
	
	if (status < 0) {
		puts("Already publisher test failed!");
		return -1;
	}
	
	status = IGPublisher(100, thisGroup);
	
	if (status > 0) {
		puts("Already publisher test failed!");
		return -2;
	}
	else {
		puts("Already publisher test passed.");
	}
	
	return 0;
	
}

int testNotPublisher() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int status;
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "TestGroup", NULL, 0);
	
	status = IGPublish(100, thisGroup, "Test");
	
	if (status > 0) {
		puts("Not publisher test failed!");
		return -2;
	}
	else {
		puts("Not publisher test passed.");
	}
	
	return 0;
}

int testPublisherInvalidInterestGroup() {
	IGInit();
	
	int status;
	
	status = IGPublisher(100, -1);
	
	if (status > 0) {
		puts("Publisher invalid interest group test failed!");
		return -1;
	}
	else {
		puts("Publisher invalid interest group test passed.");
	}
	
	return 0;
}

int testAlreadySubscriber() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int status;
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "TestGroup", NULL, 0);
	
	status = IGSubscriber(100, thisGroup);
	
	if (status < 0) {
		puts("Already subscriber test failed!");
		return -1;
	}
	
	status = IGSubscriber(100, thisGroup);
	
	if (status > 0) {
		puts("Already subscriber test failed!");
		return -2;
	}
	else {
		puts("Already subscriber test passed.");
	}
	
	return 0;
	
}

int testNotSubscriber() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int status;
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "TestGroup", NULL, 0);
	
	char message[MAX_MESSAGE_SIZE];
	status = IGRetrive(100, thisGroup, message);
	
	if (status > 0) {
		puts("Not subscriber test failed!");
		return -2;
	}
	else {
		puts("Not subscriber test passed.");
	}
	
	return 0;
	
}

int testSubscribeInvalidInterestGroup() {
	IGInit();
	
	int status;
	
	status = IGSubscriber(100, -1);
	
	if (status > 0) {
		puts("Subscribe invalid interest group test failed!");
		return -1;
	}
	else {
		puts("Subscribe invalid interest group test passed.");
	}
	
	return 0;
}

int testRetriveNoMessages() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int status;
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "TestGroup", NULL, 0);
	
	status = IGSubscriber(100, thisGroup);
	
	if (status < 0) {
		puts("No messages test failed!");
		return -1;
	}
	
	char message[MAX_MESSAGE_SIZE];
	status = IGRetrive(100, thisGroup, message);
	
	if (status > 0) {
		puts("No messages test failed!");
		return -2;
	}
	else {
		puts("No messages test passed.");
	}
	
	return 0;
}

int testAlreadyRetrivedAllMessages() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int status;
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "TestGroup", NULL, 0);
	char message[MAX_MESSAGE_SIZE];
	
	status = IGSubscriber(100, thisGroup);
	
	if (status < 0) {
		puts("Already checked all messages test failed!");
		return -1;
	}
	
	status = IGPublisher(100, thisGroup);
	
	if (status < 0) {
		puts("Already checked all messages test failed!");
		return -2;
	}
	
	status = IGPublish(100, thisGroup, "Test message");
	
	if (status < 0) {
		puts("Already checked all messages test failed!");
		return -3;
	}
	
	status = IGRetrive(100, thisGroup, message);
	
	if (status < 0) {
		puts("Already checked all messages test failed!");
		return -4;
	}
	
	status = IGRetrive(100, thisGroup, message);
	
	if (status > 0) {
		puts("Already checked all messages test failed!");
		return -5;
	}
	else {
		puts("Already checked all messages test passed.");
	}
	
	return 0;
}

int testOverfillMessages() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int status;
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "TestGroup", NULL, 0);
	
	status = IGPublisher(100, thisGroup);
	
	if (status < 0) {
		puts("Overfill messages test failed!");
		return -1;
	}
	
	int i;
	for (i = 0; i < MAX_GROUP_MESSAGES; i++) {
		status = IGPublish(100, thisGroup, "Test");
		
		if (status < 0) {
			puts("Overfill messages test failed!");
			return -2;
		}
	}
	
	// This should fail
	status = IGPublish(100, thisGroup, "Test");
	
	if (status > 0) {
		puts("Overfill messages test failed!");
		return -3;
	}
	else {
		puts("Overfill messages test passed.");
	}
	
	return 0;
}


/* Project 3 Tests */

int testAuthSuperUser() {
	IGInit();
	
	// Test incorrect password
	int success1 = AuthSuperUser(10, -1);
	
	if (success1 == 0) {
		printf("Auth super user test failed!\n");
		return -1;
	}
	
	int success2 = AuthSuperUser(10, SUPER_USER_PASSWORD);
	
	if (success2 < 0) {
		printf("Auth super user test failed!\n");
		return -2;
	}
	else {
		printf("Auth super user test passed.\n");
		return 0;
	}
}

int testAddGroupLeader() {
	IGInit();
	
	// Test without being super user
	int success1 = AddGroupLeader(10, 10);
	
	if (success1 == 0) {
		puts("Add group leader test failed!");
		return -1;
	}
	
	// Now create a super user
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	
	// Now we should be able to authorize this group
	int success2 = AddGroupLeader(10, 100);
	
	if (success2 != 0) {
		puts("Add group leader test failed!");
		return -2;
	}
	
	puts("Add group leader test passed.");
	return 0;
}

int testRemoveGroupLeader() {
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	
	// Try to remove not as super user
	int success1 = RemoveGroupLeader(5, 100);
	
	if (success1 == 0) {
		printf("Remove group leader test failed with code %d!", success1);
		return -1;
	}
	
	// Try to remove as super user but not a valid pid
	int success2 = RemoveGroupLeader(10, 100);
	
	if (success2 == 0) {
		printf("Remove group leader test failed with code %d!", success2);
		return -2;
	}
	
	AddGroupLeader(10, 100);
	
	// Now try to remove valid group as valid super user
	int success3 = RemoveGroupLeader(10, 100);
	
	if (success3 != 0) {
		printf("Remove group leader test failed with code %d!", success3);
		return -3;
	}
	
	puts("Remove group leader test passed.");
	return 0;
}

int testPublicGroup() {
	IGInit();
	int status;
	char message[MAX_MESSAGE_SIZE];
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int thisGroup = IGCreate(100, PUBLIC_GROUP, "TestGroup", NULL, 0);
	
	if (thisGroup < 0) {
		puts("Public group test failed!");
		return -1;
	}
	
	// Add valid subscriber
	status = IGSubscriber(100, thisGroup);
	
	if (status != 0) {
		puts("Public group test failed!");
		return -2;
	}
	
	// Add valid publisher
	status = IGPublisher(100, thisGroup);

	if (status != 0) {
		puts("Public group test failed!");
		return -3;
	}
	
	// Valid publish
	status = IGPublish(100, thisGroup, "Test message");
	
	if (status != 0) {
		puts("Public group test failed!");
		return -4;
	}
	
	// Valid retrive
	status = IGRetrive(100, thisGroup, message);
	
	if (status != 0) {
		puts("Public group test failed!");
		return -5;
	}
	
	return 0;
}

int testPrivateGroup() {
	IGInit();
	int status;
	char message[MAX_MESSAGE_SIZE];
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int allowed_pids[] = {100, 200};
	int private_group1 = IGCreate(100, SECURE_GROUP, "Private group 1", allowed_pids, 2);
	
	// Try to publish from unauthorized PID
	status = IGPublish(300, private_group1, "Test fail message");
	
	if (status > 0) {
		puts("Private group test failed!");
		return -1;
	}
	
	// Publish from authorized PID
	status = IGPublish(100, private_group1, "Test message");
	
	if (status != 0) {
		puts("Private group test failed!");
		return -2;
	}
	
	// Retrieve from unauthorized PID
	status = IGRetrive(300, private_group1, message);
	
	if (status > 0) {
		puts("Private group test failed!");
		return -3;
	}
	
	// Retrieve from authorized PID
	status = IGRetrive(100, private_group1, message);
	
	if (status != 0) {
		puts("Private group test failed!");
		return -4;
	}
	
	return 0;
}

int testRemoveGroup() {
	IGInit();
	int status;
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	AddGroupLeader(super_user_pid, 100);
	
	int public_group = IGCreate(100, PUBLIC_GROUP, "Test Group", NULL, 0);
	int allowed_pids[] = {100, 200};
	int private_group = IGCreate(100, SECURE_GROUP, "Private group 1", allowed_pids, 2);
	
	// Unauthorized remove of public group
	status = IGRemove(200, public_group);
	
	if (status == 0) {
		printf("Remove group test failed with code %d!\n", status);
		return -1;
	}
	
	// Authorized super user remove
	status = IGRemove(10, public_group);
	
	if (status != 0) {
		printf("Remove group test failed with code %d!\n", status);
		return -2;
	}
	
	// Authorized removal of private group from group leader
	status = IGRemove(100, private_group);
	
	if (status != 0) {
		printf("Remove group test failed with code %d!\n", status);
		return -3;
	}
	
	return 0;
}

int (*test_pointers[16]) ();

int main()
{
	/* Unit tests */
	
	test_pointers[0] = testOverfillIG;
	test_pointers[1] = testAlreadyPublisher;
	test_pointers[2] = testNotPublisher;
	test_pointers[3] = testPublisherInvalidInterestGroup;
	test_pointers[4] = testAlreadySubscriber;
	test_pointers[5] = testNotSubscriber;
	test_pointers[6] = testSubscribeInvalidInterestGroup;
	test_pointers[7] = testRetriveNoMessages;
	test_pointers[8] = testAlreadyRetrivedAllMessages;
	test_pointers[9] = testOverfillMessages;
	
	test_pointers[10] = testAuthSuperUser;
	test_pointers[11] = testAddGroupLeader;
	test_pointers[12] = testRemoveGroupLeader;
	test_pointers[13] = testPublicGroup;
	test_pointers[14] = testPrivateGroup;
	test_pointers[15] = testRemoveGroup;
	
	int result;
	int i;
	int all_passed = 1;
	for (i = 0; i < 16; i++) {
		result = (*test_pointers[i]) ();
		
		if (result != 0) {
			printf("Test failed with return error: %d\n", result);
			all_passed = 0;
			break;
		}
	}
	
	if (all_passed) {
		printf("\n\n\nALL UNIT TESTS PASSED!");
	}
	
	printf("\n\n\n");
	
	
	/* Functional tests */
	
	int status = -100;
	
	IGInit();
	
	int super_user_pid = AuthSuperUser(10, SUPER_USER_PASSWORD);
	printf("Super user pid: %d\n", super_user_pid);
	status = AddGroupLeader(10, 100);
	
	if (status != 0) {
		printf("Status obtained from add group leader: %d\n", status);
		
		return -1;
	}
	
	int public_group1 = IGCreate(100, PUBLIC_GROUP, "group_1", NULL, 0);
	
	printIGs();
	
	status = IGPublisher(100, public_group1);
	
	if (status != 0) {
		puts("Publisher 1 FAIL!");
	}
	
	status = IGPublisher(200, public_group1);
	
	if (status != 0) {
		puts("Publisher 2 FAIL!");
	}

	status = IGSubscriber(100, public_group1);
	
	if (status != 0) {
		puts("Subscriber 1 FAIL!");
	}
	
	status = IGSubscriber(200, public_group1);
	
	if (status != 0) {
		puts("Subscriber 2 FAIL!");
	}
	
	
	
	status = IGPublish(100, public_group1, "Test message 1");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	status = IGPublish(100, public_group1, "Test message 2");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	status = IGPublish(100, public_group1, "Test message 3");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	
	char message[MAX_MESSAGE_SIZE];

	status = IGRetrive(100, public_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}

	status = IGRetrive(100, public_group1, message);
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}

	status = IGRetrive(200, public_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(200, public_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}

	status = IGRetrive(100, public_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(100, public_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(200, public_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(100, public_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	int allowed_pids[] = {100, 200};
	int private_group1 = IGCreate(100, SECURE_GROUP, "Private group 1", allowed_pids, 2);
	
	printIGs();
	
	status = IGPublish(100, private_group1, "Test message 1");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	status = IGPublish(100, private_group1, "Test message 2");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	status = IGPublish(100, private_group1, "Test message 3");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	
	status = IGRetrive(100, private_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(100, private_group1, message);
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(200, private_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(200, private_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(100, private_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(100, private_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(200, private_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(100, private_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	
	status = IGRetrive(300, private_group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
    return 0;
}