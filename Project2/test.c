#include <igipc.h>

int main()
{
	int group1 = IGCreate("group_1");
	
	printf("Registered group %d\n", group1);
	
	IGSubscriber(100, group1);
	IGSubscriber(200, group1);
	
	IGPublish(100, group1, "Test message 1");
	IGPublish(100, group1, "Test message 2");
	IGPublish(100, group1, "Test message 3");
	
	
	
	IGRetrive(100, group1);

	IGRetrive(100, group1);
	
	
	IGRetrive(100, group1);
	
	IGRetrive(200, group1);
	
	
	IGRetrive(100, group1);
	
    
    return 0;
}