#pragma once
#ifdef DX12_BACKEND
//#include "backends/vk/storageBufferVk.h"
#include "backends/dx12/uniformBufferDx12.h"
#include "drawable.h"
#include "frustum.h"
#include "gameRendererInterface.h"
#include "backends/dx12/driverDx12.h"

namespace IKIGAI::SCENE_SYSTEM {
	class Scene;
}

namespace IKIGAI::ECS {
	class CameraComponent;
}

namespace IKIGAI::CORE {
	class Core;
}

namespace IKIGAI::RENDER {
	struct FrameResource;
	class GBuffer;
	class ShaderDx12;
	class TextureDx12;
	struct LightOGL;

	class GameRendererDx12:  public GameRendererInterface {
	public:
		GameRendererDx12(HINSTANCE hInstance, IKIGAI::CORE::Core& context);
		~GameRendererDx12() override;
		//void createDx12Resources();
		void renderScene() override;

		//DriverDx12* getDriver() { return mDriver; }
	//protected:
		std::shared_ptr<TextureDx12> mEmptyTexture;

		//std::shared_ptr<ShaderStorageBufferVk<LightOGL>> mLightSSBO;
		//std::shared_ptr<UniformBufferVk<LightOGL>> mLightUBO;
		//std::shared_ptr<UniformTexturesDx12> mDeferredTexturesUniform;
		//std::shared_ptr<UniformTexturesDx12> mRenderToScreenTexturesUniform;
		//std::shared_ptr<UniformBufferVk<EngineUBO>> mEngineUbo;

		std::unordered_map<std::string, std::shared_ptr<TextureDx12>> mTextures;
		//std::unordered_map<std::string, std::shared_ptr<FrameBufferDx12>> mFramebuffers;
		std::unordered_map<std::string, std::shared_ptr<ShaderDx12>> mShaders;

		//void drawDeferredGBuffer();

		void sendEngineUBO();
		void renderScene(IKIGAI::UTILS::Ref<IKIGAI::ECS::CameraComponent> mainCameraComponent);

		void begin();
		void end();

		//void drawDrawableDeferred(const Drawable& p_toDraw);

		//void updateLights(SCENE_SYSTEM::Scene& scene);
		//void updateLightsInFrustum(SCENE_SYSTEM::Scene& scene, const Frustum& frustum);

		IKIGAI::CORE::Core& mContext;

		OpaqueDrawables	mOpaqueMeshesForward;
		TransparentDrawables mTransparentMeshesForward;

		OpaqueDrawables	mOpaqueMeshesDeferred;
		TransparentDrawables mTransparentMeshesDeferred;
		DriverDx12* mDriver;
		///
		///

		static GameRendererDx12* GetApp();
		static GameRendererDx12* mApp;

		HINSTANCE AppInst()const;
		HWND MainWnd()const;
		float AspectRatio()const;

		//int Run();

		virtual bool Initialize();
		virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		bool InitMainWindow();
		bool mAppPaused = false;
		bool mMinimized = false;
		bool mMaximized = false;
		bool mResizing = false;
		bool mFullscreenState = false;
		
		HINSTANCE mhAppInst = nullptr;
		HWND mhMainWnd = nullptr;
		int mClientWidth = 800;
		int mClientHeight = 600;


		void Update();
		void Draw();
		void OnResize();

		void OnMouseDown(WPARAM btnState, int x, int y);
		void OnMouseUp(WPARAM btnState, int x, int y);
		void OnMouseMove(WPARAM btnState, int x, int y);

		void OnKeyboardInput();
		void UpdateMainPassCB();
		void LoadTextures();

		void BuildFrameResources();
		void DrawRenderItems(ID3D12GraphicsCommandList* cmdList);
		void DrawFullscreenQuad(ID3D12GraphicsCommandList* cmdList);

		std::vector<std::unique_ptr<FrameResource>> mFrameResources;
		FrameResource* mCurrFrameResource = nullptr;
		//std::unordered_map<std::string, std::shared_ptr<TextureDx12>> mTextures;

		std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

		UINT mSkyTexHeapIndex = 0;

		//Camera mCamera;

		std::unique_ptr<GBuffer> mGBuffer;
		POINT mLastMousePos;
	};
}
#endif

