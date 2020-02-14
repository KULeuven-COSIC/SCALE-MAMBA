/////////////////////////////////////////////////////////////
// Created by: Synopsys DC Ultra(TM) in wire load mode
// Version   : P-2019.03
// Date      : Fri Feb  7 14:54:09 2020
/////////////////////////////////////////////////////////////


module MZeroEqual64 ( a, res );
  input [63:0] a;
  output [0:0] res;
  wire   n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16,
         n17, n18, n19, n20, n21, n22, n23, n24, n25, n26, n27, n28, n29, n30,
         n31, n32, n33, n34, n35, n36, n37, n38, n39, n40, n41, n42, n43, n44,
         n45, n46, n47, n48, n49, n50, n51, n52, n53, n54, n55, n56, n57, n58,
         n59, n60, n61, n62, n63, n64, n65, n66, n67, n68, n69, n70, n71, n72,
         n73, n74, n75, n76, n77, n78, n79, n80, n81, n82, n83, n84, n85, n86,
         n87, n88, n89, n90, n91, n92, n93, n94, n95, n96, n97, n98, n99, n100,
         n101, n102, n103, n104, n105, n106, n107, n108, n109, n110, n111,
         n112, n113, n114, n115, n116, n117, n118, n119, n120, n121, n122,
         n123, n124, n125, n126;

  INV U2 ( .A(a[63]), .ZN(n2) );
  INV U3 ( .A(a[60]), .ZN(n1) );
  AND2 U4 ( .A(n2), .B(n1), .Z(n6) );
  INV U5 ( .A(a[62]), .ZN(n4) );
  INV U6 ( .A(a[61]), .ZN(n3) );
  AND2 U7 ( .A(n4), .B(n3), .Z(n5) );
  AND2 U8 ( .A(n6), .B(n5), .Z(n14) );
  INV U9 ( .A(a[51]), .ZN(n8) );
  INV U10 ( .A(a[48]), .ZN(n7) );
  AND2 U11 ( .A(n8), .B(n7), .Z(n12) );
  INV U12 ( .A(a[50]), .ZN(n10) );
  INV U13 ( .A(a[49]), .ZN(n9) );
  AND2 U14 ( .A(n10), .B(n9), .Z(n11) );
  AND2 U15 ( .A(n12), .B(n11), .Z(n13) );
  AND2 U16 ( .A(n14), .B(n13), .Z(n30) );
  INV U17 ( .A(a[59]), .ZN(n16) );
  INV U18 ( .A(a[56]), .ZN(n15) );
  AND2 U19 ( .A(n16), .B(n15), .Z(n20) );
  INV U20 ( .A(a[58]), .ZN(n18) );
  INV U21 ( .A(a[57]), .ZN(n17) );
  AND2 U22 ( .A(n18), .B(n17), .Z(n19) );
  AND2 U23 ( .A(n20), .B(n19), .Z(n28) );
  INV U24 ( .A(a[55]), .ZN(n22) );
  INV U25 ( .A(a[52]), .ZN(n21) );
  AND2 U26 ( .A(n22), .B(n21), .Z(n26) );
  INV U27 ( .A(a[54]), .ZN(n24) );
  INV U28 ( .A(a[53]), .ZN(n23) );
  AND2 U29 ( .A(n24), .B(n23), .Z(n25) );
  AND2 U30 ( .A(n26), .B(n25), .Z(n27) );
  AND2 U31 ( .A(n28), .B(n27), .Z(n29) );
  AND2 U32 ( .A(n30), .B(n29), .Z(n62) );
  INV U33 ( .A(a[15]), .ZN(n32) );
  INV U34 ( .A(a[12]), .ZN(n31) );
  AND2 U35 ( .A(n32), .B(n31), .Z(n36) );
  INV U36 ( .A(a[14]), .ZN(n34) );
  INV U37 ( .A(a[13]), .ZN(n33) );
  AND2 U38 ( .A(n34), .B(n33), .Z(n35) );
  AND2 U39 ( .A(n36), .B(n35), .Z(n44) );
  INV U40 ( .A(a[3]), .ZN(n38) );
  INV U41 ( .A(a[0]), .ZN(n37) );
  AND2 U42 ( .A(n38), .B(n37), .Z(n42) );
  INV U43 ( .A(a[2]), .ZN(n40) );
  INV U44 ( .A(a[1]), .ZN(n39) );
  AND2 U45 ( .A(n40), .B(n39), .Z(n41) );
  AND2 U46 ( .A(n42), .B(n41), .Z(n43) );
  AND2 U47 ( .A(n44), .B(n43), .Z(n60) );
  INV U48 ( .A(a[11]), .ZN(n46) );
  INV U49 ( .A(a[8]), .ZN(n45) );
  AND2 U50 ( .A(n46), .B(n45), .Z(n50) );
  INV U51 ( .A(a[10]), .ZN(n48) );
  INV U52 ( .A(a[9]), .ZN(n47) );
  AND2 U53 ( .A(n48), .B(n47), .Z(n49) );
  AND2 U54 ( .A(n50), .B(n49), .Z(n58) );
  INV U55 ( .A(a[7]), .ZN(n52) );
  INV U56 ( .A(a[4]), .ZN(n51) );
  AND2 U57 ( .A(n52), .B(n51), .Z(n56) );
  INV U58 ( .A(a[6]), .ZN(n54) );
  INV U59 ( .A(a[5]), .ZN(n53) );
  AND2 U60 ( .A(n54), .B(n53), .Z(n55) );
  AND2 U61 ( .A(n56), .B(n55), .Z(n57) );
  AND2 U62 ( .A(n58), .B(n57), .Z(n59) );
  AND2 U63 ( .A(n60), .B(n59), .Z(n61) );
  AND2 U64 ( .A(n62), .B(n61), .Z(n126) );
  INV U65 ( .A(a[47]), .ZN(n64) );
  INV U66 ( .A(a[44]), .ZN(n63) );
  AND2 U67 ( .A(n64), .B(n63), .Z(n68) );
  INV U68 ( .A(a[46]), .ZN(n66) );
  INV U69 ( .A(a[45]), .ZN(n65) );
  AND2 U70 ( .A(n66), .B(n65), .Z(n67) );
  AND2 U71 ( .A(n68), .B(n67), .Z(n76) );
  INV U72 ( .A(a[35]), .ZN(n70) );
  INV U73 ( .A(a[32]), .ZN(n69) );
  AND2 U74 ( .A(n70), .B(n69), .Z(n74) );
  INV U75 ( .A(a[34]), .ZN(n72) );
  INV U76 ( .A(a[33]), .ZN(n71) );
  AND2 U77 ( .A(n72), .B(n71), .Z(n73) );
  AND2 U78 ( .A(n74), .B(n73), .Z(n75) );
  AND2 U79 ( .A(n76), .B(n75), .Z(n92) );
  INV U80 ( .A(a[43]), .ZN(n78) );
  INV U81 ( .A(a[40]), .ZN(n77) );
  AND2 U82 ( .A(n78), .B(n77), .Z(n82) );
  INV U83 ( .A(a[42]), .ZN(n80) );
  INV U84 ( .A(a[41]), .ZN(n79) );
  AND2 U85 ( .A(n80), .B(n79), .Z(n81) );
  AND2 U86 ( .A(n82), .B(n81), .Z(n90) );
  INV U87 ( .A(a[39]), .ZN(n84) );
  INV U88 ( .A(a[36]), .ZN(n83) );
  AND2 U89 ( .A(n84), .B(n83), .Z(n88) );
  INV U90 ( .A(a[38]), .ZN(n86) );
  INV U91 ( .A(a[37]), .ZN(n85) );
  AND2 U92 ( .A(n86), .B(n85), .Z(n87) );
  AND2 U93 ( .A(n88), .B(n87), .Z(n89) );
  AND2 U94 ( .A(n90), .B(n89), .Z(n91) );
  AND2 U95 ( .A(n92), .B(n91), .Z(n124) );
  INV U96 ( .A(a[31]), .ZN(n94) );
  INV U97 ( .A(a[28]), .ZN(n93) );
  AND2 U98 ( .A(n94), .B(n93), .Z(n98) );
  INV U99 ( .A(a[30]), .ZN(n96) );
  INV U100 ( .A(a[29]), .ZN(n95) );
  AND2 U101 ( .A(n96), .B(n95), .Z(n97) );
  AND2 U102 ( .A(n98), .B(n97), .Z(n106) );
  INV U103 ( .A(a[19]), .ZN(n100) );
  INV U104 ( .A(a[16]), .ZN(n99) );
  AND2 U105 ( .A(n100), .B(n99), .Z(n104) );
  INV U106 ( .A(a[18]), .ZN(n102) );
  INV U107 ( .A(a[17]), .ZN(n101) );
  AND2 U108 ( .A(n102), .B(n101), .Z(n103) );
  AND2 U109 ( .A(n104), .B(n103), .Z(n105) );
  AND2 U110 ( .A(n106), .B(n105), .Z(n122) );
  INV U111 ( .A(a[27]), .ZN(n108) );
  INV U112 ( .A(a[24]), .ZN(n107) );
  AND2 U113 ( .A(n108), .B(n107), .Z(n112) );
  INV U114 ( .A(a[26]), .ZN(n110) );
  INV U115 ( .A(a[25]), .ZN(n109) );
  AND2 U116 ( .A(n110), .B(n109), .Z(n111) );
  AND2 U117 ( .A(n112), .B(n111), .Z(n120) );
  INV U118 ( .A(a[23]), .ZN(n114) );
  INV U119 ( .A(a[20]), .ZN(n113) );
  AND2 U120 ( .A(n114), .B(n113), .Z(n118) );
  INV U121 ( .A(a[22]), .ZN(n116) );
  INV U122 ( .A(a[21]), .ZN(n115) );
  AND2 U123 ( .A(n116), .B(n115), .Z(n117) );
  AND2 U124 ( .A(n118), .B(n117), .Z(n119) );
  AND2 U125 ( .A(n120), .B(n119), .Z(n121) );
  AND2 U126 ( .A(n122), .B(n121), .Z(n123) );
  AND2 U127 ( .A(n124), .B(n123), .Z(n125) );
  AND2 U128 ( .A(n126), .B(n125), .Z(res[0]) );
endmodule

