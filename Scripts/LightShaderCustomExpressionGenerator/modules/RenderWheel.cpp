float cp_renderGobo(float3 pos, WheelInfo wheel){
	float2 texCoor = pos.xy;
	float scale = 1 / wheel.size;
	float offset = wheel.index / wheel.size;

	texCoor.x = texCoor.x * scale + offset;

	return wheel.txtP.SampleLevel(wheel.txtPSampler, texCoor, 0);
}