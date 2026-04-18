#include "CameraForGPU.h"

//==============================================================================
//　JSON <- CameraConfig
//==============================================================================
void to_json(nlohmann::json& json, const CameraConfig& cameraConfig) {

	json["filePath"] = cameraConfig.filePath;

	json["transform"] = cameraConfig.transform_;
	json["offset"] = cameraConfig.offset_;
	json["offsetDelta"] = cameraConfig.offsetDelta_;
	json["fovX"] = cameraConfig.fovX_;
	json["aspectRatio"] = cameraConfig.aspectRatio_;
	json["nearClip"] = cameraConfig.nearClip_;
	json["farClip"] = cameraConfig.farClip_;
	json["yaw"] = cameraConfig.yaw_;
	json["pitch"] = cameraConfig.pitch_;
}


//============================================================================
//　JSON -> CameraConfig
//============================================================================
void from_json(const nlohmann::json& json, CameraConfig& cameraConfig) {
	if (json.contains("filePath"))json.at("filePath").get_to(cameraConfig.filePath);
	
	if (json.contains("transform"))json.at("transform").get_to(cameraConfig.transform_);
	if (json.contains("offset"))json.at("offset").get_to(cameraConfig.offset_);
	if (json.contains("offsetDelta"))json.at("offsetDelta").get_to(cameraConfig.offsetDelta_);
	if (json.contains("fovX"))json.at("fovX").get_to(cameraConfig.fovX_);
	if (json.contains("aspectRatio"))json.at("aspectRatio").get_to(cameraConfig.aspectRatio_);
	if (json.contains("nearClip"))json.at("nearClip").get_to(cameraConfig.nearClip_);
	if (json.contains("farClip"))json.at("farClip").get_to(cameraConfig.farClip_);
	if (json.contains("yaw"))json.at("yaw").get_to(cameraConfig.yaw_);
	if (json.contains("pitch"))json.at("pitch").get_to(cameraConfig.pitch_);
}