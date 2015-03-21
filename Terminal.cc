#include <raindance/Raindance.hh>
#include <raindance/Core/Camera/Camera.hh>
#include <raindance/Core/Transformation.hh>
#include <raindance/Core/Text.hh>
#include <raindance/Core/Font.hh>
#include <raindance/Core/Console/Shell.hh>

class DemoWindow : public rd::Window
{
public:
    DemoWindow(const char* title, int width, int height, bool fullscreen = false)
    : Window(title, width, height, fullscreen)
    {
        m_Camera.setOrthographicProjection(0, 1000, 0, 1000, -100, 600);
        m_Camera.lookAt(glm::vec3(0.0, 0.0, 200.0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glEnable(GL_DEPTH_TEST);

        m_Shell = new Shell();
    }

    virtual ~DemoWindow()
    {
        SAFE_DELETE(m_Shell);
    }

    virtual void initialize(Context* context)
    {
        (void) context;
    }

    virtual void draw(Context* context)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        Transformation transformation;
        m_Shell->draw(context, m_Camera, transformation);
    }

    virtual void idle(Context* context)
    {
        (void) context;
    }

    void onKey(int key, int scancode, int action, int mods) override
    {
        m_Shell->onKey(key, scancode, action, mods);
    }

    void onChar(unsigned int codepoint) override
    {   
        m_Shell->onChar(codepoint);
    }

private:
    Camera m_Camera;
    Shell* m_Shell;
};

int main(int argc, char** argv)
{
    auto demo = new Raindance(argc, argv);
    demo->add(new DemoWindow("Fonts", 1024, 728));
    demo->run();
    delete demo;
}
