#include <igipc.h>

int main(void)
{
    int groupName = 34;
    
    printf("Hello %i", 18);
    
    IGLookup(groupName);
    IGCreate(groupName);
    IGPublisher(groupName);
    IGSubscriber(groupName);
    IGPublish(groupName, groupName);
    IGRetrive(groupName, groupName);
    return 0;
}