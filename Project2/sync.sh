#!/bin/sh
    
cd ~/Project2
if test .ipaddr:
then
ip=$(cat .ipaddr)
fi
rsync byron@$ip:~/CS551/CS551_Group20/Project2/* . &&
    sh copyAllFiles && 
    cd /usr/src/servers/pm &&
        make && make install
        read trash &&
    cd /usr/src/releasetools &&
        make hdboot
        read trash &&
        cd ~/Project2
    cc test.c -o test && ./test
    #reboot

