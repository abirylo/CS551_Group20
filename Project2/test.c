#include <igipc.h>

int main(void)
{
    int groupName = 88;
    int id = 123;
    int dataSend = 555;
    int dataRecv = 0;
    
    IGInit();
    
    if(IGLookup(groupName) == 0)
        printf("IGLookup could not find group created\n");
    
    if(IGCreate(groupName))
        printf("IGCreate created group\n");
     
    if(IGLookup(groupName))
        printf("IGLookup fround group created\n");
    
    if(IGPublisher(id,groupName))
        printf("Registered as Publisher\n");
        
    if(IGSubscriber(id,groupName))
        printf("Registered as Subscriber\n");
    
    if(IGPublish(id, groupName,dataSend))
        printf("Published data\n");
    
    int r = IGRetrive(id, groupName, &dataRecv);
    if(r > 0)
    {   
        printf("Data Found %i\n",r);
        if(r == dataSend)
            printf("Data Recieved\n");
    }
    
    return 0;
}