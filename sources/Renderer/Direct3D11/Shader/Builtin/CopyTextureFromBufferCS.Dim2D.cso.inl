#if 0
//
// Generated by Microsoft (R) D3D Shader Disassembler
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Input
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// no Output
cs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[4], immediateIndexed
dcl_resource_raw t0
dcl_uav_typed_texture2darray (uint,uint,uint,uint) u0
dcl_input vThreadID.xyz
dcl_temps 3
dcl_thread_group 1, 1, 1
imad r0.x, vThreadID.z, cb0[1].y, vThreadID.y
imad r0.x, r0.x, cb0[1].x, vThreadID.x
imul null, r0.y, cb0[1].x, cb0[2].x
ult r0.z, r0.y, cb0[2].w
iadd r0.y, -r0.y, cb0[2].w
and r0.y, r0.y, r0.z
imad r0.z, r0.y, vThreadID.y, cb0[0].w
imad r0.y, cb0[1].x, cb0[2].x, r0.y
imul null, r0.y, r0.y, cb0[1].y
ult r0.w, r0.y, cb0[3].x
iadd r0.y, -r0.y, cb0[3].x
and r0.y, r0.y, r0.w
imad r0.y, r0.y, vThreadID.z, r0.z
switch cb0[2].y
  case l(1)
  switch cb0[2].z
    case l(8)
    ushr r0.z, r0.x, l(2)
    imad r0.z, r0.z, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r0.z, r0.z, t0.xxxx
    bfi r0.w, l(2), l(3), r0.x, l(0)
    ushr r0.z, r0.z, r0.w
    and r1.x, r0.z, l(255)
    break 
    case l(16)
    ushr r0.z, r0.x, l(1)
    imad r0.z, r0.z, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r0.z, r0.z, t0.xxxx
    bfi r0.w, l(1), l(4), r0.x, l(0)
    ushr r0.z, r0.z, r0.w
    and r1.x, r0.z, l(0x0000ffff)
    break 
    case l(32)
    imad r0.z, r0.x, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r1.x, r0.z, t0.xxxx
    break 
    default 
    mov r1.x, l(0)
    break 
  endswitch 
  mov r1.yzw, l(0,0,0,0)
  break 
  case l(2)
  switch cb0[2].z
    case l(8)
    ushr r0.z, r0.x, l(1)
    imad r0.z, r0.z, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r0.z, r0.z, t0.xxxx
    bfi r0.w, l(1), l(4), r0.x, l(0)
    ushr r0.z, r0.z, r0.w
    and r1.x, r0.z, l(255)
    ubfe r1.y, l(8), l(8), r0.z
    break 
    case l(16)
    imad r0.z, r0.x, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r0.z, r0.z, t0.xxxx
    and r1.x, r0.z, l(0x0000ffff)
    ushr r1.y, r0.z, l(16)
    break 
    case l(32)
    imad r0.z, r0.x, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r1.xy, r0.z, t0.xyxx
    break 
    default 
    mov r1.xy, l(0,0,0,0)
    break 
  endswitch 
  mov r1.zw, l(0,0,0,0)
  break 
  case l(3)
  ieq r0.z, cb0[2].z, l(32)
  imad r0.w, r0.x, cb0[1].w, r0.y
  ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r2.xyz, r0.w, t0.xyzx
  and r1.xyz, r0.zzzz, r2.xyzx
  mov r1.w, l(0)
  break 
  case l(4)
  switch cb0[2].z
    case l(8)
    imad r0.z, r0.x, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r0.z, r0.z, t0.xxxx
    and r1.x, r0.z, l(255)
    ubfe r1.yz, l(0, 8, 8, 0), l(0, 8, 16, 0), r0.zzzz
    ushr r1.w, r0.z, l(24)
    break 
    case l(16)
    imad r0.z, r0.x, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r0.zw, r0.z, t0.xxxy
    and r1.xz, r0.zzwz, l(0x0000ffff, 0, 0x0000ffff, 0)
    ushr r1.yw, r0.zzzw, l(0, 16, 0, 16)
    break 
    case l(32)
    imad r0.x, r0.x, cb0[1].w, r0.y
    ld_raw_indexable(raw_buffer)(mixed,mixed,mixed,mixed) r1.xyzw, r0.x, t0.xyzw
    break 
    default 
    mov r1.xyzw, l(0,0,0,0)
    break 
  endswitch 
  break 
  default 
  mov r1.xyzw, l(0,0,0,0)
  break 
endswitch 
iadd r0.xyzw, vThreadID.xyzz, cb0[0].xyzz
store_uav_typed u0.xyzw, r0.xyzw, r1.xyzw
ret 
// Approximately 0 instruction slots used
#endif

const BYTE g_CopyTextureFromBufferCS_Dim2D[] =
{
     68,  88,  66,  67, 144, 249, 
     54,  64,  79,  25, 146, 219, 
     84,  77, 184,  18,  20, 147, 
    214, 107,   1,   0,   0,   0, 
    132,  10,   0,   0,   3,   0, 
      0,   0,  44,   0,   0,   0, 
     60,   0,   0,   0,  76,   0, 
      0,   0,  73,  83,  71,  78, 
      8,   0,   0,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
     79,  83,  71,  78,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      8,   0,   0,   0,  83,  72, 
     69,  88,  48,  10,   0,   0, 
     80,   0,   5,   0, 140,   2, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      4,   0,   0,   0, 161,   0, 
      0,   3,   0, 112,  16,   0, 
      0,   0,   0,   0, 156,  64, 
      0,   4,   0, 224,  17,   0, 
      0,   0,   0,   0,  68,  68, 
      0,   0,  95,   0,   0,   2, 
    114,   0,   2,   0, 104,   0, 
      0,   2,   3,   0,   0,   0, 
    155,   0,   0,   4,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  35,   0, 
      0,   8,  18,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
      2,   0,  26, 128,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  26,   0,   2,   0, 
     35,   0,   0,   9,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  10,   0,   2,   0, 
     38,   0,   0,  10,   0, 208, 
      0,   0,  34,   0,  16,   0, 
      0,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,  79,   0, 
      0,   8,  66,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
     30,   0,   0,   9,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16, 128,  65,   0, 
      0,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      1,   0,   0,   7,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  35,   0, 
      0,   9,  66,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,   2,   0,  58, 128, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  35,   0, 
      0,  11,  34,   0,  16,   0, 
      0,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  10, 128, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     38,   0,   0,   9,   0, 208, 
      0,   0,  34,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     79,   0,   0,   8, 130,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  30,   0,   0,   9, 
     34,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16, 128, 
     65,   0,   0,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   7, 
     34,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   0,   0,   0,   0, 
     35,   0,   0,   8,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,   2,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  76,   0,   0,   4, 
     26, 128,  32,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      6,   0,   0,   3,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     76,   0,   0,   4,  42, 128, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,   6,   0, 
      0,   3,   1,  64,   0,   0, 
      8,   0,   0,   0,  85,   0, 
      0,   7,  66,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,  35,   0,   0,  10, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  58, 128, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
    165,   0,   0, 137, 194,   2, 
      0, 128, 131, 153,  25,   0, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   6, 112, 
     16,   0,   0,   0,   0,   0, 
    140,   0,   0,  11, 130,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      3,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  85,   0,   0,   7, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  58,   0, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   7,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
    255,   0,   0,   0,   2,   0, 
      0,   1,   6,   0,   0,   3, 
      1,  64,   0,   0,  16,   0, 
      0,   0,  85,   0,   0,   7, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     35,   0,   0,  10,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  58, 128,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0, 165,   0, 
      0, 137, 194,   2,   0, 128, 
    131, 153,  25,   0,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,   6, 112,  16,   0, 
      0,   0,   0,   0, 140,   0, 
      0,  11, 130,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   4,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     85,   0,   0,   7,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0, 255, 255, 
      0,   0,   2,   0,   0,   1, 
      6,   0,   0,   3,   1,  64, 
      0,   0,  32,   0,   0,   0, 
     35,   0,   0,  10,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  58, 128,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0, 165,   0, 
      0, 137, 194,   2,   0, 128, 
    131, 153,  25,   0,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,   6, 112,  16,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   1,  10,   0,   0,   1, 
     54,   0,   0,   5,  18,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   1, 
     23,   0,   0,   1,  54,   0, 
      0,   8, 226,   0,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      2,   0,   0,   1,   6,   0, 
      0,   3,   1,  64,   0,   0, 
      2,   0,   0,   0,  76,   0, 
      0,   4,  42, 128,  32,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   6,   0,   0,   3, 
      1,  64,   0,   0,   8,   0, 
      0,   0,  85,   0,   0,   7, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     35,   0,   0,  10,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  58, 128,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0, 165,   0, 
      0, 137, 194,   2,   0, 128, 
    131, 153,  25,   0,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,   6, 112,  16,   0, 
      0,   0,   0,   0, 140,   0, 
      0,  11, 130,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   4,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     85,   0,   0,   7,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0, 255,   0, 
      0,   0, 138,   0,   0,   9, 
     34,   0,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      8,   0,   0,   0,   1,  64, 
      0,   0,   8,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,   2,   0,   0,   1, 
      6,   0,   0,   3,   1,  64, 
      0,   0,  16,   0,   0,   0, 
     35,   0,   0,  10,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  58, 128,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0, 165,   0, 
      0, 137, 194,   2,   0, 128, 
    131, 153,  25,   0,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,   6, 112,  16,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0, 255, 255, 
      0,   0,  85,   0,   0,   7, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,  16,   0,   0,   0, 
      2,   0,   0,   1,   6,   0, 
      0,   3,   1,  64,   0,   0, 
     32,   0,   0,   0,  35,   0, 
      0,  10,  66,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0, 165,   0,   0, 137, 
    194,   2,   0, 128, 131, 153, 
     25,   0,  50,   0,  16,   0, 
      1,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     70, 112,  16,   0,   0,   0, 
      0,   0,   2,   0,   0,   1, 
     10,   0,   0,   1,  54,   0, 
      0,   8,  50,   0,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      2,   0,   0,   1,  23,   0, 
      0,   1,  54,   0,   0,   8, 
    194,   0,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   1,   6,   0,   0,   3, 
      1,  64,   0,   0,   3,   0, 
      0,   0,  32,   0,   0,   8, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  42, 128,  32,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
     32,   0,   0,   0,  35,   0, 
      0,  10, 130,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0, 165,   0,   0, 137, 
    194,   2,   0, 128, 131, 153, 
     25,   0, 114,   0,  16,   0, 
      2,   0,   0,   0,  58,   0, 
     16,   0,   0,   0,   0,   0, 
     70, 114,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   7, 
    114,   0,  16,   0,   1,   0, 
      0,   0, 166,  10,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   2,   0,   0,   0, 
     54,   0,   0,   5, 130,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   1, 
      6,   0,   0,   3,   1,  64, 
      0,   0,   4,   0,   0,   0, 
     76,   0,   0,   4,  42, 128, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,   6,   0, 
      0,   3,   1,  64,   0,   0, 
      8,   0,   0,   0,  35,   0, 
      0,  10,  66,   0,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     58, 128,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0, 165,   0,   0, 137, 
    194,   2,   0, 128, 131, 153, 
     25,   0,  66,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
      6, 112,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0, 255,   0,   0,   0, 
    138,   0,   0,  15,  98,   0, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
      8,   0,   0,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,  16,   0,   0,   0, 
      0,   0,   0,   0, 166,  10, 
     16,   0,   0,   0,   0,   0, 
     85,   0,   0,   7, 130,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
     24,   0,   0,   0,   2,   0, 
      0,   1,   6,   0,   0,   3, 
      1,  64,   0,   0,  16,   0, 
      0,   0,  35,   0,   0,  10, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,  58, 128, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
    165,   0,   0, 137, 194,   2, 
      0, 128, 131, 153,  25,   0, 
    194,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,   6, 116, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,  10,  82,   0, 
     16,   0,   1,   0,   0,   0, 
    166,  11,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
    255, 255,   0,   0,   0,   0, 
      0,   0, 255, 255,   0,   0, 
      0,   0,   0,   0,  85,   0, 
      0,  10, 162,   0,  16,   0, 
      1,   0,   0,   0, 166,  14, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,  16,   0,   0,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   0,   2,   0,   0,   1, 
      6,   0,   0,   3,   1,  64, 
      0,   0,  32,   0,   0,   0, 
     35,   0,   0,  10,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  58, 128,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0, 165,   0, 
      0, 137, 194,   2,   0, 128, 
    131, 153,  25,   0, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  70, 126,  16,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   1,  10,   0,   0,   1, 
     54,   0,   0,   8, 242,   0, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   1, 
     23,   0,   0,   1,   2,   0, 
      0,   1,  10,   0,   0,   1, 
     54,   0,   0,   8, 242,   0, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   1, 
     23,   0,   0,   1,  30,   0, 
      0,   7, 242,   0,  16,   0, 
      0,   0,   0,   0,  70,  10, 
      2,   0,  70, 138,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 164,   0,   0,   7, 
    242, 224,  17,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     62,   0,   0,   1
};