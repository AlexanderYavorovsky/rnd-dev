# rnd-dev
## What is it
Linux Kernel module driver for character device which generates pseudorandom bytes.

This module uses [CRS](https://en.wikipedia.org/wiki/Linear_recurrence_with_constant_coefficients) to generate pseudorandom numbers. The [gf library](https://github.com/AlexanderYavorovsky/gf) is also used, but here it was rewriten for kernel space.

## Getting Started
### Requirements
* Linux
* Make
* GCC, Clang

### Build and Run
1. Copy repo:
```
git clone https://github.com/AlexanderYavorovsky/rnd-dev
```

2. Navigate to directory and build:
```
cd rnd-dev
make
```

3. Insert module with your parameters, for instance:
```
sudo insmod rnddev.ko crs_len=6 crs_coeffs=1,2,3,4,5,6 crs_init_elems=3,4,8,2,3,4 crs_c=9
```
* *Tip: to get info about parameters, write:*
```
modinfo rnddev.ko
```


4. Make character device:
```
sudo mknod /dev/rnddev c 238 0
```
* *Note: you should probably use other numbers. To find them out, write:*
```
sudo dmesg
```
*then, you will see line like this:*
```
Create chrdev using: mknod /dev/rnddev c 238 0
```

5. Get that random stuff!
```
xxd /dev/rnddev
```

6. Remove module and character device if necessary:
```
sudo rmmod rnddev
sudo rm /dev/rnddev
```

* To clean up, use:
```
make clean
```
