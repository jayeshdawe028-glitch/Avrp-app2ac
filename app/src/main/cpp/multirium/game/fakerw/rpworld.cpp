//
// Created by viski on 14.05.2025.
//

#include "RenderWare.h"
#include "game/core/common.h"
#include "vendor/armhook/patch.h"

RpClump* RpClumpForAllAtomics(RpClump* clump, RpAtomicCallBack callback, void* data) {
    if(clump)
        return CHook::CallFunction<RpClump*>(g_libGTASA + (VER_x32 ? 0x00213D66 + 1 : 0x2BA020), clump, callback, data);
}

RpGeometry* RpGeometryForAllMaterials(RpGeometry* geometry, RpMaterialCallBack fpCallBack, void* data) {
    return CHook::CallFunction<RpGeometry*>(g_libGTASA + (VER_x32 ? 0x00215F30 + 1 : 0x2BCE78), geometry, fpCallBack, data);
}

RwBool RpClumpDestroy(RpClump* clump) {
    return CHook::CallFunction<RwBool>(g_libGTASA + (VER_x32 ? 0x0021458C + 1 : 0x2BAAB0), clump);
}

RpClump* RpClumpRender(RpClump* clump) {
    return CHook::CallFunction<RpClump*>("_Z13RpClumpRenderP7RpClump", clump);
}

RpLight* RpLightCreate(RwInt32 type) {
    return CHook::CallFunction<RpLight*>(g_libGTASA + (VER_x32 ? 0x00216DB0 + 1 : 0x2BE078), type);
}

RwBool RpLightDestroy(RpLight* light) {
    return CHook::CallFunction<RwBool>(g_libGTASA + (VER_x32 ? 0x216EF4 + 1 : 0x2BE210), light);
}

RpWorld* RpWorldCreate(RwBBox* boundingBox) {
    return CHook::CallFunction<RpWorld*>(g_libGTASA + (VER_x32 ? 0x0021D144 + 1 : 0x2C6714), boundingBox);
}

RpWorld* RpWorldAddCamera(RpWorld* world, RwCamera* camera) {
    return CHook::CallFunction<RpWorld*>(g_libGTASA + (VER_x32 ? 0x0021DF84 + 1 : 0x2C78F0), world, camera);
}

RpLight* RpLightSetColor(RpLight* light, const RwRGBAReal* color) {
    return CHook::CallFunction<RpLight*>(g_libGTASA + (VER_x32 ? 0x00216746 + 1 : 0x2BD930), light, color);
}

RpAtomic* AtomicDefaultRenderCallBack(RpAtomic* atomic) {
    return CHook::CallFunction<RpAtomic*>(g_libGTASA + (VER_x32 ? 0x002138DC + 1 : 0x2B9B08), atomic);
}

RpWorld* RpWorldAddLight(RpWorld* world, RpLight* light) {
    return CHook::CallFunction<RpWorld*>(g_libGTASA + (VER_x32 ? 0x0021E7B0 + 1 : 0x2C8588), world, light);
}

RpWorld* RpWorldRemoveLight(RpWorld* world, RpLight* light) {
    return CHook::CallFunction<RpWorld*>(g_libGTASA + (VER_x32 ? 0x0021E7F4 + 1 : 0x2C85F4), world, light);
}

RwBool RpAtomicDestroy(RpAtomic* atomic) {
    return CHook::CallFunction<RwBool>(g_libGTASA + (VER_x32 ? 0x0021416C + 1 : 0x2BA534), atomic);
}

void RpClumpGtaCancelStream() {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x5D0BA8 + 1 : 0x6F4E38));
}

RwInt32 RpGeometryRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB) {
    return CHook::CallFunction<RwInt32>("_Z24RpGeometryRegisterPluginijPFPvS_iiES1_PFS_S_PKviiE", size, pluginID, constructCB, destructCB, copyCB);
}

RwInt32 RpGeometryRegisterPluginStream(RwUInt32 pluginID, RwPluginDataChunkReadCallBack readCB, RwPluginDataChunkWriteCallBack writeCB, RwPluginDataChunkGetSizeCallBack getSizeCB) {
    return CHook::CallFunction<RwInt32>("_Z30RpGeometryRegisterPluginStreamjPFP8RwStreamS0_iPviiEPFS0_S0_iPKviiEPFiS5_iiE", pluginID, readCB, writeCB, getSizeCB);
}

RpGeometry* RpGeometryLock(RpGeometry* geometry, RwInt32 lockMode) {
    return CHook::CallFunction<RpGeometry*>("_Z14RpGeometryLockP10RpGeometryi", geometry, lockMode);
}

RpGeometry* RpGeometryUnlock(RpGeometry* geometry) {
    return CHook::CallFunction<RpGeometry*>("_Z16RpGeometryUnlockP10RpGeometry", geometry);
}

char* RxNodeDefinitionGetD3D9AtomicAllInOne() {
    return CHook::CallFunction<char*>("_Z39RxNodeDefinitionGetOpenGLAtomicAllInOnev");
}

void _rpAtomicResyncInterpolatedSphere(RpAtomic *atomic)
{
    RwInt32 numMorphTargets;
    RpMorphTarget *morphTarget;
    RwSphere boundingSphere;

    RpGeometry* geometry = atomic->geometry;
    if (geometry)
    {
        int startMorphTarget = (uint16_t)atomic->interpolator.startMorphTarget;
        int endMorphTarget = (uint16_t)atomic->interpolator.endMorphTarget;

        numMorphTargets = geometry->numMorphTargets;

        if (startMorphTarget != endMorphTarget)
        {
            if (numMorphTargets > startMorphTarget && numMorphTargets > endMorphTarget)
            {
                morphTarget = geometry->morphTarget;

                float *v7 = (float *)&morphTarget[endMorphTarget];
                float *v8 = (float *)&morphTarget[startMorphTarget];
                float v9 = atomic->interpolator.recipTime * atomic->interpolator.position;

                atomic->boundingSphere.radius = v8[5] + (v9 * (v7[5] - v8[5]));

                float v10 = v7[2] - v8[2];
                float v12 = v7[3] - v8[3];
                float v13 = v7[4];
                float v14 = v8[4];

                atomic->boundingSphere.center.x = v9 * v10 + v8[2];
                atomic->boundingSphere.center.y = v9 * v12 + v8[3];
                atomic->boundingSphere.center.z = v9 * (v13 - v14) + v8[4];

                atomic->object.object.privateFlags |= 1;
                atomic->interpolator.flags &= ~2u;
                return;
            }
        }

        RpMorphTarget* v17 = geometry->morphTarget;
        if (numMorphTargets <= startMorphTarget || numMorphTargets <= endMorphTarget)
            boundingSphere = v17->boundingSphere;
        else
            boundingSphere = v17[startMorphTarget].boundingSphere;

        atomic->boundingSphere = boundingSphere;
        atomic->object.object.privateFlags |= 1;
        atomic->interpolator.flags &= ~2u;
    }
}

RwInt32 _rwPluginRegistryAddPlugin(
        RwPluginRegistry *reg,
        RwInt32 size,
        RwUInt32 pluginID,
        RwPluginObjectConstructor constructCB,
        RwPluginObjectDestructor destructCB,
        RwPluginObjectCopy copyCB)
{
    return CHook::CallFunction<RwInt32>("_Z26_rwPluginRegistryAddPluginP16RwPluginRegistryijPFPvS1_iiES3_PFS1_S1_PKviiE",
            reg,
            size,
            pluginID,
            constructCB,
            destructCB,
            copyCB);
}

RwInt32 RpAtomicRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB)
{
    RwPluginRegistry atomicTKList = *(RwPluginRegistry*)(g_libGTASA + (VER_x32 ? 0x683B48 : 0x85BE80));
    return _rwPluginRegistryAddPlugin(&atomicTKList, size, pluginID, constructCB, destructCB, copyCB);
}

RwInt32 RwFrameRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB)
{
    RwPluginRegistry frameTKList = *(RwPluginRegistry*)(g_libGTASA + (VER_x32 ? 0x682554 : 0x85A628));
    return _rwPluginRegistryAddPlugin(&frameTKList, size, pluginID, constructCB, destructCB, copyCB);
}

RwInt32 RpClumpRegisterPlugin(RwInt32 size, RwUInt32 pluginID, RwPluginObjectConstructor constructCB, RwPluginObjectDestructor destructCB, RwPluginObjectCopy copyCB)
{
    RwPluginRegistry clumpTKList = *(RwPluginRegistry*)(g_libGTASA + (VER_x32 ? 0x683B60 : 0x85BEA0));
    return _rwPluginRegistryAddPlugin(&clumpTKList, size, pluginID, constructCB, destructCB, copyCB);
}

RpMaterial* RpMaterialSetTexture(RpMaterial *material, RwTexture *texture)
{
    RwTexture* tex = material->texture;
    if ( tex != texture )
    {
        if ( texture )
        {
            ++texture->refCount;
            tex = material->texture;
        }
        if ( tex )
            RwTextureDestroy(tex);
        material->texture = texture;
    }
    return material;
}