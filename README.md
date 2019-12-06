# Virtual Block Device Driver

This driver has been tested and functional on MX_Linux with a 4.19 kernel. It is most likely compatible with any kernel from version 2.6 to version 4.*

This driver has however found to be unable to be compiled using version 5 kernels, in paricular kernel version 5.3

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



