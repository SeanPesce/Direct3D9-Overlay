// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#pragma once

#include <ctime>
#include <list>
#include <string>
#include <d3dx9core.h>

// Default overlay text attributes
#define _SP_DEFAULT_TEXT_HEIGHT_ 28
#define _SP_DEFAULT_TEXT_SHADOW_X_OFFSET_ 2
#define _SP_DEFAULT_TEXT_SHADOW_Y_OFFSET_ 2
#define _SP_DEFAULT_TEXT_BORDER_THICKNESS_ 2
#define _SP_DEFAULT_TEXT_COLOR_ D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f)
#define _SP_DEFAULT_TEXT_BORDER_COLOR_ D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f)
#define _SP_DEFAULT_TEXT_SHADOW_COLOR_ D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f)
#define _SP_DEFAULT_TEXT_FORMAT_ (DT_NOCLIP | DT_TOP | DT_CENTER)
#define _SP_DEFAULT_TEXT_STYLE_ SP_DX9_BORDERED_TEXT
#define _SP_DEFAULT_TEXT_FONT_ "Arial"
// Total number of supported overlay text feed colors in multicolor mode
#define _SP_DX9_TEXT_COLOR_COUNT_ 10
// Default overlay text feed title
#define _SP_DEFAULT_OVERLAY_TEXT_FEED_TITLE_ "Direct3D Overlay by Sean Pesce"
// Text overlay watermark string length
#define _SP_OL_TXT_WATERMARK_STR_LENGTH_ 128

// Enumerator whose values denote the supported overlay text styles (outlined, shadowed, or plain)
enum SP_DX9_TEXT_OVERLAY_STYLES {
	SP_DX9_BORDERED_TEXT,
	SP_DX9_SHADOWED_TEXT,
	SP_DX9_PLAIN_TEXT
};
// Enumerator whose values denote the supported overlay text feed colors in multicolor mode
enum SP_DX9_TEXT_OVERLAY_COLORS_ENUM {
	SP_DX9_TEXT_COLOR_WHITE_OR_DEFAULT,
	SP_DX9_TEXT_COLOR_RED,
	SP_DX9_TEXT_COLOR_ORANGE,
	SP_DX9_TEXT_COLOR_YELLOW,
	SP_DX9_TEXT_COLOR_GREEN,
	SP_DX9_TEXT_COLOR_CYAN,
	SP_DX9_TEXT_COLOR_BLUE,
	SP_DX9_TEXT_COLOR_PURPLE,
	SP_DX9_TEXT_COLOR_PINK,
	SP_DX9_TEXT_COLOR_CYCLE_ALL
};
// Enumerator whose values denote whether to display the various overlay text watermark attributes
enum SP_DX9_TEXT_OVERLAY_WATERMARK_ENUM {
	SP_DX9_WATERMARK_TITLE = 1,
	SP_DX9_WATERMARK_DATE = 2,
	SP_DX9_WATERMARK_TIME = 4,
	SP_DX9_WATERMARK_FPS = 8
};
// Data structure for a single message entry in the overlay text feed:
typedef struct SP_DX9_TEXT_OVERLAY_FEED_ENTRY {
	std::string message = std::string("");	// Message
	unsigned long long expire_time;			// Time at which the message expires
	bool show_timestamp;					// Enable/disable prepending a timestamp to the message
	char timestamp[12];						// Char buffer for timestamp
	unsigned int text_color;				// Text color
} SP_DX9_TEXT_OVERLAY_FEED_ENTRY;
// Data structure for the overlay text feed:
typedef struct SP_DX9_FULLSCREEN_TEXT_OVERLAY {
	bool enabled;				// Enable/disable overlay text feed
	RECT text_plain_rect;		// Screenspace for positioning plain text
	RECT text_shadow_rect[2];	// Screenspace for positioning shadowed text
	RECT text_outline_rect[9];	// Screenspace for positioning outlined text
	int text_shadow_x_offset;	// Horizontal offset (in pixels) for text shadow
	int text_shadow_y_offset;	// Vertical offset (in pixels) for text shadow
	unsigned int text_outline_thickness; // Text outline thickness (in pixels)
	ID3DXFont *font;			// Font data structure which holds attributes for the overlay text feed font
	TCHAR font_name[32];		// Character array to hold the font family name
	LPCTSTR text[_SP_DX9_TEXT_COLOR_COUNT_]; // Array of pointers to text buffers that will hold the text data of each color
	DWORD text_format;			// Determines horizontal and vertical positioning of text on screen
	int text_style;				// Overlay text style (Bordered, shadowed, or plain)
	D3DXCOLOR text_color;		// Default text color (only color if multicolor mode is disabled)
	D3DXCOLOR text_border_color; // Text outline color
	D3DXCOLOR text_shadow_color; // Text shadow color
} SP_DX9_FULLSCREEN_TEXT_OVERLAY;

class myIDirect3DDevice9 : public IDirect3DDevice9
{
public:
	
	UINT id; // Adapter number
	bool is_windowed; // Specifies whether the program is running in windowed or exclusive full-screen mode
	bool using_present = false;
	bool in_scene = false; // Indicates whether the program is currently between a BeginScene() and EndScene() call
	HWND *game_window = NULL; // Main game window (should be pointer to focus_window or device_window)
	RECT *game_window_rect = NULL;
	HWND focus_window = NULL; // Game focus window
	RECT focus_window_rect;
	HWND device_window = NULL; // Game device window
	RECT device_window_rect;
	RECT back_buffer_rect;
	SP_DX9_FULLSCREEN_TEXT_OVERLAY text_overlay; // Data structure for overlay text feed
	bool use_alt_fps_counter = false; // Some games might need frames to be counted from the Present() method instead of the normal EndScene() method
	bool drawing_to_display = false;
	int fps; // Number of frames rendered in the last second
	int present_calls = 0; // Number of times Present() was called thus far in the current second (used to determine FPS)
	int endscene_calls = 0; // Number of times EndScene() was called thus far in the current second (used to determine FPS)
	int get_back_buffer_calls = 0; // Number of times GetBackBuffer() was called thus far in the current second (used to FPS/Vsync)
	bool render_ol; // Signifies that EndScene() is called exactly once per call to Present()
	UINT_PTR fps_timer_id; // ID of the timer used to update FPS values every second
	int show_text_watermark; // Denotes whether to display the various text watermark attributes
	char text_watermark[_SP_OL_TXT_WATERMARK_STR_LENGTH_]; // Buffer to hold the text overlay watermark string
	std::list<SP_DX9_TEXT_OVERLAY_FEED_ENTRY> text_overlay_feed; // List of entries in the overlay text feed
	std::string text_overlay_feed_text[_SP_DX9_TEXT_COLOR_COUNT_]; // Array of strings to hold text (of each supported color) that will be printed to the text overlay
	bool multicolor_overlay_text_feed_enabled; // If disabled, all printed text is the color indicated by text_overlay.text_color
	DWORD cycle_all_colors_current_rgb_vals[3]; // Current RGB values for text whose color cycles through the color spectrum
	ID3DXFont* text_overlay_old_font; // Unused font from last call to SP_DX9_set_text_height(), which will be released when SP_DX9_set_text_height() is called again (temporarily stored to avoid race conditions)

	// ARGB values for the supported overlay text feed colors
	D3DXCOLOR dx9_text_colors[_SP_DX9_TEXT_COLOR_COUNT_] =
	{
		D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),	// White
		D3DXCOLOR(0xFFF44E42),				// Red
		D3DXCOLOR(0xFFF4BC42),				// Orange
		D3DXCOLOR(0xFFEEF442),				// Yellow
		D3DXCOLOR(0xFF42F450),				// Green
		D3DXCOLOR(0xFF42D4F4),				// Cyan
		D3DXCOLOR(0xFF4286F4),				// Blue
		D3DXCOLOR(0xFFBC42F4),				// Purple
		D3DXCOLOR(0xFFF442EB),				// Pink
		D3DXCOLOR(0xFF000000)				// Starting color when cycling all colors
	};

	myIDirect3DDevice9(UINT Adapter, IDirect3DDevice9* pOriginal, HWND new_focus_window, D3DPRESENT_PARAMETERS *present_params); // Constructor
	virtual ~myIDirect3DDevice9(void);

	// Original DX9 function definitions:
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObj);
	ULONG   __stdcall AddRef(void);
	ULONG   __stdcall Release(void);
	HRESULT __stdcall TestCooperativeLevel(void);
	UINT    __stdcall GetAvailableTextureMem(void);
	HRESULT __stdcall EvictManagedResources(void);
	HRESULT __stdcall GetDirect3D(IDirect3D9** ppD3D9);
	HRESULT __stdcall GetDeviceCaps(D3DCAPS9* pCaps);
	HRESULT __stdcall GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode);
	HRESULT __stdcall GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters);
	HRESULT __stdcall SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap);
	void    __stdcall SetCursorPosition(int X, int Y, DWORD Flags);
	BOOL    __stdcall ShowCursor(BOOL bShow);
	HRESULT __stdcall CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain);
	HRESULT __stdcall GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain);
	UINT    __stdcall GetNumberOfSwapChains(void);
	HRESULT __stdcall Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
	HRESULT __stdcall Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	HRESULT __stdcall GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer);
	HRESULT __stdcall GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus);
	HRESULT __stdcall SetDialogBoxMode(BOOL bEnableDialogs);
	void    __stdcall SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	void    __stdcall GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp);
	HRESULT __stdcall CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle);
	HRESULT __stdcall CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle);
	HRESULT __stdcall CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle);
	HRESULT __stdcall CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle);
	HRESULT __stdcall CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle);
	HRESULT __stdcall CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	HRESULT __stdcall CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	HRESULT __stdcall UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint);
	HRESULT __stdcall UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture);
	HRESULT __stdcall GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface);
	HRESULT __stdcall GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface);
	HRESULT __stdcall StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
	HRESULT __stdcall ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color);
	HRESULT __stdcall CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle);
	HRESULT __stdcall SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget);
	HRESULT __stdcall GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget);
	HRESULT __stdcall SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
	HRESULT __stdcall GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface);
	HRESULT __stdcall BeginScene(void);
	HRESULT __stdcall EndScene(void);
	HRESULT __stdcall Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	HRESULT __stdcall SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	HRESULT __stdcall GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
	HRESULT __stdcall MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	HRESULT __stdcall SetViewport(CONST D3DVIEWPORT9* pViewport);
	HRESULT __stdcall GetViewport(D3DVIEWPORT9* pViewport);
	HRESULT __stdcall SetMaterial(CONST D3DMATERIAL9* pMaterial);
	HRESULT __stdcall GetMaterial(D3DMATERIAL9* pMaterial);
	HRESULT __stdcall SetLight(DWORD Index, CONST D3DLIGHT9* pLight);
	HRESULT __stdcall GetLight(DWORD Index, D3DLIGHT9* pLight);
	HRESULT __stdcall LightEnable(DWORD Index, BOOL Enable);
	HRESULT __stdcall GetLightEnable(DWORD Index, BOOL* pEnable);
	HRESULT __stdcall SetClipPlane(DWORD Index, CONST float* pPlane);
	HRESULT __stdcall GetClipPlane(DWORD Index, float* pPlane);
	HRESULT __stdcall SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
	HRESULT __stdcall GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue);
	HRESULT __stdcall CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB);
	HRESULT __stdcall BeginStateBlock(void);
	HRESULT __stdcall EndStateBlock(IDirect3DStateBlock9** ppSB);
	HRESULT __stdcall SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus);
	HRESULT __stdcall GetClipStatus(D3DCLIPSTATUS9* pClipStatus);
	HRESULT __stdcall GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture);
	HRESULT __stdcall SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture);
	HRESULT __stdcall GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	HRESULT __stdcall SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	HRESULT __stdcall GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue);
	HRESULT __stdcall SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	HRESULT __stdcall ValidateDevice(DWORD* pNumPasses);
	HRESULT __stdcall SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries);
	HRESULT __stdcall GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries);
	HRESULT __stdcall SetCurrentTexturePalette(UINT PaletteNumber);
	HRESULT __stdcall GetCurrentTexturePalette(UINT *PaletteNumber);
	HRESULT __stdcall SetScissorRect(CONST RECT* pRect);
	HRESULT __stdcall GetScissorRect(RECT* pRect);
	HRESULT __stdcall SetSoftwareVertexProcessing(BOOL bSoftware);
	BOOL    __stdcall GetSoftwareVertexProcessing(void);
	HRESULT __stdcall SetNPatchMode(float nSegments);
	float   __stdcall GetNPatchMode(void);
	HRESULT __stdcall DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	HRESULT __stdcall DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	HRESULT __stdcall DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	HRESULT __stdcall DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	HRESULT __stdcall ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags);
	HRESULT __stdcall CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl);
	HRESULT __stdcall SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);
	HRESULT __stdcall GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl);
	HRESULT __stdcall SetFVF(DWORD FVF);
	HRESULT __stdcall GetFVF(DWORD* pFVF);
	HRESULT __stdcall CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader);
	HRESULT __stdcall SetVertexShader(IDirect3DVertexShader9* pShader);
	HRESULT __stdcall GetVertexShader(IDirect3DVertexShader9** ppShader);
	HRESULT __stdcall SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	HRESULT __stdcall GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	HRESULT __stdcall SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	HRESULT __stdcall GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	HRESULT __stdcall SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	HRESULT __stdcall GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	HRESULT __stdcall SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride);
	HRESULT __stdcall GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride);
	HRESULT __stdcall SetStreamSourceFreq(UINT StreamNumber, UINT Divider);
	HRESULT __stdcall GetStreamSourceFreq(UINT StreamNumber, UINT* Divider);
	HRESULT __stdcall SetIndices(IDirect3DIndexBuffer9* pIndexData);
	HRESULT __stdcall GetIndices(IDirect3DIndexBuffer9** ppIndexData);
	HRESULT __stdcall CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader);
	HRESULT __stdcall SetPixelShader(IDirect3DPixelShader9* pShader);
	HRESULT __stdcall GetPixelShader(IDirect3DPixelShader9** ppShader);
	HRESULT __stdcall SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount);
	HRESULT __stdcall GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount);
	HRESULT __stdcall SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount);
	HRESULT __stdcall GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount);
	HRESULT __stdcall SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount);
	HRESULT __stdcall GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount);
	HRESULT __stdcall DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	HRESULT __stdcall DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	HRESULT __stdcall DeletePatch(UINT Handle);
	HRESULT __stdcall CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery);
	// End of original DX9 function definitions

	// Public overlay function definitions:
	void myIDirect3DDevice9::SP_DX9_set_text_height(int new_text_height); // Changes the font height of the overlay text feed
	// NOTE: Overlay text feed currently does NOT support multi-line messages. Print each line as a separate message instead.
	void myIDirect3DDevice9::print_to_overlay_feed(const char *message, unsigned long long duration, bool include_timestamp); // Prints default-colored text to the overlay text feed
	void myIDirect3DDevice9::print_to_overlay_feed(const char *message, unsigned long long duration, bool include_timestamp, int text_color); // Prints text to the overlay text feed in the specified color
	RECT *myIDirect3DDevice9::get_viewport_as_rect(RECT *rect); // Constructs a RECT struct from the device viewport
	RECT *myIDirect3DDevice9::get_viewport_as_rect(RECT *rect, D3DVIEWPORT9 *viewport); // Constructs a RECT struct from the device viewport (and stores the viewport)
	void myIDirect3DDevice9::print_debug_data(unsigned long long duration, bool show_timestamp); // Prints various game window data to the overlay text feed

private:
	IDirect3DDevice9 *m_pIDirect3DDevice9;

	// Private overlay function definitions:
	void myIDirect3DDevice9::SP_DX9_init_text_overlay(int text_height, unsigned int text_border_thickness, int text_shadow_x_offset, int text_shadow_y_offset, D3DXCOLOR text_color, D3DXCOLOR text_border_color, D3DXCOLOR text_shadow_color, DWORD text_format, int text_style); // Initializes overlay text feed data structure
	void myIDirect3DDevice9::SP_DX9_draw_overlay_text_feed(); // Renders the overlay text feed (monochromatic)
	void myIDirect3DDevice9::SP_DX9_draw_overlay_text_feed_multicolor(); // Renders the overlay text feed (multicolor)
	void myIDirect3DDevice9::clean_text_overlay_feed(); // Removes expired messages from the overlay text feed
	void myIDirect3DDevice9::build_text_overlay_feed_string(); // Constructs the overlay text feed from the current list of messages (monochromatic)
	void myIDirect3DDevice9::build_text_overlay_feed_string_multicolor(); // Constructs the overlay text feed from the current list of messages (multicolor)
	void myIDirect3DDevice9::cycle_text_colors(); // Calculates the next ARGB color value for text whose color cycles through all colors
	void myIDirect3DDevice9::init_text_overlay_rects(); // Initializes the RECT structures that denote the usable screenspace for the overlay text feed
	void myIDirect3DDevice9::init_text_overlay_rects(RECT new_rect); // Initializes the RECT structures that denote the usable screenspace for the overlay text feed
	void myIDirect3DDevice9::update_overlay_text_watermark(); // Updates the various text watermark attributes
	RECT myIDirect3DDevice9::update_overlay_parameters();
};

void CALLBACK update_fps(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime); // Updates the FPS counter every second
void rect_to_string(RECT *rect, const char *rect_name, std::string *str); // Constructs a string describing the specified RECT struct and stores it in the given std::string
