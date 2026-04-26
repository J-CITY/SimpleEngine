#include "materialInterface.h"

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
