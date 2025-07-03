```Assembly
halo1.dll+BD9D09 - mov r13,[halo1.dll+1C55C68]
halo1.dll+BD9D4A - movsxd  rdx,dword ptr [r13+58] ;  Load BSP VertexBlock offset. Note: I believe [r13+54] is the vertex count.
halo1.dll+BD9D93 - mov rcx,rdx         
;  Now RCX, RDX = VertexBlockMapOffset

;  Translate from map relative address to memory address
halo1.dll+BD9D96 - sub rcx,[halo1.dll+2EA3410] ;  Subtract MapAddress
halo1.dll+BD9D9D - add rcx,[halo1.dll+2D9CE10] ;  Add RelocatedMapAddress
;  Now RCX = VertexBlockBase

;  Read vertex data...
halo1.dll+BD9DD2 - add rdx,rdx             ; Double the index (sizeof BSPVertex = 16 bytes)
halo1.dll+BD9DD5 - movss xmm0,[rcx+rdx*8]
```
