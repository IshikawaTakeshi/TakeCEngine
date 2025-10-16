#pragma once

//===========================================
//	ブレンドモード列挙型
//===========================================

enum class BlendState {
	NORMAL              = 0,
	ADD                 = 1,
	SUBTRACT            = 2,
	MULTIPLY            = 3,
	SCREEN              = 4,
	SPRITE              = 5,
	BOOSTEFFECT         = 6,
	PREMULTIPLIED_ALPHA = 7,
	COUNT               = 8,
};
