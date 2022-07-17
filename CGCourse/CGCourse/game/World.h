#pragma once

// Use legacy "unsafe" functions
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>



#include "FrustumAABB.h"
#include "Lighting.h"
#include "Renderer.h"
#include "WorldGeneratorType.h"
#include "WorldGenerator.h"
//#include "../engine/window/window.h"
namespace KUMA::WINDOW_SYSTEM
{
	class Window;
}
namespace Game
{
	enum CurrentSunCycle : uint8_t
	{
		Sun_Setting = 0,
		Sun_Rising
	};

	class World
	{
	public:

		World(int seed, const KUMA::MATHGL::Vector2f& window_size, const std::string& world_name, WorldGenerationType world_gen_type, KUMA::ECS::Object* player);
		~World();

		void OnUpdate(KUMA::WINDOW_SYSTEM::Window* window);
		void RenderWorld();
		inline const std::string& GetName() noexcept { return m_WorldName; }

		// Gets a world block from the respective chunk. Returns nullptr if invalid
		std::pair<Block*, Chunk*> GetBlockFromPosition(const KUMA::MATHGL::Vector3& pos) noexcept;
		BlockType GetBlockTypeFromPosition(const KUMA::MATHGL::Vector3& pos) noexcept;
		Chunk* RetrieveChunkFromMap(int cx, int cz) noexcept;
		WorldGenerationType GetWorldGenerationType() { return m_WorldGenType; }

		bool ChunkExistsInMap(int cx, int cz);
		void SetBlockFromPosition(BlockType type, const KUMA::MATHGL::Vector3& pos);


		const std::map<std::pair<int, int>, Chunk>& GetWorldData()
		{
			return m_WorldChunks;
		}

		Chunk* EmplaceChunkInMap(int cx, int cz);
		int GetSeed();
		inline float GetSunPositionY() noexcept  { return m_SunPosition.y; };
		inline CurrentSunCycle GetSunCycleType() noexcept { return m_SunCycle; }
		inline void SetSunCycleType(CurrentSunCycle type) noexcept { m_SunCycle = type; }
		inline void SetSunPositionY(float y) noexcept { m_SunPosition.y = y; };

		uint32_t m_ChunkCount = 0;
		uint32_t p_ChunksRendered = 0;

	private:

		void UnloadFarChunks();
		//void RayCast(bool place);
		void PropogateLight();
		void RemoveLight();
		void UpdateLights();
		void TickSun();
		bool TestRayPlayerCollision(const KUMA::MATHGL::Vector3& ray_block);
		
		std::pair<float, float> m_CrosshairPosition;
		KUMA::ECS::Object* p_Player;
		// Renderers
		Renderer m_Renderer;

		std::map<std::pair<int, int>, Chunk> m_WorldChunks;
		KUMA::MATHGL::Vector3 m_StartRay;
		KUMA::MATHGL::Vector3 m_EndRay;

		//CloudManager m_CloudManager;
		//GLClasses::Texture m_CrosshairTexture;
		long long m_CurrentFrame;
		const int m_WorldSeed;

		// Lighting
		std::queue<LightNode> m_LightBFSQueue;
		std::queue<LightRemovalNode> m_LightRemovalBFSQueue;

		// Day and night cycle
		KUMA::MATHGL::Vector4 m_SunPosition;
		CurrentSunCycle m_SunCycle;

		ViewFrustum m_ViewFrustum;
		//ParticleSystem::ParticleEmitter m_ParticleEmitter;

		const std::string m_WorldName;
		WorldGenerationType m_WorldGenType;
	};
}
