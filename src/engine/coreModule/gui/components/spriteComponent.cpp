#include "spriteComponent.h"
#include "../guiObject.h"
#include <renderModule/gameRenderer.h>
#include "resourceModule/shaderManager.h"
#include "../../core/core.h"
#include "../../ecs/object.h"
#ifdef OPENGL_BACKEND
#include <renderModule/backends/gl/textureGl.h>
#endif
#include <utilsModule/pathGetter.h>

#include "resourceModule/textureManager.h"
#include "renderModule/backends/gl/shaderGl.h"
#include "renderModule/backends/gl/vertexBufferGl.h"
#include "utilsModule/time/time.h"

using namespace IKIGAI;
using namespace IKIGAI::ECS;

#ifdef OPENGL_BACKEND
SpriteBatcher::SpriteBatcher()
{
    // Get the texture uniform from the shader program.
    //m_textureUniform = glGetUniformLocation(m_shaderProgram, "tex");
    //m_texture = nullptr;

    // Setup vertex buffer

	mVertexArray = std::make_unique<RENDER::VertexArray>();
	mVbo = std::make_shared<RENDER::VertexBufferGl<BatchVertex>>();
	
	uint64_t vertexSize = sizeof(BatchVertex);
	mVertexArray->bindAttribute(0, *std::static_pointer_cast<RENDER::VertexBufferGl<BatchVertex>>(mVbo), RENDER::VertexArray::Type::FLOAT, 3, vertexSize, (intptr_t)offsetof(BatchVertex, position));
	mVertexArray->bindAttribute(1, *std::static_pointer_cast<RENDER::VertexBufferGl<BatchVertex>>(mVbo), RENDER::VertexArray::Type::FLOAT, 2, vertexSize, (intptr_t)offsetof(BatchVertex, texCoord));
	mVertexArray->bindAttribute(2, *std::static_pointer_cast<RENDER::VertexBufferGl<BatchVertex>>(mVbo), RENDER::VertexArray::Type::FLOAT, 4, vertexSize, (intptr_t)offsetof(BatchVertex, color));

	//mVbo = std::make_shared<RENDER::VertexBufferGl<BatchVertex>>();
	//std::static_pointer_cast<RENDER::VertexBufferGl<BatchVertex>>(mVbo)->bindAttribute(0, 3, GL_FLOAT, false, sizeof(BatchVertex), (void*)offsetof(BatchVertex, position));
	//std::static_pointer_cast<RENDER::VertexBufferGl<BatchVertex>>(mVbo)->bindAttribute(1, 2, GL_FLOAT, false, sizeof(BatchVertex), (void*)offsetof(BatchVertex, texCoord));
	//std::static_pointer_cast<RENDER::VertexBufferGl<BatchVertex>>(mVbo)->bindAttribute(2, 4, GL_FLOAT, false, sizeof(BatchVertex), (void*)offsetof(BatchVertex, color));
    //glGenBuffers(1, &m_vbo);
    //glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2dUVColor), (void*)0);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2dUVColor), (void*)sizeof(glm::vec2));
    //glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2dUVColor), (void*)sizeof(glm::vec4));
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Setup things related to the screen Transform
    //m_screenTransformUniform = glGetUniformLocation(m_shaderProgram, "screenTransform");
    //setScreenSize(screenSize);
}

SpriteBatcher::~SpriteBatcher() = default;



void SpriteBatcher::Draw(const std::array<MATH::Vector4f, 6>& verts, const RENDER::AtlasRect& uv, MATH::Vector4f color, 
	std::shared_ptr<RENDER::TextureInterface> texture, std::shared_ptr<RENDER::ShaderInterface> shader, bool is3D) {
    if (mTexture != texture || mShader != shader || mIs3D != is3D) {
        Flush();
		mTexture = texture;
		mShader = shader;
		mIs3D = is3D;
    }
	
	mVertexBuffer.push_back(BatchVertex({ verts[0].x, verts[0].y, verts[0].z }, { uv.mX, 1.0f - uv.mY }, color));
	mVertexBuffer.push_back(BatchVertex({ verts[1].x, verts[1].y, verts[1].z }, { uv.mX + uv.mW,  1.0f - uv.mY }, color));
	mVertexBuffer.push_back(BatchVertex({ verts[2].x, verts[2].y, verts[2].z }, { uv.mX, 1.0f - uv.mY - uv.mH }, color));
	mVertexBuffer.push_back(BatchVertex({ verts[3].x, verts[3].y, verts[3].z }, { uv.mX, 1.0f - uv.mY - uv.mH }, color));
	mVertexBuffer.push_back(BatchVertex({ verts[4].x, verts[4].y, verts[4].z }, { uv.mX + uv.mW,1.0f - uv.mY - uv.mH }, color));
	mVertexBuffer.push_back(BatchVertex({ verts[5].x, verts[5].y, verts[5].z }, { uv.mX + uv.mW,1.0f - uv.mY }, color));
}


void SpriteBatcher::Draw(const std::array<BatchVertex, 6>& verts, std::shared_ptr<RENDER::TextureInterface> texture, std::shared_ptr<RENDER::ShaderInterface> shader, bool is3D) {
	if (mTexture != texture || mShader != shader || mIs3D != is3D) {
		Flush();
		mTexture = texture;
		mShader = shader;
		mIs3D = is3D;
	}

	mVertexBuffer.push_back(verts[0]);
	mVertexBuffer.push_back(verts[1]);
	mVertexBuffer.push_back(verts[2]);
	mVertexBuffer.push_back(verts[3]);
	mVertexBuffer.push_back(verts[4]);
	mVertexBuffer.push_back(verts[5]);
}

void SpriteBatcher::Flush() {
    if (mVertexBuffer.empty() || !mTexture || !mShader) {
        return;
    }

    // Set the current shader program.
	mShader->bind();
	std::static_pointer_cast<RENDER::TextureGl>(mTexture)->bind(0);
	std::static_pointer_cast<RENDER::VertexBufferGl<BatchVertex>>(mVbo)->bufferData(mVertexBuffer.size() * sizeof(BatchVertex), mVertexBuffer.data(), GL_STATIC_DRAW);

	//TODO: get screen size
	//TODO: check is 3D
	MATH::Matrix4f projection = MATH::Matrix4f::CreateOrthographic(0.0f, static_cast<float>(800), static_cast<float>(600), 0.0f, -1, 1);
	std::static_pointer_cast<RENDER::ShaderGl>(mShader)->setMat4("u_engine_projection", projection);


	mVertexArray->bind();
    glDrawArrays(GL_TRIANGLES, 0, mVertexBuffer.size());
	mVertexArray->unbind();

	mShader->unbind();
    mVertexBuffer.clear();
}


//BATCHE END


SpriteComponent::SpriteComponent(Object& obj) : Component(obj) {
}

SpriteComponent::SpriteComponent(Object& obj, const std::string& path): Component(obj) {
	setTexture(path);
}

void SpriteComponent::setTexture(std::string path) {
	mPath = path;
#ifdef OPENGL_BACKEND
	mTexture = RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createFromFile(path, true);
	obj->getTransform()->getTransform().setLocalSize(
		{ static_cast<RENDER::TextureGl*>(mTexture.get())->width, static_cast<RENDER::TextureGl*>(mTexture.get())->height });
#endif
}

void SpriteComponent::setTextureAtlas(std::string path) {
	mPath = path;
#ifdef OPENGL_BACKEND
	mTexture = RESOURCES::ServiceManager::Get<RESOURCES::TextureLoader>().createAtlasFromFile(path, true);
	//obj->getTransform()->getTransform().setLocalSize(
	//	{ static_cast<RENDER::TextureGl*>(mTexture.get())->width, static_cast<RENDER::TextureGl*>(mTexture.get())->height });
#endif
}

std::string SpriteComponent::getTexture() {
	return mPath;
}

void SpriteComponent::setAtlasPiece(std::string name)
{
	auto rect = static_cast<RENDER::TextureAtlas*>(mTexture.get())->getPiece(name);
	obj->getTransform()->getTransform().setLocalSize({ rect.mW, rect.mH });
	mTexturePiece = name;
}

std::string SpriteComponent::getAtlasPiece()
{
	return mTexturePiece;
}
void SpriteAnimateComponent::updateAnim()
{
	mCutTime -= TIME::Timer::GetInstance().getDeltaTime().count() * mTimeScale;
	if (mCutTime < 0.0f) {
		mCutTime = mTime;
		nextFrame();
	}

	//TODO: add atlas support

	const float frameWidth = 1.f / mGridSize.x;
	const float frameHeight = 1.f / mGridSize.y;

	const int row = mCurrentFreme / mGridSize.x;
	const int col = mCurrentFreme % (int)mGridSize.x;

	mUVRect.mX = frameWidth * col;
	mUVRect.mY = frameHeight * row;
	mUVRect.mW = frameWidth;
	mUVRect.mH = frameHeight;
}

void SpriteAnimateComponent::nextFrame() {
	if (mFrameCount == mCurrentFreme + 1) {
		mCurrentFreme = 0;
	}
	else {
		mCurrentFreme++;
	}
}

SpriteParticleComponent::SpriteParticleComponent(Object& obj): SpriteComponent(obj)
{
	srand(static_cast <unsigned> (time(0)));
}

SpriteParticleComponent::SpriteParticleComponent(Object& obj, const std::string& path): SpriteComponent(obj, path)
{
	srand(static_cast <unsigned> (time(0)));
}

void SpriteParticleComponent::setEmmiters(std::vector<Emmiter> e)
{
	emmiters = std::move(e);

	int count = 0;
	for (auto emmiter : emmiters) {
		count += emmiter.count;
	}
	particles.resize(count);
	freeParticlesId.resize(count);
	for (auto i = 0; i < count; i++) {
		freeParticlesId[i] = i;
	}
}

std::vector<SpriteParticleComponent::Emmiter> SpriteParticleComponent::getEmmiters()
{
	return emmiters;
}

float randFloat0to1()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

float randFloat(float LO, float HI)
{
	return  LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
}

void SpriteParticleComponent::spawnParticle(Emmiter& emmiter, int emmiterId) {
	if (freeParticlesId.empty()) {
		return;
	}
	for (auto i = 0; i < emmiter.spawnCount; ++i) {
		if (emmiter.curParticleCount >= emmiter.count) {
			break;
		}
		auto freeId = freeParticlesId.back();
		freeParticlesId.pop_back();
		auto& newParticle = particles[freeId];

		emmiter.curParticleCount++;
		newParticle.emmiterId = emmiterId;
		newParticle.isAlive = true;
		newParticle.pos = MATH::Vector3f();
		newParticle.life = randFloat(emmiter.lifeTime.x, emmiter.lifeTime.y);
		newParticle.size = randFloat(emmiter.size.x, emmiter.size.y);
		newParticle.weight = randFloat(emmiter.weight.x, emmiter.weight.y);
		const auto angle = MATH::TO_RADIANS(randFloat(emmiter.angle.x, emmiter.angle.y));
		const auto force = randFloat(emmiter.angleForce.x, emmiter.angleForce.y);
		newParticle.speed = MATH::Vector3f(cos(angle), sin(angle), 0.0f) * force;
	}
}

void SpriteParticleComponent::update() {
	auto dt = TIME::Timer::GetInstance().getDeltaTime().count();

	int eid = 0;
	for (auto& emmiter : emmiters) {
		emmiter.curSpawnTime -= dt;
		if (emmiter.curSpawnTime < 0) {
			emmiter.curSpawnTime = emmiter.spawnTime;
			spawnParticle(emmiter, eid);
		}
		eid++;
	}

	int pid = 0;
	for (Particle& p : particles) {
		if (p.life <= 0.0f) {
			if (p.isAlive) {
				p.isAlive = false;
				emmiters[p.emmiterId].curParticleCount--;
				freeParticlesId.push_back(pid);
			}
			pid++;
			continue;
		}
		auto& emmiter = emmiters[p.emmiterId];
		p.life -= dt;
		p.speed += emmiter.gravity * (float)dt * p.weight;
		p.pos += p.speed * (float)dt;
		pid++;
	}
}

LabelComponent::LabelComponent(Object& obj, std::string label, std::shared_ptr<GUI::Font> font) :
	Component(obj), font(font) {
	mLabel = label;

	float width = 0;
	float height = 0;
	for (auto c : label) {
		GUI::Character ch = font->Characters[c];
		width += (ch.Advance >> 6);
		height = std::max(height, static_cast<float>(ch.Size.y));
	}
	obj.getTransform()->getTransform().setLocalSize({ width, height });

	//TODO: uncoment it
	//auto shader = material->getShader();
	////shader = RESOURCES::ShaderLoader::CreateFromFile("Shaders/gui/text.glsl");
	//MATHGL::Matrix4 projection = MATHGL::Matrix4::CreateOrthographic(0.0f, static_cast<float>(800), static_cast<float>(600), 0.0f, 0, 1);
	//shader->bind();
	//shader->setUniformInt("u_engine_text", 0);
	//shader->setUniformMat4("u_engine_projection", projection);


	//glGenVertexArrays(1, &VAO);
	//glGenBuffers(1, &VBO);
	//glBindVertexArray(VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
}

void SpineComponent::setSpine(SpineRefl data)
{
	skelPath = data.skelPath;
	atlasPath = data.atlasPath;
	spine = std::make_shared<RENDER::SPINE::SpineController>(skelPath, atlasPath);

	spine->setAnimation("idle", true);
}

SpineRefl SpineComponent::getSpine()
{
	return { skelPath, atlasPath };
}

SpineComponent::SpineComponent(Object& obj) : Component(obj) {
}

SpineComponent::SpineComponent(Object& obj, std::string skelPath, std::string atlasPath): Component(obj), skelPath(skelPath), atlasPath(atlasPath){
	spine = std::make_shared<RENDER::SPINE::SpineController>(skelPath, atlasPath);
}

//void LabelComponentGui::draw() {
//	if (!isEnabled) {
//		return;
//	}
//	glEnable(GL_CULL_FACE);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	//TODO: uncoment it
//	//auto shader = material->getShader();
//	//shader->bind();
//	//shader->setUniformVec3("textColor", {color.x, color.y, color.z});
//
//	glBindVertexArray(VAO);
//
//	glBindTexture(GL_TEXTURE_2D, font->texture);
//	//TODO: uncoment it
//	std::string::const_iterator c;
//	//shader->setUniformMat4("u_engine_model", obj.transform->globalModel);
//
//	float x = 0;//model(0, 3);
//	float y = 0;//model(1, 3);
//	for (auto c : label) {
//		Character ch = font->Characters[c];
//
//		float xpos = x + ch.Bearing.x * obj.transform->globalScale.x;
//		float ypos = y - (ch.Size.y - ch.Bearing.y) * obj.transform->globalScale.y;
//
//		float w = ch.Size.x * obj.transform->globalScale.x;
//		float h = ch.Size.y * obj.transform->globalScale.y;
//
//
//		// Обновляем VBO для каждого символа
//		float vertices[6][4] = {
//			{ xpos,     ypos + h, ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0 },
//			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0,  ch.Start.y / 1024.0},
//			{ xpos,     ypos,     ch.Start.x / 1024.0,               ch.Start.y / 1024.0 },
//			{ xpos,     ypos + h, ch.Start.x / 1024.0,               (ch.Start.y + ch.Size.y) / 1024.0  },
//			{ xpos + w, ypos + h, (ch.Start.x + ch.Size.x) / 1024.0, (ch.Start.y + ch.Size.y) / 1024.0 },
//			{ xpos + w, ypos,     (ch.Start.x + ch.Size.x) / 1024.0, ch.Start.y / 1024.0 }
//		};
//
//		// Визуализируем текстуру глифа поверх прямоугольника
//		//glBindTexture(GL_TEXTURE_2D, ch.textureID);
//
//		// Обновляем содержимое памяти VBO
//		glBindBuffer(GL_ARRAY_BUFFER, VBO);
//		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
//		glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//		// Рендерим прямоугольник
//		glDrawArrays(GL_TRIANGLES, 0, 6);
//
//		// Теперь производим смещение для отображения следующего глифа (обратите внимание, что данное смещение измеряется в единицах, составляющих 1/64 пикселя)
//		x += (ch.Advance >> 6) * obj.transform->globalScale.x; // побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
//	}
//	glBindVertexArray(0);
//	glBindTexture(GL_TEXTURE_2D, 0);
//}

InteractionComponent::InteractionComponent(Object& obj, float w, float h) : Component(obj), mWidth(w), mHeight(h) {
	//obj.transform->size = size;
	obj.getTransform()->getTransform().setLocalSize({ w, h });
}
bool InteractionComponent::contains(float x, float y) {
	auto left = obj->getTransform()->getWorldPosition().x;
	auto top = obj->getTransform()->getWorldPosition().y;
	auto width = obj->getTransform()->getTransform().getLocalSize().x * obj->getTransform()->getWorldScale().x;
	auto height = obj->getTransform()->getTransform().getLocalSize().y * obj->getTransform()->getWorldScale().y;
	float minX = std::min(left, (left + width));
	float maxX = std::max(left, (left + width));
	float minY = std::min(top, (top + height));
	float maxY = std::max(top, (top + height));
	
	return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
}
//void InteractionComponent::onUpdate(float dt) {
//	auto ev = GuiEventType::NONE;
//	auto mpos = RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().getMousePosition();
//	if (contains(mpos.x, mpos.y)) {
//		if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonPressed(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
//			ev = GuiEventType::PRESS;
//		}
//		else if (RESOURCES::ServiceManager::Get<INPUT_SYSTEM::InputManager>().isMouseButtonReleased(INPUT_SYSTEM::EMouseButton::MOUSE_BUTTON_1)) {
//			ev = GuiEventType::RELEASE;
//		}
//		else {
//			ev = GuiEventType::COVER;
//		}
//	}
//	else {
//		ev = GuiEventType::UNCOVER;
//	}
//
//	if (cur == ev && (cur == GuiEventType::PRESS || cur == GuiEventType::PRESS_CONTINUE)) {
//		if (onPressContinue) onPressContinue();
//	}
//
//	if (cur == ev) {
//		return;
//	}
//	cur = ev;
//	switch (ev) {
//	case GuiEventType::COVER: if (onCover) onCover();  break;
//	case GuiEventType::PRESS: if (onPress) onPress();  break;
//	case GuiEventType::RELEASE: if (onRelease) onRelease();  break;
//	case GuiEventType::UNCOVER: if (onUncover) onUncover();  break;
//	default: break;
//	}
//}
//
//void InteractionComponent::onPreUpdate(float dt) {
//	obj.transform->calculate();
//	if (obj.parent) {
//		globalX = obj.parent->childOffsetX;
//		globalY = obj.parent->childOffsetY;
//	}
//};
//
ClipComponent::ClipComponent(Object& obj, float w, float h) : Component(obj), mWidth(w), mHeight(h) {
	obj.getTransform()->getTransform().setLocalSize({ w, h });
}

ScrollComponent::ScrollComponent(Object& obj, float w, float h): Component(obj), mWidth(w), mHeight(h) {
	obj.getTransform()->getTransform().setLocalSize({ w, h });
}

////void ClipComponent::draw() {
////	
////}
//
//void ClipComponent::onPreUpdate(float dt) {
//	obj.transform->calculate();
//	globalX = obj.transform->globalModel(0, 3);
//	globalY = obj.transform->globalModel(1, 3);
//	obj.childOffsetX = globalX;
//	obj.childOffsetY = globalY;
//	obj.transform->globalModel(0, 3) = 0;
//	obj.transform->globalModel(1, 3) = 0;
//}

#endif


//#include <rttr/registration>
////TODO: think about save texture/textureAtlas
//RTTR_REGISTRATION
//{
//	rttr::registration::class_<IKIGAI::ECS::SpriteComponent>("SpriteComponent")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("TexturePath", &IKIGAI::ECS::SpriteComponent::getTexture, &IKIGAI::ECS::SpriteComponent::setTexture)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("TextureAtlasPath", &IKIGAI::ECS::SpriteComponent::getTexture, &IKIGAI::ECS::SpriteComponent::setTextureAtlas)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("TexturePiece", &IKIGAI::ECS::SpriteComponent::getAtlasPiece, &IKIGAI::ECS::SpriteComponent::setAtlasPiece)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	);
//
//	rttr::registration::class_<IKIGAI::ECS::SpriteAnimateComponent>("SpriteAnimateComponent")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("TexturePath", &IKIGAI::ECS::SpriteAnimateComponent::getTexture, &IKIGAI::ECS::SpriteAnimateComponent::setTexture)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("TextureAtlasPath", &IKIGAI::ECS::SpriteAnimateComponent::getTexture, &IKIGAI::ECS::SpriteAnimateComponent::setTextureAtlas)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("TexturePiece", &IKIGAI::ECS::SpriteAnimateComponent::getAtlasPiece, &IKIGAI::ECS::SpriteAnimateComponent::setAtlasPiece)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//		rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//	)
//	.property("GridSize", &IKIGAI::ECS::SpriteAnimateComponent::getGridSize, &IKIGAI::ECS::SpriteAnimateComponent::setGridSize)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("FrameCount", &IKIGAI::ECS::SpriteAnimateComponent::getFrameCount, &IKIGAI::ECS::SpriteAnimateComponent::setFrameCount)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("TimeScale", &IKIGAI::ECS::SpriteAnimateComponent::mTimeScale)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	);
//
//	rttr::registration::class_<IKIGAI::ECS::SpriteParticleComponent>("SpriteParticleComponent")
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//			)
//		.property("TexturePath", &IKIGAI::ECS::SpriteParticleComponent::getTexture, &IKIGAI::ECS::SpriteParticleComponent::setTexture)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//			rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//			)
//		.property("TextureAtlasPath", &IKIGAI::ECS::SpriteParticleComponent::getTexture, &IKIGAI::ECS::SpriteParticleComponent::setTextureAtlas)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//			rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//			)
//		.property("TexturePiece", &IKIGAI::ECS::SpriteParticleComponent::getAtlasPiece, &IKIGAI::ECS::SpriteParticleComponent::setAtlasPiece)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR | MetaInfo::OPTIONAL_PARAM),
//			rttr::metadata(EditorMetaInfo::EDIT_WIDGET, EditorMetaInfo::WidgetType::STRING)
//			)
//		.property("Emmiters", &IKIGAI::ECS::SpriteParticleComponent::getEmmiters, &IKIGAI::ECS::SpriteParticleComponent::setEmmiters)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//		);
//
//	rttr::registration::class_<IKIGAI::ECS::SpineRefl>("SpineRefl")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("Skel", &IKIGAI::ECS::SpineRefl::skelPath)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("Atlas", &IKIGAI::ECS::SpineRefl::atlasPath)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	);
//
//	rttr::registration::class_<IKIGAI::ECS::SpineComponent>("SpineComponent")
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("Spine", &IKIGAI::ECS::SpineComponent::getSpine, &IKIGAI::ECS::SpineComponent::setSpine)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	);
//
//	rttr::registration::class_<IKIGAI::ECS::SpriteParticleComponent::Emmiter>("Emmiter")
//	(
//	rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE)
//	)
//	.property("Count", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::count)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//	)
//	.property("LocalPos", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::localPos)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//		)
//	.property("Gravity", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::gravity)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//		)
//	.property("Color", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::color)
//	(
//		rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//		)
//		.property("Scale", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::size)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//		.property("Weight", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::weight)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//		.property("Angle", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::angle)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//		.property("AngleForce", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::angleForce)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//		.property("LifeTime", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::lifeTime)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//		.property("Piece", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::piece)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//		.property("SpawnTime", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::spawnTime)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//		.property("SpawnCount", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::spawnCount)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//		.property("Pause", &IKIGAI::ECS::SpriteParticleComponent::Emmiter::curSpawnTime)
//		(
//			rttr::metadata(MetaInfo::FLAGS, MetaInfo::SERIALIZABLE | MetaInfo::USE_IN_EDITOR_COMPONENT_INSPECTOR)
//			)
//	;
//
//	//MATH TODO: move somewhere
//	rttr::registration::class_<IKIGAI::MATHGL::Vector2f>("Vector2f")
//		.property("X", &IKIGAI::MATHGL::Vector2f::x)
//		.property("Y", &IKIGAI::MATHGL::Vector2f::y);
//
//	rttr::registration::class_<IKIGAI::MATHGL::Vector2i>("Vector2i")
//		.property("X", &IKIGAI::MATHGL::Vector2i::x)
//		.property("Y", &IKIGAI::MATHGL::Vector2i::y);
//}
