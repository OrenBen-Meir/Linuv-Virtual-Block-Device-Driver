# Virtual Block Device Driver

This driver has been tested and functional on MX_Linux with a 4.19 kernel. It is likely to be compadible with debian based
distros including possibly unbuntu.
This driver however wasn't able to compile using any Arch based distro. It is not recommended to run this on a rolling release
distro unless you switch to an older kernel and use its corresponding linux-headers.

#### 1.	
>Go to the directory where the makefile is and write 

	$ make all

#### 2.
>insert the module using 

	sudo make insert

#### 3.
>check if module is loaded using 

	lsmod myBlockDriver

#### .4
>Check the partition info using cat /proc/partitions as well as using 

	ls -l /dev/virtualdrive*
>or 

	sudo fdisk -l
>The device we created has 3 logical and extended partitioms

#### .5
>Become root using

	sudo -s

#### .6
>to write into disk use 

	cat > /dev/virtualdrive 
>,type something & press enter

#### .7
>to read back from the disk on command line use command 

	xxd /dev/virtualdrive | less

#### .8
>Be sure to switch back from root by using the command

	exit

#### .9
>remove the module using 

	sudo make remove

#### .10
>clean the folder using

	make clean



