#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include "renderModule/backends/meshFactory.h"
#include "renderModule/backends/interface/driverInterface.h"
#include "renderModule/backends/interface/meshInterface.h"

namespace IKIGAI::RENDER {
// Run against the active native driver, then validate actual GPU readback.
inline void RunBindlessVertexSmoke(DriverInterface& driver) {
    constexpr uint32_t width=512,height=128;
    TextureResource targetResource;targetResource.width=width;targetResource.height=height;
    targetResource.channels=4;targetResource.pixelType=PixelFormat::RGBA_FLOAT;
    targetResource.isFloat=true;targetResource.useMipmap=false;targetResource.storage=true;
    auto target=driver.createTexture(targetResource);
    auto depthResource=targetResource;depthResource.texType=TextureType::DEPTH;
    depthResource.channels=1;depthResource.pixelType=PixelFormat::DEPTH32_FLOAT;depthResource.storage=false;
    auto depth=driver.createTexture(depthResource);
    auto framebuffer=driver.createFrameBuffer({target},depth);
    struct Position { float x,y; };
    struct Instance { float columns[16]; uint8_t color[4]; uint32_t id; int32_t signedId; };
    std::vector<Position> positions={{-.18f,-.35f},{.18f,-.35f},{0,.35f}, {2,-.35f},{2.3f,-.35f},{2.15f,.35f}};
    std::vector<Instance> instances(4);
    std::array<std::array<uint8_t,4>,4> colors={{{255,0,0,255},{0,255,0,255},{0,0,255,255},{255,255,0,255}}};
    for (uint32_t i=0;i<instances.size();++i) {
        auto& instance=instances[i];instance.columns[0]=instance.columns[5]=instance.columns[10]=instance.columns[15]=1;
        instance.columns[12]=-.75f+.5f*i;instance.id=i;instance.signedId=-7;
        std::copy(colors[i].begin(),colors[i].end(),instance.color);
    }
    const std::vector<uint32_t> indices={3,4,5,0,1,2};
    MeshData data;data.vertexCount=positions.size();data.indices=indices;
    data.layout.streams={{0,sizeof(Position),VertexInputRate::PerVertex},{1,sizeof(Instance),VertexInputRate::PerInstance}};
    data.layout.attributes={{0,0,VertexFormat::Float2,0}};
    for (uint32_t column=0;column<4;++column) data.layout.attributes.push_back({2+column,1,VertexFormat::Float4,column*16});
    data.layout.attributes.push_back({6,1,VertexFormat::UNorm8x4,offsetof(Instance,color)});
    data.layout.attributes.push_back({7,1,VertexFormat::UInt,offsetof(Instance,id)});
    data.layout.attributes.push_back({8,1,VertexFormat::Int,offsetof(Instance,signedId)});
    data.streams={VertexStreamData::From(0,positions),VertexStreamData::From(1,instances)};
    auto mesh=CreateMesh(data);
    const std::string vertex=R"(#version 450
layout(location=0) in vec2 Position;
layout(location=2) in mat4 Transform;
layout(location=6) in vec4 Color;
layout(location=7) in uint Id;
layout(location=8) in int SignedId;
layout(location=0) out vec4 VColor;
layout(location=1) flat out uint VId;
void main(){gl_Position=Transform*vec4(Position,0,1);VColor=SignedId==-7?Color:vec4(0);VId=Id;}
)";
    const std::string boundFragment=R"(#version 450
layout(location=0) in vec4 VColor;
layout(location=1) flat in uint VId;
layout(location=0) out vec4 Output;
void main(){vec4 palette[4]=vec4[4](vec4(1,0,0,1),vec4(0,1,0,1),vec4(0,0,1,1),vec4(1,1,0,1));Output=VColor*palette[VId&3u];}
)";
    ShaderResource bound;bound.path="diagnostic/custom-instance";
    bound.sources={{ShaderType::VERTEX,vertex},{ShaderType::FRAGMENT,boundFragment}};
    auto ordinary=driver.createShader(bound);
    auto prepare=[&](const std::shared_ptr<ShaderInterface>& shader) {
        driver.setFrameBuffer(framebuffer);driver.setViewport({{0,0},{width,height}});
        driver.resetScissor();driver.resetDepth();driver.resetStencil();driver.resetBlending();driver.setCull(CullFace::NONE);
        driver.setPrimitiveMode(PrimitiveMode::TRIANGLES);driver.setShader(shader);
        driver.setClearColor(0,0,0,1);driver.clear(true,true,false);
    };
    auto finish=[&](const char* name,bool onlyLast=false) {
        driver.resetFrameBuffer();driver.submit();
        auto values=driver.readTexture(target,0,0);
        if (values.size()!=size_t(width)*height*4) throw std::runtime_error("Invalid instance readback size");
        for (uint32_t i=0;i<4;++i) {
            const size_t pixel=(size_t(height/2)*width+(64+i*128))*4;
            for (unsigned c=0;c<3;++c) {
                const float expected=onlyLast&&i!=3?0.f:colors[i][c]/255.f;
                if (!std::isfinite(values[pixel+c]) || std::abs(values[pixel+c]-expected)>.03f)
                    throw std::runtime_error(std::string("Instance GPU mismatch: ")+name+" instance="+std::to_string(i)+" channel="+std::to_string(c)+" value="+std::to_string(values[pixel+c]));
                if (std::string(name)=="bindless-compute-nonuniform" && std::abs(values[pixel+4+c]-1.f)>.03f)
                    throw std::runtime_error("Nonuniform buffer indexing failed for adjacent compute invocation");
            }
        }
        std::cout<<"BINDLESS_VERTEX_GPU_OK case="<<name<<"\n";
    };
    prepare(ordinary);mesh->bind();driver.drawIndexed(3,3,4);finish("indexed-offset-low-level");
    prepare(ordinary);mesh->bind();driver.draw(3,0,4);finish("non-indexed");
    prepare(ordinary);driver.draw(*mesh,PrimitiveMode::TRIANGLES,4);finish("mesh-overload");
    prepare(ordinary);mesh->bind();driver.draw(DrawArguments{3,3,1,0,3,true});finish("first-instance",true);
    prepare(ordinary);mesh->bind();driver.draw(DrawArguments{3,0,4,-3,0,true});finish("base-vertex");
    prepare(ordinary);mesh->bind();driver.drawIndexed(3,3,0);
    driver.draw(DrawArguments{3,3,4,0,0,true});finish("zero-then-instanced");
    instances[2].signedId=0;
    mesh->updateVertexStream(1,std::as_bytes(std::span(instances)),instances.size());
    prepare(ordinary);mesh->bind();driver.drawIndexed(3,3,4);
    const auto blue=colors[2];colors[2]={0,0,0,255};finish("instance-buffer-update");colors[2]=blue;
    instances[2].signedId=-7;mesh->updateVertexStream(1,std::as_bytes(std::span(instances)),instances.size());
    // The same program must get another native layout, rather than a cached PSO with old offsets.
    auto alternateData=data;
    alternateData.layout.streams[1].binding=2;
    for (auto& attribute : alternateData.layout.attributes) if (attribute.binding==1) attribute.binding=2;
    alternateData.streams[1].binding=2;
    auto alternate=CreateMesh(alternateData);
    prepare(ordinary);alternate->bind();driver.drawIndexed(3,3,4);
    mesh->bind();driver.drawIndexed(3,3,4);finish("same-shader-two-layouts");
    bool rejected=false;
    try {mesh->bind();driver.draw(DrawArguments{3,3,5,0,0,true});} catch(const std::runtime_error&) {rejected=true;}
    if (!rejected) throw std::runtime_error("Instance stream overflow accepted");
    bool invalidLayout=false;
    try {auto invalid=data;invalid.layout.attributes.front().offset=8;CreateMesh(invalid);} catch(const std::invalid_argument&) {invalidLayout=true;}
    if (!invalidLayout) throw std::runtime_error("Out-of-stride attribute accepted");
    prepare(ordinary);mesh->bind();auto wrongLayout=data.layout;
    for (auto& attribute : wrongLayout.attributes) if (attribute.location==7) attribute.format=VertexFormat::Int;
    driver.setVertexStreams(wrongLayout,driver.mVertexStreams);
    bool wrongType=false;try {driver.drawIndexed(3,3,4);} catch(const std::runtime_error&) {wrongType=true;}
    if (!wrongType) throw std::runtime_error("Signedness mismatch accepted");
    mesh->bind();driver.drawIndexed(3,3,4);finish("invalid-layout-recovery");
    std::vector<Vertex> legacyVertices;
    for (const auto& position : positions) legacyVertices.emplace_back(MATH::Vector3f{position.x,position.y,0});
    auto legacyMesh=CreateMesh(legacyVertices,indices);
    auto legacyResource=bound;legacyResource.path="diagnostic/legacy-instance";
    legacyResource.sources[ShaderType::VERTEX]=R"(#version 450
layout(location=0) in vec3 Position;
layout(location=0) out vec4 VColor;
layout(location=1) flat out uint VId;
void main(){VId=uint(gl_InstanceIndex);gl_Position=vec4(Position,1);gl_Position.x+=-.75+.5*float(VId);
vec4 palette[4]=vec4[4](vec4(1,0,0,1),vec4(0,1,0,1),vec4(0,0,1,1),vec4(1,1,0,1));VColor=palette[VId&3u];}
)";
    auto legacyShader=driver.createShader(legacyResource);
    prepare(legacyShader);legacyMesh->bind();driver.drawIndexed(3,3,4);finish("legacy-after-custom");
    prepare(ordinary);mesh->bind();driver.drawIndexed(3,3,4);finish("custom-after-legacy");
    const auto caps=driver.bindlessCapabilities();
    std::cout<<"BINDLESS_CAPABILITIES textures="<<caps.sampledTextures<<" buffers="<<caps.storageBuffers<<" runtime="<<caps.runtimeArrays<<"\n";
    if (caps.sampledTextures) {
        ShaderResource bindless=bound;bindless.path="diagnostic/bindless-instance";
        bindless.bindingMode=ShaderBindingMode::Bindless;
        bindless.sources[ShaderType::FRAGMENT]=R"(#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(location=0) in vec4 VColor;
layout(location=1) flat in uint VId;
layout(location=0) out vec4 Output;
layout(set=0,binding=0) uniform sampler2D TextureTable[4];
layout(set=0,binding=1) uniform sampler2D LocalTexture;
void main(){Output=VColor*texture(TextureTable[nonuniformEXT(VId&3u)],vec2(.5))*texture(LocalTexture,vec2(.5));}
)";
        auto shader=driver.createShader(bindless);
        const auto& tableReflection=shader->getReflection();
        const auto& array=tableReflection.mUniforms.at(tableReflection.mNameToUniforms.at("TextureTable"));
        if (!shader->isBindlessResource(array) || shader->descriptorCount(array)!=4)
            throw std::runtime_error("Bindless texture table was lost during compilation");
        std::vector<std::shared_ptr<TextureInterface>> textures;
        std::vector<BindlessResourceHandle> handles;
        for (uint32_t i=0;i<4;++i) {
            TextureResource resource;resource.width=resource.height=1;resource.channels=4;resource.useMipmap=false;
            resource.minFilter=resource.magFilter=MinMagFilter::NEAREST;
            resource.colorData.assign(colors[i].begin(),colors[i].end());
            auto texture=driver.createTexture(resource);textures.push_back(texture);
            handles.push_back(driver.registerBindlessTexture(0,texture,4));
        }
        TextureResource white;white.width=white.height=1;white.channels=4;white.useMipmap=false;white.colorData={255,255,255,255};
        auto localTexture=driver.createTexture(white);driver.setTexture(1,localTexture);
        prepare(shader);mesh->bind();driver.drawIndexed(3,3,4);finish("bindless-nonuniform");
        prepare(ordinary);mesh->bind();driver.drawIndexed(3,3,4);
        driver.setShader(shader);driver.drawIndexed(3,3,4);driver.setShader(ordinary);driver.drawIndexed(3,3,4);
        finish("mixed-bound-bindless");
        shader->recompile(bindless);
        prepare(shader);mesh->bind();driver.drawIndexed(3,3,4);finish("bindless-recompile");
        prepare(shader);mesh->bind();driver.drawIndexed(3,3,4);
        driver.releaseBindlessResource(handles[1]);textures[1].reset();
        driver.drawIndexed(3,3,4);colors[1]={0,0,0,255};finish("release-in-flight-default");
        bool stale=false;try {driver.releaseBindlessResource(handles[1]);} catch(const std::invalid_argument&) {stale=true;}
        if (!stale) throw std::runtime_error("Stale bindless handle accepted");
        bool overflow=false;try {TextureResource valid;valid.width=valid.height=1;valid.channels=4;valid.useMipmap=false;valid.colorData={255,255,255,255};driver.registerBindlessTexture(0,driver.createTexture(valid),4);} catch(const std::exception&) {overflow=true;}
        if (!overflow) throw std::runtime_error("Bindless capacity overflow accepted");
        if (caps.runtimeArrays) {
            auto runtime=bindless;runtime.path="diagnostic/bindless-runtime-array";runtime.bindlessCapacity=4;
            auto& source=runtime.sources[ShaderType::FRAGMENT];
            source.replace(source.find("TextureTable[4]"),std::string("TextureTable[4]").size(),"TextureTable[]");
            auto runtimeShader=driver.createShader(runtime);
            prepare(runtimeShader);mesh->bind();driver.drawIndexed(3,3,4);finish("bindless-runtime-array");
        }
    } else {
        auto unsupported=bound;unsupported.bindingMode=ShaderBindingMode::Bindless;
        bool rejected=false;try {driver.createShader(unsupported);} catch(const std::runtime_error&) {rejected=true;}
        if (!rejected) throw std::runtime_error("Unsupported bindless shader accepted");
        std::cout<<"BINDLESS_UNSUPPORTED_VERIFIED\n";
    }
    if (driver.supportsVertexPulling()) {
        struct Position4 {float x,y,z,w;};
        std::vector<Position4> words;for (const auto& position : positions) words.push_back({position.x,position.y,0,1});
        auto buffer=driver.createStorageBuffer(words.data(),words.size(),sizeof(Position4));
        auto pulling=bound;pulling.path="diagnostic/bound-pulling";pulling.vertexFetchMode=VertexFetchMode::Pulling;
        pulling.sources[ShaderType::VERTEX]=R"(#version 450
layout(std430,set=0,binding=5) readonly buffer Positions {vec4 words[];} VertexData;
layout(location=0) out vec4 VColor;
layout(location=1) flat out uint VId;
void main(){VId=uint(gl_InstanceIndex);vec4 p=VertexData.words[gl_VertexIndex];p.x+=-.75+.5*float(VId);gl_Position=p;
vec4 palette[4]=vec4[4](vec4(1,0,0,1),vec4(0,1,0,1),vec4(0,0,1,1),vec4(1,1,0,1));VColor=palette[VId&3u];}
)";
        auto shader=driver.createShader(pulling);colors[1]={0,255,0,255};
        prepare(shader);driver.setStorageBuffer(5,buffer);mesh->bind();driver.drawIndexed(3,3,4);finish("bound-vertex-pulling");
        prepare(ordinary);mesh->bind();driver.drawIndexed(3,3,4);finish("bound-pulling-to-attributes");
    }
    if (caps.storageBuffers) {
        struct PackedPosition {float x,y,z,w;};
        std::vector<PackedPosition> storagePositions;
        for (const auto& position : positions) storagePositions.push_back({position.x,position.y,0,1});
        auto buffer=driver.createStorageBuffer(storagePositions.data(),storagePositions.size(),sizeof(PackedPosition));
        driver.registerBindlessBuffer(2,buffer,2);
        ShaderResource pulling;pulling.path="diagnostic/bindless-pulling";
        pulling.bindingMode=ShaderBindingMode::Bindless;pulling.vertexFetchMode=VertexFetchMode::Pulling;
        pulling.sources[ShaderType::VERTEX]=R"(#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(std430,set=0,binding=2) readonly buffer Positions {vec4 words[];} VertexTables[2];
layout(location=0) out vec4 VColor;
layout(location=1) flat out uint VId;
void main(){VId=uint(gl_InstanceIndex);vec4 p=VertexTables[nonuniformEXT(VId&1u)].words[gl_VertexIndex];
p.x+=-.75+.5*float(VId);gl_Position=p;
vec4 palette[4]=vec4[4](vec4(1,0,0,1),vec4(0,1,0,1),vec4(0,0,1,1),vec4(1,1,0,1));VColor=palette[VId&3u];}
)";
        pulling.sources[ShaderType::FRAGMENT]=boundFragment;
        auto shader=driver.createShader(pulling);colors[1]={0,255,0,255};
        prepare(shader);mesh->bind();driver.drawIndexed(3,3,4);finish("bindless-vertex-pulling");
        prepare(ordinary);mesh->bind();driver.drawIndexed(3,3,4);finish("pulling-to-attributes");
        std::vector<PackedPosition> palette={{1,0,0,1},{0,1,0,1},{0,0,1,1},{1,1,0,1}};
        auto paletteBuffer=driver.createStorageBuffer(palette.data(),palette.size(),sizeof(PackedPosition));
        driver.registerBindlessBuffer(4,paletteBuffer,2);
        for (auto& value : palette) value={1,1,1,1};
        auto whiteBuffer=driver.createStorageBuffer(palette.data(),palette.size(),sizeof(PackedPosition));
        driver.registerBindlessBuffer(4,whiteBuffer,2);
        ShaderResource compute;compute.path="diagnostic/bindless-compute";compute.bindingMode=ShaderBindingMode::Bindless;
        compute.sources[ShaderType::COMPUTE]=R"(#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(local_size_x=8,local_size_y=1,local_size_z=1) in;
layout(std430,set=0,binding=4) readonly buffer Palette {vec4 colors[];} Payload[2];
layout(std430,set=0,binding=6) readonly buffer LocalPalette {vec4 multiplier[];} LocalData;
layout(rgba32f,set=0,binding=3) uniform writeonly image2D Result;
void main(){uvec2 p=gl_GlobalInvocationID.xy;imageStore(Result,ivec2(p),Payload[nonuniformEXT(p.x&1u)].colors[(p.x/128u)&3u]*LocalData.multiplier[0]);}
)";
        auto computeShader=driver.createShader(compute);
        prepare(computeShader);driver.setTexture(3,target);driver.setStorageBuffer(6,whiteBuffer);driver.dispatch(width/8,height,1);
        finish("bindless-compute-nonuniform");
    }
    driver.resetVertexStreams();driver.resetFrameBuffer();driver.resetViewport();driver.resetScissor();
}
}
