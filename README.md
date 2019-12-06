# Virtual Block Device Driver

This driver has been tested and functional on MX_Linux & Debian with a 4.19 kernel.
 It is most likely compatible with any kernel from version 2.6 to version 4 kernels.

This driver has however found to be unable to be compiled using version 5 kernels, in paricular kernel version 5.3

#### 1.	
>Go to the directory where the makefile is and write 

	$ make all

#### 2.
>insert the module (and make our block device completely accessible) using 

	sudo make insert

#### 3.
>check if module is loaded using 

	lsmod myBlockDriver

#### .4
>Check the partition info using cat /proc/partitions as well as using 

    lsblk
>or

	ls -l /dev/virtualdrive*
>or 

	sudo fdisk -l
>The device we created has 3 logical and extended partitions

#### .5
>You have 2 alternative ways of doing I/O with your virtual block device
>
> #### a. Using a test program 
> In our directory there is a Test directory, use 

	cd Test 
> to enter the directory. Then type 

	gcc test.c
> to compile the c file in it. Run the program using 
	./a.out
> and it will provide an interface to perform I/O with your device. 
> Originally this program was tweaked to best work with character 
> devices so you will perform I/O with ascii characters.
> You can delete a.out if you want to after, also the "gcc"
> command may not always work and may be instead gcc-[Version #]
> like gcc-8 for example.
>
> #### a. Using Linux Commands
> You can also use linux commands on your bash terminal to 
> perform the same I/O
> To write into disk use

	cat > /dev/virtualdrive 
> Type something & press ctrl+c

> to read back from the disk use the command 

	xxd /dev/virtualdrive | less

#### .6
>remove the module using 

	sudo make remove

#### .7
>clean the folder using

	make clean



