obj-m :=myBlockDriver.o

all :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:       
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insert:
	insmod myBlockDriver.ko
	chmod 777 /dev/virtualdrive
	chmod 777 /dev/virtualdrive1
	chmod 777 /dev/virtualdrive2
	chmod 777 /dev/virtualdrive3
	chmod 777 /dev/virtualdrive5
	chmod 777 /dev/virtualdrive6
	chmod 777 /dev/virtualdrive7

remove:
	rmmod myBlockDriver
