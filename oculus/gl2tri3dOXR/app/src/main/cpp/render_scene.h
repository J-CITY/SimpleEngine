#pragma once

int init_gles_scene ();
int render_gles_scene (XrCompositionLayerProjectionView &layerView,
                       render_target_t &rtarget, XrPosef &stagePose,
                       uint32_t viewID);
