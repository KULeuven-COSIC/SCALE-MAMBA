./convert.x adder64
./convert.x sub64
./convert.x mult2_64
./convert.x mult64
./convert.x divide64
./convert.x neg64
./convert.x zero_equal
./convert.x ModAdd512
./convert.x LSSS_to_GC

\cp VHDL/COSIC/SHA-3/Keccak_f.net VHDL/Keccak_f.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/Keccak_f.net 
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/Keccak_f.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/Keccak_f.net
./convert.x Keccak_f

\cp VHDL/COSIC/AES/aes_128.net VHDL/aes_128.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/aes_128.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/aes_128.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/aes_128.net
./convert.x aes_128

\cp VHDL/COSIC/AES/aes_192.net VHDL/aes_192.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/aes_192.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/aes_192.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/aes_192.net
./convert.x aes_192

\cp VHDL/COSIC/AES/aes_256.net VHDL/aes_256.net
sed -i ':begin;$!N;s/)\n      /)/;tbegin;P;D' VHDL/aes_256.net
sed -i ':begin;$!N;s/(\n      /(/;tbegin;P;D' VHDL/aes_256.net
sed -i ':begin;$!N;s/, \n      /, /;tbegin;P;D' VHDL/aes_256.net
./convert.x aes_256


