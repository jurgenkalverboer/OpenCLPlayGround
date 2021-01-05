__kernel void square(__global float* input,
		__global float* output, 
		__local float* temp,
		const unsigned int count) {
	int i = get_global_id(0); 
	if(i<count){
		temp[i] = input[i]*input[i];
		output[i]=temp[i];
	}
}