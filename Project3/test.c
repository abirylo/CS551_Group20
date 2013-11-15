#include <igipc.h>

void printIGs();
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
				printf("\t%d: %s\n", ig[i].id, ig[i].group_name);
			}
		}
	}
}

int testOverfillIG() {
	IGInit();
	
	int i;
	for (i = 0; i < MAX_SIZE_IG; i++) {
		char thisGroupName[MAX_GROUP_NAME_LENGTH];
		snprintf(thisGroupName, MAX_GROUP_NAME_LENGTH, "Group %d", i + 1);
		
		int thisGroup = IGCreate(thisGroupName);
		
		if (thisGroup < 0) {
			printf("Overfill IG test failed!\n");
			return -1;
		}
	}
	
	printIGs();
	
	// This should fail.
	int thisGroup = IGCreate("Invalid");
	
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
	
	int status;
	int thisGroup = IGCreate("testgroup");
	
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
	
	int status;
	int thisGroup = IGCreate("testgroup");
	
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
	
	int status;
	int thisGroup = IGCreate("testgroup");
	
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
	
	int status;
	int thisGroup = IGCreate("testgroup");
	
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
	
	int status;
	int thisGroup = IGCreate("testgroup");
	
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
	
	int status;
	char message[MAX_MESSAGE_SIZE];
	int thisGroup = IGCreate("testgroup");
	
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
	
	int status;
	int thisGroup = IGCreate("testgroup");
	
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

int (*test_pointers[10]) ();

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
	
	int result;
	int i;
	for (i = 0; i < 10; i++) {
		result = (*test_pointers[i]) ();
		
		if (result != 0) {
			printf("Test failed with return error: %d\n", result);
		}
	}
	
	printf("\n\n\n");
	
	/* Functional tests */
	
	int status = -1;
	
	IGInit();
	
	struct interestGroup ig[MAX_SIZE_IG];
	int group1 = IGCreate("group_1");
	
	printf("Registered group %d\n", group1);
	
	status = IGLookup(ig);
	
	if (status != 0) {
		puts("Lookup FAIL!");
	}
	else {
		puts("Interest groups available:");
		
		int i;
		for (i = 0; i < MAX_SIZE_IG; i++) {
			if (ig[i].id != 0 || i == 0) {
				printf("\t%d: %s\n", ig[i].id, ig[i].group_name);
			}
		}
	}

	status = IGPublisher(100, group1);
	
	if (status != 0) {
		puts("Publisher 1 FAIL!");
	}
	
	status = IGPublisher(200, group1);
	
	if (status != 0) {
		puts("Publisher 2 FAIL!");
	}

	status = IGSubscriber(100, group1);
	
	if (status != 0) {
		puts("Subscriber 1 FAIL!");
	}
	
	status = IGSubscriber(200, group1);
	
	if (status != 0) {
		puts("Subscriber 2 FAIL!");
	}
	
	
	
	status = IGPublish(100, group1, "Test message 1");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	status = IGPublish(100, group1, "Test message 2");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	status = IGPublish(100, group1, "Test message 3");
	
	if (status != 0) {
		printf("Publish failed with error code: %d\n", status);
	}
	
	
	char message[MAX_MESSAGE_SIZE];

	status = IGRetrive(100, group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}

	status = IGRetrive(100, group1, message);
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}

	status = IGRetrive(200, group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(200, group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}

	status = IGRetrive(100, group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(100, group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(200, group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
	
	status = IGRetrive(100, group1, message);
	
	if (status != 0) {
		printf("Retrive failed with error code: %d\n", status);
	}
	else {
		printf("Received message: %s\n", message);
	}
    
    return 0;
}