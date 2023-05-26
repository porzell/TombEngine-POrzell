#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/spotcam.h"
#include "Game/effects/weather.h"
#include "Math/Math.h"
#include "Specific/setup.h"
#include "Game/Lara/lara_tech.h"
#include "Game/misc.h"
#include "Objects/TR5/Entity/tr5_guard.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara.h"

using namespace TEN::Effects::Environment;
using namespace TEN::Math;

TEN::Renderer::RendererHudBar* g_AirBar;
TEN::Renderer::RendererHudBar* g_ExposureBar;
TEN::Renderer::RendererHudBar* g_HealthBar;
TEN::Renderer::RendererHudBar* g_StaminaBar;
TEN::Renderer::RendererHudBar* g_LoadingBar;

namespace TEN::Renderer
{
	void Renderer11::InitialiseGameBars()
	{
		constexpr auto AIR_BAR_POS		= Vector2(630.0f, 30.0f);
		constexpr auto EXPOSURE_BAR_POS = Vector2(630.0f, 70.0f);
		constexpr auto HEALTH_BAR_POS	= Vector2(20.0f, 30.0f);
		constexpr auto STAMINA_BAR_POS	= Vector2(630.0f, 50.0f);
		constexpr auto LOADING_BAR_POS	= Vector2(325.0f, 550.0f);

		static const auto AIR_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.0f, 0.0f, 0.35f, 1.0f),
			Vector4(0.0f, 0.18f, 0.38f, 1.0f),

			// Center
			Vector4(0.0f, 0.15f, 0.6f, 1.0f),

			// Bottom
			Vector4(0.0f, 0.0f, 0.35f, 1.0f),
			Vector4(0.0f, 0.18f, 0.38f, 1.0f)
		};
		
		static const auto EXPOSURE_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.18f, 0.3f, 0.72f, 1.0f),
			Vector4(0.18f, 0.3f, 0.72f, 1.0f),

			// Center
			Vector4(0.18f, 0.3f, 0.72f, 1.0f),

			// Bottom
			Vector4(0.18f, 0.3f, 0.72f, 1.0f),
			Vector4(0.18f, 0.3f, 0.72f, 1.0f)
		};

		static const auto HEALTH_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.32f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.32f, 0.0f, 1.0f),

			// Center
			Vector4(0.3f, 0.32f, 0.0f, 1.0f),

			// Bottom
			Vector4(0.32f, 0.0f, 0.0f, 1.0f),
			Vector4(0.0f, 0.32f, 0.0f, 1.0f)
		};
		
		static const auto STAMINA_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.3f, 0.02f, 0.0f, 1.0f),
			Vector4(0.55f, 0.45f, 0.02f, 1.0f),

			// Center
			Vector4(0.95f, 0.45f, 0.09f, 1.0f),

			// Bottom
			Vector4(0.3f, 0.02f, 0.0f, 1.0f),
			Vector4(0.55f, 0.45f, 0.02f, 1.0f)
		};

		static const auto LOADING_BAR_COLORS = std::array<Vector4, RendererHudBar::COLOR_COUNT>
		{
			// Top
			Vector4(0.0f, 0.0f, 0.35f, 1.0f),
			Vector4(0.0f, 0.18f, 0.38f, 1.0f),

			// Center
			Vector4(0.0f, 0.15f, 0.6f, 1.0f),

			// Bottom
			Vector4(0.0f, 0.0f, 0.35f, 1.0f),
			Vector4(0.0f, 0.18f, 0.38f, 1.0f)
		};

		g_AirBar = new RendererHudBar(m_device.Get(), AIR_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, AIR_BAR_COLORS);
		g_ExposureBar = new RendererHudBar(m_device.Get(), EXPOSURE_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, EXPOSURE_BAR_COLORS);
		g_HealthBar = new RendererHudBar(m_device.Get(), HEALTH_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, HEALTH_BAR_COLORS);
		g_StaminaBar = new RendererHudBar(m_device.Get(), STAMINA_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, STAMINA_BAR_COLORS);
		g_LoadingBar = new RendererHudBar(m_device.Get(), LOADING_BAR_POS, RendererHudBar::SIZE_DEFAULT, 1, LOADING_BAR_COLORS);
	}

	void Renderer11::DrawBar(float percent, const RendererHudBar& bar, GAME_OBJECT_ID textureSlot, int frame, bool isPoisoned)
	{
		unsigned int strides = sizeof(RendererVertex);
		unsigned int offset = 0;
	
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar.VertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar.IndexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		m_context->VSSetShader(m_vsHUD.Get(), nullptr, 0);
		m_context->PSSetShader(m_psHUDTexture.Get(), nullptr, 0);

		SetBlendMode(BLENDMODE_OPAQUE);
		SetDepthState(DEPTH_STATE_NONE);
		SetCullMode(CULL_MODE_NONE);

		BindConstantBufferVS(CB_HUD, m_cbHUD.get());

		RendererSprite* borderSprite = &m_sprites[Objects[ID_BAR_BORDER_GRAPHIC].meshIndex];
		m_stHUDBar.BarStartUV = borderSprite->UV[0];
		m_stHUDBar.BarScale = Vector2(borderSprite->Width / (float)borderSprite->Texture->Width, borderSprite->Height / (float)borderSprite->Texture->Height);
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());
		 
		BindTexture(TEXTURE_HUD, borderSprite->Texture, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(56, 0, 0);

		m_context->PSSetShaderResources(0, 1, m_sprites[Objects[textureSlot].meshIndex].Texture->ShaderResourceView.GetAddressOf());

		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar.InnerVertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar.InnerIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		m_context->VSSetShader(m_vsHUD.Get(), nullptr, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), nullptr, 0);

		m_stHUDBar.Percent = percent;
		m_stHUDBar.Poisoned = isPoisoned;
		m_stHUDBar.Frame = frame;	
		RendererSprite* innerSprite = &m_sprites[Objects[textureSlot].meshIndex];
		m_stHUDBar.BarStartUV = innerSprite->UV[0];
		m_stHUDBar.BarScale = Vector2(innerSprite->Width / (float)innerSprite->Texture->Width, innerSprite->Height / (float)innerSprite->Texture->Height);
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());

		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());
		 
		BindTexture(TEXTURE_HUD, innerSprite->Texture, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(12, 0, 0);
	}

	void Renderer11::DrawLoadingBar(float percentage)
	{
		unsigned int strides = sizeof(RendererVertex);
		unsigned int offset = 0;
		
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
	
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, g_LoadingBar->VertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(g_LoadingBar->IndexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShader(m_psHUDTexture.Get(), NULL, 0);

		SetBlendMode(BLENDMODE_OPAQUE);
		SetDepthState(DEPTH_STATE_NONE);
		SetCullMode(CULL_MODE_NONE);

		BindConstantBufferVS(CB_HUD, m_cbHUD.get());
		BindTexture(TEXTURE_HUD, &loadingBarBorder, SAMPLER_LINEAR_CLAMP);

		m_stHUDBar.BarStartUV = Vector2::Zero;
		m_stHUDBar.BarScale = Vector2::One;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());

		DrawIndexedTriangles(56, 0, 0);

		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, g_LoadingBar->InnerVertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(g_LoadingBar->InnerIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
		m_context->VSSetShader(m_vsHUD.Get(), nullptr, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), nullptr, 0);
		
		m_stHUDBar.Percent = percentage / 100.0f;
		m_stHUDBar.Poisoned = false;
		m_stHUDBar.Frame = 0;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());

		BindTexture(TEXTURE_HUD, &loadingBarInner, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(12, 0, 0);
	}

	void Renderer11::AddLine2D(const Vector2& origin, const Vector2& target, const Color& color)
	{
		auto line = RendererLine2D{ origin, target, color };
		m_lines2DToDraw.push_back(line);
	}

	void Renderer11::DrawOverlays(RenderView& view)
	{
		auto flashColor = Weather.FlashColor();
		if (flashColor != Vector3::Zero)
		{
			SetBlendMode(BLENDMODE_ADDITIVE);
			DrawFullScreenQuad(m_whiteTexture.ShaderResourceView.Get(), flashColor);
		}

		if (CurrentLevel == 0)
			return;

		if (!BinocularRange && !SpotcamOverlay)
			return;

		SetBlendMode(BLENDMODE_ALPHABLEND);

		if (BinocularRange && !LaserSight)
		{
			DrawFullScreenSprite(&m_sprites[Objects[ID_BINOCULAR_GRAPHIC].meshIndex], Vector3::One, false);
		}
		else if (BinocularRange && LaserSight)
		{
			DrawFullScreenSprite(&m_sprites[Objects[ID_LASER_SIGHT_GRAPHIC].meshIndex], Vector3::One);

			SetBlendMode(BLENDMODE_OPAQUE);

			// Draw the aiming point
			RendererVertex vertices[4];

			vertices[0].Position.x = -4.0f / m_screenWidth;
			vertices[0].Position.y = 4.0f / m_screenHeight;
			vertices[0].Position.z = 0.0f;
			vertices[0].UV.x = 0.0f;
			vertices[0].UV.y = 0.0f;
			vertices[0].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[1].Position.x = 4.0f / m_screenWidth;
			vertices[1].Position.y = 4.0f / m_screenHeight;
			vertices[1].Position.z = 0.0f;
			vertices[1].UV.x = 1.0f;
			vertices[1].UV.y = 0.0f;
			vertices[1].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[2].Position.x = 4.0f / m_screenWidth;
			vertices[2].Position.y = -4.0f / m_screenHeight;
			vertices[2].Position.z = 0.0f;
			vertices[2].UV.x = 1.0f;
			vertices[2].UV.y = 1.0f;
			vertices[2].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[3].Position.x = -4.0f / m_screenWidth;
			vertices[3].Position.y = -4.0f / m_screenHeight;
			vertices[3].Position.z = 0.0f;
			vertices[3].UV.x = 0.0f;
			vertices[3].UV.y = 1.0f;
			vertices[3].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			m_context->VSSetShader(m_vsFullScreenQuad.Get(), NULL, 0);
			m_context->PSSetShader(m_psFullScreenQuad.Get(), NULL, 0);

			m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_context->IASetInputLayout(m_inputLayout.Get());

			m_primitiveBatch->Begin();
			m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
			m_primitiveBatch->End();
		}
		else
		{
			// TODO: Vignette goes here! -- Lwmte, 21.08.21
		}
	}

	void Renderer11::DrawPostprocess(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view)
	{
		SetBlendMode(BLENDMODE_OPAQUE);

		m_context->RSSetState(m_cullCounterClockwiseRasterizerState.Get());
		m_context->ClearRenderTargetView(target, Colors::Black);
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_context->OMSetRenderTargets(1, &target, depthTarget);
		m_context->RSSetViewports(1, &view.viewport);
		ResetScissor();

		RendererVertex vertices[4];

		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = 0.0f;
		vertices[0].UV.y = 0.0f;
		vertices[0].Color = Vector4::One;

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = 1.0f;
		vertices[1].UV.y = 0.0f;
		vertices[1].Color = Vector4::One;

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -1.0f;
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = 1.0f;
		vertices[2].UV.y = 1.0f;
		vertices[2].Color = Vector4::One;

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -1.0f;
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = 0.0f;
		vertices[3].UV.y = 1.0f;
		vertices[3].Color = Vector4::One;

		m_context->VSSetShader(m_vsFinalPass.Get(), nullptr, 0);
		m_context->PSSetShader(m_psFinalPass.Get(), nullptr, 0);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_stPostProcessBuffer.FXAA = g_Configuration.Antialiasing == AntialiasingMode::Low ? 1 : 0;
		m_stPostProcessBuffer.ViewportWidth = m_screenWidth;
		m_stPostProcessBuffer.ViewportHeight = m_screenHeight;
		m_stPostProcessBuffer.ScreenFadeFactor = ScreenFadeCurrent;
		m_stPostProcessBuffer.CinematicBarsHeight = Smoothstep(CinematicBarsHeight) * SPOTCAM_CINEMATIC_BARS_HEIGHT;
		m_cbPostProcessBuffer.updateData(m_stPostProcessBuffer, m_context.Get());
		BindConstantBufferPS(CB_POSTPROCESS, m_cbPostProcessBuffer.get());

		BindTexture(TEXTURE_COLOR_MAP, &m_renderTarget, SAMPLER_ANISOTROPIC_CLAMP);

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	void Renderer11::DrawFullScreenImage(ID3D11ShaderResourceView* texture, float fade, ID3D11RenderTargetView* target,
		ID3D11DepthStencilView* depthTarget)
	{
		// Reset GPU state
		SetBlendMode(BLENDMODE_OPAQUE);
		SetCullMode(CULL_MODE_NONE);

		m_context->OMSetRenderTargets(1, &target, depthTarget);
		m_context->RSSetViewports(1, &m_viewport);
		ResetScissor();

		DrawFullScreenQuad(texture, Vector3(fade, fade, fade), true);
	}

	void Renderer11::DrawFullScreenQuad(ID3D11ShaderResourceView* texture, DirectX::SimpleMath::Vector3 color, bool fit)
	{
		Vector2 uvStart = { 0.0f, 0.0f };
		Vector2 uvEnd   = { 1.0f, 1.0f };

		if (fit)
		{
			ID3D11Texture2D* texture2D;
			texture->GetResource(reinterpret_cast<ID3D11Resource**>(&texture2D));

			D3D11_TEXTURE2D_DESC desc;
			texture2D->GetDesc(&desc);

			float screenAspect = float(m_screenWidth) / float(m_screenHeight);
			float imageAspect  = float(desc.Width) / float(desc.Height);

			if (screenAspect > imageAspect)
			{
				float diff = (screenAspect - imageAspect) / screenAspect / 2;
				uvStart.y += diff;
				uvEnd.y   -= diff;
			}
			else
			{
				float diff = (imageAspect - screenAspect) / imageAspect / 2;
				uvStart.x += diff;
				uvEnd.x   -= diff;
			}
		}

		RendererVertex vertices[4];

		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = uvStart.x;
		vertices[0].UV.y = uvStart.y;
		vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = uvEnd.x;
		vertices[1].UV.y = uvStart.y;
		vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -1.0f;
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = uvEnd.x;
		vertices[2].UV.y = uvEnd.y;
		vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -1.0f;
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = uvStart.x;
		vertices[3].UV.y = uvEnd.y;
		vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);

		m_context->VSSetShader(m_vsFullScreenQuad.Get(), nullptr, 0);
		m_context->PSSetShader(m_psFullScreenQuad.Get(), nullptr, 0);

		m_context->PSSetShaderResources(0, 1, &texture);
		ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	void Renderer11::DrawTrackers() {
		static Vector2 size = { 0.06, 0.06 };
		static bool growing = true;
		static int index = 28;
		static int counter = 0;
		static int frame = 0;
		static Vector3 color = { 0.2,0.2,0.2 };
		static ItemInfo* lastTarget = nullptr;

		static Vector2 scale{ 1.0,1.0 };

		{
			for (auto& trackedItem : g_trackedItems) {
				auto& itemPos = GetJointPosition(trackedItem, 0, Vector3i(0, 0, 0));
				Vector3 d3DPos(itemPos.x, itemPos.y, itemPos.z);
				Vector2 screenPos = GetScreenSpacePosition(d3DPos);

				// Default to white.
				Vector3 color(1.0, 1.0, 1.0);

				if (trackedItem->IsCreature()) {
					auto* creature = GetCreatureInfo(trackedItem);
					if (creature->Alerted)
						color = Vector3(0.6, 0.0, 0.0);
					else
						color = Vector3(0.0, 0.7, 0.0);
				}

				std::string stateString;
				if (trackedItem->HitPoints <= 0) {
					color = { 0.3,0.3,0.3 };
					stateString = "Neutralized";
				}
				else if (GetCreatureInfo(trackedItem)->Patrol)
					stateString = "On patrol";
				else if (trackedItem->ObjectNumber == ID_GUARD1 || trackedItem->ObjectNumber == ID_GUARD2 || trackedItem->ObjectNumber == ID_GUARD3) {
					stateString = GetGuardStateString(trackedItem->Index);

					if (stateString == "Neutralized")
						color = { 0.3,0.3,0.3 };
				}
				else if (trackedItem->Animation.Velocity.Length() > 0) {
					stateString = "Moving";
				}
				else
					stateString = "Stationary";

				//if (!(BinocularRange || TrInput & IN_LOOK))
				SetBlendMode(BLENDMODE_ADDITIVE);
				Draw2DSprite(&m_sprites[Objects[1378].meshIndex], screenPos, size, color, false);
				if (!(BinocularRange || TrInput & IN_LOOK))
					SetBlendMode(BLENDMODE_ADDITIVE);
				
				screenPos.y -= 45;
				AddString(trackedItem->Name /* + "\nHealth: " + std::to_string(trackedItem->HitPoints) */, screenPos, Color(color), 0.8f, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE, m_techFont);
				screenPos.y += 15;
				AddString("State: " + stateString /* + "\nHealth: " + std::to_string(trackedItem->HitPoints) */, screenPos, {0.8,0.8,0.8}, 0.5f, PRINTSTRING_CENTER, m_techFont);
			}
		}

		if (growing)
			size += {0.001, 0.001};
		else
			size -= {0.001, 0.001};

		if (size.x >= 0.125) {
			growing = false;
		}
		else if (size.x <= 0.06125) {
			growing = true;
		}
		/*counter++;

		if (counter == FPS) {
			counter = 0;
			index++;
		}

		if (index > 37) {
			index = 0;
		}*/

		// Draw targeting reticle
		if (GetLaraInfo(LaraItem)->TargetEntity) {

			if (GetLaraInfo(LaraItem)->TargetEntity != lastTarget)
			{
				scale = { 1.0,1.0 };
				color = { 0.1f,0.1f,0.1f };
				lastTarget = GetLaraInfo(LaraItem)->TargetEntity;
			}
			auto& itemPos = GetJointPosition(GetLaraInfo(LaraItem)->TargetEntity, 0, Vector3i(0, 0, 0));

			Vector3 d3DPos(itemPos.x, itemPos.y, itemPos.z);
			Vector2 screenPos = GetScreenSpacePosition(d3DPos);

			if (scale.x > 0.2) {
				scale.x -= 0.05;
				scale.y -= 0.05;
			}

			if (GetLaraInfo(LaraItem)->TargetEntity->HitPoints <= 0)
				color = { 0.1,0.1,0.1 };

			color.x += (1.0f) / 60;
			//color.y += (1.0f) / 30;
			//color.z += (1.0f) / 30;

			if (color.x > 0.5f) {
				color = { 0.5f,0.0f,0.0f };
			}

			SetBlendMode(BLENDMODE_ADDITIVE);
			Draw2DSprite(&m_sprites[Objects[1379].meshIndex] + (frame), screenPos, scale, color, false, 1.0f);
			
			frame++;

			if (frame >= 30)
				frame = 0;
		}
			
	}

	// Peter:
	void Renderer11::Draw2DSprite(RendererSprite* sprite, Vector2 pos, Vector2 size, DirectX::SimpleMath::Vector3 color, bool fit, float alpha)
	{
		Vector2 uvStart = { 0.0f, 0.0f };
		Vector2 uvEnd = { 1.0f, 1.0f };

		// Get screen space coords
		pos = pos / SCREEN_SPACE_RES;
		pos = pos * 2;
		pos -= Vector2(1, 1);
		pos.y = -pos.y;

		//size *= (float(m_screenHeight) / float(m_screenWidth));
		size.y *= (float(m_screenWidth)/float(m_screenHeight));

		//size = size / SCREEN_SPACE_RES;

		ID3D11ShaderResourceView* texture = sprite->Texture->ShaderResourceView.Get();

		if (fit)
		{
			float screenAspect = float(m_screenWidth) / float(m_screenHeight);
			float imageAspect = float(sprite->Width) / float(sprite->Height);

			if (screenAspect > imageAspect)
			{
				float diff = (screenAspect - imageAspect) / screenAspect / 2;
				uvStart.y += diff;
				uvEnd.y -= diff;
			}
			else
			{
				float diff = (imageAspect - screenAspect) / imageAspect / 2;
				uvStart.x += diff;
				uvEnd.x -= diff;
			}
		}

		Vector2 scale = Vector2(sprite->Width / (float)sprite->Texture->Width, sprite->Height / (float)sprite->Texture->Height);
		uvStart.x = uvStart.x * scale.x + sprite->UV[0].x;
		uvStart.y = uvStart.y * scale.y + sprite->UV[0].y;
		uvEnd.x = uvEnd.x * scale.x + sprite->UV[0].x;
		uvEnd.y = uvEnd.y * scale.y + sprite->UV[0].y;

		RendererVertex vertices[4];

		vertices[0].Position.x = pos.x - size.x;
		vertices[0].Position.y = pos.y + size.y;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = uvStart.x;
		vertices[0].UV.y = uvStart.y;
		vertices[0].Color = Vector4(color.x, color.y, color.z, alpha);

		vertices[1].Position.x = pos.x + size.x;
		vertices[1].Position.y = pos.y + size.y;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = uvEnd.x;
		vertices[1].UV.y = uvStart.y;
		vertices[1].Color = Vector4(color.x, color.y, color.z, alpha);

		vertices[2].Position.x = pos.x + size.x;
		vertices[2].Position.y = pos.y - size.y;
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = uvEnd.x;
		vertices[2].UV.y = uvEnd.y;
		vertices[2].Color = Vector4(color.x, color.y, color.z, alpha);

		vertices[3].Position.x = pos.x - size.x;
		vertices[3].Position.y = pos.y - size.y;
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = uvStart.x;
		vertices[3].UV.y = uvEnd.y;
		vertices[3].Color = Vector4(color.x, color.y, color.z, alpha);


		SetBlendMode(BLENDMODE_ADDITIVE);

		m_context->VSSetShader(m_vsFullScreenQuad.Get(), nullptr, 0);
		m_context->PSSetShader(m_psFullScreenQuad.Get(), nullptr, 0);

		m_context->PSSetShaderResources(0, 1, &texture);
		ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	void Renderer11::DrawFullScreenSprite(RendererSprite* sprite, DirectX::SimpleMath::Vector3 color, bool fit)
	{
		Vector2 uvStart = { 0.0f, 0.0f };
		Vector2 uvEnd = { 1.0f, 1.0f };

		ID3D11ShaderResourceView* texture = sprite->Texture->ShaderResourceView.Get();

		if (fit)
		{
			float screenAspect = float(m_screenWidth) / float(m_screenHeight);
			float imageAspect = float(sprite->Width) / float(sprite->Height);

			if (screenAspect > imageAspect)
			{
				float diff = (screenAspect - imageAspect) / screenAspect / 2;
				uvStart.y += diff;
				uvEnd.y -= diff;
			}
			else
			{
				float diff = (imageAspect - screenAspect) / imageAspect / 2;
				uvStart.x += diff;
				uvEnd.x -= diff;
			}
		}

		Vector2 scale = Vector2(sprite->Width / (float)sprite->Texture->Width, sprite->Height / (float)sprite->Texture->Height);
		uvStart.x = uvStart.x * scale.x + sprite->UV[0].x;
		uvStart.y = uvStart.y * scale.y + sprite->UV[0].y;
		uvEnd.x = uvEnd.x * scale.x + sprite->UV[0].x;
		uvEnd.y = uvEnd.y * scale.y + sprite->UV[0].y;

		RendererVertex vertices[4];

		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = uvStart.x;
		vertices[0].UV.y = uvStart.y;
		vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = uvEnd.x;
		vertices[1].UV.y = uvStart.y;
		vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -1.0f;
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = uvEnd.x;
		vertices[2].UV.y = uvEnd.y;
		vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -1.0f;
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = uvStart.x;
		vertices[3].UV.y = uvEnd.y;
		vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);

		m_context->VSSetShader(m_vsFullScreenQuad.Get(), nullptr, 0);
		m_context->PSSetShader(m_psFullScreenQuad.Get(), nullptr, 0);

		m_context->PSSetShaderResources(0, 1, &texture);
		ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}
}
