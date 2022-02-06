#include "Chunk.h"
#include "../engine/utils/math/Vector3.h"

using namespace Game;
using namespace KUMA::MATHGL;

Chunk::Chunk(const Vector3 chunk_position) :
	p_Position(chunk_position), 
	p_MeshState(ChunkMeshState::UNBUILT),
	p_ChunkState(ChunkState::UNGENERETED),
	p_LightMapState(ChunkLightMapState::UNMODIFIED_LIGHT_MAP),
	p_ChunkFrustumAABB(Vector3(CHUNK_SIZE_X, CHUNK_SIZE_Y, CHUNK_SIZE_Z), Vector3(chunk_position.x * CHUNK_SIZE_X, chunk_position.y * CHUNK_SIZE_Y, chunk_position.z * CHUNK_SIZE_Z))
{
	memset(&p_ChunkContents, BlockType::Air, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z);
	memset(&p_ChunkLightInformation, 0, (CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z * sizeof(std::uint8_t)));
	memset(&p_HeightMap, 0, CHUNK_SIZE_X * CHUNK_SIZE_Z * sizeof(std::uint8_t));
	memset(&p_BiomeMap, 0, CHUNK_SIZE_X * CHUNK_SIZE_Z * sizeof(std::uint8_t));
}

Chunk::~Chunk() {
	
}

void Chunk::SetBlock(BlockType type, const KUMA::MATHGL::Vector3& position) {
	Block b;
	b.p_BlockType = type;

	p_ChunkContents.at(position.x).at(position.y).at(position.z) = b;
}

int Chunk::GetTorchLightAt(int x, int y, int z)
{
	return p_ChunkLightInformation[x][y][z];
}

void Chunk::SetTorchLightAt(int x, int y, int z, int light_val)
{
	p_ChunkLightInformation[x][y][z] = light_val;
	p_LightMapState = ChunkLightMapState::MODIFIED_LIGHT_MAP;
}

void Chunk::Construct() {
	if (m_ChunkMesh.ConstructMesh(this, p_Position)) {
		p_MeshState = ChunkMeshState::BUILT;
	}
	else {
		p_MeshState = ChunkMeshState::UNBUILT;
	}
}

ChunkMesh* Chunk::GetChunkMesh()
{
	return &m_ChunkMesh;
}

Block* Chunk::GetBlock(int x, int y, int z)
{
	return &p_ChunkContents[x][y][z];
}

