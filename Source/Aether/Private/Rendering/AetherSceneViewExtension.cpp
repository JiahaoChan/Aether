/*
 * Aether: Real-Time Sky & Environment & Weather simulation plugin.
 * Copyright TA.Jiahao.Chan, Inc. All Rights Reserved.
 */

#include "Rendering/AetherSceneViewExtension.h"

#include "RenderGraphBuilder.h"
//#include "Runtime/Renderer/Private/SceneRendering.h"

#include "AetherWorldSubsystem.h"

/*
// 在包含头文件之后，IMPLEMENT宏之前，添加一个静态变量和初始化函数
static int32 GAetherDebugInitFlag = 0;

static void Debug_AetherModuleInit()
{
	GAetherDebugInitFlag = 1;
	UE_LOG(LogTemp, Log, TEXT("Aether模块静态初始化代码已执行！"));
}

// 使用一个控制台变量来触发这个初始化检查，或者使用静态全局对象
static FAutoConsoleCommand GAetherDebugCmd(
	TEXT("Aether.DebugInit"),
	TEXT("调试命令，检查模块初始化"),
	FConsoleCommandDelegate::CreateStatic(Debug_AetherModuleInit)
);
*/

IMPLEMENT_STATIC_UNIFORM_BUFFER_SLOT(Aether);

IMPLEMENT_STATIC_UNIFORM_BUFFER_STRUCT(FAetherViewParameters, "Aether", Aether);

//IMPLEMENT_UNIFORM_BUFFER_STRUCT(FAetherViewParameters, "Aether")

/*
FAetherViewParameters::FAetherViewParameters()
{
	CustomParamA = 0.0f;
	CustomParamB = FVector4f::Zero();
}
*/
FAetherSceneViewExtension::FAetherSceneViewExtension(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase(AutoRegister)
{
	
}

void FAetherSceneViewExtension::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (UWorld* RenderingWorld = InView.Family->Scene->GetWorld())
	{
		//if (RenderingWorld->WorldType == EWorldType::Type::PIE)
		{
			if (UAetherWorldSubsystem* Subsystem = UAetherWorldSubsystem::Get(RenderingWorld))
            {
            	//if (!Subsystem->CachedViewUniformShaderParameters)
            	{
            		//Subsystem->CachedViewUniformShaderParameters = MakeUnique<FAetherViewParameters>();
            	}
            	//if (Subsystem->CachedViewUniformShaderParameters)
            	{
            		//Subsystem->CachedViewUniformShaderParameters.Get()->CustomParamA = 0.5f;
            		//Subsystem->CachedViewUniformShaderParameters.Get()->CustomParamB = FVector4f::Zero();

            		//CachedViewUniformShaderParameters = MakeUnique<FAetherViewParameters>(*Subsystem->CachedViewUniformShaderParameters);
            		//CachedViewUniformShaderParameters->CustomParamA = 
            		//AetherViewUniformBuffer = TUniformBufferRef<FAetherViewParameters>::CreateUniformBufferImmediate(*Subsystem->CachedViewUniformShaderParameters, UniformBuffer_SingleFrame);
					/*
            		const FShaderParametersMetadata* Metadata = FAetherViewParameters::GetStructMetadata();
            		if (Metadata && Metadata->IsLayoutInitialized()) 
            		{
            			AetherViewUniformBuffer = TUniformBufferRef<FAetherViewParameters>::CreateUniformBufferImmediate(*Subsystem->CachedViewUniformShaderParameters, UniformBuffer_SingleFrame);
            		}
            		else 
            		{
            			// 布局未初始化，可能是模块加载问题。可以输出一条日志或延迟到下一帧再创建。
            			UE_LOG(LogTemp, Warning, TEXT("FAetherViewParameters layout is not initialized!"));
            		}
            		*/
            	}
            }
		}
	}
}

void FAetherSceneViewExtension::PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
	//FUniformBufferStaticSlot Slot = FUniformBufferStaticSlotRegistry::Get().FindSlotByName(FName("Aether"));
	//GraphBuilder.RHICmdList.SetStaticUniformBuffer(Slot, AetherViewUniformBuffer.GetReference());
}

void FAetherSceneViewExtension::PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
	/*
	if (const FSceneViewFamily* ViewFamily = InView.Family)
	{
		if (FSceneRenderer* SceneRenderer = dynamic_cast<FSceneRenderer*>(ViewFamily->GetSceneRenderer()))
		{
			FRHITexture* TextureRHI = ViewFamily->RenderTarget->GetRenderTargetTexture();
            FRDGTextureRef Texture = GraphBuilder.FindExternalTexture(TextureRHI);
            
            FCanvas& Canvas = *FCanvas::Create(GraphBuilder, Texture, nullptr, ViewFamily->Time, ViewFamily->GetFeatureLevel(), ViewFamily->DebugDPIScale);
            Canvas.SetRenderTargetRect(InView.UnconstrainedViewRect);
            
            FScreenMessageWriter Writer(Canvas, 150.0f);
			
            if (1)
            {
            	static const FText StateText = NSLOCTEXT("Aether", "RenderingFrozen", "Rendering frozen...");
            	Writer.DrawLine(StateText, 10, FLinearColor(0.8, 1.0, 0.2, 1.0));
            }
			
            SceneRenderer->OnGetOnScreenMessages.Broadcast(Writer);
            
            Canvas.Flush_RenderThread(GraphBuilder, false);
		}
	}
	*/
}