obj-m :=myBlockDriver.o

all :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:       
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insert:
	insmod myBlockDriver.ko

remove:
	rmmod myBlockDriver