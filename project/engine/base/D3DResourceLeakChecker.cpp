#include "D3DResourceLeakChecker.h"
#include <dxgidebug.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>


//============================================================================
//　デストラクタ
//============================================================================

D3DResourceLeakChecker::~D3DResourceLeakChecker() {
	Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		// DXGI_DEBUG_ALL already covers the application and D3D12 producers.
		// Reporting each producer again only duplicates every live-object entry.
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	}
}
