./convert.x ModAdd512

\cp VHDL/MpcLib/Verilog/MADD64.v VHDL/adder64.net
./convert.x adder64

\cp VHDL/MpcLib/Verilog/MSUB64.v VHDL/sub64.net
./convert.x sub64

\cp VHDL/MpcLib/Verilog/MNegate64.v VHDL/neg64.net
./convert.x neg64

\cp VHDL/MpcLib/Verilog/MZeroEqual64.v VHDL/zero_equal.net
./convert.x zero_equal

\cp VHDL/MpcLib/Verilog/MHMUL64.v VHDL/mult64.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/mult64.net 
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/mult64.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/mult64.net
./convert.x mult64

\cp VHDL/MpcLib/Verilog/MMUL64.v VHDL/mult2_64.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/mult2_64.net 
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/mult2_64.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/mult2_64.net
./convert.x mult2_64

\cp VHDL/MpcLib/Verilog/MSDIV64.v VHDL/divide64.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/divide64.net 
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/divide64.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/divide64.net
./convert.x divide64

# Steve Lu found a bug so we dont use this one 
#  - Instead use one Steve created from emp-tool
#\cp VHDL/MpcLib/Verilog/MUDIV64.v VHDL/udivide64.net
#sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/udivide64.net
#sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/udivide64.net
#sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/udivide64.net
#./convert.x udivide64

\cp VHDL/SHA-3/Keccak_f.net VHDL/Keccak_f.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/Keccak_f.net 
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/Keccak_f.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/Keccak_f.net
./convert.x Keccak_f

\cp VHDL/AES/aes_128.net VHDL/aes_128.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/aes_128.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/aes_128.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/aes_128.net
./convert.x aes_128

\cp VHDL/AES/aes_192.net VHDL/aes_192.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/aes_192.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/aes_192.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/aes_192.net
./convert.x aes_192

\cp VHDL/AES/aes_256.net VHDL/aes_256.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/aes_256.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/aes_256.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/aes_256.net
./convert.x aes_256

\cp VHDL/SHA256/dc/sha256.net VHDL/sha256.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/sha256.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/sha256.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/sha256.net
./convert.x sha256

\cp VHDL/SHA512/dc/sha512.net VHDL/sha512.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/sha512.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/sha512.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/sha512.net
./convert.x sha512

\cp VHDL/LSSS_to_GC_512/dc/LSSS_to_GC_512.net VHDL/LSSS_to_GC.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/LSSS_to_GC.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/LSSS_to_GC.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/LSSS_to_GC.net
./convert.x LSSS_to_GC

./process.x add
./process.x mul
./process.x div
./process.x sqrt
./process.x f2i
./process.x i2f
./process.x eq
./process.x lt
./process.x floor
./process.x ceil

./make-mand.x adder64
./make-mand.x aes_128
./make-mand.x aes_192
./make-mand.x aes_256
./make-mand.x AES-non-expanded
./make-mand.x divide64
./make-mand.x FP-add
./make-mand.x FP-div
./make-mand.x FP-eq
./make-mand.x FP-f2i
./make-mand.x FP-i2f
./make-mand.x FP-lt
./make-mand.x FP-mul
./make-mand.x FP-sqrt
./make-mand.x FP-floor
./make-mand.x FP-ceil
./make-mand.x Keccak_f
./make-mand.x LSSS_to_GC
./make-mand.x ModAdd512
./make-mand.x mult2_64
./make-mand.x mult64
./make-mand.x neg64
./make-mand.x sha256
./make-mand.x sha512
./make-mand.x sub64
./make-mand.x udivide64
./make-mand.x zero_equal
