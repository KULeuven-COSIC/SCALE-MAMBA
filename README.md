
# SCALE and MAMBA

*Secure Computation Algorithms from LEuven* : SCALE

*Multiparty AlgorithMs Basic Argot*         : MAMBA


First type
```
make doc
```

Then *read* the documentation!

Note: For Leuven maintainers, if wishing to recompile the basic 64 bit 
circuits then call
```
make circuits
```

These are then compiled down from the netlist down to the Bristol
fashion again, and then simplified. 

After doing this run
```
./Test-Convert.x
```
to check all is OK.


If you want to recompile the `.net` circuits from the `.vhd` see the
instructions in Circuits/README.txt

# Contributing and Releases

We run a quite extensive test suite on all changes to the system.
Indeed the testing takes around three days on our machines. We also
want to maintain stability between releases for people using the
system. Thus we have created the following rules of thumb:

i)   New releases will be about every quarter.

ii)  Only major bug fixes will be done in the mean time.

iii) Join the mailing list to get updates (spdz@googlegroups.com)

iv)  For small changes/bug fixes you find, either email the group
     or make a pull request here. We will then look at the code,
     integrate it into our private master system and perform the
     testing on it. Then we will push to GitHub with the next
     release.

v)   Major changes (more than the odd line here or there), please
     talk  with us first. You may be doing something which we
     plan to scrap in the near future. So you dont want to waste
     your time. 

