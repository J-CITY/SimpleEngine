#include "ChunkMesh.h"
#include "Chunk.h"
#include "ModelGame.h"
#include "../engine/render/buffers/indexBuffer.h"
import glmath;
#include "Vertex.h"
using namespace KUMA::MATHGL;

namespace Game {
	std::unique_ptr<KUMA::RENDER::IndexBuffer> ChunkMesh::StaticIBO;
	ChunkMesh::ChunkMesh() {
		static bool IndexBufferInitialized = false;

		

		if (IndexBufferInitialized == false) {
			IndexBufferInitialized = true;

			unsigned int index_size = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * 6;
			std::vector<unsigned int> IndexBuffer(index_size * 6, 0);

			int index_offset = 0;

			for (size_t i = 0; i < index_size; i += 6) {
				IndexBuffer[i]     = 0 + index_offset;
				IndexBuffer[i + 1] = 1 + index_offset;
				IndexBuffer[i + 2] = 2 + index_offset;
				IndexBuffer[i + 3] = 2 + index_offset;
				IndexBuffer[i + 4] = 3 + index_offset;
				IndexBuffer[i + 5] = 0 + index_offset;

				index_offset = index_offset + 4;
			}

			StaticIBO = std::make_unique<KUMA::RENDER::IndexBuffer>(KUMA::RENDER::IndexBuffer(IndexBuffer));
			isize = IndexBuffer.size();
		}
		
		p_VAO.bind();
		m_VBO.bind();
		StaticIBO->bind();
		m_VBO.bindAttribute(0, 3, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, position));
		m_VBO.bindAttribute(1, 2, GL_UNSIGNED_SHORT, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, texCoord));
		m_VBO.bindAttribute(2, 1, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, lighting_level));
		m_VBO.bindAttribute(3, 1, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, block_face_lighting));
		p_VAO.unbind();

		p_TransparentVAO.bind();
		m_TransparentVBO.bind();
		StaticIBO->bind();
		m_TransparentVBO.bindAttribute(0, 3, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, position));
		m_TransparentVBO.bindAttribute(1, 2, GL_UNSIGNED_SHORT, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, texCoord));
		m_TransparentVBO.bindAttribute(2, 1, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, lighting_level));
		m_TransparentVBO.bindAttribute(3, 1, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, block_face_lighting));
		p_TransparentVAO.unbind();

		p_ModelVAO.bind();
		m_ModelVBO.bind();
		StaticIBO->bind();
		m_ModelVBO.bindAttribute(0, 3, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, position));
		m_ModelVBO.bindAttribute(1, 2, GL_UNSIGNED_SHORT, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, texCoord));
		m_ModelVBO.bindAttribute(2, 1, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, lighting_level));
		m_ModelVBO.bindAttribute(3, 1, GL_UNSIGNED_BYTE, sizeof(Game::Vertex), (void*)offsetof(Game::Vertex, block_face_lighting));
		p_ModelVAO.unbind();

		// Set the values of the 2D planes

		m_ForwardFace[0] = KUMA::MATHGL::Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		m_ForwardFace[1] = KUMA::MATHGL::Vector4(1.0f, 0.0f, 1.0f, 1.0f);
		m_ForwardFace[2] = KUMA::MATHGL::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		m_ForwardFace[3] = KUMA::MATHGL::Vector4(0.0f, 1.0f, 1.0f, 1.0f);

		m_BackFace[0] = KUMA::MATHGL::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m_BackFace[1] = KUMA::MATHGL::Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		m_BackFace[2] = KUMA::MATHGL::Vector4(1.0f, 1.0f, 0.0f, 1.0f);
		m_BackFace[3] = KUMA::MATHGL::Vector4(0.0f, 1.0f, 0.0f, 1.0f);

		m_TopFace[0] = KUMA::MATHGL::Vector4(0.0f, 1.0f, 0.0f, 1.0f);
		m_TopFace[1] = KUMA::MATHGL::Vector4(1.0f, 1.0f, 0.0f, 1.0f);
		m_TopFace[2] = KUMA::MATHGL::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		m_TopFace[3] = KUMA::MATHGL::Vector4(0.0f, 1.0f, 1.0f, 1.0f);

		m_BottomFace[0] = KUMA::MATHGL::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m_BottomFace[1] = KUMA::MATHGL::Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		m_BottomFace[2] = KUMA::MATHGL::Vector4(1.0f, 0.0f, 1.0f, 1.0f);
		m_BottomFace[3] = KUMA::MATHGL::Vector4(0.0f, 0.0f, 1.0f, 1.0f);

		m_LeftFace[0] = KUMA::MATHGL::Vector4(0.0f, 1.0f, 1.0f, 1.0f);
		m_LeftFace[1] = KUMA::MATHGL::Vector4(0.0f, 1.0f, 0.0f, 1.0f);
		m_LeftFace[2] = KUMA::MATHGL::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		m_LeftFace[3] = KUMA::MATHGL::Vector4(0.0f, 0.0f, 1.0f, 1.0f);

		m_RightFace[0] = KUMA::MATHGL::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		m_RightFace[1] = KUMA::MATHGL::Vector4(1.0f, 1.0f, 0.0f, 1.0f);
		m_RightFace[2] = KUMA::MATHGL::Vector4(1.0f, 0.0f, 0.0f, 1.0f);
		m_RightFace[3] = KUMA::MATHGL::Vector4(1.0f, 0.0f, 1.0f, 1.0f);
	}

	ChunkMesh::~ChunkMesh() {
		m_Vertices.clear();
	}

	// Construct mesh using greedy meshing for maximum performance
	bool ChunkMesh::ConstructMesh(Chunk* chunk, const Vector3& chunk_pos)
	{
		ChunkDataTypePtr ChunkData = &chunk->p_ChunkContents;
		ChunkLightDataTypePtr ChunkLData = &chunk->p_ChunkLightInformation;

		Vector3 world_position;
		Vector3 local_position;
		m_Vertices.clear();

		ChunkDataTypePtr ForwardChunkData = _GetChunkDataForMeshing(static_cast<int>(chunk_pos.x), static_cast<int>(chunk_pos.z + 1));
		ChunkDataTypePtr BackwardChunkData = _GetChunkDataForMeshing(static_cast<int>(chunk_pos.x), static_cast<int>(chunk_pos.z - 1));
		ChunkDataTypePtr RightChunkData = _GetChunkDataForMeshing(static_cast<int>(chunk_pos.x + 1), static_cast<int>(chunk_pos.z));
		ChunkDataTypePtr LeftChunkData = _GetChunkDataForMeshing(static_cast<int>(chunk_pos.x - 1), static_cast<int>(chunk_pos.z));
		ChunkLightDataTypePtr ForwardChunkLData = _GetChunkLightDataForMeshing(static_cast<int>(chunk_pos.x), static_cast<int>(chunk_pos.z + 1));
		ChunkLightDataTypePtr BackwardChunkLData = _GetChunkLightDataForMeshing(static_cast<int>(chunk_pos.x), static_cast<int>(chunk_pos.z - 1));
		ChunkLightDataTypePtr RightChunkLData = _GetChunkLightDataForMeshing(static_cast<int>(chunk_pos.x + 1), static_cast<int>(chunk_pos.z));
		ChunkLightDataTypePtr LeftChunkLData = _GetChunkLightDataForMeshing(static_cast<int>(chunk_pos.x - 1), static_cast<int>(chunk_pos.z));

		if (ForwardChunkData && BackwardChunkData && RightChunkData && LeftChunkData)
		{
			for (int x = 0; x < CHUNK_SIZE_X; x++)
			{
				for (int y = 0; y < CHUNK_SIZE_Y; y++)
				{
					for (int z = 0; z < CHUNK_SIZE_Z; z++)
					{
						if (ChunkData->at(x).at(y).at(z).p_BlockType != BlockType::Air)
						{
							Block* block = &ChunkData->at(x).at(y).at(z);

							// To fix chunk edge mesh building issues, both faces are added if it is in the edge

							float light_level = ChunkLData->at(x).at(y).at(z);

							if (y >= 0 && y < CHUNK_SIZE_Y - 1)
							{
								light_level = ChunkLData->at(x).at(y + 1).at(z);
							}

							world_position.x = chunk_pos.x * CHUNK_SIZE_X + x;
							world_position.y = 0 * CHUNK_SIZE_Y + y;
							world_position.z = chunk_pos.z * CHUNK_SIZE_Z + z;
							local_position = Vector3(x, y, z);

							if (block->IsModel())
							{
								AddModel(chunk, local_position, block->p_BlockType, light_level);
								continue;
							}

							if (z <= 0)
							{
								if (block->IsTransparent())
								{
									if (BackwardChunkData->at(x).at(y).at(CHUNK_SIZE_Z - 1).IsTransparent() &&
										BackwardChunkData->at(x).at(y).at(CHUNK_SIZE_Z - 1).p_BlockType != block->p_BlockType)
									{
										light_level = BackwardChunkLData->at(x).at(y).at(CHUNK_SIZE_Z - 1);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level, false);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level, false);
									}

									else if (ChunkData->at(x).at(y).at(1).IsTransparent() &&
										ChunkData->at(x).at(y).at(1).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(x).at(y).at(1);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level, false);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level, false);
									}
								}

								else {
									if (BackwardChunkData->at(x).at(y).at(CHUNK_SIZE_Z - 1).IsOpaque() == false)
									{
										light_level = BackwardChunkLData->at(x).at(y).at(CHUNK_SIZE_Z - 1);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level);
									}

									else if (ChunkData->at(x).at(y).at(1).IsOpaque() == false)
									{
										light_level = ChunkLData->at(x).at(y).at(1);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level);
									}
								}
							}

							else if (z >= CHUNK_SIZE_Z - 1)
							{
								if (block->IsTransparent())
								{
									if (ForwardChunkData->at(x).at(y).at(0).IsTransparent() &&
										ForwardChunkData->at(x).at(y).at(0).p_BlockType != block->p_BlockType)
									{
										light_level = ForwardChunkLData->at(x).at(y).at(0);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level, false);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level, false);
									}

									else if (ChunkData->at(x).at(y).at(CHUNK_SIZE_Z - 2).IsTransparent() &&
										ChunkData->at(x).at(y).at(CHUNK_SIZE_Z - 2).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(x).at(y).at(CHUNK_SIZE_Z - 2);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level, false);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level, false);
									}
								}

								else
								{
									if (ForwardChunkData->at(x).at(y).at(0).IsOpaque() == false)
									{
										light_level = ForwardChunkLData->at(x).at(y).at(0);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level);
									}

									else if (ChunkData->at(x).at(y).at(CHUNK_SIZE_Z - 2).IsOpaque() == false)
									{
										light_level = ChunkLData->at(x).at(y).at(CHUNK_SIZE_Z - 2);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level);
									}
								}
							}

							else
							{
								if (block->IsTransparent())
								{
									if (ChunkData->at(x).at(y).at(z + 1).IsTransparent() &&
										ChunkData->at(x).at(y).at(z + 1).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(x).at(y).at(z + 1);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level, false);
									}

									if (ChunkData->at(x).at(y).at(z - 1).IsTransparent() &&
										ChunkData->at(x).at(y).at(z - 1).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(x).at(y).at(z - 1);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level, false);
									}
								}

								else
								{
									//If the forward block is an air block, add the forward face to the mesh
									if (ChunkData->at(x).at(y).at(z + 1).IsOpaque() == false)
									{
										light_level = ChunkLData->at(x).at(y).at(z + 1);
										AddFace(chunk, BlockDatabase::BlockFaceType::front, local_position, block->p_BlockType, light_level);
									}

									// If the back (-forward) block is an air block, add the back face to the mesh
									if (ChunkData->at(x).at(y).at(z - 1).IsOpaque() == false)
									{
										light_level = ChunkLData->at(x).at(y).at(z - 1);
										AddFace(chunk, BlockDatabase::BlockFaceType::backward, local_position, block->p_BlockType, light_level);
									}
								}
							}

							if (x <= 0)
							{
								if (block->IsTransparent())
								{
									if (LeftChunkData->at(CHUNK_SIZE_X - 1).at(y).at(z).IsTransparent() &&
										LeftChunkData->at(CHUNK_SIZE_X - 1).at(y).at(z).p_BlockType != block->p_BlockType)
									{
										light_level = LeftChunkLData->at(CHUNK_SIZE_X - 1).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level, false);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level, false);
									}

									else if (ChunkData->at(1).at(y).at(z).IsTransparent() &&
										ChunkData->at(1).at(y).at(z).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(1).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level, false);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level, false);
									}

								}

								else
								{
									if (LeftChunkData->at(CHUNK_SIZE_X - 1).at(y).at(z).IsOpaque() == false)
									{
										light_level = LeftChunkLData->at(CHUNK_SIZE_X - 1).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level);
									}

									else if (ChunkData->at(1).at(y).at(z).IsOpaque() == false)
									{
										light_level = ChunkLData->at(1).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level);
									}
								}
							}

							else if (x >= CHUNK_SIZE_X - 1)
							{
								if (block->IsTransparent())
								{
									if (RightChunkData->at(0).at(y).at(z).IsTransparent() &&
										RightChunkData->at(0).at(y).at(z).p_BlockType != block->p_BlockType)
									{
										light_level = RightChunkLData->at(0).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level, false);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level, false);
									}

									else if (ChunkData->at(CHUNK_SIZE_X - 2).at(y).at(z).IsTransparent() &&
										ChunkData->at(CHUNK_SIZE_X - 2).at(y).at(z).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(CHUNK_SIZE_X - 2).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level, false);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level, false);
									}
								}

								else
								{
									if (RightChunkData->at(0).at(y).at(z).IsOpaque() == false)
									{
										light_level = RightChunkLData->at(0).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level);
									}

									else if (ChunkData->at(CHUNK_SIZE_X - 2).at(y).at(z).IsOpaque() == false)
									{
										light_level = ChunkLData->at(CHUNK_SIZE_X - 2).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level);
									}
								}

							}

							else
							{
								if (block->IsTransparent())
								{
									if (ChunkData->at(x + 1).at(y).at(z).IsTransparent() &&
										ChunkData->at(x + 1).at(y).at(z).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(x + 1).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level, false);
									}

									if (ChunkData->at(x - 1).at(y).at(z).IsTransparent() &&
										ChunkData->at(x - 1).at(y).at(z).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(x - 1).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level, false);
									}
								}

								else
								{
									// If the next block is an air block, add the right face to the mesh
									if (ChunkData->at(x + 1).at(y).at(z).IsOpaque() == false)
									{
										light_level = ChunkLData->at(x + 1).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::right, local_position, block->p_BlockType, light_level);
									}

									// If the previous block is an air block, add the left face to the mesh
									if (ChunkData->at(x - 1).at(y).at(z).IsOpaque() == false)
									{
										light_level = ChunkLData->at(x - 1).at(y).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::left, local_position, block->p_BlockType, light_level);
									}
								}
							}

							if (y <= 0)
							{
								if (ChunkData->at(x).at(y + 1).at(z).IsOpaque() == false)
								{
									AddFace(chunk, BlockDatabase::BlockFaceType::bottom, local_position, block->p_BlockType, light_level);
								}
							}

							else if (y >= CHUNK_SIZE_Y - 1)
							{
								AddFace(chunk, BlockDatabase::BlockFaceType::top, local_position, block->p_BlockType, light_level);
							}

							else
							{
								if (block->IsTransparent())
								{
									if (ChunkData->at(x).at(y - 1).at(z).IsTransparent() &&
										ChunkData->at(x).at(y - 1).at(z).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(x).at(y - 1).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::bottom, local_position, block->p_BlockType, light_level, false);
									}

									if (ChunkData->at(x).at(y + 1).at(z).IsTransparent() &&
										ChunkData->at(x).at(y + 1).at(z).p_BlockType != block->p_BlockType)
									{
										light_level = ChunkLData->at(x).at(y + 1).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::top, local_position, block->p_BlockType, light_level, false);
									}
								}

								else
								{
									// If the top block is an air block, add the top face to the mesh
									if (ChunkData->at(x).at(y - 1).at(z).IsOpaque() == false)
									{
										light_level = ChunkLData->at(x).at(y - 1).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::bottom, local_position, block->p_BlockType, light_level);
									}

									// If the bottom block is an air block, add the top face to the mesh
									if (ChunkData->at(x).at(y + 1).at(z).IsOpaque() == false)
									{
										light_level = ChunkLData->at(x).at(y + 1).at(z);
										AddFace(chunk, BlockDatabase::BlockFaceType::top, local_position, block->p_BlockType, light_level);
									}
								}
							}
						}
					}
				}
			}

			// Upload the data to the GPU whenever the mesh is reconstructed

			p_VerticesCount = 0;
			p_TransparentVerticesCount = 0;
			p_ModelVerticesCount = 0;

			if (m_Vertices.size() > 0)
			{
				m_VBO.bufferData(this->m_Vertices.size() * sizeof(Game::Vertex), &this->m_Vertices.front(), GL_STATIC_DRAW);
				p_VerticesCount = m_Vertices.size();
				m_Vertices.clear();
			}

			if (m_TransparentVertices.size() > 0)
			{
				m_TransparentVBO.bufferData(this->m_TransparentVertices.size() * sizeof(Game::Vertex), &this->m_TransparentVertices.front(), GL_STATIC_DRAW);
				p_TransparentVerticesCount = m_TransparentVertices.size();
				m_TransparentVertices.clear();
			}

			if (m_ModelVertices.size() > 0)
			{
				m_ModelVBO.bufferData(this->m_ModelVertices.size() * sizeof(Game::Vertex), &this->m_ModelVertices.front(), GL_STATIC_DRAW);
				p_ModelVerticesCount = m_ModelVertices.size();
				m_ModelVertices.clear();
			}

			return true;
		}

		return false;
	}

	Vector3 ConvertWorldPosToBlock(const Vector3& position) {
		int block_chunk_x = static_cast<int>(floor(position.x / CHUNK_SIZE_X));
		int block_chunk_z = static_cast<int>(floor(position.z / CHUNK_SIZE_Z));
		int lx = position.x - (block_chunk_x * CHUNK_SIZE_X);
		int ly = static_cast<int>(floor(position.y));
		int lz = position.z - (block_chunk_z * CHUNK_SIZE_Z);

		return Vector3(lx, ly, lz);
	}

	bool HasShadow(Chunk* chunk, int x, int y, int z) {
		constexpr int max_shadow = 24;
		for (int i = y + 1; i < y + max_shadow; i++)
		{
			if (i < CHUNK_SIZE_Y)
			{
				if (chunk->p_ChunkContents.at(x).at(i).at(z).CastsShadow())
				{
					return true;
				}
			}
		}

		return false;
	}

	void ChunkMesh::AddFace(Chunk* chunk, BlockDatabase::BlockFaceType face_type, const Vector3& position, BlockType type, uint8_t light_level,
		bool buffer)
	{
		KUMA::MATHGL::Vector4 translation = KUMA::MATHGL::Vector4(position.x, position.y, position.z, 0.0f); // No need to create a model matrix. 
		// Adding the position to the translation will do the samething but much much faster

		Game::Vertex v1, v2, v3, v4;

		// To fix a face culling issue. I inverted the vertices for the left, front and bottom face so the winding order will be correct
		bool reverse_texture_coordinates = false;

		// Order
		// Top, bottom, front, back, left, right
		static const uint8_t lighting_levels[6] = { 10, 3, 6, 7, 6, 7 };

		switch (face_type)
		{
		case BlockDatabase::BlockFaceType::top:
		{
			uint8_t face_light_level = 10;

			if (HasShadow(chunk, position.x, position.y, position.z))
			{
				face_light_level -= 2;
			}

			v1.position = translation + m_TopFace[0];
			v2.position = translation + m_TopFace[1];
			v3.position = translation + m_TopFace[2];
			v4.position = translation + m_TopFace[3];

			// Set the lighting level for the vertex
			v1.lighting_level = light_level;
			v2.lighting_level = light_level;
			v3.lighting_level = light_level;
			v4.lighting_level = light_level;

			v1.block_face_lighting = face_light_level;
			v2.block_face_lighting = face_light_level;
			v3.block_face_lighting = face_light_level;
			v4.block_face_lighting = face_light_level;

			break;
		}

		case BlockDatabase::BlockFaceType::bottom:
		{
			v1.position = translation + m_BottomFace[3];
			v2.position = translation + m_BottomFace[2];
			v3.position = translation + m_BottomFace[1];
			v4.position = translation + m_BottomFace[0];

			// Set the lighting level for the vertex
			v1.lighting_level = light_level;
			v2.lighting_level = light_level;
			v3.lighting_level = light_level;
			v4.lighting_level = light_level;

			v1.block_face_lighting = lighting_levels[1];
			v2.block_face_lighting = lighting_levels[1];
			v3.block_face_lighting = lighting_levels[1];
			v4.block_face_lighting = lighting_levels[1];

			reverse_texture_coordinates = true;

			break;
		}

		case BlockDatabase::BlockFaceType::front:
		{
			v1.position = translation + m_ForwardFace[3];
			v2.position = translation + m_ForwardFace[2];
			v3.position = translation + m_ForwardFace[1];
			v4.position = translation + m_ForwardFace[0];

			// Set the lighting level for the vertex
			v1.lighting_level = light_level;
			v2.lighting_level = light_level;
			v3.lighting_level = light_level;
			v4.lighting_level = light_level;

			v1.block_face_lighting = lighting_levels[2];
			v2.block_face_lighting = lighting_levels[2];
			v3.block_face_lighting = lighting_levels[2];
			v4.block_face_lighting = lighting_levels[2];

			reverse_texture_coordinates = true;

			break;
		}

		case BlockDatabase::BlockFaceType::backward:
		{
			v1.position = translation + m_BackFace[0];
			v2.position = translation + m_BackFace[1];
			v3.position = translation + m_BackFace[2];
			v4.position = translation + m_BackFace[3];

			v1.lighting_level = light_level;
			v2.lighting_level = light_level;
			v3.lighting_level = light_level;
			v4.lighting_level = light_level;

			v1.block_face_lighting = lighting_levels[3];
			v2.block_face_lighting = lighting_levels[3];
			v3.block_face_lighting = lighting_levels[3];
			v4.block_face_lighting = lighting_levels[3];

			break;
		}

		case BlockDatabase::BlockFaceType::left:
		{
			v1.position = translation + m_LeftFace[3];
			v2.position = translation + m_LeftFace[2];
			v3.position = translation + m_LeftFace[1];
			v4.position = translation + m_LeftFace[0];

			v1.lighting_level = light_level;
			v2.lighting_level = light_level;
			v3.lighting_level = light_level;
			v4.lighting_level = light_level;

			v1.block_face_lighting = lighting_levels[4];
			v2.block_face_lighting = lighting_levels[4];
			v3.block_face_lighting = lighting_levels[4];
			v4.block_face_lighting = lighting_levels[4];

			reverse_texture_coordinates = true;

			break;
		}

		case BlockDatabase::BlockFaceType::right:
		{
			v1.position = translation + m_RightFace[0];
			v2.position = translation + m_RightFace[1];
			v3.position = translation + m_RightFace[2];
			v4.position = translation + m_RightFace[3];

			v1.lighting_level = light_level;
			v2.lighting_level = light_level;
			v3.lighting_level = light_level;
			v4.lighting_level = light_level;

			v1.block_face_lighting = lighting_levels[5];
			v2.block_face_lighting = lighting_levels[5];
			v3.block_face_lighting = lighting_levels[5];
			v4.block_face_lighting = lighting_levels[5];

			break;
		}

		default:
		{
			// Todo : Throw an error here
			break;
		}
		}

		// Get required texture coordinates

		const std::array<uint16_t, 8>& TextureCoordinates = BlockDatabase::GetBlockTexture(type, face_type);

		if (reverse_texture_coordinates)
		{
			v1.texCoord = { TextureCoordinates[6], TextureCoordinates[7] };
			v2.texCoord = { TextureCoordinates[4], TextureCoordinates[5] };
			v3.texCoord = { TextureCoordinates[2], TextureCoordinates[3] };
			v4.texCoord = { TextureCoordinates[0], TextureCoordinates[1] };
		}

		else
		{
			v1.texCoord = { TextureCoordinates[0], TextureCoordinates[1] };
			v2.texCoord = { TextureCoordinates[2], TextureCoordinates[3] };
			v3.texCoord = { TextureCoordinates[4], TextureCoordinates[5] };
			v4.texCoord = { TextureCoordinates[6], TextureCoordinates[7] };
		}

		if (buffer)
		{
			m_Vertices.push_back(v1);
			m_Vertices.push_back(v2);
			m_Vertices.push_back(v3);
			m_Vertices.push_back(v1);
			m_Vertices.push_back(v3);
			m_Vertices.push_back(v4);
		}

		else if (!buffer)
		{
			m_TransparentVertices.push_back(v1);
			m_TransparentVertices.push_back(v2);
			m_TransparentVertices.push_back(v3);
			m_TransparentVertices.push_back(v1);
			m_TransparentVertices.push_back(v3);
			m_TransparentVertices.push_back(v4);
		}
	}

	// Adds a model such as a flower or a deadbush to the chunk mesh
	void ChunkMesh::AddModel(Chunk* chunk, const Vector3& local_pos, BlockType type, float light_level)
	{
		Matrix4 translation = Matrix4();
		translation(0, 3) = local_pos.x;
		translation(1, 3) = local_pos.y;
		translation(2, 3) = local_pos.z;
		Game::ModelGame model(type);

		uint8_t face_light = 10;

		if (HasShadow(chunk, local_pos.x, local_pos.y, local_pos.z))
		{
			face_light -= 2;
		}

		for (int i = 0; i < model.p_ModelVertices.size(); i++)
		{
			Game::Vertex vertex;

			auto v = model.p_ModelVertices.at(i).position;
			KUMA::MATHGL::Vector4 pos = KUMA::MATHGL::Vector4(v.x, v.y, v.z, 1.0f);
			pos = translation * pos;

			vertex.position = Vector3(pos.x, pos.y, pos.z);
			vertex.texCoord = model.p_ModelVertices.at(i).tex_coords;
			vertex.lighting_level = light_level;
			vertex.block_face_lighting = face_light;

			m_ModelVertices.push_back(vertex);
		}
	}
}
