#include <igipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    int groupName = 88;
    int id = getpid();
    //char *dataSend = malloc(1024);
    char dataSend[1024];
    char dataRecv[1024];
    strcpy(dataSend,"Yello, world");
    //char *dataRecv = malloc(1024);
    //int dataSend = 555;
    //int dataRecv = 0;
    
    IGInit();
    
    if(IGLookup(groupName) == 0)
        printf("IGLookup could not find group created\n");
    
    if(IGCreate(groupName))
        printf("IGCreate created group\n");
     
    if(IGLookup(groupName))
        printf("IGLookup fround group created\n");
    
    if(IGPublisher(id,groupName))
        printf("Registered as Publisher with pid %i \n",id);
        
    if(IGSubscriber(id,groupName))
        printf("Registered as Subscriber with pid %i \n",id);
    sleep(3);
    int r=IGPublish(id, groupName,dataSend);
    if(r>=0)
        printf("Published data, retval:%i \n");
    else
        printf("Failed to publish, retval:%i;\n",r);
    sleep(3);
    r = IGRetrive(id, groupName, dataRecv);
    if(r >= 0)
    {   
        printf("Data Found %i\n",r);
        printf("Data Recieved\n",dataRecv);
        printf(dataRecv);
    }
    else
        printf("Returned error code on Retrieve: %i\n",r);
    
    return 0;
}
