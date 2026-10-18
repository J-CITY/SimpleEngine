#include "materialInterface.h"
#include "driverInterface.h"

IKIGAI::RENDER::BindlessResourceHandle IKIGAI::RENDER::MaterialInterface::setBindlessTexture(
    const std::string& tableName,const std::string& indexUniform,std::shared_ptr<TextureInterface> texture) {
    const auto shader=getShader();
    if (!shader || shader->bindingMode()!=ShaderBindingMode::Bindless) throw std::runtime_error("Material shader is not bindless");
    const auto& reflection=shader->getReflection();
    const auto& uniform=reflection.mUniforms.at(reflection.mNameToUniforms.at(tableName));
    if (!shader->isBindlessResource(uniform)) throw std::runtime_error("Material resource is not a bindless array");
    auto handle=DriverInterface::Get()->registerBindlessTexture(uniform.mBind,std::move(texture),shader->descriptorCount(uniform));
    set(indexUniform,static_cast<int>(handle.index));return handle;
}

IKIGAI::RENDER::MaterialInterface::MaterialInterface(const MaterialResource& res) :
	mPath(res.path),
	mBlendable(res.Blendable),
	mBackfaceCulling(res.BackfaceCulling),
	mFrontfaceCulling(res.FrontfaceCulling),
	mDepthTest(res.DepthTest),
	mDepthWriting(res.DepthWriting),
	mColorWriting(res.ColorWriting),
	mGpuInstances(res.GpuInstances),
	mIsDeferred(res.IsDeferred),
	mDepthFunc(res.DepthFunc) {
}
