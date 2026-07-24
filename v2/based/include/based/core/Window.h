#pragma once

namespace based
{
    struct WindowProperties final
    {
        std::string_view title;
        int w, h;
        int wMin, hMin;
        int flags;
        float fAspectRatio;
        Vec3 clearColor;

        WindowProperties();
    };

    enum class eFullscreenMode : uint8
    {
        kNone,
        kBorderlessFullscreen,
        kExclusiveFullscreen
    };

    enum class eCursorMode : uint8
    {
        kDefault,
        kMove,
        kPointer,
        kResize,
        kCross,
        kText,
        kUnavailable,
        kCursorCount
    };
    constexpr size_t kCursorCount = static_cast<size_t>(eCursorMode::kCursorCount);

    class IWindow : public NonCopyable
    {
        friend class WindowFactory;
    public:
        IWindow() = default;
        virtual ~IWindow() = default;
        
        virtual void Shutdown() = 0;

        virtual void PumpEvents() = 0;

        virtual void* GetNativeHandle() = 0;
        virtual IVec2 GetSize() = 0;
        std::string_view GetWindowTitle() const { return m_WindowProps.title; }
        virtual void* GetSurface() const = 0;

        virtual void SetWindowTitle(const std::string& pStrTitle) = 0;
        // This isn't really needed for all platforms, but it's harmless enough that you can just no-op it on consoles or whatever
        virtual void SetWindowFullscreen(eFullscreenMode mode) = 0;

        // All the framebuffer/presentation/swapping stuff from v1 has been removed
        // because that stuff should be handled by the Render Manager

    private:
        
        // For platform specific creation logic
        virtual bool Create_Internal() = 0;
        virtual void* CreateSurface() = 0;

    protected:
        WindowProperties m_WindowProps;
    };

    class WindowFactory final : public NonMoveable
    {
    public:
        static IWindow* Create(const WindowProperties& props);
        static void* CreateAndGetWindowSurface(IWindow* pWindow);
    };
}
