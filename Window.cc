#include <raindance/Raindance.hh>

class DemoWindow : public rd::Window
{
public:
    DemoWindow(rd::Window::Settings* settings)
    : Window(settings)
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

    rd::Window::Settings settings;
    settings.Title = std::string("Window");
    settings.Width = 1024;
    settings.Height = 728;

    demo->add(new DemoWindow(&settings));

    // TODO : demo->add(new DemoWindow("Window 2", 800, 600));

    demo->run();

    delete demo;
}
