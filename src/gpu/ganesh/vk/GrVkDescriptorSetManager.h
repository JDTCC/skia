/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkDescriptorSetManager_DEFINED
#define GrVkDescriptorSetManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/ganesh/GrResourceHandle.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorPool.h"
#include "src/gpu/ganesh/vk/GrVkSampler.h"

class GrVkDescriptorSet;
class GrVkGpu;
class GrVkUniformHandler;

/**
 * This class handles the allocation of descriptor sets for a given VkDescriptorSetLayout. It will
 * try to reuse previously allocated descriptor sets if they are no longer in use by other objects.
 */
class GrVkDescriptorSetManager {
public:
    GR_DEFINE_RESOURCE_HANDLE_CLASS(Handle)

    static GrVkDescriptorSetManager* CreateUniformManager(GrVkGpu* gpu);
    static GrVkDescriptorSetManager* CreateSamplerManager(GrVkGpu* gpu, VkDescriptorType type,
                                                          const GrVkUniformHandler&);
    // See GrVkResourceProvider::getZeroSamplerDescriptorSetHandle() for more info on what the zero
    // sampler is for.
    static GrVkDescriptorSetManager* CreateZeroSamplerManager(GrVkGpu* gpu);
    static GrVkDescriptorSetManager* CreateInputManager(GrVkGpu* gpu);

    ~GrVkDescriptorSetManager() {}

    void release(GrVkGpu* gpu);

    VkDescriptorSetLayout layout() const { return fPoolManager.fDescLayout; }

    const GrVkDescriptorSet* getDescriptorSet(GrVkGpu* gpu, const Handle& handle);

    void recycleDescriptorSet(const GrVkDescriptorSet*);

    bool isCompatible(VkDescriptorType type, const GrVkUniformHandler*) const;

    bool isZeroSampler() const;

private:
    struct DescriptorPoolManager {
        DescriptorPoolManager(VkDescriptorSetLayout, VkDescriptorType type,
                              uint32_t descCountPerSet);

        ~DescriptorPoolManager() {
            SkASSERT(!fDescLayout);
            SkASSERT(!fPool);
        }

        bool getNewDescriptorSet(GrVkGpu* gpu, VkDescriptorSet* ds);

        void freeGPUResources(GrVkGpu* gpu);

        VkDescriptorSetLayout  fDescLayout;
        VkDescriptorType       fDescType;
        uint32_t               fDescCountPerSet;
        uint32_t               fMaxDescriptors;
        uint32_t               fCurrentDescriptorCount;
        GrVkDescriptorPool*    fPool;

    private:
        enum {
            kMaxDescriptors = 1024,
            kStartNumDescriptors = 16, // must be less than kMaxUniformDescriptors
        };

        bool getNewPool(GrVkGpu* gpu);
    };

    static GrVkDescriptorSetManager* Create(GrVkGpu* gpu,
                                            VkDescriptorType,
                                            const skia_private::TArray<uint32_t>& visibilities,
                                            const skia_private::TArray<const GrVkSampler*>& immutableSamplers);

    GrVkDescriptorSetManager(GrVkGpu* gpu,
                             VkDescriptorType, VkDescriptorSetLayout, uint32_t descCountPerSet,
                             const skia_private::TArray<uint32_t>& visibilities,
                             const skia_private::TArray<const GrVkSampler*>& immutableSamplers);


    DescriptorPoolManager                    fPoolManager;
    skia_private::TArray<const GrVkDescriptorSet*, true> fFreeSets;
    SkSTArray<4, uint32_t>                   fBindingVisibilities;
    SkSTArray<4, const GrVkSampler*>         fImmutableSamplers;
};

#endif
