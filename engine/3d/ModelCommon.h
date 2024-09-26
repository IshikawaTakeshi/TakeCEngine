#pragma once
class DirectXCommon;
class ModelCommon {
public:

	static ModelCommon* GetInstance();

	void Initialize(DirectXCommon* dxCommon);

	void Finalize();

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

private:

	ModelCommon() = default;
	~ModelCommon() = default;
	ModelCommon(const ModelCommon&) = delete;
	ModelCommon& operator=(const ModelCommon&) = delete;

private:

	static ModelCommon* instance_;

	DirectXCommon* dxCommon_ = nullptr;
};

