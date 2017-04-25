// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#include "StdAfx.h"
#include "myIDirect3DDevice9.h"

myIDirect3DDevice9::myIDirect3DDevice9(IDirect3DDevice9* pOriginal)
{
	m_pIDirect3DDevice9 = pOriginal; // Store the pointer to original object

	D3DDEVICE_CREATION_PARAMETERS creation_params;
	m_pIDirect3DDevice9->GetCreationParameters(&creation_params);
	GetClientRect(creation_params.hFocusWindow, &window_rect);

	// Initialize fullscreen text overlay
	SP_DX9_init_text_overlay(_SP_DEFAULT_TEXT_HEIGHT_,
		_SP_DEFAULT_TEXT_BORDER_THICKNESS_,
		_SP_DEFAULT_TEXT_SHADOW_X_OFFSET_,
		_SP_DEFAULT_TEXT_SHADOW_Y_OFFSET_,
		_SP_DEFAULT_TEXT_COLOR_,
		_SP_DEFAULT_TEXT_BORDER_COLOR_,
		_SP_DEFAULT_TEXT_SHADOW_COLOR_,
		_SP_DEFAULT_TEXT_FORMAT_,
		_SP_DEFAULT_TEXT_STYLE_);
}

myIDirect3DDevice9::~myIDirect3DDevice9(void)
{
}

HRESULT myIDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
{
	// Check if original dll can provide interface. then send *our* address
	*ppvObj = NULL;

	HRESULT hRes = m_pIDirect3DDevice9->QueryInterface(riid, ppvObj);

	if (hRes == NOERROR)
	{
		*ppvObj = this;
	}

	return hRes;
}

ULONG myIDirect3DDevice9::AddRef(void)
{
	return(m_pIDirect3DDevice9->AddRef());
}

ULONG myIDirect3DDevice9::Release(void)
{
	// ATTENTION: This is a booby-trap! Watch out!
	// If we create our own sprites, surfaces, etc. (thus increasing the ref counter
	// by external action), we need to delete those objects before calling the
	// original Release() function.

	// global var
	extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

	// Release/delete own objects
	// .....

	// Call original function
	ULONG count = m_pIDirect3DDevice9->Release();

	// Now that the original object has deleted itself, so do we
	gl_pmyIDirect3DDevice9 = NULL;
	delete(this);  // Destructor will be called automatically

	return (count);
}

HRESULT myIDirect3DDevice9::TestCooperativeLevel(void)
{
	return(m_pIDirect3DDevice9->TestCooperativeLevel());
}

UINT myIDirect3DDevice9::GetAvailableTextureMem(void)
{
	return(m_pIDirect3DDevice9->GetAvailableTextureMem());
}

HRESULT myIDirect3DDevice9::EvictManagedResources(void)
{
	return(m_pIDirect3DDevice9->EvictManagedResources());
}

HRESULT myIDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
	return(m_pIDirect3DDevice9->GetDirect3D(ppD3D9));
}

HRESULT myIDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{
	return(m_pIDirect3DDevice9->GetDeviceCaps(pCaps));
}

HRESULT myIDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	return(m_pIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT myIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	return(m_pIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT myIDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	return(m_pIDirect3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap));
}

void    myIDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
	return(m_pIDirect3DDevice9->SetCursorPosition(X, Y, Flags));
}

BOOL    myIDirect3DDevice9::ShowCursor(BOOL bShow)
{
	return(m_pIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT myIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
	return(m_pIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain));
}

HRESULT myIDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	return(m_pIDirect3DDevice9->GetSwapChain(iSwapChain, pSwapChain));
}

UINT    myIDirect3DDevice9::GetNumberOfSwapChains(void)
{
	return(m_pIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT myIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	return(m_pIDirect3DDevice9->Reset(pPresentationParameters));
}

HRESULT myIDirect3DDevice9::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	// We may want to draw own things here before flipping surfaces
	// ... draw own stuff, etc ...

	// Call original routine
	return m_pIDirect3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT myIDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	return(m_pIDirect3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer));
}

HRESULT myIDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	return(m_pIDirect3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus));
}

HRESULT myIDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
	return(m_pIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void    myIDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	return(m_pIDirect3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp));
}

void    myIDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	return(m_pIDirect3DDevice9->GetGammaRamp(iSwapChain, pRamp));
}

HRESULT myIDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	return(m_pIDirect3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	return(m_pIDirect3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	return(m_pIDirect3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	return(m_pIDirect3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	return(m_pIDirect3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return(m_pIDirect3DDevice9->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return(m_pIDirect3DDevice9->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle));
}

HRESULT myIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	return(m_pIDirect3DDevice9->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint));
}

HRESULT myIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
	return(m_pIDirect3DDevice9->UpdateTexture(pSourceTexture, pDestinationTexture));
}

HRESULT myIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	return(m_pIDirect3DDevice9->GetRenderTargetData(pRenderTarget, pDestSurface));
}

HRESULT myIDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	return(m_pIDirect3DDevice9->GetFrontBufferData(iSwapChain, pDestSurface));
}

HRESULT myIDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	return(m_pIDirect3DDevice9->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter));
}

HRESULT myIDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	return(m_pIDirect3DDevice9->ColorFill(pSurface, pRect, color));
}

HRESULT myIDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return(m_pIDirect3DDevice9->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle));
}

HRESULT myIDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	return(m_pIDirect3DDevice9->SetRenderTarget(RenderTargetIndex, pRenderTarget));
}

HRESULT myIDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	return(m_pIDirect3DDevice9->GetRenderTarget(RenderTargetIndex, ppRenderTarget));
}

HRESULT myIDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	return(m_pIDirect3DDevice9->SetDepthStencilSurface(pNewZStencil));
}

HRESULT myIDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	return(m_pIDirect3DDevice9->GetDepthStencilSurface(ppZStencilSurface));
}

HRESULT myIDirect3DDevice9::BeginScene(void)
{
	return(m_pIDirect3DDevice9->BeginScene());
}

HRESULT myIDirect3DDevice9::EndScene(void)
{
	// Draw anything you want before the scene is shown to the user

	SP_DX9_draw_text_overlay();

	return(m_pIDirect3DDevice9->EndScene());
}

HRESULT myIDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return(m_pIDirect3DDevice9->Clear(Count, pRects, Flags, Color, Z, Stencil));
}

HRESULT myIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	return(m_pIDirect3DDevice9->SetTransform(State, pMatrix));
}

HRESULT myIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
	return(m_pIDirect3DDevice9->GetTransform(State, pMatrix));
}

HRESULT myIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	return(m_pIDirect3DDevice9->MultiplyTransform(State, pMatrix));
}

HRESULT myIDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	return(m_pIDirect3DDevice9->SetViewport(pViewport));
}

HRESULT myIDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{
	return(m_pIDirect3DDevice9->GetViewport(pViewport));
}

HRESULT myIDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	return(m_pIDirect3DDevice9->SetMaterial(pMaterial));
}

HRESULT myIDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{
	return(m_pIDirect3DDevice9->GetMaterial(pMaterial));
}

HRESULT myIDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	return(m_pIDirect3DDevice9->SetLight(Index, pLight));
}

HRESULT myIDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9* pLight)
{
	return(m_pIDirect3DDevice9->GetLight(Index, pLight));
}

HRESULT myIDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable)
{
	return(m_pIDirect3DDevice9->LightEnable(Index, Enable));
}

HRESULT myIDirect3DDevice9::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	return(m_pIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT myIDirect3DDevice9::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	return(m_pIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT myIDirect3DDevice9::GetClipPlane(DWORD Index, float* pPlane)
{
	return(m_pIDirect3DDevice9->GetClipPlane(Index, pPlane));
}

HRESULT myIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	return(m_pIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT myIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	return(m_pIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT myIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	return(m_pIDirect3DDevice9->CreateStateBlock(Type, ppSB));
}

HRESULT myIDirect3DDevice9::BeginStateBlock(void)
{
	return(m_pIDirect3DDevice9->BeginStateBlock());
}

HRESULT myIDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	return(m_pIDirect3DDevice9->EndStateBlock(ppSB));
}

HRESULT myIDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	return(m_pIDirect3DDevice9->SetClipStatus(pClipStatus));
}

HRESULT myIDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	return(m_pIDirect3DDevice9->GetClipStatus(pClipStatus));
}

HRESULT myIDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	return(m_pIDirect3DDevice9->GetTexture(Stage, ppTexture));
}

HRESULT myIDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	return(m_pIDirect3DDevice9->SetTexture(Stage, pTexture));
}

HRESULT myIDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	return(m_pIDirect3DDevice9->GetTextureStageState(Stage, Type, pValue));
}

HRESULT myIDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return(m_pIDirect3DDevice9->SetTextureStageState(Stage, Type, Value));
}

HRESULT myIDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	return(m_pIDirect3DDevice9->GetSamplerState(Sampler, Type, pValue));
}

HRESULT myIDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return(m_pIDirect3DDevice9->SetSamplerState(Sampler, Type, Value));
}

HRESULT myIDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
	return(m_pIDirect3DDevice9->ValidateDevice(pNumPasses));
}

HRESULT myIDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
	return(m_pIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	return(m_pIDirect3DDevice9->GetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return(m_pIDirect3DDevice9->SetCurrentTexturePalette(PaletteNumber));
}

HRESULT myIDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	return(m_pIDirect3DDevice9->GetCurrentTexturePalette(PaletteNumber));
}

HRESULT myIDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{
	return(m_pIDirect3DDevice9->SetScissorRect(pRect));
}

HRESULT myIDirect3DDevice9::GetScissorRect(RECT* pRect)
{
	return(m_pIDirect3DDevice9->GetScissorRect(pRect));
}

HRESULT myIDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return(m_pIDirect3DDevice9->SetSoftwareVertexProcessing(bSoftware));
}

BOOL    myIDirect3DDevice9::GetSoftwareVertexProcessing(void)
{
	return(m_pIDirect3DDevice9->GetSoftwareVertexProcessing());
}

HRESULT myIDirect3DDevice9::SetNPatchMode(float nSegments)
{
	return(m_pIDirect3DDevice9->SetNPatchMode(nSegments));
}

float   myIDirect3DDevice9::GetNPatchMode(void)
{
	return(m_pIDirect3DDevice9->GetNPatchMode());
}

HRESULT myIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return(m_pIDirect3DDevice9->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return(m_pIDirect3DDevice9->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount));
}

HRESULT myIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return(m_pIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return(m_pIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	return(m_pIDirect3DDevice9->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT myIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	return(m_pIDirect3DDevice9->CreateVertexDeclaration(pVertexElements, ppDecl));
}

HRESULT myIDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	return(m_pIDirect3DDevice9->SetVertexDeclaration(pDecl));
}

HRESULT myIDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	return(m_pIDirect3DDevice9->GetVertexDeclaration(ppDecl));
}

HRESULT myIDirect3DDevice9::SetFVF(DWORD FVF)
{
	return(m_pIDirect3DDevice9->SetFVF(FVF));
}

HRESULT myIDirect3DDevice9::GetFVF(DWORD* pFVF)
{
	return(m_pIDirect3DDevice9->GetFVF(pFVF));
}

HRESULT myIDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	return(m_pIDirect3DDevice9->CreateVertexShader(pFunction, ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	return(m_pIDirect3DDevice9->SetVertexShader(pShader));
}

HRESULT myIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	return(m_pIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return(m_pIDirect3DDevice9->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return(m_pIDirect3DDevice9->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return(m_pIDirect3DDevice9->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return(m_pIDirect3DDevice9->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return(m_pIDirect3DDevice9->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return(m_pIDirect3DDevice9->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT myIDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	return(m_pIDirect3DDevice9->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride));
}

HRESULT myIDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	return(m_pIDirect3DDevice9->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride));
}

HRESULT myIDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{
	return(m_pIDirect3DDevice9->SetStreamSourceFreq(StreamNumber, Divider));
}

HRESULT myIDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* Divider)
{
	return(m_pIDirect3DDevice9->GetStreamSourceFreq(StreamNumber, Divider));
}

HRESULT myIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	return(m_pIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT myIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	return(m_pIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT myIDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	return(m_pIDirect3DDevice9->CreatePixelShader(pFunction, ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	return(m_pIDirect3DDevice9->SetPixelShader(pShader));
}

HRESULT myIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	return(m_pIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return(m_pIDirect3DDevice9->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return(m_pIDirect3DDevice9->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return(m_pIDirect3DDevice9->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return(m_pIDirect3DDevice9->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return(m_pIDirect3DDevice9->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return(m_pIDirect3DDevice9->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT myIDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	return(m_pIDirect3DDevice9->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo));
}

HRESULT myIDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	return(m_pIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT myIDirect3DDevice9::DeletePatch(UINT Handle)
{
	return(m_pIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT myIDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	return(m_pIDirect3DDevice9->CreateQuery(Type, ppQuery));
}

void myIDirect3DDevice9::SP_DX9_draw_text_overlay()
{
	if (text_overlay.enabled)
	{
		clean_text_overlay_feed();

		build_text_overlay_feed_string(); // Build text feed string

		switch (text_overlay.text_style) {
			case SP_DX9_SHADOWED_TEXT:
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_shadow_rect[1], text_overlay.text_format, text_overlay.text_shadow_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_shadow_rect[0], text_overlay.text_format, text_overlay.text_color);
				break;
			case SP_DX9_PLAIN_TEXT:
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &window_rect, text_overlay.text_format, text_overlay.text_color);
				break;
			case SP_DX9_BORDERED_TEXT:
			default:
				// Draw bordered text
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[1], text_overlay.text_format, text_overlay.text_border_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[2], text_overlay.text_format, text_overlay.text_border_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[3], text_overlay.text_format, text_overlay.text_border_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[4], text_overlay.text_format, text_overlay.text_border_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[5], text_overlay.text_format, text_overlay.text_border_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[6], text_overlay.text_format, text_overlay.text_border_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[7], text_overlay.text_format, text_overlay.text_border_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[8], text_overlay.text_format, text_overlay.text_border_color);
				text_overlay.font->DrawText(NULL, text_overlay.text, -1, &text_overlay.text_outline_rect[0], text_overlay.text_format, text_overlay.text_color);
				break;
		}
	}
}

void myIDirect3DDevice9::SP_DX9_init_text_overlay(int text_height,
	unsigned int text_border_thickness,
	int text_shadow_x_offset,
	int text_shadow_y_offset,
	D3DXCOLOR text_color,
	D3DXCOLOR text_border_color,
	D3DXCOLOR text_shadow_color,
	DWORD text_format,
	int text_style)
{
	if (strcpy_s(text_overlay.font_name, _SP_DEFAULT_TEXT_FONT_) != 0)
	{
		// Handle error
	}

	// Initialize overlay font
	HRESULT font_hr = D3DXCreateFont(
		m_pIDirect3DDevice9,	// D3D device
		text_height,			// Height
		0,						// Width
		FW_BOLD,				// Weight
		1,						// MipLevels, 0 = autogen mipmaps
		FALSE,					// Italic
		DEFAULT_CHARSET,		// CharSet
		OUT_DEFAULT_PRECIS,		// OutputPrecision
		ANTIALIASED_QUALITY,	// Quality
		DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
		text_overlay.font_name,	// pFaceName
		&text_overlay.font);	// ppFont
	if (FAILED(font_hr))
	{
		// Handle error
	}


	// Set text colors, style, and format
	text_overlay.text_color = text_color;
	text_overlay.text_border_color = text_border_color;
	text_overlay.text_shadow_color = text_shadow_color;
	text_overlay.text_format = text_format;
	text_overlay.text_style = text_style;


	// Inititialize main shadowed text rect
	if (text_shadow_x_offset >= 0 && text_shadow_y_offset >= 0)
	{
		// x and y offsets are both positive
		SetRect(&text_overlay.text_shadow_rect[0],
			window_rect.left,
			window_rect.top,
			window_rect.right - text_shadow_x_offset,
			window_rect.bottom - text_shadow_y_offset);
	}
	else if (text_shadow_x_offset <= 0 && text_shadow_y_offset >= 0)
	{
		// x offset is negative; y offset is positive
		SetRect(&text_overlay.text_shadow_rect[0],
			window_rect.left - text_shadow_x_offset,
			window_rect.top,
			window_rect.right,
			window_rect.bottom - text_shadow_y_offset);
	}
	else if (text_shadow_x_offset >= 0 && text_shadow_y_offset <= 0)
	{
		// x offset is positive; y offset is negative
		SetRect(&text_overlay.text_shadow_rect[0],
			window_rect.left,
			window_rect.top - text_shadow_y_offset,
			window_rect.right - text_shadow_x_offset,
			window_rect.bottom);
	}
	else
	{
		// x and y offsets are both negative
		SetRect(&text_overlay.text_shadow_rect[0],
			window_rect.left - text_shadow_x_offset,
			window_rect.top - text_shadow_y_offset,
			window_rect.right,
			window_rect.bottom);
	}


	// Initialize text shadow rect
	SetRect(&text_overlay.text_shadow_rect[1],
		text_overlay.text_shadow_rect[0].left + text_shadow_x_offset,
		text_overlay.text_shadow_rect[0].top + text_shadow_y_offset,
		text_overlay.text_shadow_rect[0].right + text_shadow_x_offset,
		text_overlay.text_shadow_rect[0].bottom + text_shadow_y_offset);


	// Inititialize main bordered text rect
	SetRect(&text_overlay.text_outline_rect[0],
		window_rect.left + text_border_thickness,
		window_rect.top + text_border_thickness,
		window_rect.right - text_border_thickness,
		window_rect.bottom - text_border_thickness);


	// Initialize text border rects
	SetRect(&text_overlay.text_outline_rect[1],
		text_overlay.text_outline_rect[0].left - text_border_thickness,
		text_overlay.text_outline_rect[0].top - text_border_thickness,
		text_overlay.text_outline_rect[0].right - text_border_thickness,
		text_overlay.text_outline_rect[0].bottom - text_border_thickness);
	SetRect(&text_overlay.text_outline_rect[2],
		text_overlay.text_outline_rect[0].left + text_border_thickness,
		text_overlay.text_outline_rect[0].top - text_border_thickness,
		text_overlay.text_outline_rect[0].right + text_border_thickness,
		text_overlay.text_outline_rect[0].bottom - text_border_thickness);
	SetRect(&text_overlay.text_outline_rect[3],
		text_overlay.text_outline_rect[0].left - text_border_thickness,
		text_overlay.text_outline_rect[0].top + text_border_thickness,
		text_overlay.text_outline_rect[0].right - text_border_thickness,
		text_overlay.text_outline_rect[0].bottom + text_border_thickness);
	SetRect(&text_overlay.text_outline_rect[4],
		text_overlay.text_outline_rect[0].left + text_border_thickness,
		text_overlay.text_outline_rect[0].top + text_border_thickness,
		text_overlay.text_outline_rect[0].right + text_border_thickness,
		text_overlay.text_outline_rect[0].bottom + text_border_thickness);
	SetRect(&text_overlay.text_outline_rect[5],
		text_overlay.text_outline_rect[0].left - text_border_thickness,
		text_overlay.text_outline_rect[0].top,
		text_overlay.text_outline_rect[0].right - text_border_thickness,
		text_overlay.text_outline_rect[0].bottom);
	SetRect(&text_overlay.text_outline_rect[6],
		text_overlay.text_outline_rect[0].left,
		text_overlay.text_outline_rect[0].top - text_border_thickness,
		text_overlay.text_outline_rect[0].right,
		text_overlay.text_outline_rect[0].bottom - text_border_thickness);
	SetRect(&text_overlay.text_outline_rect[7],
		text_overlay.text_outline_rect[0].left + text_border_thickness,
		text_overlay.text_outline_rect[0].top,
		text_overlay.text_outline_rect[0].right + text_border_thickness,
		text_overlay.text_outline_rect[0].bottom);
	SetRect(&text_overlay.text_outline_rect[8],
		text_overlay.text_outline_rect[0].left,
		text_overlay.text_outline_rect[0].top + text_border_thickness,
		text_overlay.text_outline_rect[0].right,
		text_overlay.text_outline_rect[0].bottom + text_border_thickness);

	text_overlay_feed_text = std::string(_SP_DEFAULT_OVERLAY_TEXT_MESSAGE_);
	text_overlay.text = text_overlay_feed_text.c_str();

	text_overlay.enabled = false;
}

void myIDirect3DDevice9::SP_DX9_set_text_height(int new_text_height)
{
	bool reenable_overlay;
	if (text_overlay.enabled)
	{
		reenable_overlay = true;
		text_overlay.enabled = false;
	}
	else
	{
		reenable_overlay = false;
	}

	D3DXFONT_DESC font_desc;
	HRESULT font_desc_hr = text_overlay.font->GetDesc(&font_desc);
	if (FAILED(font_desc_hr))
	{
		// Handle error
	}

	// Re-initialize overlay font
	HRESULT font_hr = D3DXCreateFont(
		m_pIDirect3DDevice9,	// D3D device
		new_text_height,		// Height
		font_desc.Width,		// Width
		font_desc.Weight,		// Weight
		font_desc.MipLevels,	// MipLevels, 0 = autogen mipmaps
		font_desc.Italic,		// Italic
		font_desc.CharSet,		// CharSet
		font_desc.OutputPrecision, // OutputPrecision
		font_desc.Quality,		// Quality
		font_desc.PitchAndFamily, // PitchAndFamily
		font_desc.FaceName,		// pFaceName
		&text_overlay.font);	// ppFont
	if (FAILED(font_hr))
	{
		// Handle error
	}

	if (reenable_overlay)
	{
		text_overlay.enabled = true;
	}
}

// Adds a message to the text overlay feed; the message expires in a number of
//	milliseconds denoted by the duration parameter.
void myIDirect3DDevice9::print_to_overlay_feed(const char *message, unsigned long long duration, bool include_timestamp)
{
	bool reenable_overlay;
	if (text_overlay.enabled)
	{
		reenable_overlay = true;
		text_overlay.enabled = false;
	}
	else
	{
		reenable_overlay = false;
	}

	unsigned long long ms_since_epoch =	std::chrono::system_clock::now().time_since_epoch() /
										std::chrono::milliseconds(1);

	SP_DX9_TEXT_OVERLAY_FEED_ENTRY new_message;
	new_message.message = message;
	if (duration == 0)
	{
		// Messages with an expire time of 0 never expire
		new_message.expire_time = 0;
	}
	else
	{
		new_message.expire_time = (ms_since_epoch + duration);
	}
	new_message.show_timestamp = include_timestamp;


	// Build timestamp string
	time_t current_time_t = time(0);
	struct tm * current_time = localtime(&current_time_t);
	new_message.timestamp[0] = '[';
	if (current_time->tm_hour < 10)
	{
		new_message.timestamp[1] = '0';
		new_message.timestamp[2] = std::to_string(current_time->tm_hour).c_str()[0];
	}
	else
	{
		new_message.timestamp[1] = std::to_string(current_time->tm_hour).c_str()[0];
		new_message.timestamp[2] = std::to_string(current_time->tm_hour).c_str()[1];
	}
	new_message.timestamp[3] = ':';
	if (current_time->tm_min < 10)
	{
		new_message.timestamp[4] = '0';
		new_message.timestamp[5] = std::to_string(current_time->tm_min).c_str()[0];
	}
	else
	{
		new_message.timestamp[4] = std::to_string(current_time->tm_min).c_str()[0];
		new_message.timestamp[5] = std::to_string(current_time->tm_min).c_str()[1];
	}
	new_message.timestamp[6] = ':';
	if (current_time->tm_sec < 10)
	{
		new_message.timestamp[7] = '0';
		new_message.timestamp[8] = std::to_string(current_time->tm_sec).c_str()[0];
	}
	else
	{
		new_message.timestamp[7] = std::to_string(current_time->tm_sec).c_str()[0];
		new_message.timestamp[8] = std::to_string(current_time->tm_sec).c_str()[1];
	}
	new_message.timestamp[9] = ']';
	new_message.timestamp[10] = ' ';
	new_message.timestamp[11] = '\0';
	// Finished constructing timestamp string

	text_overlay_feed.push_back(new_message);

	if (reenable_overlay)
	{
		text_overlay.enabled = true;
	}
}

void myIDirect3DDevice9::clean_text_overlay_feed()
{
	unsigned long long ms_since_epoch = std::chrono::system_clock::now().time_since_epoch() /
										std::chrono::milliseconds(1);

	std::list<SP_DX9_TEXT_OVERLAY_FEED_ENTRY>::const_iterator iterator = text_overlay_feed.begin();
	while (text_overlay.enabled && iterator != text_overlay_feed.end())
	{
		if (text_overlay.enabled && (*iterator).expire_time != 0 && ms_since_epoch >= (*iterator).expire_time)
		{
			iterator = text_overlay_feed.erase(iterator);
		}
		else
		{
			iterator++;
		}
	}
}

void myIDirect3DDevice9::build_text_overlay_feed_string()
{
	text_overlay_feed_text.clear();

	std::list<SP_DX9_TEXT_OVERLAY_FEED_ENTRY>::const_iterator iterator;
	for (iterator = text_overlay_feed.begin(); iterator != text_overlay_feed.end(); iterator++)
	{
		if (iterator != text_overlay_feed.begin())
		{
			text_overlay_feed_text.append("\n");
		}
		if ((*iterator).show_timestamp)
		{
			text_overlay_feed_text.append(iterator->timestamp);
		}
		text_overlay_feed_text.append((*iterator).message);
	}

	text_overlay.text = text_overlay_feed_text.c_str();
}