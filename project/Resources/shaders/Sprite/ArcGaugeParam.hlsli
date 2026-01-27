struct ArcGaugeParam {
	float2 center; // 中心座標
	float fillAmount; // 塗りつぶし量
	float innerRadius; // 内半径
	float outerRadius; // 外半径
	float startAngle; // 開始角度
	float arcRange; // 弧の最大角度
	int clockwise; // 回転方向
};