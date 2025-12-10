#include "../Audio/Audio.h"
#include <cassert>
#include <fstream>
#include <algorithm>
#include <vector>

using SoundData = AudioManager::SoundData;

//================================================================================================
// 初期化処理
//================================================================================================

AudioManager& AudioManager::GetInstance() {
	static AudioManager instance;
	return instance;
}

void AudioManager::Initialize() {

	//XAudio2オブジェクトの生成
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	//マスターボイスの生成
	hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
	assert(SUCCEEDED(hr));
}

SoundData AudioManager::LoadSound(const std::string& filename) {
	std::string fullpath = "Resources/AudioSources/" + filename;
	if (IsWaveFile(fullpath)) {
		return SoundLoadWave(fullpath.c_str());
	} else if (IsMP3File(fullpath)) {
		return LoadMP3File(fullpath.c_str());
	} else {
		assert(false && "対応していない音声形式です");
		return {};
	}
}

//================================================================================================
// 音声データ読み込み
//================================================================================================

SoundData AudioManager::SoundLoadWave(const std::string& filename) {

#pragma region ファイルオープン
	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios::binary);
	//ファイルオープン失敗を検出
	assert(file.is_open());
#pragma endregion

#pragma region .wavデータの読み込み
	//RIFFヘッダの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	//ファイルがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	//FMTチャンクの読み込み
	FormatChunk format = {};
	//チャンクヘッダの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	//チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	//Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	//JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur); //読み取り位置をチャンクの終わりまで進める
		file.read((char*)&data, sizeof(data));     //再読み込み
	}

	//LISTチャンクを検出した場合
	if (strncmp(data.id, "LIST", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	//Dataチャンクかチェック
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	//Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//waveファイルを閉じる
	file.close();

#pragma endregion

#pragma region 読み込んだ音声データをreturn

	SoundData soundData;
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;
	soundData.pSourceVoice = nullptr; // 初期状態ではVoiceは未設定

#pragma endregion

	return soundData;
}

///-------------------------------------------/// 
/// 音声データの読み込み(MP3)
///-------------------------------------------///
SoundData AudioManager::LoadMP3File(const std::string& filename) {

	/// ===ファイルオープン=== ///
	// ファイル名を wide 文字列に変換
	int wideSize = MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, nullptr, 0);
	std::wstring wideFilename(wideSize, 0);
	MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, &wideFilename[0], wideSize);

	// Media Foundation の初期化
	HRESULT result = MFStartup(MF_VERSION);
	assert(SUCCEEDED(result));

	// SourceReader を作成
	IMFSourceReader* sourceReader = nullptr;
	result = MFCreateSourceReaderFromURL(wideFilename.c_str(), nullptr, &sourceReader);
	assert(SUCCEEDED(result));

	/// ===音声データ読み込み=== ///
	// 出力形式を PCM 形式に設定
	IMFMediaType* audioType = nullptr;
	result = MFCreateMediaType(&audioType);
	assert(SUCCEEDED(result));
	result = audioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	assert(SUCCEEDED(result));
	result = audioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	assert(SUCCEEDED(result));
	result = sourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, audioType);
	assert(SUCCEEDED(result));

	// PCM 形式のフォーマットを取得
	IMFMediaType* outputType = nullptr;
	result = sourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outputType);
	assert(SUCCEEDED(result));

	WAVEFORMATEX* pWaveFormat = nullptr;
	UINT32 cbFormat = 0;
	result = MFCreateWaveFormatExFromMFMediaType(outputType, &pWaveFormat, &cbFormat);
	assert(SUCCEEDED(result));

	// フォーマットを保存
	FormatChunk format = {};
	memcpy(&format.fmt, pWaveFormat, sizeof(WAVEFORMATEX));
	CoTaskMemFree(pWaveFormat);
	outputType->Release();

	/// ===音声データをバッファに読み込み=== ///
	ChunkHeader data = {};
	std::vector<BYTE> buffer;

	while (true) {
		IMFSample* sample = nullptr;
		DWORD flags = 0;
		result = sourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, &sample);
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break; // データの終端に到達
		}
		assert(SUCCEEDED(result));

		if (sample) {
			IMFMediaBuffer* mediaBuffer = nullptr;
			result = sample->ConvertToContiguousBuffer(&mediaBuffer);
			assert(SUCCEEDED(result));

			BYTE* audioData = nullptr;
			DWORD audioDataLength = 0;
			result = mediaBuffer->Lock(&audioData, nullptr, &audioDataLength);
			assert(SUCCEEDED(result));

			// データをバッファに追加
			buffer.insert(buffer.end(), audioData, audioData + audioDataLength);

			result = mediaBuffer->Unlock();
			assert(SUCCEEDED(result));
			mediaBuffer->Release();
			sample->Release();
		}
	}

	// Data チャンクのデータ部をセット
	data.size = static_cast<uint32_t>(buffer.size());
	char* pBuffer = new char[data.size];
	memcpy(pBuffer, buffer.data(), data.size);

	// リソース解放
	sourceReader->Release();
	audioType->Release();
	MFShutdown();

	/// ====読み込んだ音声データを return ==== ///
	SoundData soundData = {};
	soundData.wfex = format.fmt; // フォーマットを設定
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}
//================================================================================================
// 音声データの開放
//================================================================================================

void AudioManager::SoundUnload(SoundData* soundData) {
	// 再生中のボイスを停止して破棄
	if (soundData->pSourceVoice) {
		soundData->pSourceVoice->Stop();
		soundData->pSourceVoice->DestroyVoice();
		soundData->pSourceVoice = nullptr;
		soundData->isPlaying = false; // 再生中フラグを下ろす
	}

	//バッファの解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

//================================================================================================
// 音声再生
//================================================================================================

void AudioManager::SoundPlayWave(SoundData& soundData, float volume, bool isLoop) {

	HRESULT result;

	// すでに再生中なら一旦止める
	if (soundData.pSourceVoice) {
		soundData.pSourceVoice->Stop();
		soundData.pSourceVoice->DestroyVoice();
		soundData.pSourceVoice = nullptr;
		soundData.isPlaying = false;
	}

	//波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	soundData.pSourceVoice = pSourceVoice;
	//音量の設定
	pSourceVoice->SetVolume(volume);

	//再生する波形データの設定
	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = soundData.pBuffer;
	buffer.AudioBytes = soundData.bufferSize;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	//ループ再生の設定
	if (isLoop) {
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE; // 無限ループ
	} else {
		buffer.LoopCount = 0; // ループなし
	}

	//波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buffer);
	result = pSourceVoice->Start();

	soundData.isPlaying = true; // 再生中フラグを立てる
}

void AudioManager::StopSound(SoundData& soundData) {
	if (soundData.pSourceVoice) {
		//再生位置の取得
		XAUDIO2_VOICE_STATE state{};
		soundData.pSourceVoice->GetState(&state);

		// 再生位置（バイト） -> サンプル数（playBeginに使う）
		soundData.playCursor = static_cast<uint32_t>(
			state.SamplesPlayed % (soundData.bufferSize / soundData.wfex.nBlockAlign)
			);

		soundData.pSourceVoice->Stop();
		soundData.pSourceVoice->DestroyVoice();
		soundData.pSourceVoice = nullptr;
		soundData.isPlaying = false; // 再生中フラグを下ろす
	}
}

void AudioManager::ResumeWave(SoundData& soundData) {
	if (soundData.pSourceVoice != nullptr) return; // すでに再生中なら無視

	if (soundData.pSourceVoice) {
		soundData.pSourceVoice->Start(); // 再生を再開
		soundData.isPlaying = true; // 再生中フラグを立てる
	}
}

void AudioManager::PauseWave(SoundData& soundData) {
	if (soundData.pSourceVoice) {
		// 再生位置の取得
		XAUDIO2_VOICE_STATE state{};
		soundData.pSourceVoice->GetState(&state);

		// 再生位置（バイト） -> サンプル数（playCursorに保存）
		soundData.playCursor = static_cast<uint32_t>(
			state.SamplesPlayed % (soundData.bufferSize / soundData.wfex.nBlockAlign)
			);

		// 音声を一時停止
		soundData.pSourceVoice->Stop();
		// 再生中フラグを下ろす
		soundData.isPlaying = false;
	}
}

void AudioManager::SetVolume(SoundData& soundData, float volume) {
	if (soundData.pSourceVoice) {
		soundData.pSourceVoice->SetVolume(volume);
	}
}
bool AudioManager::IsPlaying(SoundData& soundData) const {
	if (!soundData.pSourceVoice) return false;

	XAUDIO2_VOICE_STATE state{};
	soundData.pSourceVoice->GetState(&state);

	soundData.isPlaying = (state.BuffersQueued > 0);

	return soundData.isPlaying;
}
bool AudioManager::IsWaveFile(const std::string& filename) const {
	std::string ext = filename.substr(filename.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
	return ext == "wav";
}
bool AudioManager::IsMP3File(const std::string& filename) const {
	std::string ext = filename.substr(filename.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(::tolower(c)); });
	return ext == "mp3";
}
//================================================================================================
// 解放処理
//================================================================================================

void AudioManager::Finalize() {
	//XAudio2の解放
	xAudio2_.Reset();
	MFShutdown();
}