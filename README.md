
# SCALE and MAMBA

*Secure Computation Algorithms from LEuven* : SCALE

*Multiparty AlgorithMs Basic Argot*         : MAMBA


First type
<p>
        make doc
</p>
Then *read* the documentation!

Note: For Leuven maintainers, if wishing to recompile the basic 64 bit 
circuits then call
<p>
        make circuits
</p>
These are then compiled down from the netlist down to the Bristol
fashion again, and then simplified. After doing this run
<p>
        ./Test-Convert.x
</p>
to check all is OK.


If you want to recompile the .net circuits from the .vhd see the
instructions in Circuits/README.txt

