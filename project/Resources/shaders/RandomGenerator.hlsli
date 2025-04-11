
float Rand3dTo1d(float3 value,float3 dotDir = float3(12.9898, 78.233, 37.719)) {
	
	float3 smallValue = sin(value);
	
	float randomValue = dot(smallValue, dotDir);
	
	randomValue = frac(sin(randomValue) * 143758.5453);
	
	return randomValue;
}

float3 Rand3dTo3d(float3 value) {
	return float3(
	Rand3dTo1d(value, float3(12.989, 78.233, 37.719)),
	Rand3dTo1d(value, float3(39.346, 11.135, 83.155)),
	Rand3dTo1d(value, float3(73.156, 52.235, 09.151))
	);
}

class RamdomGenerator {

	float3 seed;
	
	float3 Generate3d() {
		seed = Rand3dTo3d(seed);
		return seed;
	}
	
	float Generate1d() {
		float result = Rand3dTo1d(seed);
		seed.x = result;
		return result;
	}
};

struct PerFrame {

	float time;

	float deltaTime;
};