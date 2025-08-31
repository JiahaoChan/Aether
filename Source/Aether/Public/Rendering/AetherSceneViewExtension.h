/**
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 *		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
 */

#pragma once

#include "SceneViewExtension.h"

//#include "AetherWorldSubsystem.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FAetherViewParameters, AETHER_API)
	SHADER_PARAMETER(float, CustomParamA)
	SHADER_PARAMETER(FVector4f, CustomParamB)
END_UNIFORM_BUFFER_STRUCT()

class FAetherSceneViewExtension : public FSceneViewExtensionBase
{
public:
	TUniformBufferRef<FAetherViewParameters> AetherViewUniformBuffer;
	
	TUniquePtr<FAetherViewParameters> CachedViewUniformShaderParameters;
	TRDGUniformBufferRef<FAetherViewParameters> RDG;
	
	FAetherSceneViewExtension(const FAutoRegister& AutoRegister);
	
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	
	virtual void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;
	
	//virtual void PreRenderBasePass_RenderThread(FRDGBuilder& GraphBuilder, bool bDepthBufferIsPopulated) override;
	
	//virtual void PostRenderBasePassMobile_RenderThread(FRHICommandList& RHICmdList, FSceneView& InView) override;
	
	virtual void PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;
	
private:
	inline static TSharedPtr<FAetherSceneViewExtension, ESPMode::ThreadSafe> Instance;
	
	struct FStaticConstructor
	{
		FStaticConstructor()
		{
			FCoreDelegates::OnPostEngineInit.AddLambda([]()
			{ 
				Instance = FSceneViewExtensions::NewExtension<FAetherSceneViewExtension>();
			});
			FCoreDelegates::OnEnginePreExit.AddLambda([]()
			{ 
				Instance = nullptr;
			});
		}
	};
	
	static inline FStaticConstructor StaticConstructor;
};