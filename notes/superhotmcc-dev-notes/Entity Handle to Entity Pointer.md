Somewhere in this code from `DamageEntity`, an entity handle/identifier is turned into an entity pointer. The end result is in `rbx`, the handle starts in `rcx`.

```Assembly
mov rbx,[halo1.dll+2D9CDF8]
or r8d,-01
mov rbp,[rsp+000000E0]
add rbx,34
movaps [rax-48],xmm6
movaps [rax-58],xmm7
xorps xmm7,xmm7
movaps [rax-68],xmm8
movaps [rax-78],xmm9
movss xmm9,[rsp+00000118]
movzx r10d,cx
mov r14d,ecx
mov rcx,[halo1.dll+1C42248]
lea rdx,[r10+r10*2]
shl rdx,02
movsxd  rax,dword ptr [rcx+34]
add rax,rdx
mov [rsp+30],rdx
xor edx,edx
movsxd  r12,dword ptr [rax+rcx+08]
mov rcx,[rsp+000000E8]
add rbx,r12                        ; Now rbx = entityPtr 

```

I believe these are the relevant instructions and interpretation:

```Assembly
mov rbx,[halo1.dll+2D9CDF8]        ; rbx = pEntityArrayStruct
add rbx,34                         ; rbx = pEntityArrayStruct->entityArray
movzx r10d,cx                      ; r10 = entityHandle & 0xFFFF (entityIndex)
mov rcx,[halo1.dll+1C42248]        ; rcx = pEntityRecordArrayStruct
lea rdx,[r10+r10*2]                ; rdx = entityIndex * 3
shl rdx,02                         ; rdx = entityIndex * 12 (stride is 3 DWORDs)
movsxd  rax,dword ptr [rcx+34]     ; rax = 
                                   ; pEntityRecordArrayStruct.entityRecordsOffset
                                   ; (seems to always be 56)
add rax,rdx                        ; rax = entityRecordsOffset+entityIndex*12
movsxd  r12,dword ptr [rax+rcx+08] ; r12 = entityRecords[entityIndex].entityArrayOffset
add rbx,r12                        ; rbx = entityArray + entityArrayOffset
```

I'm not certain the entities in `entityArray` are all the same size, so this may be a misnomer.