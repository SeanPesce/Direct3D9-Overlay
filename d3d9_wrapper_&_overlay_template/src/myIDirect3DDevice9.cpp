// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#include "StdAfx.h"
#include "myIDirect3DDevice9.h"
#include <d3dx9core.h>

#define _SP_DEFAULT_TEXT_SHADOW_OFFSET_ 2
#define _SP_DEFAULT_TEXT_BORDER_THICKNESS_ 2

enum overlay_text_types {
	SP_TEXT_OUTLINED,
	SP_TEXT_SHADOWED,
	SP_TEXT_SOLID_BACKGROUND
};

// Data structures for the text overlay:
extern D3DRECT text_background;
extern RECT text_box;
extern RECT text_shadow_box;
extern RECT text_outline_boxes[8];
extern ID3DXFont* SP_font;
extern TCHAR *SP_font_name;

// Constants & Variables:
extern bool font_initialized;
extern int overlay_text_type;
extern const char *example_overlay_text;

void SP_init_text_box(RECT *text_box, D3DRECT *text_background, int x1, int y1, int x2, int y2);
void SP_display_text_box(LPDIRECT3DDEVICE9 device, RECT *text_box, D3DRECT *text_background, ID3DXFont *font, const char *text);
void SP_init_text_shadowed(RECT *text_box, RECT *text_shadow_box, int x1, int y1, int x2, int y2, int shadow_x_offset, int shadow_y_offset);
void SP_init_text_shadowed(RECT *text_box, RECT *text_shadow_box, int x1, int y1, int x2, int y2);
void SP_display_text_shadowed(LPDIRECT3DDEVICE9 device, RECT *text_box, RECT *text_shadow_box, ID3DXFont *font, const char *text);
void SP_init_text_outlined(RECT *text_box, RECT *text_outline_boxes, int x1, int y1, int x2, int y2);
void SP_display_text_outlined(LPDIRECT3DDEVICE9 device, RECT *text_box, RECT *text_outline_boxes, ID3DXFont *font, const char *text);

myIDirect3DDevice9::myIDirect3DDevice9(IDirect3DDevice9* pOriginal)
{
    m_pIDirect3DDevice9 = pOriginal; // Store the pointer to original object
}

myIDirect3DDevice9::~myIDirect3DDevice9(void)
{
}

HRESULT myIDirect3DDevice9::QueryInterface (REFIID riid, void** ppvObj)
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

HRESULT myIDirect3DDevice9::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
    return(m_pIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT myIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    return(m_pIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT myIDirect3DDevice9::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{
    return(m_pIDirect3DDevice9->SetCursorProperties(XHotSpot,YHotSpot,pCursorBitmap));
}

void    myIDirect3DDevice9::SetCursorPosition(int X,int Y,DWORD Flags)
{
    return(m_pIDirect3DDevice9->SetCursorPosition(X,Y,Flags));
}

BOOL    myIDirect3DDevice9::ShowCursor(BOOL bShow)
{
    return(m_pIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT myIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)  
{
    return(m_pIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters,pSwapChain));
}

HRESULT myIDirect3DDevice9::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{
    return(m_pIDirect3DDevice9->GetSwapChain(iSwapChain,pSwapChain));
}

UINT    myIDirect3DDevice9::GetNumberOfSwapChains(void)
{
    return(m_pIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT myIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    return(m_pIDirect3DDevice9->Reset(pPresentationParameters));
}

HRESULT myIDirect3DDevice9::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{
    // We may want to draw own things here before flipping surfaces
    // ... draw own stuff, etc ...
    
    // Call original routine
	return m_pIDirect3DDevice9->Present( pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT myIDirect3DDevice9::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
    return(m_pIDirect3DDevice9->GetBackBuffer(iSwapChain,iBackBuffer,Type,ppBackBuffer));
}

HRESULT myIDirect3DDevice9::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{
    return(m_pIDirect3DDevice9->GetRasterStatus(iSwapChain,pRasterStatus));
}

HRESULT myIDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
    return(m_pIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void    myIDirect3DDevice9::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
    return(m_pIDirect3DDevice9->SetGammaRamp(iSwapChain,Flags,pRamp));
}

void    myIDirect3DDevice9::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{
    return(m_pIDirect3DDevice9->GetGammaRamp(iSwapChain,pRamp));
}

HRESULT myIDirect3DDevice9::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
    return(m_pIDirect3DDevice9->CreateTexture(Width,Height,Levels,Usage,Format,Pool,ppTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
    return(m_pIDirect3DDevice9->CreateVolumeTexture(Width,Height,Depth,Levels,Usage,Format,Pool,ppVolumeTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
    return(m_pIDirect3DDevice9->CreateCubeTexture(EdgeLength,Levels,Usage,Format,Pool,ppCubeTexture,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
    return(m_pIDirect3DDevice9->CreateVertexBuffer(Length,Usage,FVF,Pool,ppVertexBuffer,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
    return(m_pIDirect3DDevice9->CreateIndexBuffer(Length,Usage,Format,Pool,ppIndexBuffer,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return(m_pIDirect3DDevice9->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return(m_pIDirect3DDevice9->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality,Discard,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
    return(m_pIDirect3DDevice9->UpdateSurface(pSourceSurface,pSourceRect,pDestinationSurface,pDestPoint));
}

HRESULT myIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{
    return(m_pIDirect3DDevice9->UpdateTexture(pSourceTexture,pDestinationTexture));
}

HRESULT myIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
    return(m_pIDirect3DDevice9->GetRenderTargetData(pRenderTarget,pDestSurface));
}

HRESULT myIDirect3DDevice9::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{
    return(m_pIDirect3DDevice9->GetFrontBufferData(iSwapChain,pDestSurface));
}

HRESULT myIDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
    return(m_pIDirect3DDevice9->StretchRect(pSourceSurface,pSourceRect,pDestSurface,pDestRect,Filter));
}

HRESULT myIDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{
    return(m_pIDirect3DDevice9->ColorFill(pSurface,pRect,color));
}

HRESULT myIDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return(m_pIDirect3DDevice9->CreateOffscreenPlainSurface(Width,Height,Format,Pool,ppSurface,pSharedHandle));
}

HRESULT myIDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
    return(m_pIDirect3DDevice9->SetRenderTarget(RenderTargetIndex,pRenderTarget));
}

HRESULT myIDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
    return(m_pIDirect3DDevice9->GetRenderTarget(RenderTargetIndex,ppRenderTarget));
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
	if (!font_initialized)
	{
		overlay_text_type = SP_TEXT_OUTLINED;

		// Initialize font
		HRESULT SP_font_hr = D3DXCreateFont(
			m_pIDirect3DDevice9,	// D3D device (can also use "this")
			28,						// Height
			0,						// Width
			FW_BOLD,				// Weight
			1,						// MipLevels, 0 = autogen mipmaps
			FALSE,					// Italic
			DEFAULT_CHARSET,		// CharSet
			OUT_DEFAULT_PRECIS,		// OutputPrecision
			ANTIALIASED_QUALITY,	// Quality
			DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
			SP_font_name,				// pFaceName
			&SP_font);				// ppFont

		if (FAILED(SP_font_hr))
		{
			// Handle error
		}

		SP_init_text_box(&text_box, &text_background, 2, 2, 352, 32);

		SP_init_text_shadowed(&text_box, &text_shadow_box, 2, 2, 352, 32);

		SP_init_text_outlined(&text_box, text_outline_boxes, 2, 2, 352, 32);

		font_initialized = true;
	}

	switch (overlay_text_type) {
		case SP_TEXT_SOLID_BACKGROUND:
			SP_display_text_box(m_pIDirect3DDevice9, &text_box, &text_background, SP_font, example_overlay_text);
			break;
		case SP_TEXT_SHADOWED:
			SP_display_text_shadowed(m_pIDirect3DDevice9, &text_box, &text_shadow_box, SP_font, example_overlay_text);
			break;
		case SP_TEXT_OUTLINED:
		default:
			SP_display_text_outlined(m_pIDirect3DDevice9, &text_box, text_outline_boxes, SP_font, example_overlay_text);
			break;
	}

    return(m_pIDirect3DDevice9->EndScene());
}

HRESULT myIDirect3DDevice9::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
    return(m_pIDirect3DDevice9->Clear(Count,pRects,Flags,Color,Z,Stencil));
}

HRESULT myIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
    return(m_pIDirect3DDevice9->SetTransform(State,pMatrix));
}

HRESULT myIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{
    return(m_pIDirect3DDevice9->GetTransform(State,pMatrix));
}

HRESULT myIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
    return(m_pIDirect3DDevice9->MultiplyTransform(State,pMatrix));
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

HRESULT myIDirect3DDevice9::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{
    return(m_pIDirect3DDevice9->SetLight(Index,pLight));
}

HRESULT myIDirect3DDevice9::GetLight(DWORD Index,D3DLIGHT9* pLight)
{
    return(m_pIDirect3DDevice9->GetLight(Index,pLight));
}

HRESULT myIDirect3DDevice9::LightEnable(DWORD Index,BOOL Enable)
{
    return(m_pIDirect3DDevice9->LightEnable(Index,Enable));
}

HRESULT myIDirect3DDevice9::GetLightEnable(DWORD Index,BOOL* pEnable)
{
    return(m_pIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT myIDirect3DDevice9::SetClipPlane(DWORD Index,CONST float* pPlane)
{
    return(m_pIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT myIDirect3DDevice9::GetClipPlane(DWORD Index,float* pPlane)
{
    return(m_pIDirect3DDevice9->GetClipPlane(Index,pPlane));
}

HRESULT myIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
    return(m_pIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT myIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{
    return(m_pIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT myIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{
    return(m_pIDirect3DDevice9->CreateStateBlock(Type,ppSB));
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
    return(m_pIDirect3DDevice9->GetClipStatus( pClipStatus));
}

HRESULT myIDirect3DDevice9::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{
    return(m_pIDirect3DDevice9->GetTexture(Stage,ppTexture));
}

HRESULT myIDirect3DDevice9::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
    return(m_pIDirect3DDevice9->SetTexture(Stage,pTexture));
}

HRESULT myIDirect3DDevice9::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
    return(m_pIDirect3DDevice9->GetTextureStageState(Stage,Type, pValue));
}

HRESULT myIDirect3DDevice9::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
    return(m_pIDirect3DDevice9->SetTextureStageState(Stage,Type,Value));
}

HRESULT myIDirect3DDevice9::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
    return(m_pIDirect3DDevice9->GetSamplerState(Sampler,Type, pValue));
}

HRESULT myIDirect3DDevice9::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
    return(m_pIDirect3DDevice9->SetSamplerState(Sampler,Type,Value));
}

HRESULT myIDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
    return(m_pIDirect3DDevice9->ValidateDevice( pNumPasses));
}

HRESULT myIDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
    return(m_pIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
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
    return(m_pIDirect3DDevice9->SetScissorRect( pRect));
}

HRESULT myIDirect3DDevice9::GetScissorRect( RECT* pRect)
{
    return(m_pIDirect3DDevice9->GetScissorRect( pRect));
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

HRESULT myIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
    return(m_pIDirect3DDevice9->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
    return(m_pIDirect3DDevice9->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount));
}

HRESULT myIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
    return(m_pIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
    return(m_pIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
    return(m_pIDirect3DDevice9->ProcessVertices( SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT myIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
    return(m_pIDirect3DDevice9->CreateVertexDeclaration( pVertexElements,ppDecl));
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

HRESULT myIDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
    return(m_pIDirect3DDevice9->CreateVertexShader(pFunction,ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
    return(m_pIDirect3DDevice9->SetVertexShader(pShader));
}

HRESULT myIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
    return(m_pIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
    return(m_pIDirect3DDevice9->SetVertexShaderConstantF(StartRegister,pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
    return(m_pIDirect3DDevice9->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
    return(m_pIDirect3DDevice9->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
    return(m_pIDirect3DDevice9->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
    return(m_pIDirect3DDevice9->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
    return(m_pIDirect3DDevice9->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
    return(m_pIDirect3DDevice9->SetStreamSource(StreamNumber,pStreamData,OffsetInBytes,Stride));
}

HRESULT myIDirect3DDevice9::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{
    return(m_pIDirect3DDevice9->GetStreamSource(StreamNumber,ppStreamData,OffsetInBytes,pStride));
}

HRESULT myIDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{
    return(m_pIDirect3DDevice9->SetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT myIDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{
    return(m_pIDirect3DDevice9->GetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT myIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
    return(m_pIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT myIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
    return(m_pIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT myIDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
    return(m_pIDirect3DDevice9->CreatePixelShader(pFunction,ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
    return(m_pIDirect3DDevice9->SetPixelShader(pShader));
}

HRESULT myIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
    return(m_pIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
    return(m_pIDirect3DDevice9->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
    return(m_pIDirect3DDevice9->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
    return(m_pIDirect3DDevice9->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
    return(m_pIDirect3DDevice9->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
    return(m_pIDirect3DDevice9->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
    return(m_pIDirect3DDevice9->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT myIDirect3DDevice9::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
    return(m_pIDirect3DDevice9->DrawRectPatch(Handle,pNumSegs, pRectPatchInfo));
}

HRESULT myIDirect3DDevice9::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
    return(m_pIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT myIDirect3DDevice9::DeletePatch(UINT Handle)
{
    return(m_pIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT myIDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
    return(m_pIDirect3DDevice9->CreateQuery(Type,ppQuery));
}

void SP_init_text_box(RECT *text_box, D3DRECT *text_background, int x1, int y1, int x2, int y2)
{
	SetRect(text_box, x1, y1, x2, y2);
	if (text_background != NULL)
	{
		*text_background = { x1, y1, x2, y2 };
	}
}

void SP_display_text_box(LPDIRECT3DDEVICE9 device, RECT *text_box, D3DRECT *text_background, ID3DXFont *font, const char *text)
{
	device->Clear(1, text_background, D3DCLEAR_TARGET, D3DCOLOR_ARGB(127, 0, 0, 0), 0, 0);
	font->DrawText(NULL, text, -1, text_box, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
}

void SP_init_text_shadowed(RECT *text_box, RECT *text_shadow_box, int x1, int y1, int x2, int y2, int shadow_x_offset, int shadow_y_offset)
{
	SetRect(text_box, x1, y1, x2, y2);
	if (text_shadow_box != NULL)
	{
		SetRect(text_shadow_box, x1 + shadow_x_offset, y1 + shadow_y_offset, x2 + shadow_x_offset, y2 + shadow_y_offset);
	}
}

void SP_init_text_shadowed(RECT *text_box, RECT *text_shadow_box, int x1, int y1, int x2, int y2)
{
	SetRect(text_box, x1, y1, x2, y2);
	if (text_shadow_box != NULL)
	{
		SetRect(text_shadow_box,
				x1 + _SP_DEFAULT_TEXT_SHADOW_OFFSET_,
				y1 + _SP_DEFAULT_TEXT_SHADOW_OFFSET_,
				x2 + _SP_DEFAULT_TEXT_SHADOW_OFFSET_,
				y2 + _SP_DEFAULT_TEXT_SHADOW_OFFSET_);
	}
}


void SP_display_text_shadowed(LPDIRECT3DDEVICE9 device, RECT *text_box, RECT *text_shadow_box, ID3DXFont *font, const char *text)
{
	font->DrawText(NULL, text, -1, text_shadow_box, DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, text_box, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
}

void SP_init_text_outlined(RECT *text_box, RECT *text_outline_boxes, int x1, int y1, int x2, int y2)
{
	SetRect(text_box, x1, y1, x2, y2);
	if (text_outline_boxes != NULL)
	{
		SetRect(&text_outline_boxes[0],
				x1 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				y1 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				x2 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				y2 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_);
		SetRect(&text_outline_boxes[1],
				x1 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				y1 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				x2 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				y2 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_);
		SetRect(&text_outline_boxes[2],
				x1 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				y1 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				x2 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				y2 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_);
		SetRect(&text_outline_boxes[3],
				x1 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				y1 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				x2 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_,
				y2 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_);
		SetRect(&text_outline_boxes[4],
				x1 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_ + 1,
				y1,
				x2 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_ + 1,
				y2);
		SetRect(&text_outline_boxes[5],
				x1,
				y1 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_ + 1,
				x2,
				y2 - _SP_DEFAULT_TEXT_BORDER_THICKNESS_ + 1);
		SetRect(&text_outline_boxes[6],
				x1 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_ + 1,
				y1,
				x2 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_ + 1,
				y2);
		SetRect(&text_outline_boxes[7],
				x1,
				y1 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_ + 1,
				x2,
				y2 + _SP_DEFAULT_TEXT_BORDER_THICKNESS_ + 1);
	}
}

void SP_display_text_outlined(LPDIRECT3DDEVICE9 device, RECT *text_box, RECT *text_outline_boxes, ID3DXFont *font, const char *text)
{
	font->DrawText(NULL, text, -1, &text_outline_boxes[0], DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, &text_outline_boxes[1], DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, &text_outline_boxes[2], DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, &text_outline_boxes[3], DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, &text_outline_boxes[4], DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, &text_outline_boxes[5], DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, &text_outline_boxes[6], DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, &text_outline_boxes[7], DT_NOCLIP, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	font->DrawText(NULL, text, -1, text_box, DT_NOCLIP, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
}
