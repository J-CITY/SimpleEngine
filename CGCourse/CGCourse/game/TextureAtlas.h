#pragma once

#include <vector>
#include <string>
#include <array>

#include "../engine/resourceManager/resource/texture.h"
#include "../engine/utils/math/Vector2.h"

namespace Game {

	class TextureAtlas {
	public : 

		TextureAtlas(std::shared_ptr<KUMA::RESOURCES::Texture> atlas_texture, int tx, int ty) {
			m_Atlas = atlas_texture;
			m_TileX = tx;
			m_TileY = ty;
			m_AtlasWidth = atlas_texture->width;
			m_AtlasHeight = atlas_texture->height;
		}

		TextureAtlas(const std::string& atlas_path, int tx, int ty) {
			m_Atlas = KUMA::RESOURCES::TextureLoader().createResource(atlas_path);
			m_TileX = tx;
			m_TileY = ty;
			m_AtlasWidth = m_Atlas->width;
			m_AtlasHeight = m_Atlas->height;
		}

		TextureAtlas(int width, int height, int tx, int ty) {
			m_AtlasWidth = width;
			m_AtlasHeight = height;
			m_TileX = tx;
			m_TileY = ty;
		}

		std::array<GLfloat, 8> Sample(const KUMA::MATHGL::Vector2f& start_coords, const KUMA::MATHGL::Vector2f& end_coords, bool flip = false) {
			KUMA::MATHGL::Vector2f s_coords;
			KUMA::MATHGL::Vector2f e_coords;

			s_coords = start_coords;
			e_coords = end_coords;

			if (flip) {
				s_coords = end_coords;
				e_coords = start_coords;
			}

			float width, height;
			float x2, y2;
			float x1, y1;

			std::array<GLfloat, 8> TextureCoordinates;

			x1 = s_coords.x * m_TileX;
			y1 = s_coords.y * m_TileY;
			x2 = e_coords.x * m_TileX;
			y2 = e_coords.y * m_TileY;

			x1 = x1 / m_AtlasWidth;
			y1 = y1 / m_AtlasHeight;
			x2 = x2 / m_AtlasWidth;
			y2 = y2 / m_AtlasHeight;

			TextureCoordinates[0] = x2;
			TextureCoordinates[1] = y1;
			TextureCoordinates[2] = x1;
			TextureCoordinates[3] = y1;
			TextureCoordinates[4] = x1;
			TextureCoordinates[5] = y2;
			TextureCoordinates[6] = x2;
			TextureCoordinates[7] = y2;

			return TextureCoordinates;
		}

		std::array<uint16_t, 8> SampleTexel(const KUMA::MATHGL::Vector2f& start_coords, const KUMA::MATHGL::Vector2f& end_coords, bool flip = false) {
			KUMA::MATHGL::Vector2f s_coords;
			KUMA::MATHGL::Vector2f e_coords;

			s_coords = start_coords;
			e_coords = end_coords;

			if (flip)
			{
				s_coords = end_coords;
				e_coords = start_coords;
			}

			float width, height;
			float x2, y2;
			float x1, y1;

			std::array<uint16_t, 8> TextureCoordinates;

			x1 = s_coords.x * m_TileX;
			y1 = s_coords.y * m_TileY;
			x2 = e_coords.x * m_TileX;
			y2 = e_coords.y * m_TileY;

			TextureCoordinates[0] = x2;
			TextureCoordinates[1] = y1;
			TextureCoordinates[2] = x1;
			TextureCoordinates[3] = y1;
			TextureCoordinates[4] = x1;
			TextureCoordinates[5] = y2;
			TextureCoordinates[6] = x2;
			TextureCoordinates[7] = y2;

			return TextureCoordinates;
		}

		std::array<GLfloat, 8> SampleCustom(const KUMA::MATHGL::Vector2f& start_coords, const KUMA::MATHGL::Vector2f& end_coords) {
			float width, height;
			float x2, y2;
			float x1, y1;

			width = end_coords.x - start_coords.x;
			height = end_coords.y - start_coords.y;

			std::array<GLfloat, 8> TextureCoordinates;

			x1 = start_coords.x;
			y1 = start_coords.y;
			x2 = end_coords.x;
			y2 = end_coords.y;

			x1 = x1 / m_AtlasWidth;
			y1 = y1 / m_AtlasHeight;
			x2 = x2 / m_AtlasWidth;
			y2 = y2 / m_AtlasHeight;

			TextureCoordinates[0] = x2;
			TextureCoordinates[1] = y2;
			TextureCoordinates[2] = x2;
			TextureCoordinates[3] = y1;
			TextureCoordinates[4] = x1;
			TextureCoordinates[5] = y1;
			TextureCoordinates[6] = x1;
			TextureCoordinates[7] = y2;
			
			return TextureCoordinates;
		}

	private : 

		int m_TileX, m_TileY;
		int m_AtlasWidth, m_AtlasHeight;
		std::shared_ptr<KUMA::RESOURCES::Texture> m_Atlas;
	};
}
