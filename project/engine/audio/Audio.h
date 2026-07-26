#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <fstream>
#include <string>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "shlwapi.lib")

/// <summary>
/// Audioに関する生成、更新、破棄を一元管理するクラスです。
/// </summary>
class AudioManager {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	//===================================================================================
	///			構造体
	//===================================================================================

	//チャンクヘッダ
	/// <summary>
	/// ChunkHeaderに必要な値をまとめて保持する構造体です。
	/// </summary>
	struct ChunkHeader {
		char id[4]; //チャンクID
		int32_t size; //チャンクサイズ
	};

	//RIFFヘッダチャンク
	/// <summary>
	/// RiffHeaderに必要な値をまとめて保持する構造体です。
	/// </summary>
	struct RiffHeader {
		ChunkHeader chunk; //"RIFF"
		char type[4]; // "WAVE"
	};

	//FMTチャンク
	/// <summary>
	/// FormatChunkに必要な値をまとめて保持する構造体です。
	/// </summary>
	struct FormatChunk {
		ChunkHeader chunk; //"fmt"
		WAVEFORMATEX fmt; //波形フォーマット
	};

	//音声データ
	/// <summary>
	/// SoundDataに関するデータを保持する構造体です。
	/// </summary>
	struct SoundData {

		//波形フォーマット
		WAVEFORMATEX wfex;
		//バッファの先頭アドレス
		BYTE* pBuffer;
		//バッファのサイズ
		unsigned int bufferSize;
		IXAudio2SourceVoice* pSourceVoice;
		// 再生カーソル位置
		uint32_t playCursor = 0; 

		bool isPlaying = false; // 再生中かどうか
	};

public:

	//===================================================================================
	///			publicメンバ関数
	//===================================================================================

	static AudioManager& GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 音声データの読み込み
	/// </summary>
	/// <param name="filename"></param>
	/// <returns></returns>
	SoundData LoadSound(const std::string& filename);

	/// <summary>
	/// 音声データの開放
	/// </summary>
	/// <param name="soundData"></param>
	void SoundUnload(SoundData* soundData);

	/// <summary>
	/// サウンドの再生
	/// </summary>
	/// <param name="xAudio2"></param>
	/// <param name="soundData"></param>
	void SoundPlayWave(SoundData& soundData,float volume, bool isLoop = false);

	//停止処理
	void StopSound(SoundData& soundData);
	//途中から再生
	void ResumeWave(SoundData& soundData);
	//一時停止
	void PauseWave(SoundData& soundData);

	void SetVolume(SoundData& soundData, float volume);

	bool IsPlaying(SoundData& soundData) const;



	/// <summary>
	/// 解放処理
	/// </summary>
	void Finalize();

public:

	//===================================================================================
	///			Getter
	//===================================================================================

	IXAudio2* GetXAudio2() const { return xAudio2_.Get(); }

	IXAudio2MasteringVoice* GetMasteringVoice() const { return masteringVoice_; }

private:

	AudioManager() = default;
	~AudioManager() = default;
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

private:

	ComPtr<IXAudio2> xAudio2_ = nullptr;
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;

private:

	/// 音声データ読み込み(wav)
	SoundData SoundLoadWave(const std::string& filename);
	/// 音声データ読み込み(mp3)
	SoundData LoadMP3File(const std::string& filename);
	// ファイルがWAV形式かどうかを判定
	bool IsWaveFile(const std::string& filename) const;
	// ファイルがMP3形式かどうかを判定
	bool IsMP3File(const std::string& filename) const;
};