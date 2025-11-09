#include "SpriteAnimation.h"
#include "engine/Utility/Timer.h"
#include "engine/2d/Sprite.h"
#include "engine/base/TakeCFrameWork.h"
#include <utility>

void SpriteAnimator::Initialize(Sprite* target) {
	target_ = target;
}

//=============================================================================
// 拡大アニメーション
//=============================================================================

void SpriteAnimator::PlayUpScale(
	const Vector2& startSize, const Vector2& endSize,
	float duration, float delay,
	Easing::EasingType easeType, PlayMode playMode) {

	// アニメーションジョブ作成
	AnimationJob job;
	job.id = nextJobId_++;
	job.startValueVec2 = startSize;
	job.endValueVec2 = endSize;
	job.delay = delay;
	job.easeType = easeType;
	job.playMode = playMode;
	job.state = AnimationPhase::Up;
	job.timer.Initialize(duration, 0.0f);

	// 開始時に即座にstartを反映
	if (target_) target_->SetSize(startSize);

	// ジョブ更新処理
	job.updateFunc = [this, duration, delay](AnimationJob& self) {
		if (!target_) { self.finished = true; return; }

		//Timerを進める
		self.timer.Update();

		

		switch (self.state) {
		case AnimationPhase::Up:
		{
			// 拡大中
			float eased = self.timer.GetEase(self.easeType);
			Vector2 size = Easing::Lerp(self.startValueVec2, self.endValueVec2, eased);
			target_->SetSize(size);

			if (self.timer.GetProgress() >= 1.0f) {
				//Up完了
				switch (self.playMode) {
				case PlayMode::LOOP:
				{
					// 再度Upをやり直す
					self.timer.Initialize(duration, 0.0f);
					self.state = AnimationPhase::Up;
					target_->SetSize(self.startValueVec2);
					break;
				case PlayMode::ONCE:
				default:
					self.state = AnimationPhase::None;
					target_->SetSize(self.endValueVec2);
					self.finished = true;
					break;
				}
				case PlayMode::PINGPONG_ONCE:
				case PlayMode::PINGPONG_LOOP:
				{
					//Delayを秒で扱うためtimer.durationをdelayに設定
					self.timer.Initialize(delay, 0.0f);
					self.state = AnimationPhase::Delay;
					break;
				}
				}

			}
			break;
		}
		case AnimationPhase::Delay:
		{
			// Delayはtimer.duration == delay
			if (self.timer.GetProgress() >= 1.0f) {
				// Downに移行duration に戻す）
				self.timer.Initialize(duration, 0.0f);
				self.state = AnimationPhase::Down;
			}
			break;
		}
		case AnimationPhase::Down:
		{
			//補間しつつ縮小
			float eased = self.timer.GetEase(self.easeType);
			Vector2 size = Easing::Lerp(self.endValueVec2, self.startValueVec2, eased);
			target_->SetSize(size);

			if (self.timer.GetProgress() >= 1.0f) {
				switch (self.playMode) {
				case PlayMode::PINGPONG_ONCE:
					// PINGPONG_ONCE完了、終了
					target_->SetSize(self.startValueVec2);
					self.state = AnimationPhase::None;
					self.finished = true;
					break;
				case PlayMode::LOOP:
				case PlayMode::PINGPONG_LOOP:
					// PINGPONGループならUp再開
					self.timer.Initialize(duration, 0.0f);
					self.state = AnimationPhase::Up;
					target_->SetSize(self.startValueVec2);
					break;
				case PlayMode::ONCE:
				default:
					target_->SetSize(self.startValueVec2);
					self.state = AnimationPhase::None;
					self.finished = true;
					break;
				}
			}
			break;
		}
		default:
			break;
		}
		};
	// ジョブを登録
	animationJobs_.push_back(std::move(job));
}

//=============================================================================
// 移動アニメーション
//=============================================================================
void SpriteAnimator::PlayTranslate(
	const Vector2& startPos, const Vector2& endPos,
	float duration, float delay,
	Easing::EasingType easeType, PlayMode playMode) {

	// アニメーションジョブ作成
	AnimationJob job;
	job.id = nextJobId_++;
	job.startValueVec2 = startPos;
	job.endValueVec2 = endPos;
	job.delay = delay;
	job.easeType = easeType;
	job.playMode = playMode;
	job.state = AnimationPhase::Up;
	job.timer.Initialize(duration, 0.0f);

	// 開始時に即座にstartを反映
	if (target_) target_->SetTranslate(startPos);

	// ジョブ更新処理
	job.updateFunc = [this, duration, delay](AnimationJob& self) {
		if (!target_) { self.finished = true; return; }

		//Timerを進める
		self.timer.Update();

		switch (self.state) {
		case AnimationPhase::Up:
		{
			float eased = self.timer.GetEase(self.easeType);
			Vector2 pos = Easing::Lerp(self.startValueVec2, self.endValueVec2, eased);
			target_->SetTranslate(pos);

			if (self.timer.GetProgress() >= 1.0f) {
				//Up完了
				if (self.playMode == PlayMode::PINGPONG_ONCE) {
					//Delayを秒で扱うためtimer.durationをdelayに設定
					self.timer.Initialize(delay, 0.0f);
					self.state = AnimationPhase::Delay;
				} else if (self.playMode == PlayMode::LOOP) {
					// 再度Upをやり直す
					self.timer.Initialize(duration, 0.0f);
					self.state = AnimationPhase::Up;
					target_->SetTranslate(self.startValueVec2);
				} else { //ONCE
					self.state = AnimationPhase::None;
					target_->SetTranslate(self.endValueVec2);
					self.finished = true;
				}
			}
			break;
		}
		case AnimationPhase::Delay:
		{
			// Delayはtimer.duration == delay
			if (self.timer.GetProgress() >= 1.0f) {
				// Downに移行duration に戻す）
				self.timer.Initialize(duration, 0.0f);
				self.state = AnimationPhase::Down;
			}
			break;
		}
		case AnimationPhase::Down:
		{
			//補間しつつ移動
			float eased = self.timer.GetEase(self.easeType);
			Vector2 pos = Easing::Lerp(self.endValueVec2, self.startValueVec2, eased);
			target_->SetTranslate(pos);

			if (self.timer.GetProgress() >= 1.0f) {
				if (self.playMode == PlayMode::PINGPONG_ONCE) {
					// PINGPONG_ONCE 完了、終了
					target_->SetTranslate(self.startValueVec2);
					self.state = AnimationPhase::None;
					self.finished = true;
				} else if (self.playMode == PlayMode::LOOP) {
					// ループなら Up 再開
					self.timer.Initialize(duration, 0.0f);
					self.state = AnimationPhase::Up;
					target_->SetTranslate(self.startValueVec2);
				} else { // ONCE
					target_->SetTranslate(self.startValueVec2);
					self.state = AnimationPhase::None;
					self.finished = true;
				}
			}
			break;
		}

		default:
			break;
		}
		};
	// ジョブを登録
	animationJobs_.push_back(std::move(job));
}

//=============================================================================
// フェードアニメーション
//=============================================================================
void SpriteAnimator::PlayFade(
	const float startAlpha, const float endAlpha,
	float duration, float delay,
	Easing::EasingType easeType, PlayMode playMode) {

	// アニメーションジョブ作成
	AnimationJob job;
	job.id = nextJobId_++;
	job.startValueFloat = startAlpha;
	job.endValueFloat = endAlpha;
	job.delay = delay;
	job.easeType = easeType;
	job.playMode = playMode;
	job.state = AnimationPhase::Up;
	job.timer.Initialize(duration, 0.0f);

	// 開始時に即座にstartを反映
	if (target_) {
		Vector4 color = target_->GetMaterialColor();
		color.w = startAlpha;
		target_->GetMesh()->GetMaterial()->SetMaterialColor(color);
	}

	// ジョブ更新処理
	job.updateFunc = [this, duration, delay](AnimationJob& self) {
		if (!target_) { self.finished = true; return; }
		//Timerを進める
		self.timer.Update();
		switch (self.state) {
		case AnimationPhase::Up:
		{
			// フェードイン中
			float eased = self.timer.GetEase(self.easeType);
			float alpha = Easing::Lerp(self.startValueFloat, self.endValueFloat, eased);
			Vector4 color = target_->GetMaterialColor();
			color.w = alpha;
			target_->GetMesh()->GetMaterial()->SetMaterialColor(color);

			// タイマーが完了したら
			if (self.timer.GetProgress() >= 1.0f) {
				//Up完了
				switch (self.playMode) {
				case PlayMode::LOOP:
				{
					// 再度Upをやり直す
					self.timer.Initialize(duration, 0.0f);
					self.state = AnimationPhase::Up;
					color.w = self.startValueFloat;
					target_->GetMesh()->GetMaterial()->SetMaterialColor(color);
					break;
				}
				case PlayMode::ONCE:
				default:
				{
					// ONCE完了、終了
					self.state = AnimationPhase::None;
					color.w = self.endValueFloat;
					target_->GetMesh()->GetMaterial()->SetMaterialColor(color);
					self.finished = true;
					break;
				}
				case PlayMode::PINGPONG_ONCE:
				case PlayMode::PINGPONG_LOOP:
				{
					//Delayを秒で扱うためtimer.durationをdelayに設定
					self.timer.Initialize(delay, 0.0f);
					self.state = AnimationPhase::Delay;
					break;
				}
				}

			}
			break;
		}
		case AnimationPhase::Delay:
		{
			// Delayはtimer.duration == delay
			if (self.timer.GetProgress() >= 1.0f) {
				// Downに移行duration に戻す）
				self.timer.Initialize(duration, 0.0f);
				self.state = AnimationPhase::Down;
			}
			break;
		}
		case AnimationPhase::Down:
		{
			//補間しつつフェード
			float eased = self.timer.GetEase(self.easeType);
			float alpha = Easing::Lerp(self.endValueFloat, self.startValueFloat, eased);
			Vector4 color = target_->GetMaterialColor();
			color.w = alpha;
			target_->GetMesh()->GetMaterial()->SetMaterialColor(color);
			
			if (self.timer.GetProgress() >= 1.0f) {
				switch (self.playMode) {
				case PlayMode::PINGPONG_ONCE:
					// PINGPONG_ONCE完了、終了
					color.w = self.startValueFloat;
					target_->GetMesh()->GetMaterial()->SetMaterialColor(color);
					self.state = AnimationPhase::None;
					self.finished = true;
					break;
				case PlayMode::LOOP:
				case PlayMode::PINGPONG_LOOP:
					// PINGPONGループならUp再開
					self.timer.Initialize(duration, 0.0f);
					self.state = AnimationPhase::Up;
					color.w = self.startValueFloat;
					target_->GetMesh()->GetMaterial()->SetMaterialColor(color);
					break;
				case PlayMode::ONCE:
				default:
					// ONCE完了、終了
					color.w = self.startValueFloat;
					target_->GetMesh()->GetMaterial()->SetMaterialColor(color);
					self.state = AnimationPhase::None;
					self.finished = true;
					break;
				}
			}
			break;
		}
		default:
			break;
		}
		};
	// ジョブを登録
	animationJobs_.push_back(std::move(job));
}


//=============================================================================
// 更新処理
//=============================================================================
void SpriteAnimator::Update() {

	// 登録されたアニメーション関数を実行
	for (auto& job : animationJobs_) {
		if (job.finished == false && job.updateFunc) {
			job.updateFunc(job);
		}
	}

	// 終了したアニメーションジョブを削除
	animationJobs_.erase(
		std::remove_if(
			animationJobs_.begin(), animationJobs_.end(),
			[](const AnimationJob& job) { return job.finished; }
		),
		animationJobs_.end()
	);
}

//=============================================================================
// アニメーション再生中かどうか
//=============================================================================

bool SpriteAnimator::IsPlaying() const {
	for (const auto& job : animationJobs_) {
		if (!job.finished) {
			return true;
		}
	}
	return false;
}

//=============================================================================
// アニメーションが終了したかどうか
//=============================================================================

bool SpriteAnimator::IsFinished() const {
	for (const auto& job : animationJobs_) {
		if (!job.finished) {
			return false;
		}
	}
	return true;
}