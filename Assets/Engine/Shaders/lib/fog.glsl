
struct FogParameters {
	vec3 color;
	float linearStart;
	float linearEnd;
	float density;
	
	int equation;
	bool isEnabled;
};

float getFogFactor(FogParameters params, float fogCoordinate) {
	float result = 0.0;
	if(params.equation == 0) {
		float fogLength = params.linearEnd - params.linearStart;
		result = (params.linearEnd - fogCoordinate) / fogLength;
	}
	else if(params.equation == 1) {
		result = exp(-params.density * fogCoordinate);
	}
	else if(params.equation == 2) {
		result = exp(-pow(params.density * fogCoordinate, 2.0));
	}
	result = 1.0 - clamp(result, 0.0, 1.0);
	return result;
}
