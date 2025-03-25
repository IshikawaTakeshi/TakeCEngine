
//Vector3からスカラーの乱数の取得
float Rand3dTo1d(float3 value,float3 dotDir = float3(12.9898, 78.233, 37.719)) {
	
	float3 smallValue = sin(value);
	
	float randomValue = dot(smallValue, dotDir);
	
	randomValue = frac(randomValue * 143758.5453);
	
	return randomValue;
}

//Vector3からVector3の乱数の取得
float3 Rand3dTo3d(float3 value) {
	return float3(
	Rand3dTo1d(value, float3(12.989, 78.233, 37.719)),
	Rand3dTo1d(value, float3(39.346, 11.135, 83.155)),
	Rand3dTo1d(value, float3(73.156, 52.235, 09.151))
	);
}

//乱数生成クラス
class RamdomGenerator {
	
	//乱数のシード
	float3 seed;
	
	//乱数生成(3d)
	float3 Generate3d() {
		seed = Rand3dTo3d(seed);
		return seed;
	}
	
	//乱数生成(1d)
	float Generate1d() {
		float result = Rand3dTo1d(seed);
		seed.x = result;
		return result;
	}
};

//1フレームの情報
struct PerFrame {
	//ゲームを起動してからの時間
	float time;
	//1フレームの経過時間
	float deltaTime;
};