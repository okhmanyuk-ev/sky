#pragma once

#include <Renderer/system.h>

#if defined(RENDERER_D3D11)
#include <Platform/system.h>
#include <Common/event_system.h>
#include <Common/hash.h>
#include <Renderer/low_level_api.h>

namespace Renderer
{
	using SystemCrossplatform = SystemD3D11;

	class SystemD3D11 : public System
	{
	public:
		SystemD3D11();
		~SystemD3D11();

	public:
		void setTopology(const Topology& value) override;
		void setViewport(const Viewport& value) override;
		void setScissor(const Scissor& value) override;
		void setScissor(std::nullptr_t value) override;
		void setVertexBuffer(const Buffer& value) override;
		void setIndexBuffer(const Buffer& value) override;
		void setTexture(const Texture& value) override;
		void setRenderTarget(std::shared_ptr<RenderTarget> value) override;
		void setShader(std::shared_ptr<Shader> value) override;
		void setSampler(const Sampler& value) override;
		void setDepthMode(const DepthMode& value) override;
		void setCullMode(const CullMode& value) override;
		void setBlendMode(const BlendMode& value) override;

		void clear(const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 1.0f }) override;

		void draw(size_t vertexCount, size_t vertexOffset = 0) override;
		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0) override;
		
		void present() override;

		bool isVsync() const override { return mVsync; }
		void setVsync(bool value) override { mVsync = value; }
	
	private:
		bool mVsync = false;

	public:
		static ID3D11Device* Device;
		static ID3D11DeviceContext* Context;

	private:
		IDXGISwapChain* mSwapChain;

		ID3D11Texture2D* depthStencilTexture;
		ID3D11RenderTargetView* renderTargetView;
		ID3D11DepthStencilView* depthStencilView;

		void createRenderTarget();
		void destroyRenderTarget();

		std::shared_ptr<RenderTarget> currentRenderTarget = nullptr;

	private:
		Common::EventSystem::Listener<Platform::System::ResizeEvent> mResizeListener;

	public:
		static std::map<Vertex::Attribute::Format, DXGI_FORMAT> Format;
		static std::map<Vertex::Attribute::Type, std::string> Semantic;

	private:
		void prepareForDrawing();

	private:
		struct RasterizerState
		{
			bool scissorEnabled = false;
			CullMode cullMode = CullMode::None;

			bool operator==(const RasterizerState& value) const
			{
				return scissorEnabled == value.scissorEnabled && cullMode == value.cullMode;
			}

			bool operator!=(const RasterizerState& value) const
			{
				return scissorEnabled != value.scissorEnabled || cullMode != value.cullMode;
			}
		};

		struct RasterizerHasher
		{
			size_t operator()(const RasterizerState& k) const
			{
				size_t seed = 0;
				Common::Hash::combine(seed, k.cullMode);
				Common::Hash::combine(seed, k.scissorEnabled);
				return seed;
			}
		};

		struct RasterizerComparer
		{
			bool operator()(const RasterizerState& left, const RasterizerState& right) const
			{
				return left == right;
			}
		};

		struct BlendModeHasher
		{
			size_t operator()(const BlendMode& k) const
			{
				size_t seed = 0;
				Common::Hash::combine(seed, k.alphaBlendFunction);
				Common::Hash::combine(seed, k.alphaDstBlend);
				Common::Hash::combine(seed, k.alphaSrcBlend);
				Common::Hash::combine(seed, k.colorBlendFunction);
				Common::Hash::combine(seed, k.colorDstBlend);
				Common::Hash::combine(seed, k.colorSrcBlend);
				return seed;
			}
		};

	private:
		ID3D11Buffer* mD3D11VertexBuffer = nullptr;
		ID3D11Buffer* mD3D11IndexBuffer = nullptr;
		std::unordered_map<Texture*, ID3D11ShaderResourceView*> mD3D11TextureViews;
		std::unordered_map<Sampler, ID3D11SamplerState*> mD3D11SamplerStates;
		std::unordered_map<DepthMode, ID3D11DepthStencilState*> mD3D11DepthStencilStates;
		std::unordered_map<RasterizerState, ID3D11RasterizerState*, RasterizerHasher, RasterizerComparer> mD3D11RasterizerStates;
		std::unordered_map<BlendMode, ID3D11BlendState*, BlendModeHasher> mD3D11BlendModes;

		Viewport mViewport;
		Scissor mScissor;

		std::shared_ptr<Shader> mAppliedShader = nullptr;
		std::shared_ptr<Shader> mShader = nullptr;

		RasterizerState mRasterizerState;
		bool mStateScissorEnabled = false;
		CullMode mStateCullMode = CullMode::None;
		bool mRasterizerStateApplied = false;
		void setD3D11RasterizerState(RasterizerState rasterizerState);
	};
}
#endif