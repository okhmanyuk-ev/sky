#pragma once

#include <renderer/system.h>

#if defined(RENDERER_D3D11)
#include <platform/system.h>
#include <common/event_system.h>
#include <common/hash.h>
#include <renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	using SystemCrossplatform = SystemD3D11;

	class SystemD3D11 : public System,
		Common::Event::Listenable<Platform::System::ResizeEvent>
	{
	public:
		SystemD3D11();
		~SystemD3D11();

	private:
		void onEvent(const Platform::System::ResizeEvent& e) override;

	public:
		void setTopology(const Topology& value) override;
		void setViewport(const Viewport& value) override;
		void setScissor(const Scissor& value) override;
		void setScissor(std::nullptr_t value) override;
		void setVertexBuffer(const Buffer& value) override;
		void setIndexBuffer(const Buffer& value) override;
		void setTexture(std::shared_ptr<Texture> value) override;
		void setRenderTarget(std::shared_ptr<RenderTarget> value) override;
		void setShader(std::shared_ptr<Shader> value) override;
		void setSampler(const Sampler& value) override;
		void setDepthMode(const DepthMode& value) override;
		void setStencilMode(const StencilMode& value) override;
		void setCullMode(const CullMode& value) override;
		void setBlendMode(const BlendMode& value) override;
		void setTextureAddressMode(const TextureAddress& value) override;

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f },
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0) override;

		void draw(size_t vertexCount, size_t vertexOffset = 0) override;
		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0) override;

		void readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory) override;
		void readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture) override;

		void present() override;

		bool isVsync() const override { return mVsync; }
		void setVsync(bool value) override { mVsync = value; }
	
	private:
		bool mVsync = false;

	public:
		inline static ID3D11Device* Device = nullptr;
		inline static ID3D11DeviceContext* Context = nullptr;

	private:
		IDXGISwapChain* mSwapChain;
		ID3D11Texture2D* depthStencilTexture;
		ID3D11RenderTargetView* renderTargetView;
		ID3D11DepthStencilView* depthStencilView;

		void createRenderTarget();
		void destroyRenderTarget();

		std::shared_ptr<RenderTarget> currentRenderTarget = nullptr;

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
				return !(value == *this);
			}

			struct Hasher
			{
				size_t operator()(const RasterizerState& k) const
				{
					size_t seed = 0;
					Common::Hash::combine(seed, k.cullMode);
					Common::Hash::combine(seed, k.scissorEnabled);
					return seed;
				}
			};

			struct Comparer
			{
				bool operator()(const RasterizerState& left, const RasterizerState& right) const
				{
					return left == right;
				}
			};
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

		struct DepthStencilState
		{
			DepthMode depthMode;
			StencilMode stencilMode;

			bool operator==(const DepthStencilState& value) const
			{
				return depthMode == value.depthMode && stencilMode == value.stencilMode;
			}

			bool operator!=(const DepthStencilState& value) const
			{
				return !(value == *this);
			}

			struct Hasher
			{
				size_t operator()(const DepthStencilState& k) const
				{
					size_t seed = 0;
					Common::Hash::combine(seed, k.depthMode.enabled);
					Common::Hash::combine(seed, k.depthMode.func);

					Common::Hash::combine(seed, k.stencilMode.enabled);

					Common::Hash::combine(seed, k.stencilMode.readMask);
					Common::Hash::combine(seed, k.stencilMode.writeMask);

					Common::Hash::combine(seed, k.stencilMode.depthFailOp);
					Common::Hash::combine(seed, k.stencilMode.failOp);
					Common::Hash::combine(seed, k.stencilMode.func);
					Common::Hash::combine(seed, k.stencilMode.passOp);

					return seed;
				}
			};

			struct Comparer
			{
				bool operator()(const DepthStencilState& left, const DepthStencilState& right) const
				{
					return left == right;
				}
			};
		};

		struct SamplerState
		{
			Sampler sampler = Sampler::Linear;
			TextureAddress textureAddress = TextureAddress::Clamp;
			
			struct Hasher
			{
				size_t operator()(const SamplerState& k) const
				{
					size_t seed = 0;
					Common::Hash::combine(seed, k.sampler);
					Common::Hash::combine(seed, k.textureAddress);
					return seed;
				}
			};

			struct Comparer
			{
				bool operator()(const SamplerState& left, const SamplerState& right) const
				{
					return
						left.sampler == right.sampler &&
						left.textureAddress == right.textureAddress;
				}
			};
		};

	private:
		ID3D11Buffer* mD3D11VertexBuffer = nullptr;
		ID3D11Buffer* mD3D11IndexBuffer = nullptr;
		std::unordered_map<Texture*, ID3D11ShaderResourceView*> mD3D11TextureViews;
		std::unordered_map<SamplerState, ID3D11SamplerState*, SamplerState::Hasher, SamplerState::Comparer> mD3D11SamplerStates;
		std::unordered_map<DepthStencilState, ID3D11DepthStencilState*, DepthStencilState::Hasher, DepthStencilState::Comparer> mD3D11DepthStencilStates;
		std::unordered_map<RasterizerState, ID3D11RasterizerState*, RasterizerState::Hasher, RasterizerState::Comparer> mD3D11RasterizerStates;
		std::unordered_map<BlendMode, ID3D11BlendState*, BlendModeHasher> mD3D11BlendModes;

		Viewport mViewport;
		Scissor mScissor;

		std::shared_ptr<Shader> mShader = nullptr;
		bool mShaderDirty = false;

		RasterizerState mRasterizerState;
		bool mRasterizerStateDirty = true;
		void setD3D11RasterizerState(const RasterizerState& value);

		DepthStencilState mDepthStencilState;
		bool mDepthStencilStateDirty = true;
		void setD3D11DepthStencilState(const DepthStencilState& value);

		SamplerState mSamplerState;
		bool mSamplerStateDirty = true;
		void setD3D11SamplerState(const SamplerState& value);
	};
}
#endif