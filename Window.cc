#include <raindance/Raindance.hh>

class DemoWindow : public rd::Window
{
public:
    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
        glClearColor(0.2, 0.2, 0.2, 1.0);
    }

    virtual ~DemoWindow()
    {
    }
    
    virtual void initialize(Context* context)
    {
        (void) context;
    }

    virtual void draw(Context* context)
    {
        (void) context;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    virtual void idle(Context* context)
    {
        (void) context;
    }

private:
    Camera m_Camera;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);

    demo->add(new DemoWindow("Window", 1024, 728));

    // TODO : demo->add(new DemoWindow("Window 2", 800, 600));

    demo->run();

    delete demo;
}
