#include "PrefabTests.h"
#include "LevelTests.h"
#include "examples/PrefabExample.h"
#include "engine/base/TakeCFrameWork.h"

float TakeC::TakeCFrameWork::deltaTime = 0.9f;
TakeC::JsonLoader* TakeC::TakeCFrameWork::GetJsonLoader() { return nullptr; }

int main() {
    RunPrefabTests();
    RunLevelTests();
    return 0;
}
