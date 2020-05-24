__kernel void ElemOp(__global unsigned int* in,__global unsigned int* key)
{
    int num = get_global_id(0);
    //out[num] = in[num]/2;
    in[num] = in[num] + key[num];
}