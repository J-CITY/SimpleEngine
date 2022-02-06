#include "World.h"
#include "../engine/ecs/components/cameraComponent.h"
#include "../engine/ecs/object.h"
namespace Game
{
	// The amount of chunks that gets rendered around the player
	const int render_distance = 6;
	const int render_distance_x = render_distance, render_distance_z = render_distance;

	constexpr float max_sun = 1500.0f;
	constexpr float min_sun = 10.0f;
	constexpr uint8_t LAMP_LIGHT_LEVEL = 32;

	/*
		Prints a 3 component vector on the screen
	*/

	static void PrintVec3(const KUMA::MATHGL::Vector3& val)
	{
		std::cout << std::endl << "X : " << val.x << " Y : " << val.y << " Z : " << val.z;
		return;
	}

	/*
		Tests the collision between a block and a position.
	*/
	bool TestAABB3DCollision(const KUMA::MATHGL::Vector3& pos_1, const KUMA::MATHGL::Vector3& dim_1, const KUMA::MATHGL::Vector3& pos_2, const KUMA::MATHGL::Vector3& dim_2)
	{
		if (pos_1.x < pos_2.x + dim_2.x &&
			pos_1.x + dim_1.x > pos_2.x &&
			pos_1.y < pos_2.y + dim_2.y &&
			pos_1.y + dim_1.y > pos_2.y &&
			pos_1.z < pos_2.z + dim_2.z &&
			pos_1.z + dim_1.z > pos_2.z)
		{
			return true;
		}

		return false;
	}

	/*
		Converts world position to chunk block position
	*/

	static KUMA::MATHGL::Vector3 WorldBlockToLocalBlockCoordinates(const KUMA::MATHGL::Vector3& pos)
	{
		int block_chunk_x = static_cast<int>(floor(pos.x / CHUNK_SIZE_X));
		int block_chunk_z = static_cast<int>(floor(pos.z / CHUNK_SIZE_Z));
		int bx = pos.x - (block_chunk_x * CHUNK_SIZE_X);
		int by = static_cast<int>(floor(pos.y));
		int bz = pos.z - (block_chunk_z * CHUNK_SIZE_Z);

		return KUMA::MATHGL::Vector3(bx, by, bz);
	}


	/*
		World class constructor
	*/

	World::World(int seed, const KUMA::MATHGL::Vector2& window_size, const std::string& world_name, WorldGenerationType world_gen_type, KUMA::ECS::Object* player)
		: m_WorldSeed(seed), m_WorldName(world_name), m_WorldGenType(world_gen_type), p_Player(player)
	{
		m_SunCycle = CurrentSunCycle::Sun_Rising;
		m_SunPosition = KUMA::MATHGL::Vector4(0.0f, max_sun, 0.0f, 1.0f);

		// Generate all the chunks 

		
		// Set the players position
		//p_Player->p_Position = KUMA::MATHGL::Vector3(0, 140, 0);
		
		
		//m_CrosshairTexture.CreateTexture("Resources/crosshair.png");

		float cw = floor(static_cast<float>(window_size.x) / static_cast <float>(2.0f));
		float cy = floor(static_cast<float>(window_size.y) / static_cast<float>(2.0f));

		m_CrosshairPosition = std::pair<float, float>(cw, cy);
		m_CurrentFrame = 0;

	}

	World::~World()
	{

	}


	/*
		The World::OnUpdate function is called every frame, it updates the blocks, lighting, player etc..
	*/
	void World::OnUpdate(KUMA::WINDOW_SYSTEM::Window* window)
	{
		int player_chunk_x = (int)floor(p_Player->transform->getWorldPosition().x / CHUNK_SIZE_X);
		int player_chunk_z = (int)floor(p_Player->transform->getWorldPosition().z / CHUNK_SIZE_Z);

		std::stringstream str;

		int build_distance_x = render_distance_x + 4;
		int build_distance_z = render_distance_z + 4;

		/*
		build_distance_x and build_distance_z is the build distance. It is needed to build `x` chunks
		more than what is required in order for the meshing process to work properly

		For this it is required to first generate all the chunks that are needed to be generated.
		Only after all the chunks are generated, the mesh building process with start
		*/

		for (int i = player_chunk_x - build_distance_x; i < player_chunk_x + build_distance_x; i++)
		{
			for (int j = player_chunk_z - build_distance_z; j < player_chunk_z + build_distance_z; j++)
			{
				if (ChunkExistsInMap(i, j) == false)
				{
					Chunk* chunk = EmplaceChunkInMap(i, j);
					GenerateChunk(chunk, m_WorldSeed, m_WorldGenType);
				}
			}
		}

		int flora_build_distance_x = build_distance_x - 1;
		int flora_build_distance_z = build_distance_z - 1;

		for (int i = player_chunk_x - flora_build_distance_x; i < player_chunk_x + flora_build_distance_x; i++)
		{
			for (int j = player_chunk_z - flora_build_distance_z; j < player_chunk_z + flora_build_distance_z; j++)
			{
				Chunk* chunk = RetrieveChunkFromMap(i, j);

				if (chunk->p_ChunkState == ChunkState::UNGENERETED)
				{
					GenerateChunkFlora(chunk, m_WorldSeed, m_WorldGenType);
					chunk->p_MeshState = ChunkMeshState::UNBUILT;
					chunk->p_ChunkState = ChunkState::GENERETED;
				}

			}
		}

		//p_Player->OnUpdate(window);

		// Update the view frustum
		m_ViewFrustum.Update( 
			p_Player->getComponent<KUMA::ECS::CameraComponent>()->getCamera().getProjectionMatrix() * 
			p_Player->getComponent<KUMA::ECS::CameraComponent>()->getCamera().getViewMatrix());

		// Increase the frame count 
		m_CurrentFrame++;

		// Clean up the particles every x frames
		//if (m_CurrentFrame % 300 == 0)
		//{
		//	m_ParticleEmitter.CleanUpList();
		//}

	}

	/*
		World::Render world is called to render the world.
		Called every frame
	*/
	void World::RenderWorld()
	{
		//std::cout << "X: " << p_Player->transform->getWorldPosition().x << " Y: " << p_Player->transform->getWorldPosition().y << " Z: " << p_Player->transform->getWorldPosition().z << std::endl;
		static float ambient = 0.4f;
		int player_chunk_x = 0;
		int player_chunk_z = 0;

		glDisable(GL_CULL_FACE);

		// Enable face culling and depth testing
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glFrontFace(GL_CCW);

		// Determine the player's current chunk
		player_chunk_x = (int)floor(p_Player->transform->getWorldPosition().x / CHUNK_SIZE_X);
		player_chunk_z = (int)floor(p_Player->transform->getWorldPosition().z / CHUNK_SIZE_Z);
		uint32_t chunks_rendered = 0;

		// Render chunks according to render distance

		m_Renderer.StartChunkRendering(p_Player->getComponent<KUMA::ECS::CameraComponent>().get(), KUMA::MATHGL::Vector4(ambient, ambient, ambient, 1.0f), render_distance, m_SunPosition);

		for (int i = player_chunk_x - render_distance_x; i < player_chunk_x + render_distance_x; i++)
		{
			for (int j = player_chunk_z - render_distance_z; j < player_chunk_z + render_distance_z; j++)
			{
				Chunk* chunk = RetrieveChunkFromMap(i, j);

				if (m_ViewFrustum.BoxInFrustum(chunk->p_ChunkFrustumAABB))
				{
					// Construct the chunks if the mesh isn't built
					if (chunk->p_MeshState == ChunkMeshState::UNBUILT)
					{
						chunk->Construct();
					}

					if (chunk->p_MeshState == ChunkMeshState::BUILT)
					{
						m_Renderer.RenderChunk(chunk);

						// Render the chunks
						chunks_rendered++;
					}
				}
			}
		}

		p_ChunksRendered = chunks_rendered;

		glDisable(GL_CULL_FACE);

		for (int i = player_chunk_x - render_distance_x; i < player_chunk_x + render_distance_x; i++)
		{
			for (int j = player_chunk_z - render_distance_z; j < player_chunk_z + render_distance_z; j++)
			{
				Chunk* chunk = RetrieveChunkFromMap(i, j);

				if (m_ViewFrustum.BoxInFrustum(chunk->p_ChunkFrustumAABB))
				{
					m_Renderer.RenderTransparentChunk(chunk);
				}
			}
		}

		m_Renderer.EndChunkRendering();

		glDisable(GL_CULL_FACE);

		m_Renderer.StartChunkModelRendering(p_Player->getComponent<KUMA::ECS::CameraComponent>().get(), KUMA::MATHGL::Vector4(ambient, ambient, ambient, 1.0f), render_distance, m_SunPosition);

		for (int i = player_chunk_x - render_distance_x; i < player_chunk_x + render_distance_x; i++)
		{
			for (int j = player_chunk_z - render_distance_z; j < player_chunk_z + render_distance_z; j++)
			{
				Chunk* chunk = RetrieveChunkFromMap(i, j);

				if (m_ViewFrustum.BoxInFrustum(chunk->p_ChunkFrustumAABB))
				{
					m_Renderer.RenderChunkModels(chunk);
				}
			}
		}

		m_Renderer.EndChunkModelRendering();

		glDisable(GL_DEPTH_TEST);

		//m_Renderer2D.RenderQuad(KUMA::MATHGL::Vector3(m_CrosshairPosition.first - (m_CrosshairTexture.GetWidth() / 2)
		//	, m_CrosshairPosition.second - (m_CrosshairTexture.GetHeight() / 2), 1.0f)
		//	, &m_CrosshairTexture, &m_Camera2D);

		// Tick the sun every x frames 
		if (m_CurrentFrame % 32 == 0)
		{
			TickSun();
		}

		//m_ParticleEmitter.OnUpdateAndRender(&p_Player->p_Camera, m_Renderer.GetAtlasTexture());

		/* TEMPORARY */
		//m_CubeRenderer.RenderCube(KUMA::MATHGL::Vector3(0, 135, 0), &m_CrosshairTexture, 0, p_Player->p_Camera.GetViewProjection(), glm::mat4(1.0f), nullptr);
	}

	void World::TickSun()
	{

		if (m_SunCycle == CurrentSunCycle::Sun_Rising)
		{
			if (m_SunPosition.y >= max_sun)
			{
				m_SunPosition.y = max_sun;
				m_SunCycle = CurrentSunCycle::Sun_Setting;

				return;
			}

			m_SunPosition.y += 1;
		}

		else if (m_SunCycle == CurrentSunCycle::Sun_Setting)
		{
			if (m_SunPosition.y <= min_sun)
			{
				m_SunPosition.y = min_sun;
				m_SunCycle = CurrentSunCycle::Sun_Rising;

				return;
			}

			m_SunPosition.y -= 1;
		}
	}
	std::pair<Block*, Chunk*> World::GetBlockFromPosition(const KUMA::MATHGL::Vector3& pos) noexcept
	{
		int block_chunk_x = static_cast<int>(floor(pos.x / CHUNK_SIZE_X));
		int block_chunk_z = static_cast<int>(floor(pos.z / CHUNK_SIZE_Z));
		int bx = pos.x - (block_chunk_x * CHUNK_SIZE_X);
		int by = static_cast<int>(floor(pos.y));
		int bz = pos.z - (block_chunk_z * CHUNK_SIZE_Z);

		Chunk* chunk = RetrieveChunkFromMap(block_chunk_x, block_chunk_z);

		return { &chunk->p_ChunkContents.at(bx).at(by).at(bz), chunk };
	}

	/*
		Sets a world block to the type parameter
	*/
	void World::SetBlockFromPosition(BlockType type, const KUMA::MATHGL::Vector3& pos)
	{
		int block_chunk_x = static_cast<int>(floor(pos.x / CHUNK_SIZE_X));
		int block_chunk_z = static_cast<int>(floor(pos.z / CHUNK_SIZE_Z));
		int bx = pos.x - (block_chunk_x * CHUNK_SIZE_X);
		int by = static_cast<int>(floor(pos.y));
		int bz = pos.z - (block_chunk_z * CHUNK_SIZE_Z);

		RetrieveChunkFromMap(block_chunk_x, block_chunk_z)->SetBlock(type, KUMA::MATHGL::Vector3(bx, by, bz));
	}

	/*
		Returns the type of the block at a particular position
	*/
	BlockType World::GetBlockTypeFromPosition(const KUMA::MATHGL::Vector3& pos) noexcept
	{
		int block_chunk_x = static_cast<int>(floor(pos.x / CHUNK_SIZE_X));
		int block_chunk_z = static_cast<int>(floor(pos.z / CHUNK_SIZE_Z));
		int bx = pos.x - (block_chunk_x * CHUNK_SIZE_X);
		int by = static_cast<int>(floor(pos.y));
		int bz = pos.z - (block_chunk_z * CHUNK_SIZE_Z);

		return static_cast<BlockType>(RetrieveChunkFromMap(block_chunk_x, block_chunk_z)->p_ChunkContents.at(bx).at(by).at(bz).p_BlockType);
	}

	/*
		Unloads the chunks when it is too faraway.
		Called every 200~ frames
		TODO
	*/
	void World::UnloadFarChunks()
	{


		return;
	}

	/*
		Tests whether the placed block intersects with the player
		Used to make sure that the player doesnt get stuck in a block that the player places
	*/
	bool World::TestRayPlayerCollision(const KUMA::MATHGL::Vector3& ray_block)
	{
		KUMA::MATHGL::Vector3 pos = KUMA::MATHGL::Vector3(
			p_Player->transform->getWorldPosition().x,
			p_Player->transform->getWorldPosition().y,
			p_Player->transform->getWorldPosition().z);

		if (TestAABB3DCollision(pos, KUMA::MATHGL::Vector3(0.75f, 1.5f, 0.75f), ray_block, KUMA::MATHGL::Vector3(1.0f, 1.0f, 1.0f)))
		{
			return true;
		}

		return false;
	}

	void World::PropogateLight()
	{
		while (!m_LightBFSQueue.empty())
		{
			LightNode& node = m_LightBFSQueue.front();
			KUMA::MATHGL::Vector3 pos = node.p_Position;
			Chunk* chunk = node.p_Chunk;

			// Pop the element after storing it's data
			m_LightBFSQueue.pop();

			int light_level = 0;

			if (pos.x >= 0 && pos.x < CHUNK_SIZE_X &&
				pos.z >= 0 && pos.z < CHUNK_SIZE_Z &&
				pos.y >= 0 && pos.y < CHUNK_SIZE_Y)
			{
				light_level = chunk->GetTorchLightAt(pos.x, pos.y, pos.z);
			}

			int x = floor(pos.x);
			int y = floor(pos.y);
			int z = floor(pos.z);

			KUMA::MATHGL::Vector3 chunk_pos = chunk->p_Position;
			Chunk* front_chunk = RetrieveChunkFromMap(chunk_pos.x, chunk_pos.z + 1);
			Chunk* back_chunk = RetrieveChunkFromMap(chunk_pos.x, chunk_pos.z - 1);
			Chunk* right_chunk = RetrieveChunkFromMap(chunk_pos.x + 1, chunk_pos.z);
			Chunk* left_chunk = RetrieveChunkFromMap(chunk_pos.x - 1, chunk_pos.z);

			// For lighting on chunk corners

			if (x > 0)
			{
				if (chunk->GetBlock(x - 1, y, z)->IsLightPropogatable() && chunk->GetTorchLightAt(x - 1, y, z) + 2 <= light_level)
				{
					chunk->SetTorchLightAt(x - 1, y, z, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(x - 1, y, z), chunk });
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			else if (x <= 0)
			{
				if (left_chunk->GetBlock(CHUNK_SIZE_X - 1, y, z)->IsLightPropogatable() && left_chunk->GetTorchLightAt(CHUNK_SIZE_X - 1, y, z) + 2 <= light_level)
				{
					left_chunk->SetTorchLightAt(CHUNK_SIZE_X - 1, y, z, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(CHUNK_SIZE_X - 1, y, z), left_chunk });
				}

				left_chunk->p_MeshState = ChunkMeshState::BUILT;
			}

			if (x < CHUNK_SIZE_X - 1)
			{
				if (chunk->GetBlock(x + 1, y, z)->IsLightPropogatable() && chunk->GetTorchLightAt(x + 1, y, z) + 2 <= light_level)
				{
					chunk->SetTorchLightAt(x + 1, y, z, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(x + 1, y, z), chunk });
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			else if (x >= CHUNK_SIZE_X - 1)
			{
				if (right_chunk->GetBlock(0, y, z)->IsLightPropogatable() && right_chunk->GetTorchLightAt(0, y, z) + 2 <= light_level)
				{
					right_chunk->SetTorchLightAt(0, y, z, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(0, y, z), right_chunk });
				}

				right_chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (y > 0)
			{
				if (chunk->GetBlock(x, y - 1, z)->IsLightPropogatable() && chunk->GetTorchLightAt(x, y - 1, z) + 2 <= light_level)
				{
					chunk->SetTorchLightAt(x, y - 1, z, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(x, y - 1, z), chunk });
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (y < CHUNK_SIZE_Y - 1)
			{
				if (chunk->GetBlock(x, y + 1, z)->IsLightPropogatable() && chunk->GetTorchLightAt(x, y + 1, z) + 2 <= light_level)
				{
					chunk->SetTorchLightAt(x, y + 1, z, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(x, y + 1, z), chunk });
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (z > 0)
			{
				if (chunk->GetBlock(x, y, z - 1)->IsLightPropogatable() && chunk->GetTorchLightAt(x, y, z - 1) + 2 <= light_level)
				{
					chunk->SetTorchLightAt(x, y, z - 1, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(x, y, z - 1), chunk });
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			else if (z <= 0)
			{
				if (back_chunk->GetBlock(x, y, CHUNK_SIZE_Z - 1)->IsLightPropogatable() && back_chunk->GetTorchLightAt(x, y, CHUNK_SIZE_Z - 1) + 2 <= light_level)
				{
					back_chunk->SetTorchLightAt(x, y, CHUNK_SIZE_Z - 1, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(x, y, CHUNK_SIZE_Z - 1), back_chunk });
				}

				back_chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (z < CHUNK_SIZE_Z - 1)
			{
				if (chunk->GetBlock(x, y, z + 1)->IsLightPropogatable() && chunk->GetTorchLightAt(x, y, z + 1) + 2 <= light_level)
				{
					chunk->SetTorchLightAt(x, y, z + 1, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(x, y, z + 1), chunk });
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			else if (z >= CHUNK_SIZE_Z - 1)
			{
				if (front_chunk->GetBlock(x, y, 0)->IsLightPropogatable() && front_chunk->GetTorchLightAt(x, y, 0) + 2 <= light_level)
				{
					front_chunk->SetTorchLightAt(x, y, 0, light_level - 1);
					m_LightBFSQueue.push({ KUMA::MATHGL::Vector3(x, y, 0), front_chunk });
				}

				front_chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}
		}
	}

	/*
	Goes through the bfs queue and removes the required lights at the required chunks
	*/
	void World::RemoveLight()
	{
		// Light removal bfs queue

		while (m_LightRemovalBFSQueue.empty() == false)
		{
			LightRemovalNode& node = m_LightRemovalBFSQueue.front();
			int x = node.p_Position.x;
			int y = node.p_Position.y;
			int z = node.p_Position.z;
			int light_level = (int)node.p_LightValue;
			KUMA::MATHGL::Vector3 chunk_pos = node.p_Chunk->p_Position;
			Chunk* chunk = node.p_Chunk;

			Chunk* front_chunk = RetrieveChunkFromMap(chunk_pos.x, chunk_pos.z + 1);
			Chunk* back_chunk = RetrieveChunkFromMap(chunk_pos.x, chunk_pos.z - 1);
			Chunk* right_chunk = RetrieveChunkFromMap(chunk_pos.x + 1, chunk_pos.z);
			Chunk* left_chunk = RetrieveChunkFromMap(chunk_pos.x - 1, chunk_pos.z);

			// Pop the front element
			m_LightRemovalBFSQueue.pop();

			if (x > 0)
			{
				int neighbor_level = chunk->GetTorchLightAt(x - 1, y, z);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					chunk->SetTorchLightAt(x - 1, y, z, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(x - 1, y, z), neighbor_level, chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(x - 1, y, z), chunk);
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			else if (x == 0)
			{
				int neighbor_level = left_chunk->GetTorchLightAt(CHUNK_SIZE_X - 1, y, z);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					left_chunk->SetTorchLightAt(CHUNK_SIZE_X - 1, y, z, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(CHUNK_SIZE_X - 1, y, z), neighbor_level, left_chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(CHUNK_SIZE_X - 1, y, z), left_chunk);
				}

				left_chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (x < CHUNK_SIZE_X - 1)
			{
				int neighbor_level = chunk->GetTorchLightAt(x + 1, y, z);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					chunk->SetTorchLightAt(x + 1, y, z, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(x + 1, y, z), neighbor_level, chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(x + 1, y, z), chunk);
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			else if (x == CHUNK_SIZE_X - 1)
			{
				int neighbor_level = right_chunk->GetTorchLightAt(0, y, z);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					right_chunk->SetTorchLightAt(0, y, z, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(0, y, z), neighbor_level, right_chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(0, y, z), right_chunk);
				}

				right_chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (y > 0)
			{
				int neighbor_level = chunk->GetTorchLightAt(x, y - 1, z);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					chunk->SetTorchLightAt(x, y - 1, z, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y - 1, z), neighbor_level, chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y - 1, z), chunk);
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (y < CHUNK_SIZE_Y - 1)
			{
				int neighbor_level = chunk->GetTorchLightAt(x, y + 1, z);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					chunk->SetTorchLightAt(x, y + 1, z, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y + 1, z), neighbor_level, chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y + 1, z), chunk);
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (z > 0)
			{
				int neighbor_level = chunk->GetTorchLightAt(x, y, z - 1);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					chunk->SetTorchLightAt(x, y, z - 1, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y, z - 1), neighbor_level, chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y, z - 1), chunk);
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			else if (z == 0)
			{
				int neighbor_level = back_chunk->GetTorchLightAt(x, y, CHUNK_SIZE_Z - 1);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					back_chunk->SetTorchLightAt(x, y, CHUNK_SIZE_Z - 1, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y, CHUNK_SIZE_Z - 1), neighbor_level, back_chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y, CHUNK_SIZE_Z - 1), back_chunk);
				}

				back_chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			if (z < CHUNK_SIZE_Z - 1)
			{
				int neighbor_level = chunk->GetTorchLightAt(x, y, z + 1);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					chunk->SetTorchLightAt(x, y, z + 1, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y, z + 1), neighbor_level, chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y, z + 1), chunk);
				}

				chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}

			else if (z == CHUNK_SIZE_Z - 1)
			{
				int neighbor_level = front_chunk->GetTorchLightAt(x, y, 0);

				if (neighbor_level != 0 && neighbor_level < light_level)
				{
					front_chunk->SetTorchLightAt(x, y, 0, 0);
					m_LightRemovalBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y, 0), neighbor_level, front_chunk);
				}

				else if (neighbor_level >= light_level)
				{
					m_LightBFSQueue.emplace(KUMA::MATHGL::Vector3(x, y, 0), front_chunk);
				}

				front_chunk->p_MeshState = ChunkMeshState::UNBUILT;
			}
		}
	}

	/*
		Updates all the chunks
	*/
	void World::UpdateLights()
	{
		PropogateLight();
		RemoveLight();
		PropogateLight();
	}

	/*
		Checks if the chunk exists in the chunk map
	*/
	bool World::ChunkExistsInMap(int cx, int cz)
	{
		std::map<std::pair<int, int>, Chunk>::iterator chunk_exists = m_WorldChunks.find(std::pair<int, int>(cx, cz));

		if (chunk_exists == m_WorldChunks.end())
		{
			return false;
		}

		return true;
	}

	/*
		Gets a chunk if it exists in the chunk map
	*/
	Chunk* World::RetrieveChunkFromMap(int cx, int cz) noexcept
	{
		auto chk = m_WorldChunks.find(std::pair<int, int>(cx, cz));

		if (chk == m_WorldChunks.end())
		{
			return nullptr;
		}

		Chunk* ret_val = &m_WorldChunks.at(std::pair<int, int>(cx, cz));
		return ret_val;
	}

	/*
		Emplaces a chunk in the chunk map
	*/
	Chunk* World::EmplaceChunkInMap(int cx, int cz)
	{
		std::stringstream str;

		str << "Chunk Building ! X : " << cx << " | Z : " << cz;

		std::map<std::pair<int, int>, Chunk>::iterator chunk_exists = m_WorldChunks.find(std::pair<int, int>(cx, cz));

		if (chunk_exists == m_WorldChunks.end())
		{
			//Timer timer(str.str());

			m_WorldChunks.emplace(std::pair<int, int>(cx, cz), KUMA::MATHGL::Vector3(cx, 0, cz));
			m_ChunkCount++;
		}

		return &m_WorldChunks.at(std::pair<int, int>(cx, cz));
	}

	/*
		Gets the world seed
	*/
	int World::GetSeed()
	{
		return m_WorldSeed;
	}
}