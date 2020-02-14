module AND2 ( A, B, Z );
    input A;
    input B;
    output Z;
    assign Z = A & B;
endmodule

module NOR2 ( A, B, ZN );
    input A;
    input B;
    output ZN;
    assign ZN = ~(A|B);
endmodule

module XOR2 ( A, B, Z );
    input A;
    input B;
    output Z;
    assign Z = A^B;
endmodule

module INV ( A, ZN );
    input A;
    output ZN;
    assign ZN = ~A;
endmodule
