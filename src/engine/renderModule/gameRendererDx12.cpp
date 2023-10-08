#include "gameRendererDx12.h"

#include "backends/dx12/materialDx12.h"

#ifdef DX12_BACKEND
#include <DirectXMath.h>
#include "backends/dx12/GBuffer.h"
#include "backends/dx12/GeometryGenerator.h"
#include "backends/dx12/meshDx12.h"
#include "backends/dx12/shaderDx12.h"
#include "backends/dx12/textureDx12.h"

#include "backends/dx12/FrameResource.h"
#include "backends/dx12/UploadBuffer.h"
#include "backends/dx12/d3dUtil.h"
#include <coreModule/core/core.h>
#include <sceneModule/sceneManager.h>
#include <windowModule/window/window.h>
#include <utilsModule/loader.h>
#include <utilsModule/vertex.h>
const int gNumFrameResources = 3;

import glmath;
namespace IKIGAI
{
	namespace INPUT_SYSTEM
	{
		enum class EKey;
	}
}

using namespace IKIGAI;
using namespace IKIGAI::RENDER;

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
std::optional<IKIGAI::Ref<IKIGAI::ECS::CameraComponent>> mainCameraComponentDx = std::nullopt;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return GameRendererDx12::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

GameRendererDx12* GameRendererDx12::mApp = nullptr;
GameRendererDx12* GameRendererDx12::GetApp() {
	return mApp;
}

GameRendererDx12::GameRendererDx12(HINSTANCE hInstance, IKIGAI::CORE_SYSTEM::Core& context): mContext(context), mhAppInst(hInstance){
	assert(mApp == nullptr);
	mApp = this;

	mDriver = dynamic_cast<DriverDx12*>(context.driver.get());
}

GameRendererDx12::~GameRendererDx12() {
	mDriver->deinit();
}


HINSTANCE GameRendererDx12::AppInst()const
{
	return mhAppInst;
}

HWND GameRendererDx12::MainWnd()const
{
	return mhMainWnd;
}

float GameRendererDx12::AspectRatio()const {
	return static_cast<float>(mClientWidth) / mClientHeight;
}

void GameRendererDx12::renderScene() {

	mainCameraComponentDx = std::nullopt;
	if (mContext.sceneManager->hasCurrentScene()) {
		mainCameraComponentDx = mContext.sceneManager->getCurrentScene().findMainCamera();
	}

	if (mainCameraComponentDx) {
		auto [winWidth, winHeight] = mContext.window->getSize();
		const auto& cameraPosition = mainCameraComponentDx.value()->obj->getTransform()->getWorldPosition();
		const auto& cameraRotation = mainCameraComponentDx.value()->obj->getTransform()->getWorldRotation();
		mainCameraComponentDx->getPtr()->getCamera().cacheMatrices(winWidth, winHeight, cameraPosition, cameraRotation);

		//sendEngineUBO();
		renderScene(mainCameraComponentDx.value());
	}

	renderScene(mainCameraComponentDx.value());
}

void GameRendererDx12::renderScene(IKIGAI::Ref<IKIGAI::ECS::CameraComponent> mainCameraComponent) {
	auto [winWidth, winHeight] = mContext.window->getSize();
	auto& currentScene = mContext.sceneManager->getCurrentScene();

	auto& camera = mainCameraComponent->getCamera();
	const auto& cameraPosition = mainCameraComponent->obj->getTransform()->getWorldPosition();
	std::tie(mOpaqueMeshesForward, mTransparentMeshesForward, mOpaqueMeshesDeferred, mTransparentMeshesDeferred) =
		currentScene.findDrawables(cameraPosition, camera, nullptr, nullptr);

	//TODO: move to window ( hwnd move to window too)
	MSG msg = { 0 };
	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//else {
		//mTimer.Tick();
		//if (!mAppPaused) {
			Update();
			Draw();
		//}
		//else {
		//	Sleep(100);
		//}
	//}
}

#include <windowsx.h>
LRESULT GameRendererDx12::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			mAppPaused = true;
			//mTimer.Stop();
		}
		else {
			mAppPaused = false;
			//mTimer.Start();
		}
		return 0;
	case WM_SIZE:
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (mDriver->mDevice) {
			if (wParam == SIZE_MINIMIZED) {
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED) {
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				mDriver->onResize();
			}
			else if (wParam == SIZE_RESTORED) {
				// Restoring from minimized state?
				if (mMinimized) {
					mAppPaused = false;
					mMinimized = false;
					mDriver->onResize();
				}
				// Restoring from maximized state?
				else if (mMaximized) {
					mAppPaused = false;
					mMaximized = false;
					mDriver->onResize();
				}
				else if (mResizing) {
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else {
					mDriver->onResize();
				}
			}
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		//mTimer.Stop();
		return 0;
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		//mTimer.Start();
		mDriver->onResize();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));  //GET_X_LPARAMÕâ¸öºê¶¨ÒåÔÚ windowsx.h Í·ÎÄ¼þÖÐ
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			mDriver->switchMSAA();

		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool GameRendererDx12::InitMainWindow() {
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", L"!!!",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
	if (!mhMainWnd) {
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

std::shared_ptr<ShaderDx12> shader1;
std::shared_ptr<ShaderDx12> shader2;

std::shared_ptr<UploadBuffer<EngineUBO>> engineUBO;
std::shared_ptr<UploadBuffer<MATHGL::Matrix4>> pushModel;


bool GameRendererDx12::Initialize() {
	auto [winWidth, winHeight] = mContext.window->getSize();
	if (!InitMainWindow()) {
		return false;
	}
	mDriver->init();

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mDriver->mCommandList->Reset(
		mDriver->mDirectCmdListAlloc.Get(), nullptr));

	mEmptyTexture = TextureDx12::Create(IKIGAI::UTILS::getRealPath("Textures/snow.png"));


	mTextures["gPositionTex"] = TextureDx12::CreateForAttach(winWidth, winHeight);
	mTextures["gNormalTex"] = TextureDx12::CreateForAttach(winWidth, winHeight);
	mTextures["gAlbedoSpecTex"] = TextureDx12::CreateForAttach(winWidth, winHeight);
	mTextures["gRoughAOTex"] = TextureDx12::CreateForAttach(winWidth, winHeight);
	auto texs = std::vector{ mTextures["gPositionTex"] , mTextures["gNormalTex"] , mTextures["gAlbedoSpecTex"], mTextures["gRoughAOTex"] };
	mGBuffer = std::make_unique<GBuffer>(texs);


	mDriver->mUseDepth = true;
	shader1 = ShaderDx12::Create("Shaders/dx12/defferredGBuffer.hlsl", "Shaders/dx12/defferredGBuffer.hlsl");
	mDriver->mUseDepth = false;
	shader2 = ShaderDx12::Create("Shaders/dx12/defferredLightning.hlsl", "Shaders/dx12/defferredLightning.hlsl");
	mShaders["deferredRender"] = shader1;

	engineUBO = std::make_shared<UploadBuffer<EngineUBO>>(1, true);
	pushModel = std::make_shared<UploadBuffer<MATHGL::Matrix4>>(1, true);
	//LoadTextures();
	BuildFrameResources();

	ThrowIfFailed(mDriver->mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mDriver->mCommandList.Get() };
	mDriver->mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	mDriver->flushCommandQueue();
	return true;
}

//void MyApp::CreateRtvAndDsvDescriptorHeaps()
//{
//	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
//	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//	rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 3;
//	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	rtvHeapDesc.NodeMask = 0;
//	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));
//
//	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
//	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
//	dsvHeapDesc.NumDescriptors = 1;
//	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	dsvHeapDesc.NodeMask = 0;
//	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
//}

void GameRendererDx12::OnResize() {
	mDriver->onResize();

	//mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);


	//if (mGBuffer != nullptr) {
	//	FlushCommandQueue();
	//	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
	//	mGBuffer->OnResize(mClientWidth, mClientHeight);PassConstants
	//}

}


void GameRendererDx12::Update()
{
	OnKeyboardInput();

	mDriver->mCurrFrameResourceIndex = (mDriver->mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mDriver->mCurrFrameResourceIndex].get();


	if (mCurrFrameResource->Fence != 0 && mDriver->mFence->GetCompletedValue() < mCurrFrameResource->Fence) {
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mDriver->mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	//AnimateMaterials(gt);
	//UpdateObjectCBs(gt);

	//auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	////XMMATRIX world = XMLoadFloat4x4(&e->World);
	////XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);
	//ObjectConstants objConstants;
	////XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
	////XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));
	////objConstants.MaterialIndex = e->Mat->MatCBIndex;
	//currObjectCB->CopyData(0, objConstants);
	//
	//
	////UpdateMaterialBuffer(gt);
	//auto currMaterialBuffer = mCurrFrameResource->MaterialBuffer.get();
	//XMMATRIX matTransform = XMLoadFloat4x4(&material->MatTransform);
	//MaterialData matData;
	//matData.DiffuseAlbedo = material->DiffuseAlbedo;
	//matData.FresnelR0 = material->FresnelR0;
	//matData.Roughness = material->Roughness;
	//XMStoreFloat4x4(&matData.MatTransform, XMMatrixTranspose(matTransform));
	//currMaterialBuffer->CopyData(0, matData);
	//
	//
	//UpdateMainPassCB();

	sendEngineUBO();

	/* std::wstring windowText = mMainWndCaption +
		 L"  mclientW: " + std::to_wstring(mGBuffer->Width()) +
	 L"  mclientH: " + std::to_wstring(mGBuffer->Height()) + L"aspect" + std::to_wstring(mGBuffer->Width()/ mGBuffer->Height());

	 SetWindowText(mhMainWnd, windowText.c_str());*/

}

void GameRendererDx12::sendEngineUBO() {
	auto [winWidth, winHeight] = mContext.window->getSize();
	const auto& cameraPosition = mainCameraComponentDx.value()->obj->getTransform()->getWorldPosition();
	EngineUBO data;
	auto proj = mainCameraComponentDx.value()->getCamera().getProjectionMatrix();
	data.Projection = (proj);
	data.View = (mainCameraComponentDx.value()->getCamera().getViewMatrix());
	data.ViewPos = cameraPosition;
	data.ViewportSize = MATHGL::Vector2f(winWidth, winHeight);
	engineUBO->CopyData(0, data);
}

void GameRendererDx12::begin()
{
	mDriver->mCurrFrameResourceIndex = (mDriver->mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mDriver->mCurrFrameResourceIndex].get();
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	ThrowIfFailed(cmdListAlloc->Reset());
	ThrowIfFailed(mDriver->mCommandList->Reset(cmdListAlloc.Get(), shader1->mPSO.Get()));
}

void GameRendererDx12::end()
{
	ThrowIfFailed(mDriver->mCommandList->Close());

	ID3D12CommandList* cmdLists[] = { mDriver->mCommandList.Get() };
	mDriver->mCommandQueue->ExecuteCommandLists(1, cmdLists);

	ThrowIfFailed(mDriver->mSwapChain->Present(0, 0));
	mDriver->mCurrBackBuffer = (mDriver->mCurrBackBuffer + 1) % mDriver->SwapChainBufferCount;

	mCurrFrameResource->Fence = ++mDriver->mCurrentFence;

	mDriver->mCommandQueue->Signal(mDriver->mFence.Get(), mDriver->mCurrentFence);
}

void GameRendererDx12::Draw()
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	ThrowIfFailed(cmdListAlloc->Reset());
	ThrowIfFailed(mDriver->mCommandList->Reset(cmdListAlloc.Get(), shader1->mPSO.Get()));

	mDriver->mCommandList->RSSetViewports(1, &mGBuffer->Viewport());
	mDriver->mCommandList->RSSetScissorRects(1, &mGBuffer->ScissorRect());
	
	mDriver->mCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(mTextures["gPositionTex"]->Resource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	mDriver->mCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(mTextures["gNormalTex"]->Resource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	mDriver->mCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(mTextures["gAlbedoSpecTex"]->Resource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));
	mDriver->mCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(mTextures["gRoughAOTex"]->Resource.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET));


	for (int i = 0; i < 4; i++) {
		mDriver->mCommandList->ClearRenderTargetView(mGBuffer->Rtv(BufferType(i)),
			mGBuffer->clear(), 0, nullptr);
	}

	mDriver->mCommandList->ClearDepthStencilView(mDriver->depthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mDriver->mCommandList->OMSetRenderTargets(4,
		&mGBuffer->Rtv(BufferType(0)),
		true,
		&mDriver->depthStencilView());
	//std::vector<ID3D12DescriptorHeap*> descriptorHeaps;
	//ID3D12DescriptorHeap* descriptorHeaps0[] = { mSrvDescriptorHeap.Get() };
	//mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps0), descriptorHeaps0);
	//for (auto t : mTextures) {
		//descriptorHeaps.push_back(t.second->UploadHeap.Get());
		//auto root_index = shader->binding_to_root_index.at(binding);
	//}
	//mCommandList->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());



	mDriver->mCommandList->SetGraphicsRootSignature(shader1->mRootSignature.Get());

	auto& data = shader1->uniformNameToSlotId;
	
	mDriver->mCommandList->SetGraphicsRootConstantBufferView(data["engine_UBO"], engineUBO->Resource()->GetGPUVirtualAddress());
	//mDriver->mCommandList->SetGraphicsRootConstantBufferView(data["pushModel"], pushModel->Resource()->GetGPUVirtualAddress());

	//auto matCB = mCurrFrameResource->MaterialBuffer->Resource();
	//mDriver->mCommandList->SetGraphicsRootShaderResourceView(data["gMaterialData"], matCB->GetGPUVirtualAddress());
	//
	//ID3D12DescriptorHeap* descriptorHeaps[] = { mTextures["bricksDiffuseMap"]->UploadHeap.Get() };
	//mDriver->mCommandList->SetDescriptorHeaps(1, descriptorHeaps);
	//mDriver->mCommandList->SetGraphicsRootDescriptorTable(data["gTextureMapsAlbedo"], mTextures["bricksDiffuseMap"]->UploadHeap->GetGPUDescriptorHandleForHeapStart());
	//
	//ID3D12DescriptorHeap* descriptorHeaps1[] = { mTextures["bricksNormalMap"]->UploadHeap.Get() };
	//mDriver->mCommandList->SetDescriptorHeaps(1, descriptorHeaps1);
	//mDriver->mCommandList->SetGraphicsRootDescriptorTable(data["gTextureMapsNormal"], mTextures["bricksNormalMap"]->UploadHeap->GetGPUDescriptorHandleForHeapStart());

	DrawRenderItems(mDriver->mCommandList.Get());

	mDriver->mCommandList->OMSetRenderTargets(1,
		&mDriver->currentBackBufferView(),
		true,
		&mDriver->depthStencilView());
	mDriver->mCommandList->SetGraphicsRootSignature(shader2->mRootSignature.Get());
	mDriver->mCommandList->SetPipelineState(shader2->mPSO.Get());

	auto& data2 = shader2->uniformNameToSlotId;

	mDriver->mCommandList->SetGraphicsRootConstantBufferView(data["engineUBO"], engineUBO->Resource()->GetGPUVirtualAddress());

	ID3D12DescriptorHeap* dh0[] = { mTextures["gPositionTex"]->UploadHeap.Get() };
	mDriver->mCommandList->SetDescriptorHeaps(1, dh0);
	mDriver->mCommandList->SetGraphicsRootDescriptorTable(data2["u_PositionMap"], mTextures["gPositionTex"]->UploadHeap->GetGPUDescriptorHandleForHeapStart());

	ID3D12DescriptorHeap* dh1[] = { mTextures["gNormalTex"]->UploadHeap.Get() };
	mDriver->mCommandList->SetDescriptorHeaps(1, dh1);
	mDriver->mCommandList->SetGraphicsRootDescriptorTable(data2["u_NormalMap"], mTextures["gNormalTex"]->UploadHeap->GetGPUDescriptorHandleForHeapStart());

	ID3D12DescriptorHeap* dh2[] = { mTextures["gAlbedoSpecTex"]->UploadHeap.Get() };
	mDriver->mCommandList->SetDescriptorHeaps(1, dh2);
	mDriver->mCommandList->SetGraphicsRootDescriptorTable(data2["u_AlbedoSpecMap"], mTextures["gAlbedoSpecTex"]->UploadHeap->GetGPUDescriptorHandleForHeapStart());

	ID3D12DescriptorHeap* dh3[] = { mTextures["gRoughAOTex"]->UploadHeap.Get() };
	mDriver->mCommandList->SetDescriptorHeaps(1, dh3);
	mDriver->mCommandList->SetGraphicsRootDescriptorTable(data2["u_RoughAO"], mTextures["gRoughAOTex"]->UploadHeap->GetGPUDescriptorHandleForHeapStart());

	DrawFullscreenQuad(mDriver->mCommandList.Get());

	mDriver->mCommandList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(mDriver->currentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(mDriver->mCommandList->Close());

	ID3D12CommandList* cmdLists[] = { mDriver->mCommandList.Get() };
	mDriver->mCommandQueue->ExecuteCommandLists(1, cmdLists);

	ThrowIfFailed(mDriver->mSwapChain->Present(0, 0));
	mDriver->mCurrBackBuffer = (mDriver->mCurrBackBuffer + 1) % mDriver->SwapChainBufferCount;

	mCurrFrameResource->Fence = ++mDriver->mCurrentFence;

	mDriver->mCommandQueue->Signal(mDriver->mFence.Get(), mDriver->mCurrentFence);
}

void GameRendererDx12::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void GameRendererDx12::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void GameRendererDx12::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		//mCamera.Pitch(dy);
		//mCamera.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
#include "../inputManager/inputManager.h"
std::unordered_map<INPUT_SYSTEM::EKey, int> toDx12Key = {
	{INPUT_SYSTEM::EKey::KEY_W, 'W'},
	{INPUT_SYSTEM::EKey::KEY_S, 'S'},
	{INPUT_SYSTEM::EKey::KEY_A, 'A'},
	{INPUT_SYSTEM::EKey::KEY_D, 'D'},
	{INPUT_SYSTEM::EKey::KEY_Z, 'Z'},
};

void GameRendererDx12::OnKeyboardInput() {
	auto& win = RESOURCES::ServiceManager::Get<WINDOW_SYSTEM::Window>();
	for (auto key : toDx12Key) {
		if (GetAsyncKeyState(key.second) & 0x8000) {
			win.keyPressedEvent.run(static_cast<int>(key.first));
		}
		else {
			win.keyReleasedEvent.run(static_cast<int>(key.first));
		}
	}


	//if (GetAsyncKeyState('W') & 0x8000)
	//	mCamera.Walk(10.0f * dt);
	//
	//if (GetAsyncKeyState('S') & 0x8000)
	//	mCamera.Walk(-10.0f * dt);
	//
	//if (GetAsyncKeyState('A') & 0x8000)
	//	mCamera.Strafe(-10.0f * dt);
	//
	//if (GetAsyncKeyState('D') & 0x8000)
	//	mCamera.Strafe(10.0f * dt);

	//mCamera.UpdateViewMatrix();
}

import glmath;

//void GameRendererDx12::UpdateMainPassCB() {
//
//	auto _view = mainCameraComponentDx.value()->getCamera().getViewMatrix();
//	auto _proj = mainCameraComponentDx.value()->getCamera().getProjectionMatrix();
//	auto _pos = mainCameraComponentDx.value()->obj->getTransform()->getWorldPosition();
//	XMMATRIX view = XMMATRIX(_view.data._values);
//	XMMATRIX proj = XMMATRIX(_proj.data._values);
//
//	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
//	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
//	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
//	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
//
//	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
//	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
//	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
//	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
//	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
//	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
//	mMainPassCB.EyePosW = {_pos.x, _pos.y, _pos.z};
//	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
//	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
//	mMainPassCB.NearZ = 1.0f;
//	mMainPassCB.FarZ = 1000.0f;
//	mMainPassCB.TotalTime = 0.0f;//gt.TotalTime();
//	mMainPassCB.DeltaTime = 0.0f;//gt.DeltaTime();
//	mMainPassCB.AmbientLight = { 0.3f, 0.3f, 0.35f, 1.0f };
//	//mMainPassCB.Lights[0].forward = { 0.57735f, -0.57735f, 0.57735f };
//	mMainPassCB.Lights[0].intensity = 0.8;
//	//mMainPassCB.Lights[1].forward = { -0.57735f, -0.57735f, 0.57735f };
//	mMainPassCB.Lights[1].intensity = 0.8;
//	//mMainPassCB.Lights[2].forward = { 0.0f, -0.707f, -0.707f };
//	mMainPassCB.Lights[2].intensity = 0.8;
//
//	auto currPassCB = mCurrFrameResource->PassCB.get();
//	currPassCB->CopyData(0, mMainPassCB);
//}

void GameRendererDx12::LoadTextures()
{
	std::vector<std::string> texNames = {
		"bricksDiffuseMap",
		"bricksNormalMap",
		"tileDiffuseMap",
		"tileNormalMap",
		"defaultDiffuseMap",
		"defaultNormalMap",
		"skyCubeMap"
	};

	std::vector<std::string> texFilenames = {
		"../../Textures/brick_albedo.jpg",
		"../../Textures/brick_normal.jpg",
		"../../Textures/brick_albedo.jpg",
		"../../Textures/brick_normal.jpg",
		"../../Textures/white1x1.png",
		"../../Textures/white1x1.png",
		"../../Textures/brick_albedo.jpg"
	};

	for (int i = 0; i < (int)texNames.size(); ++i) {
		auto texMap = TextureDx12::Create(texFilenames[i]);//std::make_unique<Texture>();
		texMap->Name = texNames[i];
		texMap->Filename = texFilenames[i];
		//ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		//	mCommandList.Get(), texMap->Filename.c_str(),
		//	texMap->Resource, texMap->UploadHeap));

		mTextures[texMap->Name] = texMap;
	}
}

void GameRendererDx12::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>());
	}
}

void GameRendererDx12::DrawRenderItems(ID3D12GraphicsCommandList* cmdList)
{
	auto& data = shader1->uniformNameToSlotId;
	for (const auto& [distance, drawable] : mOpaqueMeshesDeferred) {
		if (drawable.material->getGPUInstances() > 0) {
			reinterpret_cast<MaterialDx12*>(drawable.material.get())->fillUniformsWithShader(mShaders["deferredRender"], mEmptyTexture, true);
			//mShaders["deferredRender"]->setUniform(*mEngineUbo);
			//mShaders["deferredRender"]->setPushConstant(MATHGL::Matrix4::Transpose(p_toDraw.world));

			pushModel->CopyData(0, (drawable.world));
			mDriver->mCommandList->SetGraphicsRootConstantBufferView(data["pushModel"], pushModel->Resource()->GetGPUVirtualAddress());

			auto ri = reinterpret_cast<MeshDx12*>(drawable.mesh.get());

			cmdList->IASetVertexBuffers(0, 1, &std::dynamic_pointer_cast<VertexBufferDx12<Vertex>>(ri->vertexBuffer)->VertexBufferView());
			cmdList->IASetIndexBuffer(&std::dynamic_pointer_cast<IndexBufferDx12>(ri->indexBuffer)->IndexBufferView());
			cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->DrawIndexedInstanced(std::dynamic_pointer_cast<IndexBufferDx12>(ri->indexBuffer)->mSize,
				1, 0, 0, 0);
			//mShaders["deferredRender"]->bindDescriptorSets();
			//mDriver->draw(mShaders["deferredRender"], *p_toDraw.mesh, p_toDraw.material->getGPUInstances());
		}
	}
	//for (const auto& [distance, drawable] : mTransparentMeshesDeferred) {
	//	drawDrawableDeferred(drawable);
	//}

	//auto& data = shader1->uniformNameToSlotId;

	//UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	//auto objectCB = mCurrFrameResource->ObjectCB->Resource();

	//for (size_t i = 0; i < ritems.size(); ++i) {
	//auto ri = mesh;// ritems[i];

	//cmdList->IASetVertexBuffers(0, 1, &std::dynamic_pointer_cast<VertexBufferDx12<Vertex>>(ri->vertexBuffer)->VertexBufferView());
	//cmdList->IASetIndexBuffer(&std::dynamic_pointer_cast<IndexBufferDx12>(ri->indexBuffer)->IndexBufferView());
	//cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//
	//D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress();// +ri->ObjCBIndex * objCBByteSize;
	//
	//cmdList->SetGraphicsRootConstantBufferView(data["cbPerObject"], objCBAddress);

	//cmdList->DrawIndexedInstanced(std::dynamic_pointer_cast<IndexBufferDx12>(ri->indexBuffer)->mSize,
	//	1, 0, 0, 0);
	//}
}

void GameRendererDx12::DrawFullscreenQuad(ID3D12GraphicsCommandList* cmdList) {
	cmdList->IASetVertexBuffers(0, 1, nullptr);
	cmdList->IASetIndexBuffer(nullptr);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	cmdList->DrawInstanced(24, 1, 0, 0);
}


#endif
