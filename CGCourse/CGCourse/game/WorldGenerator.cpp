#include "WorldGenerator.h"

#include <random>
#undef max

namespace Game
{

    class Random {
    public:
        Random(unsigned int seed = std::random_device()()) {
            m_Engine.seed(seed);
        }

        float Float() {
            return (float)m_Distribution(m_Engine) / (float)std::numeric_limits<uint32_t>::max();
        }

        int Int(int limit) {
            return (int)m_Distribution(m_Engine) % limit;
        }

        unsigned int UnsignedInt(unsigned int limit) {
            return (unsigned int)m_Distribution(m_Engine) % limit;
        }

    private:

        // Uses the std::mt19937 random engine
        std::mt19937 m_Engine;
        std::uniform_int_distribution<std::mt19937::result_type> m_Distribution;
    };
	
    static FastNoise BiomeGenerator(8213); // Biome generation
    Game::Biome GetBiome(float chunk_noise);

    // Water levels
    int water_min_default = 2;
    int water_max_default = 72;

    BlockType vein_block = BlockType::Sand;

    BlockType GetUnderwaterBlock(Random& random_gen)
    {
        switch (random_gen.UnsignedInt(6))
        {
        case 0:
            return BlockType::Sand;
            break;

        case 5:
            return BlockType::Clay;
            break;
        default:
            return BlockType::Sand;
            break;
        }

        return BlockType::Sand;
    }

    // Sets the vertical blocks based on the biome
    void SetVerticalBlocks(Chunk* chunk, int x, int z, int y_level, float real_x, float real_z)
    {
        BiomeGenerator.SetNoiseType(FastNoise::Simplex);

        Biome biome = chunk->p_BiomeMap[x][z];

        for (int i = 0; i < y_level; i++)
        {
            if (i > 1)
            {
                if (biome == Biome::Grassland)
                {
                    if (i >= y_level - 1)
                    {
                        chunk->SetBlock(BlockType::Grass, KUMA::MATHGL::Vector3(x, i, z));
                    }

                    else if (i >= y_level - 5)
                    {
                        chunk->SetBlock(BlockType::Dirt, KUMA::MATHGL::Vector3(x, i, z));
                    }

                    else
                    {
                        chunk->SetBlock(BlockType::Stone, KUMA::MATHGL::Vector3(x, i, z));
                    }
                }

                else if (biome == Biome::Desert)
                {
                    if (i >= y_level - 6)
                    {
                        chunk->SetBlock(BlockType::Sand, KUMA::MATHGL::Vector3(x, i, z));
                    }

                    else
                    {
                        chunk->SetBlock(BlockType::Stone, KUMA::MATHGL::Vector3(x, i, z));
                    }
                }
            }

            else
            {
                chunk->SetBlock(BlockType::Bedrock, KUMA::MATHGL::Vector3(x, i, z));
            }
        }

        return;
    }

    void AddWaterBlocks(Chunk* chunk, const int water_min, const int water_max)
    {
        /*
        Generates water in the areas needed inside of the chunk
        */

        Random water_gen(chunk->p_Position.x + chunk->p_Position.y);
        ChunkDataTypePtr chunk_data = &chunk->p_ChunkContents;

        for (int x = 0; x < CHUNK_SIZE_X; x++)
        {
            for (int y = water_min; y < water_max + 6; y++)
            {
                for (int z = 0; z < CHUNK_SIZE_Z; z++)
                {
                    Block* block = &chunk_data->at(x).at(y).at(z);

                    if (block->p_BlockType == BlockType::Air)
                    {
                        if (y > water_min && y < water_max)
                        {
                            BiomeGenerator.SetNoiseType(FastNoise::Simplex);
                            //float real_x = x + chunk->p_Position.x * CHUNK_SIZE_X;
                            //float real_z = z + chunk->p_Position.z * CHUNK_SIZE_Z;
                            
                            chunk_data->at(x).at(y).at(z).p_BlockType = BlockType::Water;

                            if (x > 0 && y < water_max - 1)
                            {
                                if (chunk_data->at(x - 1).at(y).at(z).IsOpaque())
                                {
                                    chunk_data->at(x - 1).at(y).at(z).p_BlockType = GetUnderwaterBlock(water_gen);
                                }
                            }

                            if (x < CHUNK_SIZE_X - 1 && y < water_max - 1)
                            {
                                if (chunk_data->at(x + 1).at(y).at(z).IsOpaque())
                                {
                                    chunk_data->at(x + 1).at(y).at(z).p_BlockType = GetUnderwaterBlock(water_gen);
                                }
                            }

                            if (z > 0 && y < water_max - 1)
                            {
                                if (chunk_data->at(x).at(y).at(z - 1).IsOpaque())
                                {
                                    chunk_data->at(x).at(y).at(z - 1).p_BlockType = GetUnderwaterBlock(water_gen);
                                }
                            }

                            if (z < CHUNK_SIZE_Z - 1 && y < water_max - 1)
                            {
                                if (chunk_data->at(x).at(y).at(z + 1).IsOpaque())
                                {
                                    chunk_data->at(x).at(y).at(z + 1).p_BlockType = GetUnderwaterBlock(water_gen);
                                }
                            }

                            if (y > 0)
                            {
                                if (chunk_data->at(x).at(y - 1).at(z).IsOpaque())
                                {
                                    chunk_data->at(x).at(y - 1).at(z).p_BlockType = GetUnderwaterBlock(water_gen);
                                }
                            }

                            if (y < water_max)
                            {
                                if (chunk_data->at(x).at(y + 1).at(z).IsOpaque())
                                {
                                    chunk_data->at(x).at(y + 1).at(z).p_BlockType = GetUnderwaterBlock(water_gen);
                                }
                            }
                        }
                    }

                    else if (block->p_BlockType != BlockType::Air && y > water_max - 2 && y < water_max + 4)
                    {
                        chunk_data->at(x).at(y).at(z) = { BlockType::Sand };
                    }
                }
            }
        }
    }

    BlockType GenerateFlower()
    {
        static Random random;

        switch (random.UnsignedInt(6))
        {
        case 0:
            return BlockType::Flower_allium;
            break;
        case 1:
            return BlockType::Flower_orchid;
            break;
        case 2:
            return BlockType::Flower_tulip_red;
            break;

        case 3:
            return BlockType::Flower_tulip_pink;
            break;
        case 4:
            return BlockType::Flower_rose;
            break;

        case 5:
            return BlockType::Flower_dandelion;
            break;

        default:
            return BlockType::Flower_tulip_red;
            break;
        }
        return BlockType::Flower_tulip_red;
    }

    Biome GetBiome(float chunk_noise)
    {
        // Quantize the noise into various levels and frequency

        if (chunk_noise < 80)
        {
            return Biome::Desert;
        }

        else 
        {
            return Biome::Grassland;
        }
    }

    constexpr double e = 2.718281828459f; // eulers number

    void GenerateChunk(Chunk* chunk, const int WorldSeed, WorldGenerationType gen_type)
    {
        int water_min = water_min_default;
        int water_max = water_max_default;

        static FastNoise WorldGenerator(WorldSeed);
        static FastNoise WorldGenerator_1(WorldSeed);
        
        // Set the chunk state
        WorldGenerator.SetNoiseType(FastNoise::SimplexFractal);

        if (gen_type == WorldGenerationType::Generation_Normal)
        {
            WorldGenerator.SetFrequency(0.006);
            WorldGenerator.SetFractalOctaves(5);
            WorldGenerator.SetFractalLacunarity(2.0f);
        }

        if (gen_type == WorldGenerationType::Generation_Normal || gen_type == WorldGenerationType::Generation_Islands ||
            gen_type == WorldGenerationType::Generation_Hilly)
        {
            int generated_x = 0;
            int generated_y = 0;
            int generated_z = 0;

            if (gen_type == WorldGenerationType::Generation_Islands)
            {
                water_max = 110;
                water_max_default = water_max;
            }

            if (gen_type == WorldGenerationType::Generation_Hilly)
            {
                water_max = 80;
                water_max_default = water_max;
            }

            // Generates the world using perlin noise to generate a height map

            for (int x = 0; x < CHUNK_SIZE_X; x++)
            {
                for (int z = 0; z < CHUNK_SIZE_Z; z++)
                {
                    int real_x = x + chunk->p_Position.x * CHUNK_SIZE_X;
                    int real_z = z + chunk->p_Position.z * CHUNK_SIZE_Z;

                    float height_at = 0.0f;

                    if (gen_type == WorldGenerationType::Generation_Islands || gen_type == WorldGenerationType::Generation_Hilly)
                    {
                        height_at = WorldGenerator.GetNoise(real_x, real_z) +
                            (0.5 * WorldGenerator.GetNoise(real_x, real_z)) *
                            WorldGenerator.GetNoise(real_x * 0.4f, real_z * 0.4f);
                    }

                    else if (gen_type == WorldGenerationType::Generation_Normal)
                    {
                        height_at = (WorldGenerator.GetNoise(real_x / 2.0f, real_z / 2.0f));
                    }

                    // Generate the biome
                    float column_noise = BiomeGenerator.GetNoise(real_x, real_z);
                    column_noise = ((column_noise + 1.0f) / 2) * 200;

                    Biome biome = GetBiome(column_noise);
                    chunk->p_BiomeMap[x][z] = biome;

                    generated_x = x;
                    generated_z = z;

                    generated_y = ((height_at + 1.0f) / 2) * 240;

                    if (generated_y >= CHUNK_SIZE_Y)
                    {
                        generated_y = CHUNK_SIZE_Y - 2;
                    }

                    chunk->p_HeightMap[x][z] = static_cast<uint8_t>(generated_y);
                    SetVerticalBlocks(chunk, generated_x, generated_z, generated_y, real_x, real_z);
                }
            }

            AddWaterBlocks(chunk, water_min, water_max);
        }

        else if (gen_type == WorldGenerationType::Generation_Flat || gen_type == WorldGenerationType::Generation_FlatWithoutStructures)
        {
            for (int x = 0; x < CHUNK_SIZE_X; x++)
            {
                for (int z = 0; z < CHUNK_SIZE_Z; z++)
                {
                    const uint8_t height = 127;

                    float real_x = x + chunk->p_Position.x * CHUNK_SIZE_X;
                    float real_z = z + chunk->p_Position.z * CHUNK_SIZE_Z;
                    chunk->p_HeightMap[x][z] = height;

                    SetVerticalBlocks(chunk, x, z, height, real_x, real_z);
                }
            }
        }
    }

    // To freaking slow
    void GenerateCaves(Chunk* chunk, const int seed)
    {
        constexpr int cave_level = 55;

        static FastNoise noise_1(seed);
        static FastNoise noise_2(seed);

        noise_1.SetNoiseType(FastNoise::Simplex);
        noise_2.SetNoiseType(FastNoise::Simplex);

        for (int x = 0; x < CHUNK_SIZE_X; x++)
        {
            for (int y = 0; y < CHUNK_SIZE_Y; y++)
            {
                for (int z = 0; z < CHUNK_SIZE_Z; z++)
                {
                    if (y < 2)
                    {
                        chunk->p_ChunkContents.at(x).at(y).at(z) = { BlockType::Bedrock };
                    }

                    else if (y < cave_level)
                    {
                        float real_x = x + chunk->p_Position.x * CHUNK_SIZE_X;
                        float real_z = z + chunk->p_Position.z * CHUNK_SIZE_Z;

                        float n1 = noise_1.GetNoise(real_x, y, real_z);
                        n1 = n1 * n1;

                        float n2 = noise_2.GetNoise(real_x, y, real_z);
                        n2 = n2 * n2;

                        float res = n1 + n2;

                        bool cave = res < 0.02 ? true : false;

                        if (cave)
                        {
                            chunk->p_ChunkContents.at(x).at(y).at(z) = { BlockType::Air };
                        }
                    }
                }
            }
        }
    }

    float _rounded(const KUMA::MATHGL::Vector2& coord)
    {
        auto bump = [](float t) { return std::max(0.0f, 1.0f - std::pow(t, 6.0f)); };
        float b = bump(coord.x) * bump(coord.y);
        return b * 0.9f;
    }

    TreeStructure WorldStructureTree;
    CactusStructure WorldStructureCactus;
    Random WorldTreeGenerator(1234);
    Random WorldModelGenerator(1234);

    bool FillInWorldStructure(WorldStructure* structure, int x, int y, int z)
    {
        if (y <= 0 || y >= CHUNK_SIZE_Y)
        {
            return false;
        }

        for (int i = 0; i < structure->p_Structure.size(); i++)
        {
            Block* block = GetWorldBlock(KUMA::MATHGL::Vector3(x + structure->p_Structure.at(i).x, y + structure->p_Structure.at(i).y, z + structure->p_Structure.at(i).z));
            *block = structure->p_Structure.at(i).block;
        }

        return true;
    }

    void GenerateChunkFlora(Chunk* chunk, const int WorldSeed, WorldGenerationType gen_type)
    {
        if (gen_type == WorldGenerationType::Generation_FlatWithoutStructures)
        {
            return;
        }

        for (int x = 0; x < CHUNK_SIZE_X; x++)
        {
            for (int z = 0; z < CHUNK_SIZE_Z; z++)
            {
                int real_x = x + chunk->p_Position.x * CHUNK_SIZE_X;
                int real_z = z + chunk->p_Position.z * CHUNK_SIZE_Z;
                int structure_freq = 0; // The higher the less likely it is to spawn
                bool added_structure = false;

                Biome biome = chunk->p_BiomeMap[x][z];

                if (chunk->p_HeightMap[x][z] + MAX_STRUCTURE_Y < CHUNK_SIZE_Y)
                {
                    int structure_x = real_x;
                    int structure_y = chunk->p_HeightMap[x][z];
                    int structure_z = real_z;

                    WorldStructure* structure = nullptr;

                    if (biome == Biome::Grassland)
                    {
                        structure_x = real_x - 2;
                        structure_z = real_z - 2;
                        structure_freq = 200;
                        structure = &WorldStructureTree;
                    }

                    else if (biome == Biome::Desert)
                    {
                        structure_freq = 300;
                        structure = &WorldStructureCactus;
                    }

                    if (structure && WorldTreeGenerator.UnsignedInt(structure_freq) == 0 && chunk->p_HeightMap[x][z] > water_max_default + 6)
                    {
                        added_structure = true;
                        FillInWorldStructure(structure, structure_x, structure_y, structure_z);
                    }
                }

                if (chunk->p_HeightMap[x][z] + 1 < CHUNK_SIZE_Y && chunk->p_HeightMap[x][z] + 1 > water_max_default + 6 && !added_structure)
                {
                    int y = chunk->p_HeightMap[x][z];
                    BlockType model_type = BlockType::Air;

                    if (biome == Biome::Grassland)
                    {
                        int num = WorldModelGenerator.UnsignedInt(150);

                        if (num > 135)
                        {
                            model_type = BlockType::Model_Grass;
                        }

                        else if (num == 40 || num == 30)
                        {
                            model_type = GenerateFlower();
                        }
                    }

                    else if (biome == Biome::Desert)
                    {
                        int num = WorldModelGenerator.UnsignedInt(280);

                        if (num == 40 || num == 30)
                        {
                            model_type = BlockType::Model_Deadbush;
                        }
                    }

                    chunk->p_ChunkContents.at(x).at(y).at(z).p_BlockType = model_type;
                }
            }
        }
    }
   
}
