To run the project, simply start by getting the files into the VM.  This step depends on the VM type and isn't covered here.

With that done, simply run:

make

Then within that folder, to test the modifications, run:

./test

The output should be similar to this:

# ./test
Interest groups available:
	1 - Name: Group 1, Type: Public, Leader: 100
	2 - Name: Group 2, Type: Public, Leader: 100
	3 - Name: Group 3, Type: Public, Leader: 100
	4 - Name: Group 4, Type: Public, Leader: 100
	5 - Name: Group 5, Type: Public, Leader: 100
	6 - Name: Group 6, Type: Public, Leader: 100
	7 - Name: Group 7, Type: Public, Leader: 100
	8 - Name: Group 8, Type: Public, Leader: 100
	9 - Name: Group 9, Type: Public, Leader: 100
	10 - Name: Group 10, Type: Public, Leader: 100
	11 - Name: Group 11, Type: Public, Leader: 100
	12 - Name: Group 12, Type: Public, Leader: 100
	13 - Name: Group 13, Type: Public, Leader: 100
	14 - Name: Group 14, Type: Public, Leader: 100
	15 - Name: Group 15, Type: Public, Leader: 100
	16 - Name: Group 16, Type: Public, Leader: 100
	17 - Name: Group 17, Type: Public, Leader: 100
	18 - Name: Group 18, Type: Public, Leader: 100
	19 - Name: Group 19, Type: Public, Leader: 100
	20 - Name: Group 20, Type: Public, Leader: 100
Overfill IG test passed.
Already publisher test passed.
Not publisher test passed.
Publisher invalid interest group test passed.
Already subscriber test passed.
Not subscriber test passed.
Subscribe invalid interest group test passed.
No messages test passed.
Already checked all messages test passed.
Overfill messages test passed.
Auth super user test passed.
Add group leader test passed.
Remove group leader test passed.



ALL UNIT TESTS PASSED!


Super user pid: 10
Interest groups available:
	1 - Name: group_1, Type: Public, Leader: 100
Received message: Test message 1
Received message: Test message 2
Received message: Test message 1
Received message: Test message 2
Received message: Test message 3
Retrive failed with error code: -1
Received message: Test message 3
Retrive failed with error code: -1
Interest groups available:
	1 - Name: group_1, Type: Public, Leader: 100
	2 - Name: Private group 1, Type: Secure, Leader: 100
		2 Authorized PIDs:
			1: 100
			2: 200
Received message: Test message 1
Received message: Test message 2
Received message: Test message 1
Received message: Test message 2
Received message: Test message 3
Retrive failed with error code: -1
Received message: Test message 3
Retrive failed with error code: -1
Retrive failed with error code: -1
